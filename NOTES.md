# Notes

## 2026-10-12 — fork created
- Forked psi29a/d2decomped at 625fd8f (770 commits, all by Bret Curtis; GPL-3.0-or-later)
  as jarvis-jhk/diablo2-couch. Remote `upstream` = psi29a/d2decomped (read-only).
- Upstream state: Act I complete (38 areas, 6 quests); Act II under way (branches act2,
  act2-more, PR #19 "polish-act2"). d2d can join an original-game TCP/IP host; d2d hosting,
  dedicated server and controllers are planned, not built. Upstream uses Claude skills
  (.claude/skills) and CodeRabbit reviews.
- Background research: JARVIS report "Diablo 2 Couch-Koop: Stand und Aufwand" (2026-10-12).
- Upstream README kept as docs/D2DECOMP-README.md; README.md is the fork's.
- Next: M0 in ROADMAP.md.

## 2026-10-12 — M0 baseline
- CI: the fork keeps upstream's `.github/workflows/ci.yml` unchanged (Linux GCC,
  Windows MSVC, macOS Clang; build, clang-tidy, ctest, package). Actions are enabled
  on the fork. macOS stays: the repo is public so minutes are free, and an untouched
  ci.yml merges cleanly; Linux + Windows are the required gate, macOS informational.
  Upstream's own run on 625fd8f (38260650029) passed all three in ~11 min.
  No local build here (Debian 12 container has no SDL3/StormLib/Qt6); CI is the gate.
- Upstream policy: no CONTRIBUTING / templates / AI statement; never an outside PR.
  Contact plan in docs/COUCH.md "Upstream": one honest issue (AI-written, asking if
  PRs are welcome) only once we have something concrete. Nothing opened yet.
- Survey: docs/COUCH.md. Key finding: our M1 server is upstream's own planned
  "N-player World, still in process" (net-host.md stage 1, ~2 weeks, deferred by
  upstream until after Act 2). Doing it alone in the fork means conflicts with nearly
  every upstream world.cpp/fight.cpp commit — ask upstream first. Client-side seams
  (host + N Town clients, per-player input, shared camera, panel origin, pause) are
  ours and narrow.
- First fork CI: a fork's push runs stayed gated even with Actions enabled by API;
  adding `workflow_dispatch:` to ci.yml (2 lines, the only fork change there) and
  pushing released them. Run 38276091534 on 813e56c: Linux GCC, Windows MSVC and
  macOS Clang all green; ctest 34/34 passed on each (MPQ-dependent tests SKIP, no
  game data on runners). M0 complete. Next: M1 needs the N-player World — decide
  between the upstream issue (contact plan) and client-side seams that don't need it
  (host/N-client split, per-player input, shared camera).

## 2026-10-12 — REA set up (Jan's request via adhoc)
- REA (rea-agents 6.4.0, Ghidra 12.1.4, JDK 21) installed in JARVIS's toolchain, not in
  the repo. Works on Debian 12 despite `rea doctor`; ~20 min import per call on the d2d
  ELF; PIE addresses are nm + 0x100000. Details: docs/research/rea.md.
