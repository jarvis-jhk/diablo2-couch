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
Done when (combined roadmap item diablo2-four-players-packaging): prerelease v0.0.2 where
host plus three pads play four heroes on one screen (letterboxed at the Deck's 1280x800),
the launcher checks our releases (silent under .warpzone-managed) and shows the changelog
once after an update, d2d remembers --data, crashes post an anonymous fault report that
becomes a GitHub issue; Flatpak for Linux/Deck, Windows zip, assets checksum-verified.

- [x] Four players: the host plus three pads (or four pads with pad_player1); the guests'
      corners fit along the top, the 800x600 game letterboxed at 1280x800 (2026-10-16).
- [~] Linux/Windows/Deck builds, installer, update notice, first-start changelog (2026-10-16:
      launcher update check against our releases, CHANGELOG.md shown once after an update).
      Flatpak = Linux/Deck download; the Linux tarball needs glibc 2.43 (SDL3 and Qt 6.5
      only exist on Ubuntu 25.04+). Open: a self-installing update (today the launcher
      links the release page), an older-glibc tarball (SDL3/Qt built from source).
- [x] Anonymous fault relay (ntfy → GitHub issue → lane), 2026-10-16.
- [x] Prerelease v0.0.2 with the above (2026-10-16); combined item closed.
- [~] Warpzone tier 0: `d2d --data <dir>` now persists `data =` (2026-10-16); the launcher's
      update notice stays silent with `.warpzone-managed`. Open: recheck
      games/diablo2-couch.toml in jarvis-jhk/warpzone against the release assets.
- [ ] First full release (Act I couch co-op with packaging), hardware evidence recorded
      separately. Our versions are v0.0.x for now: upstream's own v0.1.0 / v0.3.0 tags exist
      and our tags must never collide with them.

## M3 — Full game
- [~] Follow upstream Acts II–V; each act tested in couch co-op as it arrives.
      Act II (combined item diablo2-act-ii-couch), done when: prerelease v0.0.3 where the
      party travels with the host into and through Act II, the Arcane Sanctuary's teleport
      pads take the party along, and each guest's save gets the party's progress (Acts
      II/III opened, Andariel/Duriel done, waypoints reached, the act it is in).
      2026-10-16: couch/progress.hpp + pads pull; v0.0.3 published, Act II item closed.
- [ ] Online co-op through Warpzone (rooms/invites, hole-punch/relay, C API
      crates/capi/include/warpzone.h), after the couch milestones. No chat or free-text
      player content; warpzone.virtualtabletop.io is the only Jan domain allowed in the repo.
