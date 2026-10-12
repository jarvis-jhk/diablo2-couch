<p align="center"><img src="apps/icon/hero.svg" alt="" width="520"></p>

# Diablo 2 Couch

**Diablo II: Lord of Destruction for up to four players on one screen — one sofa,
one TV or Steam Deck, one controller and one hero each.**

Diablo II has never had couch co-op. Not the 2000 original, not Resurrected, not on
consoles. Diablo III showed how good it feels: everyone on the same screen, the camera
following the group. This project brings that to Diablo II, built on
[D2Decomp](https://github.com/psi29a/d2decomped), an open reimplementation of the
Diablo II engine that runs natively on Linux, Windows and macOS.

> ⚠️ **Early work in progress. There is no release yet.** Today this repository is
> D2Decomp plus our plans. Couch co-op is being built; this page will say when you can play it.

## What it will be

- 🎮 **One to four players, one screen.** Each with their own controller and their own
  character and save. One shared camera that keeps the party together.
- 🎒 **Nobody waits.** Inventory, stash, merchants and the character sheet belong to one
  player and never pause the game or steal anyone else's controls.
- 🕹️ **Made for controllers.** Move with the stick, skills on buttons, aim assist —
  no mouse emulation.
- 🐧 **Linux, Windows and Steam Deck**, with an installer, safe automatic updates and a
  "what's new" screen after each update.

## Where it stands

| | |
|---|---|
| Act I (all areas and quests) | ✅ playable in D2Decomp |
| Acts II–V | 🚧 being built upstream (Act II has started) |
| Two players on one screen | 📋 next — first prototype in Act I |
| Controllers | 📋 planned |
| Four players, Deck packaging | 📋 planned |

The plan is in [ROADMAP.md](ROADMAP.md), the running log in [NOTES.md](NOTES.md).
New acts arrive here as D2Decomp builds them.

## Bring your own game

You need your own copy of **Diablo II + Lord of Destruction** (the classic game, not
Resurrected). This project contains no Blizzard files: it reads the game data from your
installation. It is not affiliated with or endorsed by Blizzard Entertainment.
See [LEGAL.md](LEGAL.md).

## Build

Same as D2Decomp: a C++26 compiler, CMake 3.28+, Ninja, SDL3, OpenAL, FFmpeg, zlib,
StormLib, and Qt 6.5+ for the launcher.

```sh
cmake -S . -B build -G Ninja
cmake --build build
build/apps/d2d/d2d --data /path/to/your/MPQ-folder
```

More in the [D2Decomp README](docs/D2DECOMP-README.md).

## Credits and licence

This is a **modified version of D2Decomp** by Bret Curtis and contributors — all of the
engine is their work; this fork adds local co-op, controller play and packaging on top
and follows upstream closely. Improvements that fit D2Decomp itself are offered back.

Licensed under the GNU General Public License, version 3 or later — see [LICENSE](LICENSE).
The app icon artwork is under its own licence, see [LEGAL.md](LEGAL.md).

Bugs and ideas: [open an issue](../../issues).
