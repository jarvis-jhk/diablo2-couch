# Diablo 2 Couch roadmap

Fork-specific milestones. Engine progress (Acts II–V, hosting, dedicated server) is
upstream's plan in `docs/PLAN.md`; we merge it as it lands.

## M0 — Fork baseline
- [x] Upstream builds and its tests pass on our CI (Linux, Windows; macOS optional).
- [x] Upstream contribution policy and AI stance checked; contact plan recorded in NOTES.
- [x] Survey: how d2d structures client/server in one process; where per-player state
      (input, cursor, UI panels, camera) is global today. Write docs/COUCH.md.

## M1 — Two local players, Act I, shared screen (prototype)
Done when (combined roadmap item diablo2-two-players-act-i): a d2d prerelease where two
players, each with their own saved hero, walk and fight together in Act I on one shared
camera, with keyboard+pad or two pads.

- [x] Several local heroes in one game, each with its own character/save (2026-10-16):
      components/couch/party (guest Worlds lent the host's game state; test_couch_party,
      needs MPQs). Upstream's PlayerSession split (net-host.md stage 1) would replace it;
      asked upstream in psi29a/d2decomped#30.
- [x] One shared camera following the group, with a leash keeping players on screen
      (rules 2026-10-14, wired 2026-10-16).
- [x] Controller input per player (SDL3 gamepads), direct stick movement, skill buttons
      (2026-10-16, apps/d2d/couch). Menus and panels still need the mouse.
- [ ] Per-player inventory/stash/merchant panels that do not pause the world.
- [~] Group rules for waypoints, town portals and area transitions: player 1 leads, the
      guests follow (2026-10-16). Guests' own portals and waypoints: not yet.
- [ ] Guests' missiles credit their own kills; per-player town portals.

## M2 — Four players, packaging
- [ ] Four players; UI layout scaling for 1280×800 (Deck).
- [ ] Linux/Windows/Deck builds, installer, safe auto-update, first-start changelog.
- [ ] Anonymous fault relay (ntfy → GitHub issue).
- [ ] Warpzone tier 0 (launcher install/update/data hand-over): `d2d --data <dir>` persists
      `data =` into d2d.cfg (the hand-over passes it only once); any own updater stays silent
      when the install dir has `.warpzone-managed`; recheck games/diablo2-couch.toml in
      jarvis-jhk/warpzone against the release assets (docs/integration.md there).
- [ ] First full release (Act I couch co-op with packaging), hardware evidence recorded
      separately. Our versions are v0.0.x for now: upstream's own v0.1.0 / v0.3.0 tags exist
      and our tags must never collide with them.

## M3 — Full game
- [ ] Follow upstream Acts II–V; each act tested in couch co-op as it arrives.
- [ ] Online co-op through Warpzone (rooms/invites, hole-punch/relay, C API
      crates/capi/include/warpzone.h), after the couch milestones. No chat or free-text
      player content; warpzone.virtualtabletop.io is the only Jan domain allowed in the repo.
