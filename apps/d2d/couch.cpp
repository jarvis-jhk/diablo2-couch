// SPDX-License-Identifier: GPL-3.0-or-later
#include "couch.hpp"

#include "common.hpp"
#include "game_api.hpp"
#include "gamepad_map.hpp"
#include "scene.hpp"
#include "world_view.hpp"

#include <d2s_items.hpp>
#include <group_camera.hpp>
#include <log.hpp>
#include <pads.hpp>
#include <party.hpp>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_guid.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_joystick.h>
#include <SDL3/SDL_stdinc.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace d2d::client {

namespace {

// FNV-1a over the pad's GUID and serial: the same pad after a reconnect
// (pads.hpp Seats). Two identical pads without serials share an identity;
// then a dropped one may come back as the other, which is harmless.
std::uint64_t pad_identity(SDL_JoystickID id, SDL_Gamepad* handle) {
    std::uint64_t hash = 0xcbf29ce484222325;
    const SDL_GUID guid = SDL_GetGamepadGUIDForID(id);
    for (const auto byte : guid.data) hash = (hash ^ byte) * 0x100000001b3;
    if (const char* serial = handle ? SDL_GetGamepadSerial(handle) : nullptr)
        for (const char* letter = serial; *letter; ++letter) hash = (hash ^ std::uint8_t(*letter)) * 0x100000001b3;
    return hash;
}

constexpr float axis(std::int16_t value) { return std::clamp(float(value) / 32767.f, -1.f, 1.f); }

// A save as the World takes it (main.cpp's char-select load).
Character from_save(const Scene& scene, int index) {
    const auto slot = std::size_t(index);
    const auto& header = scene.saves[slot];
    Character out;
    out.character_class = header.cls;
    out.name = header.name;
    out.appearance = header.look();
    out.items = slot < scene.save_items.size() ? scene.save_items[slot] : std::vector<d2d::d2s::Item>{};
    out.stats = slot < scene.save_stats.size() ? scene.save_stats[slot] : d2d::d2s::Stats{};
    out.corpse = slot < scene.save_corpses.size() ? scene.save_corpses[slot] : std::vector<d2d::d2s::Item>{};
    out.merc_items = slot < scene.save_mercs.size() ? scene.save_mercs[slot] : std::vector<d2d::d2s::Item>{};
    out.panel = panel_stats(scene, header, out.items, out.stats);
    out.expansion = header.expansion();
    out.header = header;
    return out;
}

// The skills a pad can put on its left / right: Attack and every skill the
// hero has a level in that isn't passive (and may go left, for the left).
std::vector<int> pad_skills(const Scene& scene, const Character& character, bool on_left) {
    std::vector<int> out{ 0 };
    const auto cls = std::size_t(std::max(character.character_class, 0));
    for (const int id : scene.skills.class_ids[cls]) {
        const auto* skill = scene.skills.get(id);
        if (skill && !skill->passive && (!on_left || skill->leftskill) && skill_level(scene, character, id, {}) > 0) out.push_back(id);
    }
    return out;
}

int next_skill(const std::vector<int>& skills, int now, int step) {
    const auto found = std::ranges::find(skills, now);
    const auto here = found == skills.end() ? 0 : int(found - skills.begin());
    return skills[std::size_t((here + step + int(skills.size())) % int(skills.size()))];
}

void fill(std::vector<std::uint8_t>& framebuffer, int left, int top, int width, int height, std::uint8_t red, std::uint8_t green, std::uint8_t blue, int alpha = 255) {
    for (int y = std::max(top, 0); y < std::min(top + height, int(kScreenHeight)); ++y)
        for (int x = std::max(left, 0); x < std::min(left + width, int(kScreenWidth)); ++x) {
            auto* pixel = &framebuffer[(std::size_t(y) * kScreenWidth + std::size_t(x)) * 4];
            pixel[0] = std::uint8_t((pixel[0] * (255 - alpha) + red * alpha) / 255);
            pixel[1] = std::uint8_t((pixel[1] * (255 - alpha) + green * alpha) / 255);
            pixel[2] = std::uint8_t((pixel[2] * (255 - alpha) + blue * alpha) / 255);
        }
}

}  // namespace

