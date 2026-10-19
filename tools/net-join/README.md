# net-join

Joins a game hosted by the real game.exe, stays a few seconds, leaves,
and writes every packet to a log.

## Run it

```sh
export D2_GAME_EXE=~/Workspace/private/diablo2/bin/game.exe
build/tools/net-join/net-join ~/Library/Preferences/d2d/save/Tux.d2s 192.168.50.68 --seconds 10
```

The host must be hosting a TCP/IP game. **Only use hosts on your own
network.**

## Why it exists

- The smallest test of d2d's network join: no window, no game, just
  "can we get in and out cleanly".
- The packet log shows exactly what the host sent, for working out
  packets d2d doesn't handle yet.
- It's the C++ twin of `tools/emu/join_live.py`.

## More uses

| You want | Command |
|---|---|
| Stay longer | `--seconds 60` |
| Log to a set file | `--log /tmp/join.log` |
| game.exe somewhere else | `--game-exe /path/to/game.exe` |

To play in the joined game instead, use d2d: `d2d --join 192.168.50.68`.

## When it breaks

| You see | Fix |
|---|---|
| Table check fails | game.exe must be 1.14d (`--game-exe`, or `D2_GAME_EXE`) |
| Refused by the host | The host is full, or the character's name or class doesn't fit the game (hardcore vs. softcore, expansion) |
| Times out | Is the host in a game? Is port 4000 allowed through its firewall? |
