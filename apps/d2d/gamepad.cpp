// SPDX-License-Identifier: GPL-3.0-or-later
// Definitions for gamepad.hpp.
#include "gamepad.hpp"
#include "gamepad_map.hpp"

#include <log.hpp>

#include <algorithm>
#include <cstddef>

namespace d2d::client {

namespace {
float axis(SDL_Gamepad* pad, SDL_GamepadAxis which) {
    return std::clamp(float(SDL_GetGamepadAxis(pad, which)) / 32767.f, -1.f, 1.f);
}
}

void Gamepads::open(SDL_JoystickID id) {
    if (pad_) return;   // one pad plays; a second one stays closed for now
    pad_ = SDL_OpenGamepad(id);
    if (!pad_) { d2d::log::warn("gamepad: {}", SDL_GetError()); return; }
    id_ = id;
    d2d::log::info("gamepad: {} connected", SDL_GetGamepadName(pad_) ? SDL_GetGamepadName(pad_) : "unnamed pad");
}

void Gamepads::close() {
    if (!pad_) return;
    d2d::log::info("gamepad: {} disconnected", SDL_GetGamepadName(pad_) ? SDL_GetGamepadName(pad_) : "unnamed pad");
    SDL_CloseGamepad(pad_);
    pad_ = nullptr;
    id_ = 0;
}

void Gamepads::open_connected() {
    int count = 0;
    if (SDL_JoystickID* ids = SDL_GetGamepads(&count)) {
        for (int i = 0; i < count && !pad_; ++i) open(ids[i]);
        SDL_free(ids);
    }
}

void Gamepads::handle_event(const SDL_Event& event) {
    if (event.type == SDL_EVENT_GAMEPAD_ADDED) {
        open(event.gdevice.which);
    } else if (event.type == SDL_EVENT_GAMEPAD_REMOVED && pad_ && event.gdevice.which == id_) {
        close();
        open_connected();   // another pad takes over
    }
}

pad::Sample Gamepads::sample() const {
    pad::Sample out;
    if (!pad_) return out;
    out.lx = axis(pad_, SDL_GAMEPAD_AXIS_LEFTX);   out.ly = axis(pad_, SDL_GAMEPAD_AXIS_LEFTY);
    out.rx = axis(pad_, SDL_GAMEPAD_AXIS_RIGHTX);  out.ry = axis(pad_, SDL_GAMEPAD_AXIS_RIGHTY);
    out.left_trigger = std::max(0.f, axis(pad_, SDL_GAMEPAD_AXIS_LEFT_TRIGGER));
    out.right_trigger = std::max(0.f, axis(pad_, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER));
    using pad::Button;
    const auto set = [&](Button which, SDL_GamepadButton button) { out.held[std::size_t(which)] = SDL_GetGamepadButton(pad_, button); };
    set(Button::South, SDL_GAMEPAD_BUTTON_SOUTH);              set(Button::East, SDL_GAMEPAD_BUTTON_EAST);
    set(Button::West, SDL_GAMEPAD_BUTTON_WEST);                set(Button::North, SDL_GAMEPAD_BUTTON_NORTH);
    set(Button::Back, SDL_GAMEPAD_BUTTON_BACK);                set(Button::Start, SDL_GAMEPAD_BUTTON_START);
    set(Button::LeftShoulder, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
    set(Button::RightShoulder, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
    set(Button::LeftStick, SDL_GAMEPAD_BUTTON_LEFT_STICK);     set(Button::RightStick, SDL_GAMEPAD_BUTTON_RIGHT_STICK);
    set(Button::DpadUp, SDL_GAMEPAD_BUTTON_DPAD_UP);           set(Button::DpadDown, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
    set(Button::DpadLeft, SDL_GAMEPAD_BUTTON_DPAD_LEFT);       set(Button::DpadRight, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
    return out;
}

SDL_Keycode pad_keycode(pad::Key key) {
    using pad::Key;
    switch (key) {
        case Key::Escape:    return SDLK_ESCAPE;
        case Key::Return:    return SDLK_RETURN;
        case Key::Up:        return SDLK_UP;
        case Key::Down:      return SDLK_DOWN;
        case Key::Left:      return SDLK_LEFT;
        case Key::Right:     return SDLK_RIGHT;
        case Key::Inventory: return SDLK_I;
        case Key::Character: return SDLK_C;
        case Key::SkillTree: return SDLK_T;
        case Key::QuestLog:  return SDLK_Q;
        case Key::Automap:   return SDLK_TAB;
        case Key::ToggleRun: return SDLK_R;
        case Key::Belt1:     return SDLK_1;
        case Key::Belt2:     return SDLK_2;
        case Key::Belt3:     return SDLK_3;
        case Key::Belt4:     return SDLK_4;
    }
    return SDLK_UNKNOWN;
}

}  // namespace d2d::client