- No D2 1.14d game.exe here (Blizzard's, user-supplied), so native analysis of the
  original is blocked until one is available locally; REA ran on our own CI build.
- Finding for M1: two hidden frame-to-frame statics, `fades` (wall see-through decided
  from ONE player's cell) and `eases` (light radius easing). Added to docs/COUCH.md;
  the wall cut-out must take every local hero on a shared screen.

## 2026-10-14 — upstream merge, group camera rules
- Merged upstream da2fd6d (22 commits): Act II complete, Act III's docks and jungle.
  Only conflict was README.md (ours kept, status table updated).
- M1 slice: `components/couch/group_camera.hpp` (header only, fork-only component) with
  `test_couch`: group camera = screen bounding-box centre clamped into a safe frame
  (one hero = today's camera exactly), even split when the party is too wide, and the
  leash rule (allowed if the party fits or does not widen). Negative controls: without
  the clamp the asymmetric-frame case fails; with the leash always true the wide-party
  case fails. Built and run locally with g++ 12 (-std=c++23 -Werror) and clang-tidy 22
  (repo .clang-tidy) clean; the full build still needs CI (no SDL3 here).
- Not wired: needs two heroes in one process (COUCH.md seams 1/3). Server-side PlayerSession
  is still upstream's planned stage 1; the upstream contact issue is still unopened.
- CI run 38506809924 on 3e6d394: Linux GCC, Windows MSVC, macOS Clang green (incl. clang-tidy
  and header checks); test_couch built and passed on all three, ctest 35/35.

## 2026-10-16 — upstream merge, controller seats and stick walk
- Merged upstream 6884077 (57 commits): Act III complete (all quests, Mephisto, hellgate),
  Act IV prep, Flatpak packaging (x86_64/ARM64 CI bundles), Steam Deck preview install
  steps. Only conflict README.md (ours kept, status table: Act III done, IV–V upstream);
  docs/D2DECOMP-README.md refreshed verbatim from upstream.
- ⚠️ Upstream's ci.yml now creates a GitHub release and uploads upstream-named packages
  (`d2decomped-*`) for ANY pushed tag. Before our first v0.x tag, gate or rename that job
  so a tag never publishes an untested or upstream-branded release on its own.
- Upstream's packaging plan (docs/design/linux-packaging-plan.md) names the SDL3 gamepad
  API and Steam Input for Deck controllers, but no gamepad code exists upstream yet.
- M1 slice: `components/couch/pads.hpp` + `test_couch_pads`: four seats (join on press,
  reconnect by identity keeps the seat, leave frees exactly one, no renumbering) and the
  stick walk (radial dead zone, fresh Move then re-aim at 160 px on screen, one stop Move
  on release). Local g++ 12 -Wpedantic -Werror and clang-tidy 23 (repo config) clean.
  Negative controls: reconnect without the identity check hands a waiting seat to a new
  pad (fails); release without the stop Move fails. Restored byte-exact.
- Not wired: needs SDL_INIT_GAMEPAD and event routing in window.cpp, and real use needs
  several heroes (seat 3). Virtual tests don't certify physical pads or the Deck.
- CI run 38872356099 on dbf5db0: Linux GCC, Windows MSVC, macOS Clang and both Flatpak
  bundles green; source SHA checked in the job logs, test_couch_pads passed, ctest 38/38.

## 2026-10-16 — Warpzone descriptor
- Warpzone (jarvis-jhk/warpzone) lists us in games/diablo2-couch.toml. Its asset globs are
  case-sensitive and matched nothing (`*linux*`, `*windows*` vs CPack's
  `d2d-<ver>-Linux-x86_64.tar.gz` / `d2d-<ver>-Windows-AMD64.zip`), and it had no `[launch]`.
  Fixed in jarvis-jhk/warpzone PR #2 (exact patterns, exec `d2d-*-<os>/d2d[.exe]`,
  prerelease = true); layout checked on the dbf5db0 CI artifacts.
- Open (ROADMAP M2): d2d does not persist `--data`, so the launcher's one-shot import only
  covers the first start. The inherited launcher's update check is off (empty
  D2LAUNCHER_UPDATE_URL); if we turn it on it must respect `.warpzone-managed`.
- Online via Warpzone is an M3 item, after couch co-op.

## 2026-10-16 — two players on one screen (M1 prototype)
- Merged upstream af67c23 (quest flag replication fixes); README conflict, ours kept.
- Asked upstream, as the contact plan says, whether they want the PlayerSession split
  (net-host.md stage 1) and SDL3 gamepad input as PRs: psi29a/d2decomped#30 (AI-written,
  stated). Tracked for the github-inbox in agent data/pr-watch/d2decomped-upstream.md.
  Not waiting for it: the fork carries a narrow party layer until upstream decides.
- Server (2d4cfca): components/couch/party, see docs/COUCH.md "Built (2026-10-16)".
  Upstream files touched: Fight::world (side_pair(), guests' sides, host-only monster and
  missile steps) and World::tick (couch_guest gates). Single player takes the same path
  as before (no guests: no sides, every gate open). At each upstream merge check new
  World/Fight/Loot fields against lend().
- Client (9ebc5bd): apps/d2d/couch, pads join with a hero picker, pad controls, group
  camera, guests drawn as other players, their HUD corners; devctl `couch` verbs.
- Release job: a tag now makes a draft prerelease of the d2d-* packages only (the Flatpak
  bundles carry upstream's app id), titled ours; publish by hand after checking assets.
- Versions: v0.0.x. Upstream's v0.1.0 tag is on our fork (from the fork) and v0.3.0
  upstream, so our first tags are v0.0.x and never collide.
- Verified: local clang 23 + libc++ build of game/couch_party/tests (-Werror), clang-tidy
  23 clean on our files, syntax check of every touched app file against SDL3/OpenAL/
  FFmpeg headers; test_couch_party compiles but SKIPs: no Diablo II data exists here or in
  the RE lab (re lane answer, 2026-10-16), so no gameplay run has been seen by us.
  Open: everything with real data, real pads and the Deck is Jan's first test.
- Known gaps (README "Couch preview"): guests have no panels, own waypoints or portals;
  guest missiles credit the host; the wall see-through follows the camera, not each hero.
