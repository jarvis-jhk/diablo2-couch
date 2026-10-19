// SPDX-License-Identifier: GPL-3.0-or-later
// SDL3 gamepads: hotplug and one frame's Sample of the pad in use. The
// mapping to actions is gamepad_map.hpp's; SDL types stay in this client layer.
#pragma once

#include "gamepad_map.hpp"
#include "platform.hpp"

namespace d2d::client {

class Gamepads {
public:
    Gamepads() = default;
    ~Gamepads() { close(); }
    Gamepads(const Gamepads&) = delete;
    Gamepads& operator=(const Gamepads&) = delete;

    // Open the pads already plugged in (SDL_INIT_GAMEPAD is up).
    void open_connected();
    // Hotplug events; other events are ignored.
    void handle_event(const SDL_Event& event);
    // The pad in use as of now: a default Sample (all let go) when none is.
    [[nodiscard]] pad::Sample sample() const;
    [[nodiscard]] bool connected() const { return pad_ != nullptr; }

    void close();

private:
    void open(SDL_JoystickID id);
    SDL_Gamepad* pad_ = nullptr;
    SDL_JoystickID id_ = 0;
};

// The SDL key a pad's Key stands for.
[[nodiscard]] SDL_Keycode pad_keycode(pad::Key key);

}  // namespace d2d::client
