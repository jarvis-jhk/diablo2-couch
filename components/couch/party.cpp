// SPDX-License-Identifier: GPL-3.0-or-later
#include "party.hpp"

#include "group_camera.hpp"
#include "progress.hpp"

#include <character.hpp>
#include <character_store.hpp>
#include <d2s_items.hpp>
#include <fight.hpp>
#include <log.hpp>
#include <protocol.hpp>
#include <world.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace d2d::couch {

using game::World;

// Everything of World, Fight and Loot that is the game's rather than a
// hero's. A field upstream adds that isn't listed here stays per hero: for
// a guest, its own copy, untouched by the game. Keep this list in step with
// world.hpp / fight.hpp / loot.hpp at each upstream merge.
void lend(World& host, World& guest) {
    using std::swap;
    // The level's NPCs and the levels left, the rolls, the sounds due.
    swap(host.npc_states, guest.npc_states);
    swap(host.other_npcs, guest.other_npcs);
    swap(host.rng, guest.rng);
    swap(host.cues.due, guest.cues.due);
    // The floor.
    swap(host.loot.ground, guest.loot.ground);
    swap(host.loot.next_id, guest.loot.next_id);
    swap(host.loot.ground_level, guest.loot.ground_level);
    swap(host.loot.kept, guest.loot.kept);
    swap(host.loot.found_uniques, guest.loot.found_uniques);
    // The monsters and missiles, the rooms in play.
    swap(host.fight.monsters, guest.fight.monsters);
    swap(host.fight.next_id, guest.fight.next_id);
    swap(host.fight.mon_level, guest.fight.mon_level);
    swap(host.fight.area_seen, guest.fight.area_seen);
    swap(host.fight.kept, guest.fight.kept);
    swap(host.fight.spawning, guest.fight.spawning);
    swap(host.fight.missiles, guest.fight.missiles);
    swap(host.fight.pending, guest.fight.pending);
    swap(host.fight.game_difficulty, guest.fight.game_difficulty);
    swap(host.fight.remote_monsters, guest.fight.remote_monsters);
    // Objects, doors, fires, portals.
    swap(host.operated, guest.operated);
    swap(host.doors, guest.doors);
    swap(host.fires, guest.fires);
    swap(host.afterglow_made, guest.afterglow_made);
    swap(host.bridge_check, guest.bridge_check);
    swap(host.portal, guest.portal);
    swap(host.cain_portal, guest.cain_portal);
    swap(host.cain_walk, guest.cain_walk);
    swap(host.traps, guest.traps);
    swap(host.treasure, guest.treasure);
    // The vendors' stock, the hire lists, the item ids.
    swap(host.stocks, guest.stocks);
    swap(host.hire_lists, guest.hire_lists);
    swap(host.next_item_id, guest.next_item_id);
    // The quest records (one per game; the quest bits stay each hero's).
    swap(host.den, guest.den);
    swap(host.andy, guest.andy);
    swap(host.burial, guest.burial);
    swap(host.tower, guest.tower);
    swap(host.tools, guest.tools);
    swap(host.cain, guest.cain);
    swap(host.radament, guest.radament);
    swap(host.lam_esen, guest.lam_esen);
    swap(host.golden_bird, guest.golden_bird);
    swap(host.blade, guest.blade);
    swap(host.khalim, guest.khalim);
    swap(host.temple, guest.temple);
    swap(host.guardian, guest.guardian);
    swap(host.had_gidbinn, guest.had_gidbinn);
    swap(host.gidbinn_timer, guest.gidbinn_timer);
    swap(host.gidbinn_altar, guest.gidbinn_altar);
    swap(host.tainted, guest.tainted);
    swap(host.staff, guest.staff);
    swap(host.arcane, guest.arcane);
    swap(host.summoner, guest.summoner);
    swap(host.tombs, guest.tombs);
    swap(host.guard_line, guest.guard_line);
    swap(host.game_bits, guest.game_bits);
    swap(host.hosted_quests, guest.hosted_quests);
    swap(host.den_left, guest.den_left);
    swap(host.den_log_at, guest.den_log_at);
    swap(host.closed_at_join, guest.closed_at_join);
    // The day, and the frames the host's tick ran.
    swap(host.wanted_near, guest.wanted_near);
    swap(host.day, guest.day);
    swap(host.day_at, guest.day_at);
    swap(host.couch_frames, guest.couch_frames);
}

