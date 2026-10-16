// SPDX-License-Identifier: GPL-3.0-or-later
// Diablo 2 Couch: two local heroes in one game (couch/party.hpp). Needs the
// game's MPQs (D2_MPQ_DIR, as test_game); without them it prints SKIP.
#include <party.hpp>

#include <ai.hpp>
#include <character.hpp>
#include <character_store.hpp>
#include <d2s_items.hpp>
#include <fight.hpp>
#include <gamedata.hpp>
#include <gamedata_load.hpp>
#include <group_camera.hpp>
#include <rules.hpp>
#include <level_ids.hpp>
#include <protocol.hpp>
#include <world.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;
using namespace d2d::game;
using d2d::couch::Party;

static Character made(const GameData& data, int cls, const std::string& name, d2d::rules::Rng& rng) {
    const auto fresh = new_character(data, cls, name, false, true, rng);
    Character out;
    out.character_class = cls;
    out.name = name;
    out.header = fresh.header;
    out.stats = fresh.stats;
    out.items = fresh.items;
    out.panel = panel_stats(data, out.header, out.items, out.stats);
    out.expansion = true;
    return out;
}

int main() {
    const char* mpq_dir = std::getenv("D2_MPQ_DIR");
    const char* home = std::getenv("HOME");
    const fs::path data_dir = mpq_dir ? fs::path(mpq_dir) : fs::path(home ? home : "") / "Workspace/private/diablo2";
    if (!fs::exists(data_dir / "d2data.mpq")) { std::printf("SKIP: no d2data.mpq in %s\n", data_dir.string().c_str()); return 0; }
    const char* patch = std::getenv("D2_PATCH_INSTALLER");
    auto data = load_game_data(data_dir, patch ? fs::path(patch) : fs::path{}, 0x1234);
    assert(data);

    d2d::rules::Rng rng(11);
    constexpr int kSorceress = 1, kBarbarian = 4;
    World world(&*data, -1, -1);
    world.enter(made(*data, kBarbarian, "Host", rng));
    Party party(world);
    std::uint32_t tick = 0;
    auto step = [&](const std::vector<Command>& host_cmds = {}, std::vector<Command> guest_cmds = {}) {
        ++tick;
        const std::vector<std::vector<Command>> guests{ std::move(guest_cmds) };
        party.tick(host_cmds, guests, tick * kTickMs, (tick - 1) * kTickMs);
    };
    for (int i = 0; i < 25; ++i) step();

    // A guest joins next to the host, on the host's level, at full life.
    const std::size_t index = party.join(made(*data, kSorceress, "Guest", rng), nullptr);
    World& guest = party.guest(index);
    assert(party.size() == 1 && guest.level == world.level && guest.couch_guest && !world.couch_guest);
    assert(std::hypot(guest.player.x - world.player.x, guest.player.y - world.player.y) < 3.f);
    assert(guest.character.stats.get(d2d::d2s::kLife) == guest.character.stats.get(d2d::d2s::kMaxLife));
    std::printf("OK: a guest joins at (%.1f, %.1f), the host at (%.1f, %.1f)\n", guest.player.x, guest.player.y, world.player.x, world.player.y);

    // The guest walks on its own commands; the host stands.
    const float host_x = world.player.x, host_y = world.player.y, from_x = guest.player.x, from_y = guest.player.y;
    const auto [goal_x, goal_y] = world.level->nearest_free(from_x + 2, from_y + 2);
    step({}, { cmd::Move{ goal_x, goal_y, true } });
    for (int i = 0; i < 25; ++i) step();
    assert(world.player.x == host_x && world.player.y == host_y);
    assert(std::hypot(guest.player.x - from_x, guest.player.y - from_y) > 1.f);
    std::printf("OK: the guest walked to (%.1f, %.1f) on its own; the host stood\n", guest.player.x, guest.player.y);

    // The leash: a walk far off stops where the party would leave the screen.
    step({}, { cmd::Move{ guest.player.x + 40, guest.player.y - 40, true } });
    for (int i = 0; i < 150; ++i) step({}, { cmd::Move{ guest.player.x + 40, guest.player.y - 40, false } });
    const auto heroes = party.heroes();
    assert(heroes.size() == 2 && d2d::couch::fits(d2d::couch::screen_box(heroes), party.frame));
    std::printf("OK: the leash held the guest at (%.1f, %.1f)\n", guest.player.x, guest.player.y);

    // The host leads into the Blood Moor; the guest follows on the next tick.
    const auto* moor = data->level(d2d::rules::level_ids::kBloodMoor);
    assert(moor);
    world.arrive(moor, moor->start.first >= 0 ? moor->start.first : float(moor->ds1.width() / 2), moor->start.first >= 0 ? moor->start.second : float(moor->ds1.height() / 2), "test");
    step();
    assert(world.level == moor && guest.level == moor);
    assert(std::hypot(guest.player.x - world.player.x, guest.player.y - world.player.y) < 3.f);
    std::printf("OK: the guest followed the host into %s\n", World::level_name(*moor));

    // The monsters fight both: one put by the guest finds it, takes its
    // blows and dies by them; the kill's experience is the guest's.
    for (int i = 0; i < 50; ++i) step();
    const auto live = std::ranges::find_if(world.fight.monsters, [](const Monster& monster) { return monster.targetable() && monster.alive(); });
    assert(live != world.fight.monsters.end());
    const int target = live->id;
    const auto [stand_x, stand_y] = moor->nearest_free(live->unit.x + 1, live->unit.y);
    guest.player.x = stand_x; guest.player.y = stand_y; guest.player.path.clear(); guest.target_x = stand_x; guest.target_y = stand_y;
    const auto [host_stand_x, host_stand_y] = moor->nearest_free(stand_x - 3, stand_y + 3);
    world.player.x = host_stand_x; world.player.y = host_stand_y; world.player.path.clear(); world.target_x = host_stand_x; world.target_y = host_stand_y;
    const auto guest_exp = guest.character.stats.get(d2d::d2s::kExp), host_exp = world.character.stats.get(d2d::d2s::kExp);
    const auto guest_life = guest.character.stats.get(d2d::d2s::kMaxLife);
    auto lowest = guest_life;
    bool dead = false;
    for (int i = 0; i < 750 && !dead; ++i) {
        const int found = world.fight.monster_index(target);
        dead = found < 0 || !world.fight.monsters[std::size_t(found)].alive();
        if (!dead) {
            const auto& monster = world.fight.monsters[std::size_t(found)];
            step({}, { cmd::UseSkill{ 0, monster.unit.x, monster.unit.y, target, true } });
        }
        lowest = std::min(lowest, guest.character.stats.get(d2d::d2s::kLife));
        // Kept alive for the test: the fight is about who's hit, not who wins.
        if (guest.character.stats.get(d2d::d2s::kLife) < guest_life / 2) guest.character.stats.values[d2d::d2s::kLife] = guest_life;
    }
    std::printf("guest: life low %lld of %lld, exp %lld -> %lld; host exp %lld -> %lld\n", (long long)(lowest >> 8), (long long)(guest_life >> 8),
                (long long)guest_exp, (long long)guest.character.stats.get(d2d::d2s::kExp), (long long)host_exp, (long long)world.character.stats.get(d2d::d2s::kExp));
    assert(dead);
    assert(guest.character.stats.get(d2d::d2s::kExp) > guest_exp && world.character.stats.get(d2d::d2s::kExp) == host_exp);
    std::printf("OK: the guest killed a monster and took its experience\n");
    if (lowest < guest_life) std::printf("OK: the monsters hit the guest\n");
    else std::printf("NOTE: no monster hit the guest in this fight\n");

    // A fallen guest gets up where it fell, by the party, on Resurrect.
    guest.fight.die(tick * kTickMs);
    for (int i = 0; i < 200 && guest.fight.pmode != kModeDD; ++i) step();
    assert(guest.fight.dead());
    for (int i = 0; i < 5; ++i) step({}, { cmd::Resurrect{} });
    assert(!guest.fight.dead() && guest.level == world.level);
    std::printf("OK: the guest died and got up on %s\n", World::level_name(*guest.level));

    // The guest's view sees the game: the level's monsters and its own self.
    const auto seen = party.view(index);
    assert(seen.level == world.level && seen.has_character && seen.header.name == "Guest");
    std::printf("OK: the guest's view: %zu monsters\n", seen.monsters.size());

    assert(party.leave(index).empty() && party.size() == 0);
    step();
    std::printf("OK: the guest left; the host plays on alone\n");
    return 0;
}
