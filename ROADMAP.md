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
- [ ] Per-player inventory/stash/merchant panels that do not pause the world.
- [ ] Group rules for waypoints, town portals and area transitions.

## M2 — Four players, packaging
- [ ] Four players; UI layout scaling for 1280×800 (Deck).
- [ ] Linux/Windows/Deck builds, installer, safe auto-update, first-start changelog.
- [ ] Anonymous fault relay (ntfy → GitHub issue).
- [ ] First release v0.1.0 (Act I couch co-op), hardware evidence recorded separately.

## M3 — Full game
- [ ] Follow upstream Acts II–V; each act tested in couch co-op as it arrives.