void CouchPads::event(const SDL_Event& event) {
    if (event.type == SDL_EVENT_GAMEPAD_ADDED) {
        const SDL_JoystickID id = event.gdevice.which;
        if (std::ranges::any_of(pads, [&](const CouchPad& pad) { return pad.id == id; })) return;
        SDL_Gamepad* handle = SDL_OpenGamepad(id);
        if (!handle) { d2d::log::warn("couch: gamepad {}: {}", id, SDL_GetError()); return; }
        CouchPad pad{ .handle = handle, .id = id, .identity = pad_identity(id, handle) };
        const auto seat = seats.connect(id, pad.identity);
        d2d::log::info("couch: gamepad {} ({}) connected{}", id, SDL_GetGamepadName(handle) ? SDL_GetGamepadName(handle) : "?",
                       seat ? std::format(", back in seat {}", *seat + 1) : std::string{});
        pads.push_back(pad);
        return;
    }
    const auto found = std::ranges::find_if(pads, [&](const CouchPad& pad) {
        return (event.type == SDL_EVENT_GAMEPAD_REMOVED && pad.id == event.gdevice.which)
            || ((event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN || event.type == SDL_EVENT_GAMEPAD_BUTTON_UP) && pad.id == event.gbutton.which)
            || (event.type == SDL_EVENT_GAMEPAD_AXIS_MOTION && pad.id == event.gaxis.which);
    });
    if (found == pads.end()) return;
    CouchPad& pad = *found;
    if (event.type == SDL_EVENT_GAMEPAD_REMOVED) {
        const auto seat = seats.disconnect(pad.id);
        d2d::log::info("couch: gamepad {} gone{}", pad.id, seat ? std::format(", seat {} waits for it", *seat + 1) : std::string{});
        SDL_CloseGamepad(pad.handle);
        pads.erase(found);
        return;
    }
    if (event.type == SDL_EVENT_GAMEPAD_AXIS_MOTION) {
        const float value = axis(event.gaxis.value);
        switch (event.gaxis.axis) {
            case SDL_GAMEPAD_AXIS_LEFTX: pad.left.x = value; break;
            case SDL_GAMEPAD_AXIS_LEFTY: pad.left.y = value; break;
            case SDL_GAMEPAD_AXIS_RIGHTX: pad.right.x = value; break;
            case SDL_GAMEPAD_AXIS_RIGHTY: pad.right.y = value; break;
            case SDL_GAMEPAD_AXIS_LEFT_TRIGGER: pad.left_trigger = std::max(0.f, value); break;
            case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER: pad.right_trigger = std::max(0.f, value); break;
            default: break;
        }
        return;
    }
    const auto button = std::size_t(event.gbutton.button);
    if (button >= pad.down.size()) return;
    pad.down[button] = event.gbutton.down;
    if (!event.gbutton.down) return;
    pad.pressed[button] = true;
    const bool had = seats.of(pad.id).has_value();
    if (const auto seat = seats.press(pad.id, pad.identity); seat && !had) joined.push_back(*seat);
}

void CouchPads::begin_frame() {
    for (auto& pad : pads) pad.pressed.fill(false);
    joined.clear();
}

CouchPad* CouchPads::seat(std::size_t index) {
    const auto& seat = seats[index];
    if (!seat.connected()) return nullptr;
    const auto found = std::ranges::find(pads, seat.device, &CouchPad::id);
    return found == pads.end() ? nullptr : &*found;
}

CouchPads::~CouchPads() {
    if (SDL_WasInit(SDL_INIT_GAMEPAD) == 0) return;   // SDL_Quit already closed them
    for (const auto& pad : pads) SDL_CloseGamepad(pad.handle);
}

std::size_t Couch::index_of(const World* guest) const {
    for (std::size_t i = 0; i < party.size(); ++i)
        if (&party.guest(i) == guest) return i;
    return party.size();
}

