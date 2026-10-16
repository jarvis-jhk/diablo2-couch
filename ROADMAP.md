# Diablo 2 Couch roadmap

Fork-specific milestones. Engine progress (Acts II–V, hosting, dedicated server) is
upstream's plan in `docs/PLAN.md`; we merge it as it lands.

## M0 — Fork baseline
- [x] Upstream builds and its tests pass on our CI (Linux, Windows; macOS optional).
- [x] Upstream contribution policy and AI stance checked; contact plan recorded in NOTES.
- [x] Survey: how d2d structures client/server in one process; where per-player state
      (input, cursor, UI panels, camera) is global today. Write docs/COUCH.md.

## M1 — Two local players, Act I, shared screen (prototype)
- [ ] Two in-process clients joined to the embedded server, each with its own character/save.
- [ ] One shared camera following the group, with a leash keeping players on screen.
      Rules done and unit-tested (components/couch, 2026-10-14); wiring waits for two heroes.
- [ ] Controller input per player (SDL3 gamepads), direct stick movement, skill buttons.
      Seats (join on press, reconnect keeps the seat) and stick walk done and unit-tested
      (components/couch/pads.hpp, 2026-10-16); SDL3 gamepad events and skill buttons not wired.
- [ ] Per-player inventory/stash/merchant panels that do not pause the world.
- [ ] Group rules for waypoints, town portals and area transitions.

## M2 — Four players, packaging
- [ ] Four players; UI layout scaling for 1280×800 (Deck).
- [ ] Linux/Windows/Deck builds, installer, safe auto-update, first-start changelog.
- [ ] Anonymous fault relay (ntfy → GitHub issue).
- [ ] Warpzone tier 0 (launcher install/update/data hand-over): `d2d --data <dir>` persists
      `data =` into d2d.cfg (the hand-over passes it only once); any own updater stays silent
      when the install dir has `.warpzone-managed`; recheck games/diablo2-couch.toml in
      jarvis-jhk/warpzone against the release assets (docs/integration.md there).
- [ ] First release v0.1.0 (Act I couch co-op), hardware evidence recorded separately.

## M3 — Full game
- [ ] Follow upstream Acts II–V; each act tested in couch co-op as it arrives.
- [ ] Online co-op through Warpzone (rooms/invites, hole-punch/relay, C API
      crates/capi/include/warpzone.h), after the couch milestones. No chat or free-text
      player content; warpzone.virtualtabletop.io is the only Jan domain allowed in the repo.
