# SPDX-License-Identifier: GPL-3.0-or-later
"""Spawn area ids (FUN_0061b130) against d2d's room_areas.

    uv run python area_ids.py <levels> <level> <first>-<last>

One game per seed as `diff_drlg.py game` makes it (objgroups.game_dump:
every level in <levels> in turn, its rooms up in drlg.walk_order, then
populated). As each room of <level> populates, game.exe's own FUN_0061b130
(room1, x, y) is called at every tile's middle subtile; d2d's id there is
the one of its room_areas rect holding the tile (drlg-dump game,
$AREA_IDS). Ids carry across rooms (FUN_0066d040 / FUN_0066c770), so this
checks the labels, the merges and the lookup together.
"""
import os
import re
import subprocess
import sys

import drlg
import emu
import objgroups


def ours(levels, lid, seed):
    env = dict(os.environ, ORDER="shuffle", LEVELS=",".join(map(str, levels)), AREA_IDS=str(lid))
    env.setdefault("D2_PATCH_INSTALLER", os.path.expanduser("~/Downloads/Diablo II + LoD/patch/LODPatch_114d.exe"))
    err = subprocess.run([str(emu.BUILD / "tools/drlg-dump/drlg-dump"), emu.data_path(), f"{seed}-{seed}", str(lid), "game"],
                         env=env, capture_output=True, text=True).stderr
    rooms = {}
    for line in err.splitlines():
        if m := re.match(r"areas (-?\d+),(-?\d+):(.*)", line):
            rooms[(int(m[1]), int(m[2]))] = [tuple(map(int, re.split("[,=]", a))) for a in m[3].split()]
    return rooms


def game(e, levels, lid, seed):
    """{(room x, y): {(tile x, y): id}}, level-relative, as each room of `lid` populates."""
    src = open(objgroups.__file__).read()
    hook = "                e.call(0x52d0f0, ecx=g, edx=room1)\n"
    probe = ("                if lid == LID:\n"
             "                    rx, ry, rw, rh = (e.s32(r + k) for k in (0x34, 0x38, 0x3c, 0x40))\n"
             "                    OUT[(rx - x0, ry - y0)] = {(tx - x0, ty - y0): e.call(0x61b130, room1, tx * 5 + 2, ty * 5 + 2)\n"
             "                                               for ty in range(ry, ry + rh) for tx in range(rx, rx + rw)}\n")
    assert hook in src
    out = {}
    ns = {"__name__": "objgroups_area_ids", "LID": lid, "OUT": out}
    exec(compile(src.replace(hook, hook + probe, 1), objgroups.__file__, "exec"), ns)
    ns["game_dump"](e, seed, levels, "shuffle")
    return out


def main():
    levels = [int(v) for v in sys.argv[1].split(",")]
    lid = int(sys.argv[2])
    first, last = (int(v) for v in sys.argv[3].split("-"))
    e = drlg.boot()
    bad = 0
    for seed in range(first, last + 1):
        if e.brk - emu.HEAP > emu.HEAP_SIZE * 7 // 10: e = drlg.boot(fresh=True)
        theirs, mine = game(e, levels, lid, seed), ours(levels, lid, seed)
        tiles = same = 0
        for room, cells in theirs.items():
            rects = mine.get(room, [])
            for (tx, ty), want in cells.items():
                got = next((i for l, t, r, b, i in rects if l <= tx < r and t <= ty < b), 0xffffffff)
                tiles += 1
                same += got == (want & 0xffffffff)
                if got != (want & 0xffffffff) and tiles - same <= 3: print(f"seed {seed} room {room} tile {tx},{ty}: game {want} ours {got}")
        bad += tiles != same
        print(f"seed {seed}: {same}/{tiles} tiles, {len(theirs)} rooms")
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