void Couch::join(const Scene& scene, const CharacterStore* store, std::size_t seat, int save) {
    const auto index = party.join(from_save(scene, save), store);
    seat_hero[seat] = &party.guest(index);
    heroes[seat] = CouchHero{};
    guest_cmds.resize(party.size());
    const World& guest = party.guest(index);
    prev.resize(party.size()); cur.resize(party.size());
    prev[index] = cur[index] = { guest.player.x, guest.player.y };
}

std::string Couch::join_named(const Scene& scene, const CharacterStore* store, const std::string& name) {
    if (party.size() >= 3) return "four heroes already play";
    const auto found = std::ranges::find(scene.saves, name, &d2d::d2s::Header::name);
    if (found == scene.saves.end()) return "no save named " + name;
    if (name == party.host().character.name) return name + " is the host";
    for (std::size_t k = 0; k < party.size(); ++k)
        if (party.guest(k).character.name == name) return name + " already plays";
    for (std::size_t seat = pad_hosts ? 1 : 0; seat < d2d::couch::kMaxSeats; ++seat)
        if (!seat_hero[seat] && !(pads && pads->seats[seat].taken)) {
            join(scene, store, seat, int(found - scene.saves.begin()));
            return {};
        }
    return "no free seat";
}

void Couch::leave(std::size_t seat) {
    if (const auto index = index_of(seat_hero[seat]); index < party.size()) {
        if (const auto err = party.leave(index); !err.empty()) d2d::log::warn("couch: a guest's save: {}", err);
        if (index < guest_cmds.size()) guest_cmds.erase(guest_cmds.begin() + std::ptrdiff_t(index));
        if (index < prev.size()) prev.erase(prev.begin() + std::ptrdiff_t(index));
        if (index < cur.size()) cur.erase(cur.begin() + std::ptrdiff_t(index));
    }
    seat_hero[seat] = nullptr;
    heroes[seat] = CouchHero{};
    if (pads) pads->seats.leave(seat);
    if (picker && picker->seat == seat) picker.reset();
}

void Couch::clear() {
    for (std::size_t seat = 0; seat < seat_hero.size(); ++seat)
        if (seat_hero[seat]) leave(seat);
    picker.reset();
}

std::string Couch::save() {
    std::string out;
    for (const auto& err : party.save())
        if (!err.empty()) out += (out.empty() ? "" : "; ") + err;
    return out;
}

void Couch::ticked() {
    prev = cur;
    cur.resize(party.size());
    prev.resize(party.size());
    for (std::size_t i = 0; i < party.size(); ++i) cur[i] = { party.guest(i).player.x, party.guest(i).player.y };
}

d2d::couch::Point Couch::slid(std::size_t guest, float blend) const {
    if (guest >= cur.size()) return {};
    const auto& from = guest < prev.size() ? prev[guest] : cur[guest];
    const auto& now = cur[guest];
    if (std::hypot(now.x - from.x, now.y - from.y) > 2.f) return now;   // a jump (following the host) snaps
    return { from.x + (now.x - from.x) * blend, from.y + (now.y - from.y) * blend };
}