std::size_t Party::join(const game::Character& character, const game::CharacterStore* store) {
    World& host = *host_;
    auto guest = std::make_unique<World>(host.game_data, -1, -1);
    World& hero = *guest;
    hero.couch_guest = true;
    hero.character.character_class = character.character_class; hero.character.name = character.name;
    hero.character.appearance = character.appearance; hero.character.items = character.items; hero.character.stats = character.stats;
    hero.character.panel = character.panel; hero.character.expansion = character.expansion; hero.character.header = character.header;
    hero.character.merc_items = character.merc_items;
    // A new game's full life, mana and stamina (World::enter).
    for (const auto& [cur, max] : { std::pair{ d2d::d2s::kLife, d2d::d2s::kMaxLife }, { d2d::d2s::kMana, d2d::d2s::kMaxMana },
                                    { d2d::d2s::kStamina, d2d::d2s::kMaxStamina } })
        hero.character.stats.values[cur] = hero.character.stats.values[max];
    hero.characters = store;
    hero.now = host.now;
    hero.level = host.level;
    std::tie(hero.player.x, hero.player.y) = host.level->nearest_free(host.player.x + 1, host.player.y);
    hero.player.dir = 4;
    hero.target_x = hero.player.x; hero.target_y = hero.player.y;
    hero.running = host.running;
    if (!character.corpse.empty()) {
        const auto [spot_x, spot_y] = host.level->nearest_free(hero.player.x + 1, hero.player.y + 1);
        hero.corpses.push_back({ host.level, spot_x, spot_y, 4, character.corpse, 0,
                                 character.appearance ? *character.appearance : host.game_data->starting_gear[std::size_t(character.header.cls % 7)] });
    }
    lend(host, hero);
    hero.spawn_merc();
    hero.set_waypoint(0);
    hero.item_ids();
    lend(host, hero);
    if (const int act = host.act(); act > 0) {                   // the party is further on: the save starts there
        open_acts(act, hero.quests());
        hero.set_act(act);
    }
    d2d::log::info("couch: {} joins {}'s game", hero.character.name, host.character.name);
    guests_.push_back(std::move(guest));
    return guests_.size() - 1;
}

std::string Party::leave(std::size_t guest) {
    if (guest >= guests_.size()) return "no such guest";
    auto err = guests_[guest]->characters ? guests_[guest]->save() : std::string{};
    d2d::log::info("couch: {} leaves", guests_[guest]->character.name);
    guests_.erase(guests_.begin() + std::ptrdiff_t(guest));
    return err;
}

void Party::follow(World& hero, const World& leader) {
    const game::Level* from = hero.level;
    const int from_act = hero.act();
    const auto [to_x, to_y] = leader.level->nearest_free(leader.player.x + 1, leader.player.y);
    hero.fight.pets_cross(from, leader.level, hero.player.x - to_x, hero.player.y - to_y);
    hero.level = leader.level;
    hero.player.x = to_x; hero.player.y = to_y;
    hero.player.walking = false; hero.player.path.clear();
    hero.target_x = to_x; hero.target_y = to_y;
    if (hero.merc) {
        hero.merc->unit.path.clear();
        std::tie(hero.merc->unit.x, hero.merc->unit.y) = leader.level->nearest_free(to_x + 1, to_y + 1);
    }
    hero.fight.attack_mon = hero.fight.merc_target = -1;      // monster indices are the new level's
    for (auto& pet : hero.fight.pets) pet.target = -1;
    hero.fight.footstep(int(std::floor(to_x * 5)), int(std::floor(to_y * 5)), leader.now);
    hero.interact_npc = hero.pick_item = hero.take_warp = hero.take_portal = hero.take_corpse = -1;
    hero.arrived_at = leader.now;
    if (hero.act() != from_act) hero.set_act(hero.act());   // the save starts in the act the party is in
}

std::vector<Point> Party::heroes() const {
    std::vector<Point> out{ { host_->player.x, host_->player.y } };
    for (const auto& guest : guests_)
        if (guest->level == host_->level) out.push_back({ guest->player.x, guest->player.y });
    return out;
}

