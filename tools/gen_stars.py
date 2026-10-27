#!/usr/bin/env python3
"""Generates tests/vectors_stars.txt: stardust of the ORIGINAL code in the 6502 emulator: STARS (views 0 to 3: STARS1,
STARS6, STARS2 with ST2), nWq and FLIP, with real PIXEL2 and PIXEL (on the hires screen; PIXEL logs X, A and ZZ on entry). Random inputs: NOSTM, SX..SZL, ALPHA,
ALP1, ALP2, BETA, BET1, BET2, DELT4 and RAND. Outputs: each PIXEL, the tables, RAND and ALPHA/ALP2/BET2 (ST2 restores them).
Checked with tests/test_stars (make test_stars). Usage: python3 tools/gen_stars.py [n] [seed]"""
import sys, random, re, math
sys.path.insert(0, 'tools')
from frag6502 import *

n = int(sys.argv[1]) if len(sys.argv) > 1 else 2000
random.seed(int(sys.argv[2]) if len(sys.argv) > 2 else 5)
def hx(b): return ''.join('%02x' % v for v in b)
maths = []
for i, l in enumerate(L):
    m = re.match(r'^;\s+Name: (\S+)', l)
    if not m: continue
    for j in range(i, min(i + 8, len(L))):
        cc = re.match(r'^;\s+Category: (.*)$', L[j])
        if cc:
            if cc.group(1).strip().startswith('Maths (Arithmetic)') and m.group(1) != 'PIX1':   # PIX1 falls into PIXEL2: goes with it
                bl = preprocess(L[i:nx(i + 1) - 1])
                if not any(re.match(r'^FOR\b', code(x)) for x in bl): maths += bl
            break
src = maths
for r in ('STARS', 'STARS1', 'STARS6', 'STARS2', 'nWq'): src += routine(r)
src += ['.WPSHPS', ' RTS']                         # nWq falls into WPSHPS (erase the ships from the radar): apart
for r in ('FLIP', 'PIX1', 'PIXEL2'): src += routine(r)
src += routine('PIXEL') + routine('TWOS3')             # real PIXEL (the carry it returns with matters), on the hires screen
for t in ('SCTBL', 'SCTBH', 'SCTBH2'): src += routine(t)
src += ['.SCTBX1'] + [' EQUB %d' % ((i + 8) % 7) for i in range(256)] + ['.SCTBX2'] + [' EQUB %d' % ((i + 8) // 7) for i in range(256)]
src = [re.sub(r'\s+AND\s+(?=\d)', ' & ', l) for l in src if code(l) != 'SKIP 0']
def equd(l):                                      # EQUD -> 4 EQUB
    m = re.match(r'^EQUD\s+([%$])([0-9A-Fa-f]+)$', code(l))
    if not m: return l
    v = int(m.group(2), 2 if m.group(1) == '%' else 16)
    return ' EQUB %d, %d, %d, %d' % (v & 255, v >> 8 & 255, v >> 16 & 255, v >> 24 & 255)
src = [equd(l) for l in src]
cpu, S = build(src, None, {'log': 0x9000, 'logL': 0x9100, 'alogh': 0x9200})
lg = [0x0803] + [int(0x2000 * math.log(i) / math.log(2) + 0.5) for i in range(1, 256)]
for i in range(256):
    cpu.mem[0x9000 + i] = (lg[i] >> 8) & 255; cpu.mem[0x9100 + i] = lg[i] & 255
    cpu.mem[0x9200 + i] = int(2 ** ((i / 2 + 128) / 16) + 0.5) >> 8 & 255
print('stubs:', sorted(k for k, v in S.items() if 0x3E00 <= v < 0x3F00), file=sys.stderr)
cpu.hooks[S['PIXEL']] = lambda c: c.log.append((c.x, c.a, c.mem[S['ZZ']]))
NOST = S['NOST']
TABS = ('SX', 'SY', 'SZ', 'SXL', 'SYL', 'SZL')
out = []
def B(): return random.choice([0, 1, 2, 4, 8, 16, 60, 64, 100, 110, 115, 116, 119, 120, 127, 128, 129, 144, 159, 160, 200, 255]) if random.random() < 0.3 else random.randrange(256)
for _ in range(n):
    entry = random.choice(['STARS'] * 6 + ['nWq', 'FLIP'])
    view = random.randrange(4)
    nostm = random.choice([NOST, NOST, 3, 1])
    tabs = {t: [B() for _ in range(NOST + 1)] for t in TABS}
    for k in range(NOST + 1):
        if random.random() < 0.7: tabs['SZ'][k] = random.randrange(8, 256)
    al = random.randrange(256) if random.random() < 0.7 else 0
    a1 = random.randrange(0, 32); a2 = random.choice([0, 0x80]); be = random.randrange(256) if random.random() < 0.7 else 0
    b1 = random.randrange(0, 32); b2 = random.choice([0, 0x80]); d4 = [random.randrange(256), random.randrange(0, 11)]
    rnd = [random.randrange(256) for _ in range(4)]
    for t in TABS: cpu.mem[S[t]:S[t] + NOST + 1] = bytes(tabs[t])
    vals = dict(NOSTM=nostm, VIEW=view, ALPHA=al, ALP1=a1, BETA=be, BET1=b1)
    for k, v in vals.items(): cpu.mem[S[k]] = v
    cpu.mem[S['ALP2']] = a2; cpu.mem[S['ALP2'] + 1] = a2 ^ 0x80
    cpu.mem[S['BET2']] = b2; cpu.mem[S['BET2'] + 1] = b2 ^ 0x80
    cpu.mem[S['DELT4']] = d4[0]; cpu.mem[S['DELT4'] + 1] = d4[1]
    cpu.mem[S['RAND']:S['RAND'] + 4] = bytes(rnd)
    c = random.randrange(2)
    cpu.log = []; cpu.c = c; cpu.run(S[entry])
    px = ' '.join('%d %d %d' % e for e in cpu.log)
    st = ' '.join(hx(cpu.mem[S[t]:S[t] + NOST + 1]) for t in TABS)
    out.append('%s %d %d %d %d %d %d %d %d %d %d %d %s %s : %s %s %02x%02x%02x%02x%02x%02x : %s' % (
        entry, view, nostm, al, a1, a2, be, b1, b2, d4[0], d4[1], c, hx(rnd), ' '.join(hx(tabs[t]) for t in TABS),
        st, hx(cpu.mem[S['RAND']:S['RAND'] + 4]), cpu.mem[S['ALPHA']], cpu.mem[S['ALP2']], cpu.mem[S['ALP2'] + 1],
        cpu.mem[S['BET2']], cpu.mem[S['BET2'] + 1], cpu.mem[S['BETA']], px))
open('tests/vectors_stars.txt', 'w').write('\n'.join(out) + '\n')
print(len(out), 'vectores')