std::vector<Command> Couch::commands(const Scene& scene, CouchPad& pad, CouchHero& hero, const World& self, std::uint32_t frame_ms) {
    std::vector<Command> out;
    const World& game = party.host();                 // the game's monsters, floor and NPCs are the host's
    if (!hero.skills_set) {
        const auto lefts = pad_skills(scene, self.character, true), rights = pad_skills(scene, self.character, false);
        hero.left = std::ranges::contains(lefts, int(self.character.header.left_skill)) ? int(self.character.header.left_skill) : 0;
        hero.right = std::ranges::contains(rights, int(self.character.header.right_skill)) ? int(self.character.header.right_skill)
                   : rights.size() > 1 ? rights[1] : 0;
        hero.skills_set = true;
    }
    if (self.fight.dead()) {                          // the death plays out; then A gets the hero up
        if (pad.pressed[SDL_GAMEPAD_BUTTON_SOUTH]) out.push_back(cmd::Resurrect{});
        return out;
    }
    if (pad.pressed[SDL_GAMEPAD_BUTTON_NORTH]) hero.left = next_skill(pad_skills(scene, self.character, true), hero.left, 1);
    if (pad.pressed[SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER]) hero.right = next_skill(pad_skills(scene, self.character, false), hero.right, 1);
    if (pad.pressed[SDL_GAMEPAD_BUTTON_LEFT_SHOULDER]) hero.right = next_skill(pad_skills(scene, self.character, false), hero.right, -1);
    if (pad.pressed[SDL_GAMEPAD_BUTTON_EAST]) out.push_back(cmd::Run{ !self.running });
    static constexpr std::array<SDL_GamepadButton, 4> kBelt{ SDL_GAMEPAD_BUTTON_DPAD_LEFT, SDL_GAMEPAD_BUTTON_DPAD_UP, SDL_GAMEPAD_BUTTON_DPAD_RIGHT,
                                                             SDL_GAMEPAD_BUTTON_DPAD_DOWN };
    for (std::size_t column = 0; column < kBelt.size(); ++column)
        if (pad.pressed[kBelt[column]]) out.push_back(cmd::UseBelt{ int(column), false });

    const float here_x = self.player.x, here_y = self.player.y;
    const bool monsters_here = game.fight.mon_level == self.level;
    // The nearest live monster within `reach` cells; ahead of the stick counts nearer.
    auto nearest_monster = [&](float reach) -> const Monster* {
        const Monster* best = nullptr;
        float best_score = reach;
        if (!monsters_here) return best;
        for (const auto& monster : game.fight.monsters) {
            if (!monster.targetable() || !monster.alive()) continue;
            const float dx = monster.unit.x - here_x, dy = monster.unit.y - here_y, distance = std::hypot(dx, dy);
            if (distance > reach) continue;
            const float ahead = distance > 0.01f ? (dx * hero.face_x + dy * hero.face_y) / distance : 0.f;
            if (const float score = distance - ahead; !best || score < best_score) { best = &monster; best_score = score; }
        }
        return best;
    };
    const auto stick = d2d::couch::dead_zone(pad.left);
    if (stick.x != 0 || stick.y != 0) {
        const auto toward = d2d::couch::to_world({ stick.x, stick.y });
        const float length = std::hypot(toward.x, toward.y);
        hero.face_x = toward.x / length; hero.face_y = toward.y / length;
    }

    // A: the left skill at a monster, held to keep at it; else what's close.
    const Monster* held_on = nullptr;
    if (hero.attack >= 0 && pad.down[SDL_GAMEPAD_BUTTON_SOUTH] && monsters_here)
        if (const int index = game.fight.monster_index(hero.attack); index >= 0 && game.fight.monsters[std::size_t(index)].alive())
            held_on = &game.fight.monsters[std::size_t(index)];
    if (!pad.down[SDL_GAMEPAD_BUTTON_SOUTH]) hero.attack = -1;
    bool acting = false;
    if (held_on) {
        out.push_back(cmd::UseSkill{ hero.left, held_on->unit.x, held_on->unit.y, held_on->id, true });
        acting = true;
    } else if (pad.pressed[SDL_GAMEPAD_BUTTON_SOUTH]) {
        acting = true;
        if (const Monster* monster = nearest_monster(5.f)) {
            hero.attack = monster->id;
            out.push_back(cmd::UseSkill{ hero.left, monster->unit.x, monster->unit.y, monster->id, true });
        } else {
            // An item on the floor, the hero's corpse, an object or NPC: the nearest.
            float best = 3.f;
            std::optional<Command> pick;
            if (game.loot.ground_level == self.level)
                for (const auto& lying : game.loot.ground)
                    if (const float distance = std::hypot(lying.x - here_x, lying.y - here_y); distance < std::min(best, 2.5f)) { best = distance; pick = cmd::Pickup{ lying.id }; }
            for (std::size_t k = 0; k < self.corpses.size(); ++k)
                if (self.corpses[k].level == self.level)
                    if (const float distance = std::hypot(self.corpses[k].x - here_x, self.corpses[k].y - here_y); distance < best) { best = distance; pick = cmd::Interact{ -3000 - int(k) }; }
            for (std::size_t i = 0; i < self.level->npcs.size(); ++i) {
                const auto& npc = self.level->npcs[i];
                const auto* state = i < game.npc_states.size() ? &game.npc_states[i] : nullptr;
                if ((state && state->hidden) || npc.name.empty()) continue;
                const float npc_x = state && !npc.path.empty() ? state->x : npc.x, npc_y = state && !npc.path.empty() ? state->y : npc.y;
                if (const float distance = std::hypot(npc_x - here_x, npc_y - here_y); distance < best) { best = distance; pick = cmd::Interact{ int(i) }; }
            }
            if (pick) out.push_back(*pick);
            else acting = false;
        }
    }
    // X: the right skill at the nearest monster in range, on the hero for a
    // self cast, else ahead.
    if (pad.pressed[SDL_GAMEPAD_BUTTON_WEST]) {
        const auto* skill = scene.skills.get(hero.right);
        if (skill && game::self_cast(*skill)) {
            out.push_back(cmd::UseSkill{ hero.right, here_x, here_y, -1, false });
        } else if (const Monster* monster = nearest_monster(12.f)) {
            out.push_back(cmd::UseSkill{ hero.right, monster->unit.x, monster->unit.y, monster->id, false });
        } else {
            out.push_back(cmd::UseSkill{ hero.right, here_x + hero.face_x * 4, here_y + hero.face_y * 4, -1, false });
        }
        acting = true;
    }
    // The stick walks, unless A or X acted this frame.
    if (!acting)
        if (const auto walk = hero.walk.update({ here_x, here_y }, pad.left)) out.push_back(cmd::Move{ walk->target.x, walk->target.y, walk->fresh });
    (void)frame_ms;
    return out;
}

