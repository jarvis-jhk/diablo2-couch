// SPDX-License-Identifier: GPL-3.0-or-later
// The couch group camera and leash (couch/group_camera.hpp).
#include <group_camera.hpp>

#include <array>
#include <cassert>
#include <cmath>
#include <span>

using namespace d2d::couch;

static bool near_eq(float lhs, float rhs) { return std::abs(lhs - rhs) < 1e-3f; }
static bool near_eq(Point lhs, Point rhs) { return near_eq(lhs.x, rhs.x) && near_eq(lhs.y, rhs.y); }

// Every hero's feet inside the frame, on screen, around this camera.
static bool on_screen(std::span<const Point> heroes, Point cam, const Frame& frame = {}) {
    for (const Point hero : heroes) {
        const Point pixel = to_screen({ hero.x - cam.x, hero.y - cam.y });
        if (pixel.x < -frame.left - 1e-3f || pixel.x > frame.right + 1e-3f || pixel.y < -frame.up - 1e-3f || pixel.y > frame.down + 1e-3f) return false;
    }
    return true;
}

int main() {
    // The projection matches ingame.cpp and inverts.
    static_assert(to_screen({ 1, 0 }).x == 80 && to_screen({ 1, 0 }).y == 40);
    static_assert(to_screen({ 0, 1 }).x == -80 && to_screen({ 0, 1 }).y == 40);
    assert(near_eq(to_world(to_screen({ 12.5f, -3.25f })), { 12.5f, -3.25f }));

    // One hero: exactly one-player d2d's camera.
    const std::array<Point, 1> one{ { { 100.3f, 57.9f } } };
    assert(near_eq(group_camera(one), one[0]));

    // Two heroes close together: the camera between them, both on screen.
    const std::array<Point, 2> pair{ { { 100, 100 }, { 102, 100 } } };
    assert(near_eq(group_camera(pair), { 101, 100 }));
    assert(on_screen(pair, group_camera(pair)));

    // Spread vertically: the frame is asymmetric (more room below the
    // camera point than above), the box centre would push the top hero off,
    // so the camera shifts up just enough.
    const std::array<Point, 2> tall{ { { 100, 100 }, { 105.4f, 105.4f } } };   // 432 px apart
    assert(fits(screen_box(tall)));
    const Point cam = group_camera(tall);
    assert(on_screen(tall, cam));
    assert(!on_screen(tall, { 102.7f, 102.7f }));   // the plain centre would not do
    assert(near_eq(to_screen({ 100 - cam.x, 100 - cam.y }).y, -192));   // top hero on the top edge

    // Four heroes in a ring: all on screen, the camera at their middle.
    const std::array<Point, 4> four{ { { 50, 50 }, { 53, 50 }, { 50, 53 }, { 53, 53 } } };
    assert(on_screen(four, group_camera(four)));

    // The leash. A party stretched to the frame's width: the outer hero may
    // not walk further out, may walk back in, and the inner hero may move
    // freely while the party still fits.
    const std::array<Point, 2> wide{ { { 0, 0 }, { 4.2f, -4.2f } } };   // 672 px, exactly the width
    assert(fits(screen_box(wide)));
    assert(!leash_allows(wide, 1, { 4.4f, -4.4f }));
    assert(leash_allows(wide, 1, { 4.0f, -4.0f }));
    assert(leash_allows(wide, 0, { 0.1f, -0.1f }));
    assert(!leash_allows(wide, 0, { -0.1f, 0.1f }));
    // Moving along the screen's other axis stays allowed while it fits.
    assert(leash_allows(wide, 1, { 4.3f, -4.1f }) == fits(screen_box(std::array<Point, 2>{ { { 0, 0 }, { 4.3f, -4.1f } } })));

    // Already outside (a warp, a join): no move may widen it, any move
    // that narrows it may, even if the party still doesn't fit.
    const std::array<Point, 2> apart{ { { 0, 0 }, { 10, -10 } } };
    assert(!fits(screen_box(apart)));
    assert(leash_allows(apart, 1, { 9.5f, -9.5f }));
    assert(!leash_allows(apart, 1, { 10.5f, -10.5f }));
    assert(!leash_allows(apart, 0, { -0.5f, 0.5f }));
    // Its camera splits the overflow evenly.
    const Point split = group_camera(apart);
    assert(near_eq(to_screen({ 0 - split.x, 0 - split.y }).x, -to_screen({ 10 - split.x, -10 - split.y }).x));

    // One hero never leashes itself.
    assert(leash_allows(one, 0, { 500, 500 }));
    return 0;
}
