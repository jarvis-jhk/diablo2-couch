// SPDX-License-Identifier: GPL-3.0-or-later
// The gamepad's action mapping (apps/d2d/gamepad_map.hpp): dead zones, the
// walk/run switch, click edges, per-screen keys, and a pad that goes away.
#include "gamepad_map.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <vector>

using namespace d2d::client::pad;

namespace {
bool has(const Actions& actions, Key key) { return std::ranges::find(actions.keys, key) != actions.keys.end(); }
Sample with(Button which) { Sample sample; sample.held[std::size_t(which)] = true; return sample; }
}

int main() {
    // Inside the dead zone nothing moves.
    {
        Mapper mapper;
        Sample drift; drift.lx = 0.1f; drift.ly = -0.1f; drift.rx = 0.2f;
        const Actions actions = mapper.step(drift, Context::Game, 0.016f);
        assert(!actions.move_active && actions.cursor_dx == 0 && actions.cursor_dy == 0);
    }
    // The left stick walks in the game, towards its direction; a light push walks, a full one runs.
    {
        Mapper mapper;
        Sample light; light.lx = 0.5f;
        Actions actions = mapper.step(light, Context::Game, 0.016f);
        assert(actions.move_active && !actions.run && actions.move_x > 0.99f && std::abs(actions.move_y) < 1e-4f);
        Sample full; full.lx = 1.f;
        actions = mapper.step(full, Context::Game, 0.016f);
        assert(actions.run);
        // Hysteresis: easing a little off full stays running; only well under it walks again.
        Sample mid; mid.lx = 0.8f;    // tilt (0.8-0.25)/0.75 = 0.73: between walk_below and run_above
        actions = mapper.step(mid, Context::Game, 0.016f);
        assert(actions.run);
        Sample low; low.lx = 0.55f;   // tilt 0.4
        actions = mapper.step(low, Context::Game, 0.016f);
        assert(actions.move_active && !actions.run);
        actions = mapper.step(mid, Context::Game, 0.016f);
        assert(!actions.run);         // from walking, 0.73 is under run_above
    }
    // In the menus the left stick is a cursor, not a walk.
    {
        Mapper mapper;
        Sample stick; stick.lx = 1.f;
        const Actions actions = mapper.step(stick, Context::Menu, 0.1f);
        assert(!actions.move_active && actions.cursor_dx > 0);
    }
    // The right stick moves the cursor, quicker the harder it's pushed.
    {
        Mapper mapper;
        Sample half; half.ry = 0.6f;
        Sample full; full.ry = 1.f;
        const float slow = mapper.step(half, Context::Game, 0.1f).cursor_dy;
        const float quick = mapper.step(full, Context::Game, 0.1f).cursor_dy;
        assert(slow > 0 && quick > slow * 2);
    }
    // A click is one press edge, held down, then one release; the right trigger clicks too.
    {
        Mapper mapper;
        Actions actions = mapper.step(with(Button::South), Context::Game, 0.016f);
        assert(actions.click_press && actions.click_down && !actions.click_release);
        actions = mapper.step(with(Button::South), Context::Game, 0.016f);
        assert(!actions.click_press && actions.click_down);
        actions = mapper.step(Sample{}, Context::Game, 0.016f);
        assert(actions.click_release && !actions.click_down);
        Sample trigger; trigger.right_trigger = 0.8f;
        assert(mapper.step(trigger, Context::Game, 0.016f).click_press);
        trigger.right_trigger = 0.4f;   // still held: between the press and release thresholds
        assert(mapper.step(trigger, Context::Game, 0.016f).click_down);
    }
    // A pad that goes away (an empty Sample) lets go of a held click and a walk.
    {
        Mapper mapper;
        Sample held = with(Button::South); held.lx = 1.f;
        assert(mapper.step(held, Context::Game, 0.016f).move_active);
        const Actions gone = mapper.step(Sample{}, Context::Game, 0.016f);
        assert(gone.click_release && !gone.click_down && !gone.move_active && !gone.run);
    }
    // East right-clicks in the game; in a menu it's Esc, except on the title, where Esc quits.
    {
        Mapper mapper;
        Actions actions = mapper.step(with(Button::East), Context::Game, 0.016f);
        assert(actions.rclick_press && actions.keys.empty());
        mapper.step(Sample{}, Context::Game, 0.016f);
        actions = mapper.step(with(Button::East), Context::Menu, 0.016f);
        assert(!actions.rclick_press && has(actions, Key::Escape));
        mapper.step(Sample{}, Context::Menu, 0.016f);
        actions = mapper.step(with(Button::East), Context::Title, 0.016f);
        assert(actions.keys.empty() && !actions.click_press && !actions.rclick_press);
    }
    // A key is sent once per press, not every frame it's held.
    {
        Mapper mapper;
        assert(has(mapper.step(with(Button::North), Context::Game, 0.016f), Key::Inventory));
        assert(mapper.step(with(Button::North), Context::Game, 0.016f).keys.empty());
        mapper.step(Sample{}, Context::Game, 0.016f);
        assert(has(mapper.step(with(Button::North), Context::Game, 0.016f), Key::Inventory));
    }
    // The D-pad: the belt in the game, arrows in the menus; Start is the game menu or Enter.
    {
        Mapper mapper;
        assert(has(mapper.step(with(Button::DpadLeft), Context::Game, 0.016f), Key::Belt3));
        mapper.step(Sample{}, Context::Game, 0.016f);
        assert(has(mapper.step(with(Button::DpadLeft), Context::Menu, 0.016f), Key::Left));
        mapper.step(Sample{}, Context::Menu, 0.016f);
        assert(has(mapper.step(with(Button::Start), Context::Game, 0.016f), Key::Escape));
        mapper.step(Sample{}, Context::Game, 0.016f);
        assert(has(mapper.step(with(Button::Start), Context::Title, 0.016f), Key::Return));
    }
    // Screen directions as world ones (isometric, 160x80 tiles): up the screen is (-x, -y), right is (+x, -y).
    {
        const WorldStep north = world_step(0, -1, 160, 80, 6.f);
        assert(north.x < -4.2f && north.y < -4.2f && std::abs(north.x - north.y) < 1e-4f);
        const WorldStep right = world_step(1, 0, 160, 80, 6.f);
        assert(right.x > 4.2f && right.y < -4.2f);
        assert(std::abs(std::hypot(right.x, right.y) - 6.f) < 1e-4f);
        const WorldStep none = world_step(0, 0, 160, 80, 6.f);
        assert(none.x == 0 && none.y == 0);
    }
    // Soft targeting: the nearest in the facing cone and in reach; the held target sticks.
    {
        const std::vector<Candidate> things{ { 1, 1.f, 0.f }, { 2, -1.f, 0.f }, { 3, 6.f, 0.f }, { 4, 0.f, 2.f } };
        assert(pick_target(0, 0, 1, 0, things, -1) == 0);       // facing +x: the one ahead, not behind (2) or far (3)
        assert(pick_target(0, 0, -1, 0, things, -1) == 1);
        assert(pick_target(0, 0, 0, 0, things, -1) == 0);       // no facing: nearest of any
        assert(pick_target(0, 0, 0, -1, things, -1) == -1);     // nothing that way
        assert(pick_target(0, 0, 1, 0, things, 4) == 3);        // sticky 4 (2 tiles off the cone) stays in sticky reach
        assert(pick_target(0, 0, 1, 0, things, 3) == 0);        // sticky 3 is out of sticky reach: back to the nearest
        assert(pick_target(0, 0, 1, 0, {}, -1) == -1);
    }
    return 0;
}
