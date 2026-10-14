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
