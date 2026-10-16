// SPDX-License-Identifier: GPL-3.0-or-later
// Diablo 2 Couch: which controller belongs to which local player, and how a
// stick becomes a walk (ROADMAP M1 "Controller input per player",
// docs/COUCH.md seam 4).
//
// No SDL here: the client feeds in what SDL3 reports (a device's runtime id
// on add/remove, its stable identity, the stick axes) and gets back seats and
// world targets, so the rules are unit-tested alone.
//
// Seats. Up to four. A pad takes the first free seat when its player presses
// a button (not when it is merely plugged in, so a Steam Input shadow device
// or a pad lying on the table doesn't take a hero). A pad that drops out
// (battery, Bluetooth) keeps its seat and its hero: the same pad coming back,
// recognised by its identity (GUID and serial, hashed by the caller), takes
// that seat again even under a new runtime id. Only an explicit leave frees
// a seat. Seats never renumber, so player 2 stays player 2.
//
// Walking. d2d's mouse walk sends a fresh Move on the click and re-aims it
// every frame while the button is held (Town's held-mouse branch). The stick
// does the same with a target a fixed screen distance from the hero in the
// stick's on-screen direction, so up on the stick is up on the screen
// whatever the isometric axes do. Letting go sends one Move to the hero's own
// position, which stops it where it stands instead of walking on to the last
// target.
#pragma once

#include "group_camera.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace d2d::couch {

inline constexpr std::size_t kMaxSeats = 4;

struct Seat {
    std::uint64_t identity = 0;   // stable across reconnects: GUID + serial, hashed
    std::uint32_t device = 0;     // SDL's runtime id while connected, 0 while away
    bool taken = false;
    constexpr bool connected() const { return taken && device != 0; }
};

class Seats {
public:
    // The seat a connected device drives, if any.
    constexpr std::optional<std::size_t> of(std::uint32_t device) const {
        if (device == 0) return std::nullopt;
        for (std::size_t i = 0; i < kMaxSeats; ++i)
            if (seats_[i].taken && seats_[i].device == device) return i;
        return std::nullopt;
    }

    // A pad was plugged in (or came back). It gets its old seat again if its
    // identity held one and that seat is waiting; otherwise nothing yet.
    constexpr std::optional<std::size_t> connect(std::uint32_t device, std::uint64_t identity) {
        if (const auto seat = of(device)) return seat;
        for (std::size_t i = 0; i < kMaxSeats; ++i)
            if (seats_[i].taken && seats_[i].device == 0 && seats_[i].identity == identity) {
                seats_[i].device = device;
                return i;
            }
        return std::nullopt;
    }

    // A button on this device: a pad without a seat joins the first free one
    // (none when four are taken). A pad already seated just keeps its seat.
    constexpr std::optional<std::size_t> press(std::uint32_t device, std::uint64_t identity) {
        if (const auto seat = connect(device, identity)) return seat;
        for (std::size_t i = 0; i < kMaxSeats; ++i)
            if (!seats_[i].taken) {
                seats_[i] = { identity, device, true };
                return i;
            }
        return std::nullopt;
    }

    // The pad went away. Its seat waits for it; the hero stays.
    constexpr std::optional<std::size_t> disconnect(std::uint32_t device) {
        const auto seat = of(device);
        if (seat) seats_[*seat].device = 0;
        return seat;
    }

    // The player left: the seat is free for anyone again.
    constexpr void leave(std::size_t seat) { seats_[seat] = {}; }

    constexpr const Seat& operator[](std::size_t seat) const { return seats_[seat]; }
    constexpr std::size_t taken() const {
        std::size_t count = 0;
        for (const Seat& seat : seats_) count += seat.taken ? 1 : 0;
        return count;
    }

private:
    std::array<Seat, kMaxSeats> seats_{};
};

// A stick's position, each axis in [-1, 1] as SDL3 scales it (axis / 32767),
// y down like the screen.
struct Stick {
    float x = 0, y = 0;
};

// Radial dead zone: below `inner` nothing, above `outer` full, linear in
// between, direction kept. Radial, not per axis, so diagonals aren't snapped
// to the axes.
inline Stick dead_zone(Stick stick, float inner = 0.24f, float outer = 0.95f) {
    const float length = std::hypot(stick.x, stick.y);
    if (length <= inner) return {};
    const float scaled = std::min(1.f, (length - inner) / (outer - inner));
    return { stick.x / length * scaled, stick.y / length * scaled };
}

// The world point `reach` screen pixels from `hero` in the stick's direction
// (the stick already through the dead zone and not centred).
inline Point stick_target(Point hero, Stick stick, float reach) {
    const float length = std::hypot(stick.x, stick.y);
    const Point offset = to_world({ stick.x / length * reach, stick.y / length * reach });
    return { hero.x + offset.x, hero.y + offset.y };
}

// What the stick asks for this frame: a Move to `target`, fresh or re-aimed.
struct Walk {
    Point target;
    bool fresh = false;
};

// One player's stick walk, frame to frame. 160 px is two cells across the
// screen: far enough that the walk never arrives between two frames, near
// enough that a re-aim around an obstacle stays local.
class StickWalk {
public:
    explicit constexpr StickWalk(float reach = 160.f) : reach_(reach) {}

    // The Move to send, if any. `stick` is the raw stick.
    std::optional<Walk> update(Point hero, Stick stick) {
        const Stick live = dead_zone(stick);
        if (live.x == 0 && live.y == 0) {
            if (!walking_) return std::nullopt;
            walking_ = false;
            return Walk{ hero, false };   // stop where it stands
        }
        const bool fresh = !walking_;
        walking_ = true;
        return Walk{ stick_target(hero, live, reach_), fresh };
    }
    constexpr bool walking() const { return walking_; }

private:
    float reach_;
    bool walking_ = false;
};

}  // namespace d2d::couch
