<p align="center"><img src="apps/icon/hero.svg" alt="" width="520"></p>

# Diablo 2 Couch

**Diablo II: Lord of Destruction for up to four players on one screen — one sofa,
one TV or Steam Deck, one controller and one hero each.**

Diablo II has never had couch co-op. Not the 2000 original, not Resurrected, not on
consoles. Diablo III showed how good it feels: everyone on the same screen, the camera
following the group. This project brings that to Diablo II, built on
[D2Decomp](https://github.com/psi29a/d2decomped), an open reimplementation of the
Diablo II engine that runs natively on Linux, Windows and macOS.

> ⚠️ **Early work in progress.** The first preview of couch co-op is out —
> [v0.0.1](../../releases/tag/v0.0.1): a second (third, fourth) player joins with a
> controller and their own hero. It is a prototype: see [Couch preview](#couch-preview)
> for what works and what doesn't yet.

**Download** from [Releases](../../releases): the Flatpak (`diablo2-couch-x86_64.flatpak`)
for Steam Deck and Linux, the zip for Windows.

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
| Act II (all areas and quests) | ✅ playable in D2Decomp |
| Act III (all areas and quests) | ✅ playable in D2Decomp |
| Acts IV–V | 🚧 being built upstream |
| Two to four players on one screen | 🧪 preview — guests join with a pad, shared camera |
| Controllers | 🧪 preview — stick walk, skills on buttons; menus still need the mouse |
| Per-player inventory, stash, merchants | 📋 next |
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

## Couch preview

Start a game as usual (mouse and keyboard: that hero is player 1). Then press any button
on a controller: a list of your other saved characters appears — pick one with the d-pad
and **A**, and that hero joins next to player 1. Up to three controllers can join.

| Controller | |
|---|---|
| Left stick | walk |
| A | attack the nearest monster (hold to keep attacking), or pick up / open / use what's next to you |
| X | right skill at the nearest monster |
| Y | next left skill · **LB / RB** previous / next right skill |
| D-pad | drink from belt column 1–4 |
| B | run / walk |
| A when dead | get up again, next to the party |
| Back (hold 2 s) | leave the game (your hero is saved) |

On a Steam Deck the first controller plays player 1 instead (`pad_player1 = 1` in
`d2d.cfg` does that anywhere, `0` turns it off).

How it plays: one camera follows the whole party and nobody can walk off the screen.
Player 1 leads — when they take a waypoint, portal, cave entrance or walk into the next
area, everyone comes along. The game no longer pauses under the Esc menu while guests
play. Each guest is saved when player 1 saves and leaves.

Not there yet: guests have no inventory, stash, merchant or skill-tree screens, can't
use waypoints themselves, and share player 1's town portal; a guest's missiles and spells
count their kills for player 1. Real controllers and the Deck are untested by us so far — please report what you find.

## Credits and licence

This is a **modified version of D2Decomp** by Bret Curtis and contributors — all of the
engine is their work; this fork adds local co-op, controller play and packaging on top
and follows upstream closely. Improvements that fit D2Decomp itself are offered back.

Licensed under the GNU General Public License, version 3 or later — see [LICENSE](LICENSE).
The app icon artwork is under its own licence, see [LEGAL.md](LEGAL.md).

Bugs and ideas: [open an issue](../../issues).
