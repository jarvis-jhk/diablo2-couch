// SPDX-License-Identifier: GPL-3.0-or-later
// Diablo 2 Couch: one camera for every local hero on a shared screen, and
// the leash that keeps them all on it (ROADMAP M1, docs/COUCH.md seam 5).
//
// Positions are world coordinates in d2d's camera units (cam_x/cam_y in
// Town::draw): one unit moves the screen by (+80, +40) in x and (-80, +40)
// in y (game.hpp, kIsoW/kIsoH). All heroes must be on one level, in that
// level's coordinates; the group rules keep the party together.
//
// The rules work in screen pixels relative to the camera point, because
// the screen is a rectangle and the world's axes are diagonal to it:
// - the camera sits at the centre of the heroes' screen bounding box, so
//   one hero gets exactly today's camera (the hero itself);
// - it then shifts as little as possible to keep every hero's feet inside
//   the safe frame (the screen less a margin, the HUD and a sprite's height);
// - a move is allowed if the party still fits afterwards, or if it does not
//   widen the party on either screen axis, so a hero left outside (after a
//   warp or a join) can always walk back towards the others.
#pragma once

#include <algorithm>
#include <cstddef>
#include <span>

namespace d2d::couch {

struct Point {
    float x = 0, y = 0;
};

// Where a hero's feet may stand on screen, in pixels from the camera point
// (the screen's centre column, kViewY). Defaults: 800x600 with the camera
// at y 292; 64 px either side, a hero sprite's ~100 px above the top edge,
// the 48 px control panel plus 16 px below.
struct Frame {
    float left = 336, right = 336, up = 192, down = 244;
};

// Screen offset in pixels of a world offset (the projection of
// ingame.cpp: ((dx - dy) * 80, (dx + dy) * 40)).
constexpr Point to_screen(Point world) {
    return { (world.x - world.y) * 80.f, (world.x + world.y) * 40.f };
}
// Its inverse.
constexpr Point to_world(Point screen) {
    return { (screen.x / 80.f + screen.y / 40.f) / 2.f, (screen.y / 40.f - screen.x / 80.f) / 2.f };
}

// The heroes' bounding box on screen (projected about the world origin).
struct Box {
    Point min, max;
    constexpr float width() const { return max.x - min.x; }
    constexpr float height() const { return max.y - min.y; }
};
constexpr Box screen_box(std::span<const Point> heroes) {
    Box box{ to_screen(heroes.front()), to_screen(heroes.front()) };
    for (const Point hero : heroes.subspan(1)) {
        const Point pixel = to_screen(hero);
        box.min = { std::min(box.min.x, pixel.x), std::min(box.min.y, pixel.y) };
        box.max = { std::max(box.max.x, pixel.x), std::max(box.max.y, pixel.y) };
    }
    return box;
}

// Whether a party of this extent fits the frame at all.
constexpr bool fits(const Box& box, const Frame& frame = {}) {
    return box.width() <= frame.left + frame.right && box.height() <= frame.up + frame.down;
}

// One axis: the box centre, clamped into the camera positions that keep
// [low, high] within [-before, +after] of it. With no such position (the
// party too wide) the overflow is split evenly between both edges.
constexpr float place(float low, float high, float before, float after) {
    const float first = high - after, last = low + before;   // the camera may be anywhere in [first, last]
    if (first > last) return (first + last) / 2.f;
    return std::clamp((low + high) / 2.f, first, last);
}

// The camera for these heroes (at least one), in world coordinates.
constexpr Point group_camera(std::span<const Point> heroes, const Frame& frame = {}) {
    if (heroes.size() == 1) return heroes.front();   // exactly one-player d2d
    const Box box = screen_box(heroes);
    return to_world({ place(box.min.x, box.max.x, frame.left, frame.right),
                      place(box.min.y, box.max.y, frame.up, frame.down) });
}

// Whether heroes[mover] may step to `dest`: the party fits afterwards, or it
// is no wider and no taller on screen than before.
constexpr bool leash_allows(std::span<const Point> heroes, std::size_t mover, Point dest, const Frame& frame = {}) {
    const Box before = screen_box(heroes);
    Box after{ to_screen(dest), to_screen(dest) };
    for (std::size_t i = 0; i < heroes.size(); ++i) {
        if (i == mover) continue;
        const Point pixel = to_screen(heroes[i]);
        after.min = { std::min(after.min.x, pixel.x), std::min(after.min.y, pixel.y) };
        after.max = { std::max(after.max.x, pixel.x), std::max(after.max.y, pixel.y) };
    }
    if (fits(after, frame)) return true;
    return after.width() <= before.width() && after.height() <= before.height();
}

}  // namespace d2d::couch
