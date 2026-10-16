// SPDX-License-Identifier: GPL-3.0-or-later
// Diablo 2 Couch: what the party earns goes into every guest's own save.
//
// The game's quest records run once, in the host's World (couch/party.hpp),
// so they set the host's quest flags only; upstream's records are one-player
// ("N-player gaps: no party credit"). A guest who crossed into Act II with
// the host would otherwise go home with Act II closed and Andariel alive.
//
// Shared: the act bosses' quests and the act travel words — the quests
// whose completion is the party's, with no reward a hero claims at an NPC
// (a skill point, a respec, an imbue: those stay each hero's to fetch) —
// and every waypoint the host activates while the guests are with it.
// Only what the host gains in a tick is passed on, never the host's past.
#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

namespace d2d::couch {

// Quest words (the save's 16-bit quest flags, couch's copy of
// rules/quests.hpp's numbering): Andariel (6), Act II opened (7), the Seven
// Tombs / Duriel (14), Act III opened (15), the Guardian / Mephisto (22),
// Act IV opened (23).
inline constexpr std::array<int, 6> kPartyQuests{ 6, 7, 14, 15, 22, 23 };

// The bits of `quest` the host gained this tick (before -> after) go to the
// guest, unless the guest's own quest is already done (bit 0) or closed
// (bit 15): a finished quest is never reopened by someone else's state.
template <std::size_t N>
void share_quests(const std::array<std::uint8_t, N>& before, const std::array<std::uint8_t, N>& after, std::array<std::uint8_t, N>& guest) {
    for (const int quest : kPartyQuests) {
        const auto low = std::size_t(quest) * 2, high = low + 1;
        if (high >= N) continue;
        if ((guest[low] & 1) || (guest[high] & 0x80)) continue;
        guest[low] = std::uint8_t(guest[low] | (after[low] & ~before[low]));
        guest[high] = std::uint8_t(guest[high] | (after[high] & ~before[high]));
    }
}

// Waypoints the host activated this tick are the guest's too.
template <std::size_t N>
void share_waypoints(const std::array<std::uint8_t, N>& before, const std::array<std::uint8_t, N>& after, std::array<std::uint8_t, N>& guest) {
    for (std::size_t i = 0; i < N; ++i) guest[i] = std::uint8_t(guest[i] | (after[i] & ~before[i]));
}

// A guest joining a game the host has taken into a later act: the travel
// words up to that act, so its save may start there (the act it is saved
// in follows the party's, World::set_act).
template <std::size_t N>
void open_acts(int act, std::array<std::uint8_t, N>& guest) {
    static constexpr std::array<int, 3> kTravel{ 7, 15, 23 };   // into Acts II, III, IV
    for (int i = 0; i < std::min(act, int(kTravel.size())); ++i) {
        const auto low = std::size_t(kTravel[std::size_t(i)]) * 2;
        if (low + 1 >= N || (guest[low] & 1)) continue;
        guest[low] = std::uint8_t(guest[low] | 1);            // bit 0: opened
        guest[low + 1] = std::uint8_t(guest[low + 1] | 0x20); // bit 13: in this game
    }
}

}  // namespace d2d::couch
