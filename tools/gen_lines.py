#!/usr/bin/env python3
"""Generates tests/vectors_lines.txt: LOIN, HLOIN, VLOIN (with CPIX) and PIXEL of the ORIGINAL code run in the 6502 emulator on the Apple II
high-resolution screen ($2000-$3FFF), and the dots left lit converted to game coordinates
(game x = hires column - 8; row = y). HLOIN and VLOIN go in white (COL = 12), which lights all the bits: this shows
which dots the line covers (the Apple colours are bit patterns the QL does not have). Checked with tests/test_lines
(make test_lines). Usage: python3 tools/gen_lines.py [n] [seed]"""
import sys, random, re, math
sys.path.insert(0, 'tools')
from frag6502 import *

n = int(sys.argv[1]) if len(sys.argv) > 1 else 2000
random.seed(int(sys.argv[2]) if len(sys.argv) > 2 else 77)

def fix(lines):                                   # EQUD %binary or $hex -> 4 EQUB (little endian)
    out = []
    for l in lines:
        m = re.match(r'^EQUD\s+([%$])([0-9A-Fa-f]+)$', code(l))
        if m:
            v = int(m.group(2), 2 if m.group(1) == '%' else 16)
            out.append(' EQUB %d, %d, %d, %d' % (v & 255, v >> 8 & 255, v >> 16 & 255, v >> 24 & 255))
        else: out.append(l)
    return out

