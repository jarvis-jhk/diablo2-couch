// SPDX-License-Identifier: GPL-3.0-or-later
// Diablo 2 Couch: several local heroes in one game on one screen (ROADMAP
// M1, docs/COUCH.md seams 1-3), before upstream's own N-player World
// (docs/design/net-host.md stage 1) exists.
//
// The host is the World single player always had. Each guest hero is a
// World of its own: its character, unit, merc, pets, fight state, item in
// hand. What belongs to the game rather than to a hero (the monsters and
// missiles, the floor, NPCs, objects and doors, the quest records, the day,
// the rolls) lives in the host's World and is lent to a guest for its pass
// (lend(), a swap, so a second call gives it back). A guest's tick runs with
// World::couch_guest set: it moves its own hero and nothing of the game's.
//
// One tick of the party:
// 1. every guest's side (hero, merc, pets) goes to the host's fight, so the
//    monsters pick among all heroes and their blows and missiles land on
//    whichever they reach (Fight::couch_sides);
// 2. the host ticks: its commands, the monsters, missiles, NPCs, quests,
//    level changes;
// 3. each guest ticks with the game lent to it, taking what its side was
//    dealt in step 2 (Fight::couch_taken);
// 4. the group rules: a guest who isn't on the host's level comes to the
//    host (the host leads through warps, portals, waypoints and edges), a
//    teleport pad brings the others to whoever took it, and the leash undoes
//    a step that would pull the party off the screen;
// 5. what the host earned in the tick (the act bosses, the acts opened, the
//    waypoints, the act progression) is the guests' too, in their own saves
//    (progress.hpp).
//
// Player 1 carries the party's quests: a quest object a guest uses (a
// stand, an altar, Khalim's chests, the lever, the orb, the Sanctuary's
// portal: World::quest_object) is used as the host's, and a quest item a
// guest walks to goes into the host's pack, since only the host talks to
// NPCs, cubes and wields Khalim's Will.
//
// ponytail: a guest's missiles are resolved in the host's pass, so their
// kills and leech count as the host's; a guest's town portal is the game's
// one portal pair; guests talk, trade and use waypoints through the host
// only. Each waits for upstream's PlayerSession split.
#pragma once

#include "group_camera.hpp"

#include <world.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <vector>

namespace d2d::couch {

// Swaps the game's shared state between two Worlds of one game.
void lend(game::World& host, game::World& guest);

class Party {
public:
    explicit Party(game::World& host) : host_(&host) {}

    // A guest hero joins the host's game next to the host, with its own
    // character (full life, as a new game starts) and save. Its index.
    std::size_t join(const game::Character& character, const game::CharacterStore* store);
    // The guest leaves (saved first, when it has a store): "" or why not saved.
    std::string leave(std::size_t guest);
    [[nodiscard]] std::size_t size() const { return guests_.size(); }
    [[nodiscard]] game::World& guest(std::size_t index) { return *guests_[index]; }
    [[nodiscard]] const game::World& guest(std::size_t index) const { return *guests_[index]; }
    [[nodiscard]] game::World& host() { return *host_; }
    [[nodiscard]] const game::World& host() const { return *host_; }

    // One tick: the host's commands, then each guest's (guest_cmds[i], or
    // none past its end). With no guests it is exactly World::tick.
    void tick(const std::vector<game::Command>& host_cmds, std::span<const std::vector<game::Command>> guest_cmds,
              std::uint32_t now_ms, std::uint32_t last_ms);

    // A guest's View, made with the game lent to it (what it sees of the
    // monsters, the floor, its own character).
    [[nodiscard]] game::View view(std::size_t guest);

    // Every hero's feet, host first, in the host's level's cells.
    [[nodiscard]] std::vector<Point> heroes() const;

    // Saves every guest that has a store; the errors, "" each when written.
    std::vector<std::string> save();

    Frame frame{};   // the leash's frame (group_camera.hpp)

private:
    // A hero not where the leader is comes to the leader's side: a guest to
    // the host after a level change or the host's teleport pad, everyone to
    // a guest who took a pad.
    static void follow(game::World& hero, const game::World& leader);
    game::World* host_;
    std::vector<std::unique_ptr<game::World>> guests_;
    std::vector<game::Fight::CouchSide> sides_;
};

}  // namespace d2d::couch