void Couch::frame(const Scene& scene, const CharacterStore* store, std::uint32_t frame_ms) {
    if (!pads) return;
    guest_cmds.resize(party.size());
    // A seat that holds a pad but no hero (it joined in the menus, its
    // picker closed, or another was open) asks again on its next press.
    if (!picker && pads->joined.empty())
        for (std::size_t seat = (pad_hosts ? 1 : 0); seat < d2d::couch::kMaxSeats; ++seat)
            if (pads->seats[seat].taken && !seat_hero[seat])
                if (const CouchPad* pad = pads->seat(seat); pad && std::ranges::any_of(pad->pressed, [](bool press) { return press; })) {
                    pads->joined.push_back(seat);
                    break;
                }
    // New seats: the host's seat (pad_player1) plays the host; any other
    // picks a hero, up to four heroes in all.
    for (const auto seat : pads->joined) {
        if (pad_hosts && seat == 0) { d2d::log::info("couch: pad 1 plays {}", party.host().character.name); continue; }
        if (picker) continue;                         // one picker at a time; the seat waits, pressing again opens it
        if (party.size() >= 3) {
            d2d::log::info("couch: four heroes already play; seat {} waits", seat + 1);
            continue;
        }
        Picker next{ .seat = seat };
        for (int i = 0; i < int(scene.saves.size()); ++i) {
            const auto& name = scene.saves[std::size_t(i)].name;
            bool in_play = name == party.host().character.name;
            for (std::size_t k = 0; k < party.size(); ++k) in_play |= party.guest(k).character.name == name;
            if (!in_play) next.saves.push_back(i);
        }
        if (next.saves.empty()) {
            d2d::log::info("couch: seat {} has no free hero to play (make another character first)", seat + 1);
            pads->seats.leave(seat);
            continue;
        }
        picker = next;
    }
    if (picker) {
        if (CouchPad* pad = pads->seat(picker->seat)) {
            const int count = int(picker->saves.size());
            if (pad->pressed[SDL_GAMEPAD_BUTTON_DPAD_UP]) picker->index = (picker->index + count - 1) % count;
            if (pad->pressed[SDL_GAMEPAD_BUTTON_DPAD_DOWN]) picker->index = (picker->index + 1) % count;
            if (pad->pressed[SDL_GAMEPAD_BUTTON_SOUTH] && !pads->joined.empty() && pads->joined.front() == picker->seat) {
                // the press that opened it: not a pick yet
            } else if (pad->pressed[SDL_GAMEPAD_BUTTON_SOUTH] || pad->pressed[SDL_GAMEPAD_BUTTON_START]) {
                const auto seat = picker->seat;
                const int save = picker->saves[std::size_t(picker->index)];
                picker.reset();
                join(scene, store, seat, save);
                pad->pressed.fill(false);             // its first command waits for the next press
            } else if (pad->pressed[SDL_GAMEPAD_BUTTON_EAST]) {
                const auto seat = picker->seat;
                picker.reset();
                pads->seats.leave(seat);
            }
        } else {
            picker.reset();                           // its pad went away
        }
    }
    // Each seated pad's hero.
    for (std::size_t seat = 0; seat < d2d::couch::kMaxSeats; ++seat) {
        CouchPad* pad = pads->seat(seat);
        if (!pad) continue;
        if (pad_hosts && seat == 0) continue;         // the host's: main's pad::Mapper (host_sample)
        if (!seat_hero[seat]) continue;
        // Back held two seconds: the guest leaves, saved.
        if (pad->down[SDL_GAMEPAD_BUTTON_BACK]) {
            if (!pad->back_since) pad->back_since = std::max<std::uint32_t>(frame_ms, 1);
            if (frame_ms - pad->back_since >= 2000) { pad->back_since = 0; leave(seat); continue; }
        } else {
            pad->back_since = 0;
        }
        const auto index = index_of(seat_hero[seat]);
        if (index >= party.size()) continue;
        auto out = commands(scene, *pad, heroes[seat], party.guest(index), frame_ms);
        guest_cmds[index].insert(guest_cmds[index].end(), out.begin(), out.end());
    }
}

