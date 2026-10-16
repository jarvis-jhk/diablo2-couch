# SPDX-License-Identifier: GPL-3.0-or-later
"""A quest record's message table (record +0xdc; quests.md "The quest
record"): 0xc4-byte blocks of up to 16 {npc hcIdx, string, kind} and a u32
count, as game.exe holds it.

    uv run python quest_msgs.py 0x73eaa0 [blocks]

Prints each block as `block n: npc string kind ...` (kind 2 a topic, else a
greeting), stopping at the first block whose count isn't 1..16.
"""
import struct
import sys

import emu


def blocks(e, table, most=12):
    out = []
    for block in range(most):
        base = table + 0xc4 * block
        count = struct.unpack("<I", e.mu.mem_read(base + 0xc0, 4))[0]
        if not 0 < count <= 16:
            break
        out.append([struct.unpack("<3I", e.mu.mem_read(base + 12 * k, 12)) for k in range(count)])
    return out


def main():
    e = emu.Emu()
    table = int(sys.argv[1], 0)
    for n, entries in enumerate(blocks(e, table, int(sys.argv[2]) if len(sys.argv) > 2 else 12)):
        print(f"block {n}: " + "  ".join(f"{npc} {string} {kind}" for npc, string, kind in entries))
    return 0


if __name__ == "__main__":
    sys.exit(main())
