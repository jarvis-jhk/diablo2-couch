# drop-dump

Prints the items d2d drops for a given seed, as text, so they can be
diffed against game.exe's drops.

## Run it

You don't run it by hand. `drops.py` in [tools/emu](../emu/README.md)
feeds it jobs and diffs the result against game.exe:

```sh
cd tools/emu
uv run python drops.py 1-100000        # drop rolls for seeds 1..100000
```

Good: `100000/100000 rolls match game.exe`. Anything less lists the
mismatching seeds.

## Why it exists

- Drops have to match game.exe bit for bit: the same seed must give the
  same items, the same quality and the same next seed.
- One wrong roll shifts every roll after it, so testing many seeds
  finds bugs fast.

## More uses (all through drops.py)

| You want | Command |
|---|---|
| Only game.exe's lines | `uv run python drops.py 1-20000 print` |
| Each treasure class's entries | `uv run python drops.py tables` |
| Made items (quality, sockets, stats) | `uv run python drops.py items 1-20000` |
| Chests and other containers | `uv run python drops.py objects 1-20000` |
| Armor stands and weapon racks | `uv run python drops.py stands 1-20000` |

By hand: `build/tools/drop-dump/drop-dump <mpq dir> [tables|items|objects|stands] < jobs.txt`.
The job and output formats are in the comment at the top of `drop-dump.cpp`.

## When it breaks

| You see | Fix |
|---|---|
| A seed mismatches | The line shows both sides; compare item by item, then read `docs/research/re/drops.md` |
| `usage: drop-dump <mpq dir> ...` | It needs the MPQ folder, and jobs on stdin |
