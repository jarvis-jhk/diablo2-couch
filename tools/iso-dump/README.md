# iso-dump

Lists or extracts the files on a Diablo II disc image (.iso).

## Run it

```sh
build/tools/iso-dump/iso-dump list ~/Downloads/d2-install.iso
```

## Why it exists

- The launcher installs the game straight from the discs' ISOs; this is
  the same reader (components/iso9660) as a command, to check an ISO or
  pull one file out.

## More uses

| You want | Command |
|---|---|
| Everything on the disc, in a folder | `iso-dump extract disc.iso /tmp/disc` |
| Just the MPQs in the list | `iso-dump list disc.iso \| grep -i mpq` |

## When it breaks

| You see | Fix |
|---|---|
| `cannot open ...` | Wrong path, or not an ISO 9660 image (some rips are .bin/.cue: convert to .iso first) |
| `usage: ...` | `list <iso>` or `extract <iso> <dest-dir>` |
