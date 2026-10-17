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

## 2026-10-16 — v0.0.1 prerelease (first couch co-op preview)
- https://github.com/jarvis-jhk/diablo2-couch/releases/tag/v0.0.1 from a727d16, CI run
  38902698855: Linux GCC / Windows MSVC / macOS Clang / both Flatpaks green, tests 39/39
  (MPQ tests SKIP), source SHA checked in the job logs. All five release assets downloaded,
  SHA-256 equal to GitHub's digests and to the run's own artifacts; the Windows zip holds
  d2d.exe, d2d-launcher.exe and SDL3.dll; the Flatpak's metadata has our id and
  devices=all. Hashes are in the release notes.
- ⚠️ The Linux tarball is built on ubuntu-latest and needs glibc 2.43 (it would not even
  start here on Debian 12): too new for Linux Mint 22 or SteamOS. The Flatpak is the Linux
  and Deck download now (own app id io.github.jarvis_jhk.diablo2couch, --device=all for
  pads, version from a VERSION file). M2: build the tarball on an older base, or drop it.
- The release job makes drafts; I published this one after the checks. A tag pushed with a
  [skip ci] commit starts no run: dispatch the workflow on the tag (uploads now happen for
  tag refs too) or tag a commit without it.
- Combined roadmap item diablo2-two-players-act-i closed with this prerelease. Open, for
  Jan's test: real game data, real pads, the Deck (nobody here has played this build).

## 2026-10-16 — M2: fault reports, update notice, changelog, --data
- Combined roadmap item diablo2-four-players-packaging; its done-when is in ROADMAP M2.
- Four players were already in the M1 code: host + three guests (`party.size() >= 3` caps
  the guests), seats 0–3, corners 3 × 196 px along the 800 px top; at 1280×800 the game is
  letterboxed (SDL logical presentation) to 1067×800. Nothing to change there.
- Fault reports (05c26a8): components/couch/fault_report.hpp + test_couch_fault (negative
  control: accepting `-g` dev stamps fails it). Only a tag's own build (`git describe` =
  `vX.Y.Z`) reports; a `d2d.running` mark left in the user dir is a session that never
  ended, reported once at the next windowed start. d2d spawns a detached curl
  (System32\curl.exe on Windows); with no curl (the Flatpak runtime may have none) the line
  waits in `fault-report.txt` and the launcher posts it. Topic
  `diablo2-couch-faults-ae4dac602ec557e8` (public), `.github/workflows/fault-reports.yml`
  every 30 min → issue labelled fault-report → JARVIS_REPORT_URL (repo secret, set) → lane.
  A double Ctrl-C (`_exit(130)`) or power loss also counts as a crash: acceptable noise.
- Launcher: update check against `/releases?per_page=5` (ours are prereleases, which
  `/releases/latest` hides), numeric vX.Y.Z compare, silent with `.warpzone-managed` in
  its dir or the one above; CHANGELOG.md (Qt resource) section shown once per new version
  (QSettings `changelog/seen`; a fresh install shows nothing). No self-installing update.
- d2d remembers a working `--data` as `data =` unless `--headless`/`--no-save`.
- Warpzone: jarvis-jhk/warpzone PR #2 (descriptor patterns) is still open, and its
  Linux/Deck entries point at the tarball, which needs glibc 2.43 — so Warpzone on the
  Deck needs an older-base tarball first.
- Warpzone lane asked (t_mvb0neix3xjch8) for WARPZONE_DATA_D2LOD (launcher ≥0.1.2, warpzone
  PR #8): d2d now takes it like --data when neither --data nor D2_MPQ_DIR is given and the
  folder holds d2data.mpq (else the usual search goes on), and remembers it as data =.
  The descriptor's [[data]] (`./d2exp.mpq`, import_args `--data {path}`) already matches;
  no `present` (d2d keeps the path in d2d.cfg, not as a file) and no `known` fingerprints
  (no game data here to hash), so no warpzone PR for it.
- Checked on the v0.0.2 draft's Windows zip under Wine (no game data, no display): a
  planted `d2d.running` (v0.0.1) became the pending line "Diablo 2 Couch v0.0.1 Windows
  crash" (Wine has no curl.exe), the mark was gone after exit, WARPZONE_DATA_D2LOD (a dir
  with an empty d2data.mpq) was taken and written as `data =`, and a second start read it
  from d2d.cfg. The launcher under Xvfb showed the v0.0.2 changelog box after a seeded
  `changelog/seen = v0.0.1` and stored v0.0.2. Found: the modal box held the pending
  report back until closed — 7646e1f sends first; v0.0.2 retagged on it.
