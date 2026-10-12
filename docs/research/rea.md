# REA in this fork

REA ("Reverse Engineer Anything", MIT) is a CLI/MCP layer over Ghidra headless. It returns
one evidence JSON per question: the function's pseudocode, assembly, callers, callees,
data references and value flow, each tagged with where it came from. Use only the
genuine package: npm `rea-agents` (maintainer morluto, repo github.com/morluto/rea).
Lookalike repos with "Download rea" exist and are probably malware.

It does not replace upstream's `.claude/skills/reverse-engineering` (Ghidra scripts plus
the tools/emu unicorn oracle). That stays the way to prove "matches game.exe". REA is
useful for a quick, evidence-tagged answer to "who calls this, what does it read and write".

## Setup (JARVIS container, 2026-10-12)

Everything sits outside the repo under JARVIS's data/toolchain, never system-wide:

- rea-agents 6.4.0 (`npm i rea-agents@6.4.0` in toolchain/rea)
- Ghidra 12.1.4 (zip sha256 ddac49f9…d4db, as published on the GitHub release)
- Temurin JDK 21.0.12.1+1 (sha256 checked against Adoptium)

    export JAVA_HOME=…/jdk-21.0.12.1+1 PATH=$JAVA_HOME/bin:$PATH \
           GHIDRA_INSTALL_DIR=…/ghidra_12.1.4_PUBLIC REA_GHIDRA_STARTUP_TIMEOUT_MS=1800000
    rea function <binary> <address> --format json

`rea doctor` calls Debian 12 an unsupported host, but the Ghidra provider works.
Commands that need pwntools (`inspect-binary-layout` on ELF) or Hopper are not set up.

## Pitfalls seen

- Every call imports and auto-analyses the binary again: about 20 minutes for the
  10 MB d2d ELF, with two CPU cores. `--snapshot` saves the result but does not skip
  the import. Ask several questions per session, not one at a time.
- Ghidra rebases a PIE ELF to 0x100000. An `nm` address must be shifted by that much;
  passing the raw `nm` address returned a different (CLI11) function without complaint.
  Check `procedure.name` in the result every time.
- On our own optimised C++ build Ghidra split `d2d::client::frame_light` after 49 bytes
  (its body continues past 0x323800), so references made by the rest of the function,
  such as its static `eases`, are missing from the evidence. Function boundaries on
  heavily inlined C++ need checking against the assembly; game.exe (MSVC, 32-bit) is
  the better fit.

## The original game binary

The question REA is meant for in this fork is native analysis of Diablo II 1.14d
`game.exe`: checking ported functions and finding which state the original keeps per
player. There is no game.exe in this environment; it is Blizzard's and comes only from
a user's own install. Until one is supplied locally, REA here runs on our own build.
Never commit game.exe, MPQs or decompiled game code; record addresses and behaviour in
our own words under docs/research/re/ as upstream does, and note the REA/Ghidra version.

## First question: hidden per-view state in our own client (d2d at 813e56c)

M1 runs several local players in one process with one shared view. Anything that keeps
state between frames outside an object is a candidate for being shared by mistake.
The evidence was the d2d Linux CI artifact of run 38276091534 (813e56c, binary sha256
40344bd1…c8fc4c): its symbol table (`nm`, writable `b/d` objects in `d2d::`) for the
inventory, REA/Ghidra for the call graph, then the source.

Function-local statics that carry state across frames, besides the screen mask already
listed in COUCH.md:

- `fades` in `render_world` (apps/d2d/world_view.cpp): see-through walls. Keyed by level
  pointer and wall; whether a wall fades is decided from ONE player's cell (the camera).
  On a shared screen a wall in front of the second hero would not fade. Needs every
  local hero's cell, as a parameter rather than a static.
- `eases` in `frame_light` (apps/d2d/town.cpp): radius easing per light, keyed by light.
  Harmless with one view; a second render path in the same process would share it.
- `stat_*` counters in `run_windowed`: frame timing statistics only, harmless.

REA also showed that `frame_light` is reached from two direct call sites (the devctl
`register_game_verbs` lambda and the town draw path at 0x336e76), so the light grid has
one entry point to give a list of heroes.
