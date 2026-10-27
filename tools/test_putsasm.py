#!/usr/bin/env python3
"""Compares src/ql_text.asm (ql_puts, run with tools/m68k_mini.py) with its C version (text.c): draws consecutive cells while the
character is printable (32..127) and maxn is not reached; each glyph line r is pp ^ (fm[b] & dd), a long word at dst + 128 * r (the next
cell, 4 bytes further on); returns how many it drew. Random tables, strings, colours, maxn and addresses; it must not touch anything else in
memory nor change the stack. Usage: python3 tools/test_putsasm.py [cases]"""
import os, random, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import m68k_mini

root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
n = int(sys.argv[1]) if len(sys.argv) > 1 else 2000
src = open(os.path.join(root, 'src', 'ql_text.asm')).read()
rnd = random.Random(11)
SCR, FM, FONT, STR, STK = 0x20000, 0x10000, 0x12000, 0x1C000, 0x1F000
bad = steps = 0
for k in range(n):
    cpu = m68k_mini.CPU(src)
    cpu.a[7] = STK
    fm = [rnd.getrandbits(32) for _ in range(256)]
    fi = [[4 * rnd.randrange(256) for _ in range(8)] for _ in range(96)]
    pp, dd = rnd.getrandbits(32), rnd.getrandbits(32)
    kind = k % 4
    L = rnd.randrange(0, 33)
    s = [rnd.randrange(32, 128) for _ in range(L)]
    if kind == 1 and L: s[rnd.randrange(L)] = rnd.choice([0, 10, 13, 8, 31, 128, 200, 255])     # a character that cuts the strip
    if kind == 2: s = [32, 127, 32, 127][:L]                                                    # the ends of the range
    s += [0, 65, 66]                                                                              # end of string and whatever follows
    maxn = rnd.choice([0, 1, 2, 5, 32, 100, rnd.randrange(0, 40)])
    ncell = 32
    dst = SCR + 1024 * rnd.randrange(31) + 4 * rnd.randrange(32 - min(maxn, 32) if maxn < 32 else 1)
    for i, v in enumerate(fm): cpu.wr(FM + 4 * i, 4, v)
    for g in range(96):
        for r in range(8): cpu.wr(FONT + 16 * g + 2 * r, 2, fi[g][r])
    cpu.mem[STR:STR + len(s)] = bytes(s)
    cpu.mem[SCR:SCR + 32768] = bytes(rnd.randrange(256) for _ in range(32768))
    for v in (maxn, dd, pp, FM, FONT, STR, dst):         # arguments: dst, s, font, fm, pp, dd, maxn (the first one on top)
        cpu.a[7] -= 4; cpu.wr(cpu.a[7], 4, v)
    sp0 = cpu.a[7]
    before = bytes(cpu.mem)
    steps += cpu.run('_ql_puts')
    want = bytearray(before)
    cnt = 0
    while cnt < maxn:
        c = s[cnt] - 32
        if c < 0 or c > 95: break
        for r in range(8):
            w = (pp ^ (fm[fi[c][r] >> 2] & dd)).to_bytes(4, 'big')
            o = dst + 4 * cnt + 128 * r
            want[o:o + 4] = w
        cnt += 1
    after = bytes(cpu.mem)
    if after[:0x1EF00] != bytes(want[:0x1EF00]) or after[0x1F000:] != bytes(want[0x1F000:]) or cpu.a[7] != sp0 or cpu.d[0] != cnt:
        bad += 1
        if bad <= 5: print('DIFERENTE caso', k, hex(dst), 'maxn', maxn, 'cuenta asm', cpu.d[0], 'C', cnt)
print('%d casos (%d instrucciones), %d fallos' % (n, steps, bad))
sys.exit(1 if bad else 0)