void Party::tick(const std::vector<game::Command>& host_cmds, std::span<const std::vector<game::Command>> guest_cmds,
                 std::uint32_t now_ms, std::uint32_t last_ms) {
    World& host = *host_;
    if (guests_.empty()) {
        host.tick(host_cmds, now_ms, last_ms);
        return;
    }
    // Where everyone stood: the leash's "before".
    std::vector<Point> before{ { host.player.x, host.player.y } };
    for (const auto& guest : guests_) before.push_back({ guest->player.x, guest->player.y });
    const game::Level* level_before = host.level;
    const int pads_before = host.couch_pads;
    const auto quests_before = host.quests();
    const int progression_before = host.character.header.progression;
    const auto waypoints_before = host.character.header.waypoints[std::size_t(std::clamp(host.character.header.active_difficulty(), 0, 2))];
    // 1. The guests' sides for the monsters; everyone in everyone's way.
    sides_.resize(guests_.size());
    host.couch_crowd.clear();
    for (std::size_t i = 0; i < guests_.size(); ++i) {
        World& guest = *guests_[i];
        lend(host, guest);
        sides_[i].foes = guest.fight.couch_side(now_ms);
        lend(host, guest);
        host.couch_crowd.push_back(&guest.player);
        if (guest.merc) host.couch_crowd.push_back(&guest.merc->unit);
    }
    // 2. The host and the game.
    host.fight.couch_sides = &sides_;
    host.tick(host_cmds, now_ms, last_ms);
    host.fight.couch_sides = nullptr;
    const bool same_level = host.level == level_before;
    // A teleport pad (the Arcane Sanctuary's) is a way on, not a step off
    // the screen: whoever takes one, the others come along.
    bool pulled = host.couch_pads != pads_before;
    auto leash = [&](std::size_t mover, game::UnitState& unit, game::World& world) {
        std::vector<Point> now_at;
        now_at.push_back({ host.player.x, host.player.y });
        for (const auto& guest : guests_) now_at.push_back({ guest->player.x, guest->player.y });
        now_at[mover] = before[mover];
        if (leash_allows(now_at, mover, { unit.x, unit.y }, frame)) return;
        unit.x = before[mover].x; unit.y = before[mover].y;
        unit.walking = false; unit.path.clear();
        world.target_x = unit.x; world.target_y = unit.y;
    };
    if (same_level && !pulled) leash(0, host.player, host);
    // 3. Each guest, with the game lent to it.
    for (std::size_t i = 0; i < guests_.size(); ++i) {
        World& guest = *guests_[i];
        const bool moved = guest.level != host.level || host.couch_pads != pads_before;
        if (moved) follow(guest, host);
        const int guest_pads = guest.couch_pads;
        guest.couch_guest = true;
        guest.couch_crowd.clear();
        guest.couch_crowd.push_back(&host.player);
        if (host.merc) guest.couch_crowd.push_back(&host.merc->unit);
        for (const auto& other : guests_)
            if (other.get() != &guest) {
                guest.couch_crowd.push_back(&other->player);
                if (other->merc) guest.couch_crowd.push_back(&other->merc->unit);
            }
        guest.fight.couch_taken = &sides_[i].foes;
        static const std::vector<game::Command> kNone;
        lend(host, guest);
        guest.tick(i < guest_cmds.size() ? guest_cmds[i] : kNone, now_ms, last_ms);
        lend(host, guest);
        guest.fight.couch_taken = nullptr;
        guest.events.clear();                                     // a guest has no panels for them yet
        if (guest.level != host.level) follow(guest, host);      // can't happen (a guest's tick moves no level); the rule holds anyway
        else if (guest.couch_pads != guest_pads) {                // the guest took a pad: the host and the others to it
            follow(host, guest);
            for (const auto& other : guests_)
                if (other.get() != &guest) follow(*other, guest);
            pulled = true;
        } else if (same_level && !moved && !pulled) leash(i + 1, guest.player, guest);
        // The quest objects the guest used and the quest items it reached
        // are the host's: player 1 carries the party's quests (talks, cubes,
        // wields Khalim's Will), so nothing a guest finds strands a quest.
        // An item with no room in the host's pack stays on the floor.
        for (const int npc : guest.couch_for_host)
            if (guest.level == host.level && std::size_t(npc) < host.level->npcs.size()) host.operate(npc, now_ms);
        for (const int id : guest.couch_quest_items)
            if (const int ground = host.loot.index_of(id); ground >= 0) host.pick_up(std::size_t(ground));
        guest.couch_for_host.clear(); guest.couch_quest_items.clear();
        if (guest.level != host.level) follow(guest, host);      // the host took a quest's portal
    }
    // 4. What the party earned this tick, in each guest's own save.
    const auto& host_waypoints_now = host.character.header.waypoints[std::size_t(std::clamp(host.character.header.active_difficulty(), 0, 2))];
    for (const auto& guest : guests_) {
        share_quests(quests_before, host.quests(), guest->quests());
        share_waypoints(waypoints_before, host_waypoints_now,
                        guest->character.header.waypoints[std::size_t(std::clamp(guest->character.header.active_difficulty(), 0, 2))]);
        share_progression(progression_before, host.character.header.progression, host.character.header.active_difficulty(),
                          guest->character.header.active_difficulty(), guest->character.header.progression);
    }
}

game::View Party::view(std::size_t guest) {
    World& hero = *guests_[guest];
    lend(*host_, hero);
    auto out = hero.view();
    lend(*host_, hero);
    return out;
}

std::vector<std::string> Party::save() {
    std::vector<std::string> out;
    for (const auto& guest : guests_) out.push_back(guest->characters ? guest->save() : std::string{});
    return out;
}

}  // namespace d2d::couch