src = rng(nl('LOIN') - 1, nx(nl('VLOIN')) - 1) + routine('CPIX') + routine('PIXEL') + routine('TWOS3') + routine('cleargrap') + routine('clearrow') + [' RTS']   # clearrow ends with PLA; TAY and falls into an RTS
for t in ('TWOS', 'TWOS2', 'TWFL', 'TWFR', 'SCTBL', 'SCTBH', 'SCTBH2'): src += routine(t)
src = fix([l for l in src if code(l) != 'SKIP 0'])
src += ['.SCTBX1'] + [' EQUB %d' % ((i + 8) % 7) for i in range(256)]
src += ['.SCTBX2'] + [' EQUB %d' % ((i + 8) // 7) for i in range(256)]
cpu, S = build(src, None, {'log': 0x9000, 'logL': 0x9100, 'alogh': 0x9200})
stubs = sorted(k for k, v in S.items() if 0x3E00 <= v < 0x3F00)
print('stubs:', stubs, file=sys.stderr)
assert S['SCTBX2'] + 256 < 0x9000
lg = [0x0803] + [int(0x2000 * math.log(i) / math.log(2) + 0.5) for i in range(1, 256)]
for i in range(256):
    cpu.mem[0x9000 + i] = (lg[i] >> 8) & 255; cpu.mem[0x9100 + i] = lg[i] & 255
    cpu.mem[0x9200 + i] = int(2 ** ((i / 2 + 128) / 16) + 0.5) >> 8 & 255

sctbl = cpu.mem[S['SCTBL']:S['SCTBL'] + 24]; sctbh = cpu.mem[S['SCTBH']:S['SCTBH'] + 24]
def rowaddr(y): return sctbl[y >> 3] + 256 * sctbh[y >> 3] + 0x400 * (y & 7)

def pixels():
    """lit dots (bits 0..6) of the hires screen, in game coordinates"""
    out = []
    for y in range(192):
        b = rowaddr(y)
        for col in range(40):
            v = cpu.mem[b + col] & 0x7F
            if not v: continue
            for bit in range(7):
                if v >> bit & 1:
                    gx = col * 7 + bit - 8
                    if 0 <= gx < 256: out.append((gx, y))       # the QL is 256 dots wide (PIXEL with x = 254, 255 goes off)
    return sorted(out)

def clear():
    for a in range(0x2000, 0x4000): cpu.mem[a] = 0

def call(entry, **zp):
    for k, v in zp.items(): cpu.mem[S[k]] = v
    cpu.y = 0x5A
    cpu.run(S[entry])
    assert cpu.y == 0x5A, entry + ': Y no se conserva'

def X(): return random.choice([0, 1, 2, 5, 6, 7, 127, 128, 200, 254, 255]) if random.random() < 0.15 else random.randrange(256)
def Y(): return random.choice([0, 1, 7, 8, 9, 95, 96, 190, 191]) if random.random() < 0.15 else random.randrange(192)

out = []
for _ in range(n):
    clear()
    k = random.choice([1, 1, 1, 2, 3])
    segs = []
    for _ in range(k):
        x1, y1 = X(), Y()
        r = random.random()
        if r < 0.1: x2, y2 = x1, Y()                       # vertical
        elif r < 0.2: x2, y2 = X(), y1                     # horizontal
        elif r < 0.27: x2, y2 = x1, y1                     # a dot
        elif r < 0.35:                                     # 45 degrees
            d = random.randrange(-60, 61); x2, y2 = x1 + d, y1 + random.choice([-1, 1]) * d
            if not (0 <= x2 < 256 and 0 <= y2 < 192): x2, y2 = x1, y1
        elif r < 0.6:                                      # short
            x2 = min(255, max(0, x1 + random.randrange(-12, 13))); y2 = min(191, max(0, y1 + random.randrange(-12, 13)))
        else: x2, y2 = X(), Y()
        segs.append((x1, y1, x2, y2))
        call('LOIN', X1=x1, Y1=y1, X2=x2, Y2=y2)
    out.append('L %s : %s' % (' '.join('%d %d %d %d' % s for s in segs), ' '.join('%02x%02x' % p for p in pixels())))

    clear()
    segs = []
    for _ in range(random.choice([1, 1, 2])):
        x1, x2, y = X(), X(), Y()
        segs.append((x1, x2, y))
        call('HLOIN', X1=x1, X2=x2, Y1=y, COL=12)
    out.append('H %s : %s' % (' '.join('%d %d %d' % s for s in segs), ' '.join('%02x%02x' % p for p in pixels())))

    clear()
    segs = []
    for _ in range(random.choice([1, 1, 2])):
        x, y1, y2 = random.randrange(255), Y(), Y()
        segs.append((x, y1, y2))
        call('VLOIN', X1=x, Y1=y1, Y2=y2, COL=12)
    out.append('V %s : %s' % (' '.join('%d %d %d' % s for s in segs), ' '.join('%02x%02x' % p for p in pixels())))

    clear()
    segs = []
    for _ in range(random.choice([1, 1, 2, 3])):
        zz = random.choice([0, 79, 80, 127, 128, 255, random.randrange(256)])
        x = X(); y = random.randrange(1 if zz < 80 else 0, 192)
        segs.append((x, y, zz))
        cpu.mem[S['ZZ']] = zz; cpu.x = x; cpu.a = y; cpu.y = 0x5A
        cpu.run(S['PIXEL'])
        assert cpu.y == 0x5A
    out.append('P %s : %s' % (' '.join('%d %d %d' % s for s in segs), ' '.join('%02x%02x' % p for p in pixels())))

for kind in ('G', 'R'):                               # cleargrap and clearrow 15 + 16 (CLY1) on the full screen
    for a in range(0x2000, 0x4000): cpu.mem[a] = 0x7F
    if kind == 'G': cpu.run(S['cleargrap'])
    else:
        cpu.y = 15; cpu.run(S['clearrow']); cpu.y = 16; cpu.run(S['clearrow'])
    on = set(pixels())
    off = [(x, y) for y in range(192) for x in range(256) if (x, y) not in on]
    out.append('%s : %s' % (kind, ' '.join('%02x%02x' % p for p in off)))

open('tests/vectors_lines.txt', 'w').write('\n'.join(out) + '\n')
print(len(out), 'vectores')
