// SPDX-License-Identifier: GPL-3.0-or-later
// What the couch party earns goes into each guest's save (couch/progress.hpp).
#include <progress.hpp>

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>

using namespace d2d::couch;
using Bits = std::array<std::uint8_t, 96>;

static bool bit(const Bits& bits, int quest, int index) {
    const int bit_at = quest * 16 + index;
    return bits[std::size_t(bit_at >> 3)] >> (bit_at & 7) & 1;
}
static void set(Bits& bits, int quest, int index) {
    const int bit_at = quest * 16 + index;
    bits[std::size_t(bit_at >> 3)] = std::uint8_t(bits[std::size_t(bit_at >> 3)] | 1 << (bit_at & 7));
}

int main() {
    // The host kills Andariel and takes the caravan in one stretch: the
    // guest gets Andariel's bits and Act II opened.
    Bits before{}, after{}, guest{};
    set(before, 1, 0);                       // the host's old Den of Evil: never passed on
    after = before;
    set(after, 6, 0); set(after, 6, 13);
    set(after, 7, 0); set(after, 7, 13);
    share_quests(before, after, guest);
    assert(bit(guest, 6, 0) && bit(guest, 6, 13));
    assert(bit(guest, 7, 0) && bit(guest, 7, 13));
    assert(!bit(guest, 1, 0));

    // A quest with a hero's own reward (Den of Evil, Radament) is not
    // shared even when the host finishes it in the tick.
    Bits den_after = after;
    set(den_after, 9, 0); set(den_after, 9, 1);
    Bits den_guest{};
    share_quests(after, den_after, den_guest);
    assert(!bit(den_guest, 9, 0) && !bit(den_guest, 9, 1));

    // A guest who already beat Duriel keeps its own finished quest as it is.
    Bits done{};
    set(done, 14, 0); set(done, 14, 4);
    Bits tombs_before{}, tombs_after{};
    set(tombs_after, 14, 2);
    share_quests(tombs_before, tombs_after, done);
    assert(bit(done, 14, 0) && bit(done, 14, 4) && !bit(done, 14, 2));
    // ...and one with no Tombs yet gets the host's progress, Duriel done too.
    Bits fresh{};
    set(tombs_after, 14, 0); set(tombs_after, 14, 5);
    share_quests(tombs_before, tombs_after, fresh);
    assert(bit(fresh, 14, 0) && bit(fresh, 14, 2) && bit(fresh, 14, 5));

    // Nothing gained, nothing passed on: the host's past stays the host's.
    Bits quiet{};
    share_quests(after, after, quiet);
    assert(quiet == Bits{});

    // Waypoints: only the ones activated this tick, added to the guest's own.
    std::array<std::uint8_t, 5> wp_before{ 0x07, 0, 0, 0, 0 }, wp_after{ 0x07, 0x02, 0, 0, 0 }, wp_guest{ 0x01, 0, 0, 0, 0x10 };
    share_waypoints(wp_before, wp_after, wp_guest);
    assert((wp_guest == std::array<std::uint8_t, 5>{ 0x01, 0x02, 0, 0, 0x10 }));

    // A guest joining in Act II (act 1): Act II opened, Act III not.
    Bits joined{};
    open_acts(1, joined);
    assert(bit(joined, 7, 0) && bit(joined, 7, 13));
    assert(!bit(joined, 15, 0));
    Bits joined3{};
    open_acts(2, joined3);
    assert(bit(joined3, 7, 0) && bit(joined3, 15, 0) && !bit(joined3, 23, 0));
    Bits camp{};
    open_acts(0, camp);
    assert(camp == Bits{});

    // Mephisto dies (Act III's boss): the host's word 22 bits 13, 0, 11 and
    // its progression to Act IV's (3 in Normal) reach a guest in Normal.
    Bits meph_before{}, meph_after{}, meph_guest{};
    set(meph_after, 22, 13); set(meph_after, 22, 0); set(meph_after, 22, 11);
    share_quests(meph_before, meph_after, meph_guest);
    assert(bit(meph_guest, 22, 0) && bit(meph_guest, 22, 13) && bit(meph_guest, 22, 11));
    std::uint8_t progress = 2;
    share_progression(2, 3, 0, 0, progress);
    assert(progress == 3);
    // Never lowered, nothing without a gain, never across difficulties.
    std::uint8_t ahead = 7;
    share_progression(2, 3, 0, 0, ahead);
    assert(ahead == 7);
    std::uint8_t still = 1;
    share_progression(3, 3, 0, 0, still);
    assert(still == 1);
    std::uint8_t normal = 1;
    share_progression(7, 8, 1, 0, normal);     // the host in Nightmare, the guest in Normal
    assert(normal == 1);

    // Act IV: Diablo dies (word 26) and Tyrael's portal opens Act V (28):
    // both reach the guest. Izual's (25: Tyrael's skill points) and the
    // Hellforge (27: Cain's soulstone) stay player 1's.
    Bits a4_before{}, a4_after{}, a4_guest{};
    set(a4_after, 25, 0); set(a4_after, 25, 1);
    set(a4_after, 27, 0); set(a4_after, 27, 1);
    set(a4_after, 26, 0); set(a4_after, 26, 13);
    set(a4_after, 28, 0); set(a4_after, 28, 13);
    share_quests(a4_before, a4_after, a4_guest);
    assert(bit(a4_guest, 26, 0) && bit(a4_guest, 26, 13));
    assert(bit(a4_guest, 28, 0) && bit(a4_guest, 28, 13));
    assert(!bit(a4_guest, 25, 0) && !bit(a4_guest, 27, 0));
    // Classic: Diablo's death opens Nightmare (progression 3 -> 4, World::open_act(4)).
    std::uint8_t classic = 3;
    share_progression(3, 4, 0, 0, classic);
    assert(classic == 4);
    return 0;
}
