// SPDX-License-Identifier: GPL-3.0-or-later
// Couch controller seats and the stick walk (couch/pads.hpp).
#include <group_camera.hpp>
#include <pads.hpp>

#include <cassert>
#include <cmath>
#include <cstddef>
#include <optional>

using namespace d2d::couch;

static bool near_eq(float lhs, float rhs) { return std::abs(lhs - rhs) < 1e-3f; }

int main() {
    // Seats: plugging in alone takes nothing; a press joins the first free seat.
    Seats seats;
    assert(!seats.connect(11, 0xA1));
    assert(seats.taken() == 0);
    assert(seats.press(11, 0xA1) == std::optional<std::size_t>(0));
    assert(seats.press(12, 0xB2) == std::optional<std::size_t>(1));
    assert(seats.press(11, 0xA1) == std::optional<std::size_t>(0));   // pressing again keeps the seat
    assert(seats.of(12) == std::optional<std::size_t>(1));
    assert(seats.taken() == 2);

    // Player 1's pad drops out: the seat waits, player 2 stays player 2,
    // and a new pad does not take the waiting seat.
    assert(seats.disconnect(11) == std::optional<std::size_t>(0));
    assert(seats[0].taken && !seats[0].connected());
    assert(!seats.of(11));
    assert(seats.press(13, 0xC3) == std::optional<std::size_t>(2));
    // The same pad back under a new runtime id gets seat 0 on connect alone.
    assert(seats.connect(21, 0xA1) == std::optional<std::size_t>(0));
    assert(seats[0].connected() && seats.of(21) == std::optional<std::size_t>(0));

    // Four seats, then nobody else; leaving frees exactly that seat.
    assert(seats.press(14, 0xD4) == std::optional<std::size_t>(3));
    assert(!seats.press(15, 0xE5));
    seats.leave(1);
    assert(!seats.of(12) && seats.taken() == 3);
    assert(seats.press(15, 0xE5) == std::optional<std::size_t>(1));
    // An unknown device going away is a no-op.
    assert(!seats.disconnect(99));
    // Device id 0 is "away", never a seated pad.
    assert(!seats.of(0));

    // Dead zone: radial, rescaled, direction kept.
    const Stick rest = dead_zone({ 0.2f, 0.1f });
    assert(rest.x == 0 && rest.y == 0);
    const Stick full = dead_zone({ 1, 0 });
    assert(near_eq(full.x, 1) && near_eq(full.y, 0));
    const Stick diagonal = dead_zone({ 0.5f, 0.5f });   // 0.707: neither axis alone passes 0.24 snapping
    assert(near_eq(diagonal.x, diagonal.y) && diagonal.x > 0);
    assert(near_eq(std::hypot(diagonal.x, diagonal.y), (std::hypot(0.5f, 0.5f) - 0.24f) / (0.95f - 0.24f)));

    // The target is `reach` pixels away on screen, in the stick's direction.
    const Point hero{ 100, 50 };
    for (const Stick stick : { Stick{ 1, 0 }, Stick{ 0, -1 }, Stick{ -0.6f, 0.8f } }) {
        const Point target = stick_target(hero, stick, 160);
        const Point pixel = to_screen({ target.x - hero.x, target.y - hero.y });
        assert(near_eq(pixel.x, stick.x * 160) && near_eq(pixel.y, stick.y * 160));
    }
    // Right on the stick is +x-and--y in the world (east on screen).
    const Point east = stick_target(hero, { 1, 0 }, 160);
    assert(near_eq(east.x - hero.x, 1) && near_eq(east.y - hero.y, -1));

    // The walk: nothing at rest, fresh on the first push, re-aimed while
    // held, one stop at the hero's position on release, then nothing.
    StickWalk walk;
    assert(!walk.update(hero, {}));
    const auto first = walk.update(hero, { 1, 0 });
    assert(first && first->fresh && near_eq(first->target.x, east.x));
    const Point moved{ 101, 49 };
    const auto held = walk.update(moved, { 0, 1 });
    assert(held && !held->fresh && walk.walking());
    assert(near_eq(to_screen({ held->target.x - moved.x, held->target.y - moved.y }).y, 160));
    const auto stop = walk.update(moved, { 0.1f, 0 });   // inside the dead zone
    assert(stop && !stop->fresh && near_eq(stop->target.x, moved.x) && near_eq(stop->target.y, moved.y));
    assert(!walk.walking() && !walk.update(moved, {}));
    return 0;
}