pad::Sample Couch::host_sample() const {
    pad::Sample out;
    if (!pads || !pad_hosts) return out;
    const auto& seat = pads->seats[0];
    if (!seat.connected()) return out;
    const auto found = std::ranges::find(pads->pads, seat.device, &CouchPad::id);
    if (found == pads->pads.end()) return out;
    const CouchPad& pad = *found;
    out.lx = pad.left.x;   out.ly = pad.left.y;
    out.rx = pad.right.x;  out.ry = pad.right.y;
    out.left_trigger = pad.left_trigger;   out.right_trigger = pad.right_trigger;
    using pad::Button;
    const auto set = [&](Button which, SDL_GamepadButton button) { out.held[std::size_t(which)] = pad.down[std::size_t(button)]; };
    set(Button::South, SDL_GAMEPAD_BUTTON_SOUTH);              set(Button::East, SDL_GAMEPAD_BUTTON_EAST);
    set(Button::West, SDL_GAMEPAD_BUTTON_WEST);                set(Button::North, SDL_GAMEPAD_BUTTON_NORTH);
    set(Button::Back, SDL_GAMEPAD_BUTTON_BACK);                set(Button::Start, SDL_GAMEPAD_BUTTON_START);
    set(Button::LeftShoulder, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
    set(Button::RightShoulder, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
    set(Button::LeftStick, SDL_GAMEPAD_BUTTON_LEFT_STICK);     set(Button::RightStick, SDL_GAMEPAD_BUTTON_RIGHT_STICK);
    set(Button::DpadUp, SDL_GAMEPAD_BUTTON_DPAD_UP);           set(Button::DpadDown, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
    set(Button::DpadLeft, SDL_GAMEPAD_BUTTON_DPAD_LEFT);       set(Button::DpadRight, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
    return out;
}

void Couch::into_view(View& view, float blend) const {
    for (std::size_t i = 0; i < party.size(); ++i) {
        const World& guest = party.guest(i);
        if (guest.level != view.level) continue;
        View::OtherPlayer other{ .unit = guest.player, .cls = std::max(guest.character.character_class, 0), .name = guest.character.name,
                                 .id = std::uint32_t(0x00c0c000 + i), .gfx = guest.fight.gfx(), .mode = -1, .running = guest.running };
        if (guest.fight.pmode == kModeDD) other.mode = kModeDT;
        else if (guest.fight.pmode >= 0) other.mode = guest.fight.pmode;
        other.running = guest.running && guest.character.stats.get(d2d::d2s::kStamina) > 0;
        const auto feet = slid(i, blend);
        other.unit.x = feet.x; other.unit.y = feet.y;
        view.others.push_back(std::move(other));
    }
}

void Couch::units(const Scene& scene, std::vector<Unit>& out) const {
    for (std::size_t i = 0; i < party.size(); ++i) {
        const World& guest = party.guest(i);
        if (guest.level != party.host().level) continue;
        if (guest.merc && guest.merc_npc)
            out.push_back({ guest.merc->unit.x, guest.merc->unit.y, &scene.npc_anim(*guest.merc_npc, guest.merc->mode), guest.merc->unit.dir, nullptr,
                            guest.merc->unit.mode_ms, -2 });
        for (const auto& pet : guest.fight.pets)
            if (pet.where == guest.level)
                out.push_back({ pet.monster.unit.x, pet.monster.unit.y, &scene.npc_anim(pet.monster.npc, pet.monster.mode), pet.monster.unit.dir, nullptr,
                                pet.monster.unit.mode_ms, -3 });
    }
}

void Couch::draw(std::vector<std::uint8_t>& framebuffer, const Scene& scene) const {
    const auto& pal = scene.act_pal.entries().empty() ? scene.pal : scene.act_pal;
    const auto& font = scene.font_small;
    // Each guest's corner along the top: name, life, mana, its two skills.
    int slot = 0;
    for (std::size_t seat = 0; seat < seat_hero.size(); ++seat) {
        const World* guest = seat_hero[seat];
        if (!guest) continue;
        const int left = 8 + slot * 196, top = 6;
        ++slot;
        fill(framebuffer, left, top, 184, 40, 0, 0, 0, 160);
        const auto& stats = guest->character.stats;
        const auto bar = [&](int row, std::int64_t now, std::int64_t most, std::uint8_t red, std::uint8_t green, std::uint8_t blue) {
            const int width = most > 0 ? int(std::clamp<std::int64_t>(now * 176 / most, 0, 176)) : 0;
            fill(framebuffer, left + 4, top + row, 176, 5, 40, 40, 40);
            fill(framebuffer, left + 4, top + row, width, 5, red, green, blue);
        };
        font.draw(framebuffer, kScreenWidth, kScreenHeight, pal, left + 4, top + 2, std::format("P{} {}", player_number(seat), guest->character.name));
        bar(14, stats.get(d2d::d2s::kLife), stats.get(d2d::d2s::kMaxLife), 200, 30, 30);
        bar(21, stats.get(d2d::d2s::kMana), stats.get(d2d::d2s::kMaxMana), 40, 70, 220);
        const auto& hero = heroes[seat];
        const auto skill_name = [&](int id) -> std::string { const auto* skill = scene.skills.get(id); return skill ? std::string(skill->name) : std::string("Attack"); };
        const std::string line = guest->fight.dead() ? std::string("Dead - A: get up") : skill_name(hero.left) + " / " + skill_name(hero.right);
        font.draw(framebuffer, kScreenWidth, kScreenHeight, pal, left + 4, top + 28, line);
    }
    // The picker: the saves not in play, d-pad to choose, A to play, B to leave.
    if (picker) {
        const int rows = int(picker->saves.size());
        const int width = 300, height = 40 + 14 * std::min(rows, 12), left = (int(kScreenWidth) - width) / 2, top = 120;
        fill(framebuffer, left, top, width, height, 0, 0, 0, 200);
        font.draw(framebuffer, kScreenWidth, kScreenHeight, pal, left + 10, top + 6, std::format("Player {}: choose a hero (A play, B cancel)", player_number(picker->seat)));
        const int first = std::clamp(picker->index - 5, 0, std::max(rows - 12, 0));
        for (int row = first; row < std::min(rows, first + 12); ++row) {
            const auto& header = scene.saves[std::size_t(picker->saves[std::size_t(row)])];
            const int text_y = top + 26 + (row - first) * 14;
            if (row == picker->index) fill(framebuffer, left + 6, text_y - 1, width - 12, 13, 120, 90, 30, 180);
            font.draw(framebuffer, kScreenWidth, kScreenHeight, pal, left + 12, text_y, std::format("{}  (level {})", header.name, int(header.level)));
        }
    }
}

}  // namespace d2d::client
