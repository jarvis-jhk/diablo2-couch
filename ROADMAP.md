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
      2026-10-18: upstream now plans couch co-op natively — #34 session split, #36 SDL3
      gamepads, #38 gamepad menus/panels, #40 four-player shared screen, #35 Deck, #33
      remapping. Its PRs need a human owner/sign-off and gamepad footage (Jan's decision,
      NOTES 2026-10-18). Rule: when an upstream piece lands, migrate onto it and retire the
      fork's equivalent (party layer, apps/d2d/couch input) instead of keeping both.
- [x] One shared camera following the group, with a leash keeping players on screen
      (rules 2026-10-14, wired 2026-10-16).
- [x] Controller input per player (SDL3 gamepads), direct stick movement, skill buttons
      (2026-10-16, apps/d2d/couch). 2026-10-19: upstream's #36 gamepad layer landed; player 1's
      pad (pad_player1) now plays through it, menus and panels included. Guests' pads are still
      the fork's own (no cursor, no panels) until upstream's #40 seats exist.
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
      2026-10-17: the Linux tarball is built on glibc 2.28 (linux-portable.yml) and started
      on Ubuntu 24.04, Debian 12, Fedora and Arch in CI (v0.0.5). Open: a self-installing update
      (today the launcher links the release page).
- [x] Anonymous fault relay (ntfy → GitHub issue → lane), 2026-10-16.
- [x] Prerelease v0.0.2 with the above (2026-10-16); combined item closed.
- [~] Warpzone tier 0: `d2d --data <dir>` now persists `data =` (2026-10-16); the launcher's
      update notice stays silent with `.warpzone-managed`. Open: recheck
      games/diablo2-couch.toml in jarvis-jhk/warpzone against the release assets.
- [ ] Faults through Warpzone feedback (t_mvc4341b4kjxr, 2026-10-17): once libwarpzone 0.1.3 is
      released and /api/v1/feedback is live, route the existing fault_report path (pending
      crash mark, caught faults) to game "diablo2-couch" with stable titles; keep ntfy as fallback.
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
      Act III (combined item diablo2-act-iii-couch), done when: prerelease v0.0.4 where the
      party sails to Kurast with player 1 and plays Act III through Mephisto; quest items a
      guest walks to go into player 1's pack, quest objects a guest uses act for player 1,
      and Mephisto's kill reaches every guest's save.
      2026-10-16: quest objects/items to player 1, share_progression; v0.0.4 published, item closed.
      Act IV (combined item diablo2-act-iv-couch), done when: a prerelease where the party
      enters the Pandemonium Fortress with player 1 through the hellgate, plays Act IV through
      Diablo together, and Diablo's kill and the Act IV waypoints reach every guest's save.
      2026-10-16: blocked — upstream hasn't built Act IV (only branch act4-prep).
      2026-10-17: unblocked — upstream main fd86f34 builds Act IV (levels, quests, Diablo).
      2026-10-17: merged fd86f34; hellgate/Hellforge/seals/Tyrael's portal to player 1, a guest's
      boss kills run the host's quest hooks, Terror's End and Act V travel shared; v0.0.6
      published, item closed.
      Act V (combined item diablo2-act-v-couch), done when: a prerelease where the party
      follows player 1 to Harrogath, plays Act V through Baal together (Ancients fight and
      Worldstone portal take guests along), and Baal's kill and the Act V waypoints reach
      every guest's save. 2026-10-17: blocked — upstream has no Act V code or branch yet.
- [ ] Online co-op through Warpzone (rooms/invites, hole-punch/relay, C API
      crates/capi/include/warpzone.h), after the couch milestones. No chat or free-text
      player content; warpzone.virtualtabletop.io is the only Jan domain allowed in the repo.

## M4 — macOS support
Combined roadmap item diablo2-macos-support (Jan, 2026-10-19: macOS on every game's roadmap).
Comes after the items above; it does not reorder them.

Done when: every release carries a macOS build (Apple Silicon + Intel, or one universal
build) of d2d and the launcher, built and smoke-tested on a GitHub macOS runner in CI
(start with the SDL dummy driver like tests/smoke_d2d.py, checksum the uploaded asset),
and the player docs say how to install and start it. Unsigned is fine: document the
Gatekeeper workaround (right-click → Open, or `xattr -dr com.apple.quarantine`).

- [ ] Package: an .app bundle (or tarball) with SDL3, OpenAL Soft, StormLib, the Bink-only
      FFmpeg and Qt for the launcher bundled and their install names rewritten
      (macdeployqt / install_name_tool); today CI's "macOS (Clang)" job only builds and
      runs ctest against Homebrew's dylibs, and packages nothing.
- [ ] Architectures: macos-latest is arm64 only, and Homebrew bottles are single-arch, so
      a universal build means building the dependencies for both arches (or a separate
      Intel job on an x86_64 runner while one exists).
- [ ] Smoke test in CI on the packaged asset, then release upload + checksum like Linux/Windows.
- [ ] Player docs: install/start notes, where the MPQs go, Gatekeeper workaround.

Known risks:
- Rendering: SDL_CreateRenderer picks Metal on macOS; the letterboxing, scaling and
  Retina (high-DPI) pixel sizes are untested there.
- Controllers: SDL3 gamepads go through Apple's GameController framework; no Steam Input,
  and the per-player pad claiming/hotplug has only been exercised on Linux/Windows.
- Paths: userdir uses ~/Library/Preferences/<app>/ on macOS (upstream's choice); do not move
  saves or config without a migration and backup. The launcher's update check and
  `.warpzone-managed` handling need a macOS asset name and install layout.
- Warpzone: the client needs libwarpzone built for macOS (both arches) and a macOS entry
  in games/diablo2-couch.toml; fault reports must name the platform.