- v0.0.2 published: https://github.com/jarvis-jhk/diablo2-couch/releases/tag/v0.0.2 from
  7646e1f, CI run 38912670732 (Linux/Windows/macOS/both Flatpaks green, 40/40, source SHA
  checked in the Linux and Windows job logs). All five assets downloaded, SHA-256 equal to
  GitHub's digests (hashes in the release notes). The rebuilt launcher under Wine/Xvfb
  posted a pending line to the ntfy topic while the changelog box was open ("relay test
  … not a fault", which the workflow skips as foreign). `/releases?per_page=5` now lists
  v0.0.2 first. v0.0.1 launchers have no update check (they shipped with an empty URL);
  v0.0.2 onwards does.
- Combined roadmap item diablo2-four-players-packaging closed with this prerelease.
  Open for Jan's test: real data, three/four real pads, the Deck, the Flatpak's crash
  path (does its runtime have curl, else the launcher sends). Open work (ROADMAP M2):
  self-installing updates, an older-glibc Linux tarball (Warpzone Linux/Deck needs it;
  warpzone PR #2 still open), personal panels (M1).

## 2026-10-16 — Act II couch (combined item diablo2-act-ii-couch)
- Upstream main unchanged since af67c23 (nothing to merge). Read the act boundary as a guest:
  (1) the host's teleport pad (Arcane Sanctuary, OperateFn 27) moved the host across the level
  in one tick and the leash undid it — the pads were unusable with guests; (2) the quest records
  run in the host's World only and upstream marks "no party credit", so a guest crossing with
  Warriv went home with Andariel alive, Act II closed and none of the waypoints.
- Fix: World::couch_pads counts pads taken (one line in the pad branch, one field); a pad taken
  by the host brings every guest beside it, a pad taken by a guest brings the host and the other
  guests (Party::follow is now hero → leader), and no leash runs in that tick.
  components/couch/progress.hpp: after each party tick the bits the HOST GAINED in the tick for
  quest words 6/7/14/15/22/23 (act bosses, act travel) go to each guest unless its own quest is
  done (bit 0) or closed (bit 15); the waypoints the host gained likewise. A guest joining a host
  already in a later act gets the travel words up to it and set_act there.
  Not shared on purpose: quests a hero claims at an NPC (Den skill point, Radament's book,
  imbue, Akara's respec) — guests can't talk yet, so those stay player 1's.
- Test: test_couch_progress (MPQ-free). Negative controls: dropping the done/closed guard fails
  the "Duriel already beaten" case; copying `after` instead of the gain fails the "host's past
  stays the host's" case. Restored. Local clang 23 build of game/couch_party/tests -Werror clean,
  clang-tidy clean on our files. The pad pull and the shares are only exercised with real data:
  nobody here has played them (no MPQs), Jan's test.
- Known: a guest on another difficulty than the host gets the bits in its own active
  difficulty's words.
- v0.0.3 published: https://github.com/jarvis-jhk/diablo2-couch/releases/tag/v0.0.3 from 2316619.
  Push run 38917360988 and tag run 38918736788 green (Linux/Windows/macOS/both Flatpaks, 41/41,
  test_couch_progress passed; source SHA checked in the job logs). All five assets downloaded,
  SHA-256 equal to GitHub's digests (hashes in the release notes); the Windows zip holds d2d.exe,
  d2d-launcher.exe, SDL3.dll. Combined item diablo2-act-ii-couch closed with it.
  Open for Jan's test: crossing to Lut Gholein with guests, the Arcane pads, a guest's save
  starting in Act II with the shared waypoints.

## 2026-10-16 — Act III couch (combined item diablo2-act-iii-couch)
- Upstream main unchanged since af67c23. Act III is fetch quests, and a guest could strand
  every one: a guest picking up Khalim's eye/heart/brain, the jade figurine, the Gidbinn or
  Lam Esen's tome kept it in a pack it has no panel for, while only the host talks, cubes and
  wields the Will (upstream polls the host's `carries()`). A guest using a quest object ran
  it with the GUEST's quest bits and carries against the host's lent records (Khalim's chest
  drops no part to a guest whose own save finished Khalim's Will; Lam Esen's stand took the
  guest's bits; the Sanctuary's portal moved the guest, and follow() snapped it back).
- Fix: a guest's tick leaves quest objects (World::quest_object: the Act I–III stands,
  stones, altars, chests, journal, Sanctuary portal, lever, orb, hellgate) and quest items
  (item_base `quest`) to the host: `couch_for_host` / `couch_quest_items`, which Party runs
  as host.operate / host.pick_up (pick_up = the old pickup branch with the malus line,
  extracted). No room in the host's pack: the item stays on the floor. This fixes Act I/II's
  quest items too (Cube, staff pieces, scrolls). World::operate takes no distance, so the
  host uses an object the guest stands at.
- Mephisto: word 22 was already shared; World::open_act raises the header's progression,
  which no guest got (Andariel/Duriel too). share_progression passes the host's gain to a
  guest on the same difficulty, never lowering, never across difficulties. Test in
  test_couch_progress; negative control (difficulty guard off) fails the Nightmare/Normal
  case. Known: each player in Durance 3 gets a soulstone in game.exe; a guest gets bit 0xb
  without a stone (only the host's drops, and goes to the host): Act IV's Hellforge is
  player 1's, like the other NPC rewards. The hellgate is upstream's "Act 4 isn't built".
- v0.0.4 published: https://github.com/jarvis-jhk/diablo2-couch/releases/tag/v0.0.4 from c5de58b.
  Push run 38921815044 and tag run 38924043732 green (Linux/Windows/macOS/both Flatpaks, 41/41,
  test_couch_progress passed; source SHA checked in the Linux and Windows job logs). All five
  assets downloaded, SHA-256 equal to GitHub's digests (hashes in the release notes); the
  Windows zip holds d2d.exe, d2d-launcher.exe, SDL3.dll. Combined item diablo2-act-iii-couch
  closed. Open for Jan's test: Meshif to Kurast with guests, a guest opening Khalim's chests /
  picking up a part (lands in player 1's pack), Mephisto's kill in a guest's save.

## 2026-10-16 — Act IV couch blocked on upstream (combined item diablo2-act-iv-couch)
- Upstream main 5005cf3 (8 commits ahead of ours: opt-in Sentry crash reporting, Act III
  quest-state bounds fix) and branch act4-prep (3bfce4f, already contained in main): Act IV
  is not built — `kBuiltLevels` lacks it, no layout, no town, quests or Diablo; the hellgate
  says so. Per the brief we never claim an act upstream hasn't built, so the item is blocked
  ("upstream Act IV unbuilt") with its done-when in ROADMAP M3. Resume when upstream lands
  Act IV: merge, then the same road as Acts II/III (hellgate travel with guests, Hellforge
  stays player 1's, Diablo's word and the progression via progress.hpp).
- Before the next upstream merge: check upstream's Sentry reporting against our fault-relay
  rules (anonymous, version/platform/category only); keep it off/unconfigured in our builds.

## 2026-10-17 — Linux tarball for every current distro (Jan: "linux support")
- The release tarball came from ubuntu-26.04 and needed glibc 2.43: it started on no Mint,
  Ubuntu LTS, Debian or SteamOS. New `.github/workflows/linux-portable.yml` (called by
  ci.yml, and dispatchable alone) builds in quay.io/pypa/manylinux_2_28 (AlmaLinux 8,
  glibc 2.28) with gcc-toolset-14, whose libstdc++ links the newer parts statically:
  highest needs are GLIBC_2.28 / GLIBCXX_3.4.22 (a CI step fails on GLIBC ≥ 2.29). SDL3 3.4.16
  and OpenAL Soft 1.25.2 from source (they dlopen X11/Wayland/PipeWire/Pulse/ALSA/udev),
  Qt 6.8.3 from aqtinstall (the Qt Company's RHEL 8 build, GTK theme plugin removed). The
  build job of ci.yml no longer uploads its Linux tarball; the release takes this one.
- A smoke job unpacks it in ubuntu:24.04, debian:12, fedora:latest and archlinux:latest:
  ldd of every binary/library/plugin (LD_LIBRARY_PATH=lib standing in for the rpath),
  `d2d --help`, the launcher alive after 5 s on Xvfb. All four green on run 38982029235.
  Locally (Debian 12, Xvfb) the launcher drew its window. No game data: gameplay unchanged.
- Things that bit, for next time: glibc < 2.34 needs Threads::Threads (game now links
  it); Qt's deploy resolves via rpath + ldconfig, not LD_LIBRARY_PATH (CI registers the
  deps with ldconfig); with the Qt Company's Qt, plugins are deployed only for a finalized
  target whose plugin targets exist in the deploying directory (apps/ now finds Widgets/
  Network and calls qt_finalize_target); StormLib's install rules need STORM_SKIP_INSTALL.
- Deploy excludes now also cover libstdc++/libgcc_s/libz/GLib/pcre2/zstd: through DT_RPATH
  an older bundled copy would win over the system's for the GL driver too. libOpenGL
  (Ubuntu's libopengl0 isn't always installed), libselinux (Arch has none) and krb5 with
  its libcrypto.so.1.1 ship. Tarball 39 MB.
- Not verified: a real desktop session, Wayland-only sessions (Qt falls back to XWayland;
  only the xcb platform plugin ships), physical pads, the Deck.
- v0.0.5 published: https://github.com/jarvis-jhk/diablo2-couch/releases/tag/v0.0.5 from 85edbc2.
  Push run 38983296567 and tag run 38984336418 green (Linux/Windows/macOS/both Flatpaks,
  linux-portable build + all four distro starts; source SHA checked in the Linux, Windows and
  portable job logs). All five assets downloaded, SHA-256 equal to GitHub's digests (hashes in
  the release notes); the released tarball's launcher drew its window here (Debian 12, Xvfb),
  the Windows zip holds d2d.exe, d2d-launcher.exe, SDL3.dll. Warpzone's Linux/Deck entries
  point at this tarball, which now starts there. Next version > v0.0.5.

## 2026-10-17 — Act V couch blocked on upstream; Act IV unblocked
- Upstream main fd86f34: no Act V anywhere (no act5 files, branch or commit). The combined
  item diablo2-act-v-couch got its done-when (ROADMAP M3) and is blocked ("upstream Act V
  unbuilt"). Resume when upstream lands Act V: merge, then the Acts II–IV road (Anya's
  portal/Ancients' altar and the Worldstone portal take guests along, Baal's kill and the
  progression via progress.hpp).
- Upstream main now builds Act IV (cf873d8 levels + hellgate, 0fcaf4e quests, 5244c2b/bd25656
  monsters and Diablo, 202cc87 ending), not merged here yet. The Act IV item's block was
  cleared so the next quota worker merges upstream and takes it.

## 2026-10-17 — Act IV couch (combined item diablo2-act-iv-couch)
- Merged upstream fd86f34 (23 commits): Act IV built (levels, hellgate, Izual, Hellforge,
  seals, Diablo, classic ending), Diablo's target pick, opt-in Sentry crash reporting,
  Coverity on main. Conflicts: apps/CMakeLists (both kept), d2d CMakeLists (both defines,
  sentry linked), main.cpp (our session mark, then upstream's SentrySession), world.cpp (act4
  hooks host-only like acts 2/3), fight.cpp (Diablo's player score moved into side_pair(),
  so every hero's Foe carries it).
- Sentry: upstream's DSN (their Sentry org) blanked in apps/launcher/CMakeLists.txt, so the
  launcher's crash-report box stays disabled and d2d never gets SENTRY_DSN; frames and
  breadcrumbs would break our anonymous-only rule. sentry-native is still linked (smaller
  merge diff). CI's symbol upload skips without SENTRY_AUTH_TOKEN (we have none).
- lend(): the Act IV records and timers (izual, hellforge, terror, izual_ghost_at,
  diablo_at, diablo_dead_at, fortress_warp_done, end_movie, game_closed, had_soulstone).
- Fixed for every act: a guest's own blow ran the quest kill hooks with the GUEST's bits and
  the lent records, so Mephisto killed by a guest marked the host's record done without the
  host's word 22 (hellgate shut for player 1), and Diablo likewise. Now a guest's tick hands
  its kills to the host (World::couch_kills -> Party -> host.couch_kill), and step 4 shares.
- quest_object: Hellforge, the five seals, Tyrael's portal (hellgate already there).
  progress.hpp shares words 26 (Terror's End) and 28 (Act V opened); 25 (Izual's skill
  points) and 27 (Hellforge soulstone/gems) stay player 1's. Classic Diablo's open_act(4)
  reaches guests via share_progression. test_couch_progress extended; negative control
  (28 dropped from the list) fails the new case. Act IV waypoints: share_waypoints, as before.
- Not verified here (no MPQs): the hellgate crossing with guests, the seals/Hellforge used by
  a guest, Diablo's kill in a guest save. Jan's test.
