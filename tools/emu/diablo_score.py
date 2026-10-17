"""Diablo's target score (FUN_005e8530) run on game.exe, its callees fed.

    uv run python diablo_score.py [cases] [seed] [--dump ../../tests/diablo_score_cases.inc]

Each case: random stats (FUN_00625480), state 0xb (FUN_00639df0), life %
(FUN_00621f20), left / right skills (FUN_00620190 / FUN_006201d0, ids from
Skills.txt, levels FUN_006442a0), reach (FUN_00622c40), a clear line
(FUN_005dd290) and the two point distances (FUN_005dc480). The skills'
attackrank is read off the loaded table (+0x18e). A line: the inputs as
rules::DiabloPick + life, reach, line, point bonus, then the score.
"""
import random
import sys

from unicorn.x86_const import UC_X86_REG_ECX

import drlg

STATS = (0x24, 0x25, 0x27, 0x29, 0x2b, 0x16, 0x31, 0x33, 0x35, 0x37, 0x3a)
SKILL = 0x100000                       # a fake skill pointer: SKILL + id


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    dump = sys.argv[sys.argv.index("--dump") + 1] if "--dump" in sys.argv else None
    if dump: args.remove(dump)
    cases, rng = int(args[0]) if args else 300, random.Random(int(args[1]) if len(args) > 1 else 1)
    e = drlg.boot()
    table = e.r32(e.r32(0x744304) + 0xb98)
    skills = e.r32(e.r32(0x744304) + 0xba0)
    diablo, player, word = e.alloc(0x100), e.alloc(0x100), e.alloc(0x20)   # player: unit type 0
    cur = {}
    e.hook(0x622c40, lambda e: cur["reach"], 3)
    e.hook(0x5dc480, lambda e: cur["him" if e.mu.reg_read(UC_X86_REG_ECX) == diablo else "it"], 1)
    e.hook(0x625480, lambda e: cur["stats"][e.arg(1)] & 0xffffffff, 3)
    e.hook(0x639df0, lambda e: cur["state"], 2)
    e.hook(0x621f20, lambda e: cur["life"], 1)
    e.hook(0x620190, lambda e: SKILL + cur["left"][0] if cur["left"][0] >= 0 else 0, 1)
    e.hook(0x6201d0, lambda e: SKILL + cur["right"][0] if cur["right"][0] >= 0 else 0, 1)
    e.hook(0x6442a0, lambda e: cur["left" if e.arg(1) == SKILL + cur["left"][0] else "right"][1], 3)
    e.hook(0x643ce0, lambda e: e.arg(0) - SKILL, 3)
    e.hook(0x5dd290, lambda e: cur["line"], 0)
    rank = lambda id: e.read(table + id * 0x23c + 0x18e, 1)[0] if id >= 0 else 0
    lines = []
    for _ in range(cases):
        hand = lambda: (rng.randrange(skills), rng.randint(0, 30)) if rng.random() < 0.85 else (-1, 0)
        cur.update(reach=rng.random() < 0.4, line=rng.random() < 0.5, state=rng.random() < 0.3, life=rng.randint(0, 100),
                   him=rng.choice((0, 0x50, 0x56, 0x90)), it=rng.choice((0, 0x30, 0x54, 0x55, 0x80)),
                   left=hand(), right=hand(),   # equal ids would read one level for both (the 6442a0 hook)
                   stats={s: rng.randint(-100, 300) for s in STATS})
        cur["stats"][0x3a] = rng.randint(0, 0x8000)
        while cur["right"][0] >= 0 and cur["right"][0] == cur["left"][0]: cur["right"] = hand()
        has_word = rng.random() < 0.7
        got = e.call(0x5e8530, player, word if has_word else 0, edx=diablo)
        st = cur["stats"]
        bonus = has_word and cur["him"] > 0x55 and cur["it"] < 0x55
        damage = st[0x16] + st[0x31] + st[0x33] + st[0x35] + st[0x37] + (st[0x3a] >> 8)
        (lid, llv), (rid, rlv) = cur["left"], cur["right"]
        lines.append(" ".join(str(int(v)) for v in (st[0x24], st[0x25], st[0x27], st[0x29], st[0x2b], damage,
                                                     lid, llv, rank(lid), rid, rlv, rank(rid), cur["state"],
                                                     cur["life"], cur["reach"], cur["line"], bonus, got)))
    if dump:
        with open(dump, "w") as out:
            out.write("// tools/emu/diablo_score.py --dump: game.exe's FUN_005e8530, a line each.\n")
            out.writelines(f'"{line}",\n' for line in lines)
    else:
        print("\n".join(lines))


if __name__ == "__main__":
    main()
