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
