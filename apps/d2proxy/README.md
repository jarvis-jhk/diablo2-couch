# d2proxy

Sits between a game client and a game.exe host, passes every byte
through unchanged, and logs every packet both ways, decoded.

## Run it

```sh
export D2_GAME_EXE=~/Workspace/private/diablo2/bin/game.exe
build/apps/d2proxy/d2proxy --host 192.168.50.68
```

Then point the client (a real game.exe, or d2d) at **this** machine's
address instead of the host's: TCP/IP Game → Join → your Mac's IP.
**Only between machines on your own network.**

## Why it exists

- To learn what a real game.exe client sends (NPC trade, waypoints,
  ...) so d2d can send the same. Most of d2d's network packets were
  pinned down this way.
- To see exactly where d2d and a real client differ, by recording both.

## More uses

| You want | Command |
|---|---|
| Every packet's name and size | `--level debug` |
| Every packet's bytes | `--level trace` |
| Logs in one folder | `--log-dir /tmp/proxy` (one file per session) |
| One log for all sessions | `--log /tmp/proxy.log` |
| Clients from another subnet | `--allow 192.168.50.0/24,10.0.0.0/8` |
| Another port | `--port 4001` (the host's), `--listen 4010` (the clients'); both default to 4000 |
| Drop an idle client sooner | `--idle 120` (seconds) |

Logs use d2d's net-log format, the same as `net-join` and d2d's own
`net-*.log`.

## When it breaks

| You see | Fix |
|---|---|
| Table check fails at start | game.exe must be 1.14d (`--game-exe` or `D2_GAME_EXE`) |
| Client can't connect | Is it reaching the proxy's machine? Allow the `--listen` port (default 4000) through its firewall |
| Client refused right away | Its address isn't private or loopback: add `--allow` |
| `warning ... unknown` lines | Not an error: a packet nobody has decoded yet. Its bytes are logged for you |
| Second client refused | One client at a time, by design |
