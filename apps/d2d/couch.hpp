// SPDX-License-Identifier: GPL-3.0-or-later
// Diablo 2 Couch: the client's side of local co-op (ROADMAP M1,
// docs/COUCH.md seams 3-5). Controllers (SDL3 gamepads), which pad plays
// which hero, the guests' heroes driven by their pads, joining with a saved
// hero, and what a guest sees of itself on the shared screen.
//
// The heroes: the host is the hero the menus started (mouse and keyboard,
// and its pad with pad_player1); each further pad that presses a button
// joins with a hero of its own picked from the saves (couch/party.hpp).
// A pad plays without a cursor: the stick walks, A attacks the nearest
// monster or else takes what's close (an item, the hero's corpse, an object,
// an NPC), X casts the right skill at the nearest monster, Y and the
// shoulders change the left and right skill, the d-pad drinks the belt's
// columns, B toggles run. Back held for two seconds: the guest leaves (its
// save written). A guest's panels (inventory, stash, merchants) are not
// built yet (ROADMAP M1).
#pragma once

#include "common.hpp"
#include "game_api.hpp"
#include "scene.hpp"
#include "world_view.hpp"

#include <group_camera.hpp>
#include <pads.hpp>
#include <party.hpp>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_joystick.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace d2d::client {

// One connected gamepad: its buttons now and this frame's presses, its sticks.
struct CouchPad {
    SDL_Gamepad* handle = nullptr;
    SDL_JoystickID id = 0;
    std::uint64_t identity = 0;            // GUID and serial, hashed: the same pad after a reconnect
    std::array<bool, SDL_GAMEPAD_BUTTON_COUNT> down{}, pressed{};
    d2d::couch::Stick left, right;
    std::uint32_t back_since = 0;          // Back held since (frame ms), 0 not held
};

// Every pad SDL reports, and the seats they hold. Process-wide: main feeds
// it SDL's events; the game reads it each frame.
struct CouchPads {
    std::vector<CouchPad> pads;
    d2d::couch::Seats seats;
    std::vector<std::size_t> joined;       // seats a press took this frame
    void event(const SDL_Event& event);
    void begin_frame();                    // this frame's presses start empty
    [[nodiscard]] CouchPad* seat(std::size_t index);
    ~CouchPads();
};

// A pad-driven hero's client state.
struct CouchHero {
    d2d::couch::StickWalk walk{};
    int attack = -1;                       // the monster A is holding on (unit id)
    int left = 0, right = 0;               // Skills.txt ids
    bool skills_set = false;
    float face_x = 0, face_y = 1;          // the last stick direction, world units (a right skill with no target goes there)
};

struct Couch {
    explicit Couch(World& host) : party(host) {}
    d2d::couch::Party party;
    CouchPads* pads = nullptr;             // main's
    bool pad_hosts = false;                // the first seat's pad plays the host (pad_player1; on by default on a Steam Deck)
    std::array<World*, d2d::couch::kMaxSeats> seat_hero{};   // a seat's guest World, nullptr none (the host's seat: none)
    std::array<CouchHero, d2d::couch::kMaxSeats> heroes;
    // Picking a hero for a seat that just joined: the saves not in play.
    struct Picker { std::size_t seat = 0; std::vector<int> saves; int index = 0; };
    std::optional<Picker> picker;
    std::vector<std::vector<Command>> guest_cmds;   // by guest index, for the next tick
    // The guests' feet a tick ago and now (the draw slides between them).
    std::vector<d2d::couch::Point> prev, cur;

    [[nodiscard]] bool active() const { return party.size() > 0; }
    // The player a seat is: the host is player 1, so pads count on from 2
    // unless the first pad plays the host.
    [[nodiscard]] std::size_t player_number(std::size_t seat) const { return seat + (pad_hosts ? 1 : 2); }
    // The frame's pads: joins, the picker, each hero's commands (the host's
    // into `host_out`). `store`: where guests are saved.
    void frame(const Scene& scene, const CharacterStore* store, std::uint32_t frame_ms, std::vector<Command>& host_out);
    // After each World tick: the guests' positions for the slide.
    void ticked();
    // A guest's feet now, slid between its last two ticks.
    [[nodiscard]] d2d::couch::Point slid(std::size_t guest, float blend) const;
    // The guests into the host's View as other players (drawn as a joined
    // game's are; `others` isn't replicated), slid between ticks.
    void into_view(View& view, float blend) const;
    // Their mercs and pets, as units the world draws.
    void units(const Scene& scene, std::vector<Unit>& out) const;
    // Every guest's corner: name, life and mana, skills; the picker.
    void draw(std::vector<std::uint8_t>& framebuffer, const Scene& scene) const;
    // Everyone leaves (saved): a new game or the end of this one.
    void clear();
    // Saves every guest; the errors joined, "" when all were written.
    std::string save();
    // devctl (and tests): a guest joins with the save named `name` in the
    // first free seat, without a pad; "" or why not.
    std::string join_named(const Scene& scene, const CharacterStore* store, const std::string& name);
    // A seat's guest leaves (saved), its pad free for anyone.
    void clear_seat(std::size_t seat) { leave(seat); }

private:
    void join(const Scene& scene, const CharacterStore* store, std::size_t seat, int save);
    void leave(std::size_t seat);
    [[nodiscard]] std::size_t index_of(const World* guest) const;
    std::vector<Command> commands(const Scene& scene, CouchPad& pad, CouchHero& hero, const World& self, std::uint32_t frame_ms);
};

}  // namespace d2d::client
