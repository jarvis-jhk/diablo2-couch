# SPDX-License-Identifier: GPL-3.0-or-later
"""game.exe's hire offer (FUN_006637f0) as the oracle.

    uv run python hire.py <first seed>-<last seed>

For each entry seed, acts 1 to 3 (0..2), difficulties 0..2 and character
levels 1, 9, 30, 75 (FUN_00625480's stat 12, hooked): the offer game.exe
rolls on {seed, 666} for an LoD game, one line `seed act diff clvl: id
level life str dex cost exp def dmg_min dmg_max`, as `drlg-dump ... hire`
prints rules::merc_offer's.
"""
import sys

import drlg
from unicorn.x86_const import UC_X86_REG_ESP

CLVLS = (1, 9, 30, 75)


def dump(e, seed):
    out, clvl = [], [1]
    saved = bytes(e.mu.mem_read(0x625480, 5))
    e.hook(0x625480, lambda _e: clvl[0], 1)
    try:
        offer = e.alloc(0x40)
        for act in (0, 1, 2):
            for diff in (0, 1, 2):
                for level in CLVLS:
                    clvl[0] = level
                    e.mu.mem_write(offer, bytes(0x40))
                    if not e.call(0x6637f0, seed, act, diff, offer, ecx=1, edx=0): out.append(f"{seed:08x} {act} {diff} {level}: none"); continue
                    f = [e.s32(offer + 4 * k) for k in range(10)]
                    out.append(f"{seed:08x} {act} {diff} {level}: " + " ".join(str(v) for v in f))
    finally:
        e.mu.mem_write(0x625480, saved)
        e.mu.ctl_remove_cache(0x625480, 0x625485)
    return "\n".join(out)


if __name__ == "__main__":
    first, last = (int(v, 0) for v in sys.argv[1].split("-"))
    e = drlg.boot()
    print("\n".join(dump(e, s * 0x9e3779b1 & 0xffffffff) for s in range(first, last + 1)))


def entries_dump(e, game_seed, names):
    """FUN_00576070 on a bare game (its seed at game +0x1d24 +8) and an NPC record, FUN_00575ff0 hooked to
    give names 0..names-1: `name seed offered` per entry and the game seed after."""
    g, rec, ptr = e.alloc(0x2000), e.alloc(0x40), e.alloc(0x20)
    e.w32(g + 0x1d24, ptr)
    e.w32(ptr + 8, game_seed); e.w32(ptr + 12, 666)
    def names_of(_e):
        sp = e.mu.reg_read(UC_X86_REG_ESP)
        e.w32(e.r32(sp + 4), 0); e.w32(e.r32(sp + 8), names - 1)
        return names
    saved = bytes(e.mu.mem_read(0x575ff0, 5))
    e.hook(0x575ff0, names_of, 3)
    try:
        e.call(0x576070, rec, ecx=g, edx=0)
    finally:
        e.mu.mem_write(0x575ff0, saved)
        e.mu.ctl_remove_cache(0x575ff0, 0x575ff5)
    table = e.r32(rec + 0x10)
    lines = [f"{e.r32(table + 16 * k) & 0xffff} {e.r32(table + 16 * k + 4):08x} {e.r32(table + 16 * k + 12)}" for k in range(names)]
    return "\n".join(lines + [f"game {e.r32(ptr + 8):08x}"])
