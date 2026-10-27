#!/usr/bin/env python3
"""Generates tests/vectors_equip.txt by running the ORIGINAL code for the following in the 6502 emulator:
   B  BOMBON + BOMBOFF (real DORND; LOIN and CLICK log)
   W  WARP (real MAS2, ADD; BOOP and LOOK1 log)
   D  DOCKIT (with real VCSU1, TAS1-4, TAS6, TA2/NORM, SPS1, DCS1/TAS7, TA151/TA15..TA9, GOPL, MVT3, MAD...)
   K  DOKEY with the docking computer (real ZINF and DOCKIT; from LDA auto to DK15)
   G  GTHG (real Ze, DORND; NWSHP logs INWK and the type)
   F  FRS1 (NWSHP logs)
   R  FRMIS and FR1 (real FRS1, GINF, ANGRY and ABORT; NWSHP, MESS, MSBAR and SOHISS log)
Requires ELITE_SRC. Usage: python3 tools/gen_equip.py [n] [seed]"""
import sys, random
sys.path.insert(0, 'tools')
from frag6502 import *

n = int(sys.argv[1]) if len(sys.argv) > 1 else 3000
random.seed(int(sys.argv[2]) if len(sys.argv) > 2 else 1985)
hx = lambda b: ''.join('%02x' % x for x in b)
def rb(): return random.randrange(256)
def pick(*v): return random.choice(v)
out = []

def noskip(lines):                       # SKIP n -> n zero bytes (the assembler does not know SKIP)
    o = []
    for l in lines:
        m = re.match(r'^SKIP\s+(\d+)$', code(l))
        o.append(' EQUB ' + ','.join(['0'] * int(m.group(1))) if m else l)
    return o

# ---------------------------------------------------------------- B: bomb
bombl = noskip(rng(nl('BOMBOFF') - 1, nx(nl('BOMBTBY')) - 1)) + dornd
cpu, S = build(bombl, None)
lines = []
cpu.hooks[S['LOIN']] = lambda c: lines.append('%d,%d,%d,%d' % tuple(c.mem[S[k]] for k in ('X1', 'Y1', 'X2', 'Y2')))
for _ in range(n):
    rnd = [rb() for _ in range(4)]; c = rb() & 1
    cpu.mem[S['RAND']:S['RAND'] + 4] = bytes(rnd); cpu.mem[S['QQ11']] = 0
    del lines[:]
    cpu.c = c
    cpu.run(S['BOMBON'])
    out.append('B %s %d %s %s %s %s' % (hx(rnd), c, hx(cpu.mem[S['BOMBTBX']:S['BOMBTBX'] + 10]),
               hx(cpu.mem[S['BOMBTBY']:S['BOMBTBY'] + 10]), hx(cpu.mem[S['RAND']:S['RAND'] + 4]), ' '.join(lines)))

# ---------------------------------------------------------------- W: WARP
math_ = rng(nl('MULT1') - 1, nx(nl('ADD')) - 1)
cpu, S = build(routine('WARP') + routine('MAS2') + math_ + sq, None)
NI = S['NI%']
for _ in range(n):
    junk, f2, sspr, mj = pick(0, 0, 1, 2), pick(0, 0, 0, 5), pick(0, 0, 0, 1), pick(0, 0, 0, 0xFF)
    pl = [rb() for _ in range(9)]; su = [rb() for _ in range(9)]
    for s_ in (pl, su):
        if random.random() < 0.7:
            for k in (2, 5): s_[k] &= 0x81
            s_[8] = pick(0x00, 0x01, 0x02, 0x03, 0x80, 0x81, rb())
    cpu.mem[S['K%']:S['K%'] + 9] = bytes(pl); cpu.mem[S['K%'] + NI:S['K%'] + NI + 9] = bytes(su)
    for i in range(10): cpu.mem[S['FRIN'] + i] = 0
    cpu.mem[S['FRIN'] + 2 + junk] = f2; cpu.mem[S['JUNK']] = junk
    cpu.mem[S['MANY'] + S['SST']] = sspr; cpu.mem[S['MJ']] = mj
    cpu.mem[S['QQ11']] = 0; cpu.mem[S['MCNT']] = 77; cpu.log = []
    cpu.run(S['WARP'])
    did = int(any(x[0] == 'LOOK1' for x in cpu.log))
    out.append('W %d %d %d %d %s %s %d %d %d %d %d' % (junk, f2, sspr, mj, hx(pl), hx(su), cpu.mem[S['K%'] + 8],
               cpu.mem[S['K%'] + NI + 8], did, cpu.mem[S['QQ11']], cpu.mem[S['MCNT']]))

# ---------------------------------------------------------------- D and K: DOCKIT and DOKEY
dock = rng(nl('TACTICS', 6) - 1, nx(nl('DCS1')) - 1) + ['.GOPL', ' JSR SPS1', ' JMP TA151'] + \
       rng(nl('SPS1') - 1, nx(nl('NORM')) - 1) + routine('MVT3') + math_ + sq + routine('TIS2') + routine('ZINF') + \
       rng(nl('LL28') - 1, nx(nl('LL28')) - 1) + routine('SPS3') + rng(nl('LL5') - 1, nx(nl('LL5')) - 1) + dornd
i0 = [k for k in range(nl('DOKEY'), len(L)) if code(L[k]) == 'LDA auto'][0]
dokey = ['.DKAUTO'] + rng(i0, lab('DK15')) + ['.DK15', 'RTS']
cpu, S = build(dokey + dock, None, {'log': 0x2000, 'logL': 0x2100, 'alogh': 0x2200})
import math
lg = [0x0803] + [int(0x2000 * math.log(i) / math.log(2) + 0.5) for i in range(1, 256)]
for i in range(256):
    cpu.mem[0x2000 + i] = (lg[i] >> 8) & 255; cpu.mem[0x2100 + i] = lg[i] & 255
    cpu.mem[0x2200 + i] = int(2 ** ((i / 2 + 128) / 16) + 0.5) >> 8 & 255
print('stubs:', sorted(k for k, v in S.items() if 0x3E00 <= v < 0x3F00), file=sys.stderr)

def coord(r):
    v = random.randrange(-r, r + 1); m = abs(v)
    return [m & 255, (m >> 8) & 255, ((m >> 16) & 0x7F) | (0x80 if v < 0 else 0)]
def vec():                               # orientation vector: 3 sign-magnitude 16-bit components (96 << 8 = 1)
    o = []
    for _ in range(3):
        v = random.choice([random.randrange(-0x6000, 0x6001), 0x6000, -0x6000, 0, random.randrange(-0x1000, 0x1000)])
        m = abs(v); o += [m & 255, ((m >> 8) & 0x7F) | (0x80 if v < 0 else 0)]
    return o
def rand_scene():
    st = coord(pick(0x300, 0x1000, 0x8000)) + coord(pick(0x300, 0x1000, 0x8000)) + coord(pick(0x300, 0x1000, 0x8000)) + vec() + vec() + vec()
    st += [0] * (NI - len(st))
    pl = coord(0x40000) + coord(0x40000) + coord(0x40000)
    return st, pl

for _ in range(n):
    st, pl = rand_scene()
    iw = coord(pick(0x200, 0x800, 0x3000)) + coord(pick(0x200, 0x800, 0x3000)) + coord(pick(0x200, 0x800, 0x3000)) + vec() + vec() + vec()
    iw += [rb() for _ in range(NI - len(iw))]
    typ = pick(9, 10, 11, 16, 0xE0, 1)
    sspr = pick(1, 1, 1, 0); k10 = pick(0, 0, 1)
    cpu.mem[S['INWK']:S['INWK'] + NI] = bytes(iw); cpu.mem[S['K%'] + NI:S['K%'] + 2 * NI] = bytes(st)
    cpu.mem[S['K%']:S['K%'] + 9] = bytes(pl); cpu.mem[S['MANY'] + S['SST']] = sspr; cpu.mem[S['TYPE']] = typ
    cpu.mem[S['K3'] + 10] = k10; cpu.log = []
    cpu.run(S['DOCKIT'])
    out.append('D %s %s %s %d %d %d %s %d %d %d' % (hx(iw), hx(st), hx(pl), typ, sspr, k10, hx(cpu.mem[S['INWK']:S['INWK'] + NI]),
               cpu.mem[S['RAT']], cpu.mem[S['RAT2']], cpu.mem[S['CNT']]))

KY = ['KY1', 'KY2', 'KY3', 'KY4', 'KY5', 'KY6']
for _ in range(n):
    st, pl = rand_scene()
    delta, jx, jy = pick(1, 3, 12, 22, 30, 40), rb(), rb()
    keys = [pick(0, 0, 0, 0xFF) for _ in KY]
    k10 = pick(0, 0, 1)
    cpu.mem[S['K%'] + NI:S['K%'] + 2 * NI] = bytes(st); cpu.mem[S['K%']:S['K%'] + 9] = bytes(pl)
    cpu.mem[S['MANY'] + S['SST']] = 1; cpu.mem[S['auto']] = 0xFF; cpu.mem[S['K3'] + 10] = k10
    cpu.mem[S['DELTA']] = delta; cpu.mem[S['JSTX']] = jx; cpu.mem[S['JSTY']] = jy
    for k, v in zip(KY, keys): cpu.mem[S[k]] = v
    cpu.run(S['DKAUTO'])
    out.append('K %s %s %d %d %d %d %s %d %d %d %s' % (hx(st), hx(pl), delta, jx, jy, k10, ''.join('%02x' % v for v in keys),
               cpu.mem[S['DELTA']], cpu.mem[S['JSTX']], cpu.mem[S['JSTY']], ''.join('%02x' % cpu.mem[S[k]] for k in KY)))

# ---------------------------------------------------------------- G and F: GTHG and FRS1
cpu, S = build(routine('GTHG') + routine('Ze') + dornd + routine('ZINF') + rng(lab('FRS1'), lab('FRMIS')), None)
got = []
cpu.hooks[S['NWSHP']] = lambda c: got.append((c.a, bytes(c.mem[S['INWK']:S['INWK'] + NI])))
for _ in range(n):
    rnd = [rb() for _ in range(4)]; c = rb() & 1
    cpu.mem[S['RAND']:S['RAND'] + 4] = bytes(rnd); del got[:]
    cpu.c = c
    cpu.run(S['GTHG'])
    out.append('G %s %d %d %s %d %s %s' % (hx(rnd), c, got[0][0], hx(got[0][1]), got[1][0], hx(got[1][1]), hx(cpu.mem[S['RAND']:S['RAND'] + 4])))
for _ in range(n // 4):
    typ, mstg, delta = pick(11, 24, 1), pick(0xFF, 0, 3, rb()), pick(0, 3, 12, 40)
    cpu.mem[S['MSTG']] = mstg; cpu.mem[S['DELTA']] = delta; del got[:]
    cpu.x = typ
    cpu.run(S['FRS1'])
    out.append('F %d %d %d %d %s' % (typ, mstg, delta, got[0][0], hx(got[0][1])))

# ---------------------------------------------------------------- R: FRMIS (with FR1, ANGRY and ABORT)
frmc = rng(lab('FRMIS'), lab('FR1')) + routine('FR1') + routine('ABORT') + routine('ABORT2') + routine('GINF') + ['.UNIV'] + [' EQUW K%% + %d' % (i * 37) for i in range(9)]
cpu, S = build(routine('Ze') + dornd + routine('ZINF') + rng(lab('FRS1'), lab('FRMIS')) + frmc, None)
NI = S['NI%']
evl = []
def nw_ok(c):
    evl.append('N%d' % c.a); c.c = 1 if st_ok[0] else 0
st_ok = [1]
cpu.hooks[S['NWSHP']] = nw_ok
cpu.hooks[S['MESS']] = lambda c: evl.append('M%d' % c.a)
# MSBAR with colour BLACK (0): MSBAR2 exits through coolkey with A = 0 (mscol = 0) and Y = 0; FRMIS then falls into ANGRY with that A
cpu.hooks[S['MSBAR']] = lambda c: (evl.append('B%d.%d' % (c.x, c.y)), setattr(c, 'y', 0), setattr(c, 'a', 0))
cpu.hooks[S['SOHISS']] = lambda c: evl.append('S%d' % c.y)
kb = S['K%']
for _ in range(n // 2):
    st_ok[0] = pick(1, 1, 1, 0)
    nomsl, delta, mstg, typ = pick(1, 2, 3, 4), pick(0, 3, 12, 40), pick(1, 2, 3, 4, 5), pick(0, 0, 1)
    ttype = pick(1, 2, 7, 10, 11, 16, 17, 19, 20, 25, 31, rb() % 33)
    msar = pick(0, 0xFF)
    for k in range(37 * 9): cpu.mem[kb + k] = 0
    fr = S['FRIN']
    for k in range(S['NOSH'] + 1): cpu.mem[fr + k] = 0
    cpu.mem[fr] = 128; cpu.mem[fr + 1] = 130; cpu.mem[fr + 2] = pick(2, 2, 1)
    for k in (3, 4, 5):                                       # ships that were already there: the target and the rest
        cpu.mem[fr + k] = pick(0, 1, 7, 10, 16, 17, 25, 2)
        for j in range(37): cpu.mem[kb + k * 37 + j] = rb()
    for k in (0, 1, 2):
        for j in range(37): cpu.mem[kb + k * 37 + j] = rb()
    mstg = pick(3, 4, 5, 1, 2) if st_ok[0] else pick(3, 4, 5)
    if cpu.mem[fr + mstg] == 0: cpu.mem[fr + mstg] = pick(7, 10, 16, 17)
    cpu.mem[S['MSTG']] = mstg; cpu.mem[S['NOMSL']] = nomsl; cpu.mem[S['DELTA']] = delta; cpu.mem[S['TYPE']] = ttype
    cpu.mem[S['MSAR']] = msar
    cpu.mem[S['INWK']:S['INWK'] + NI] = bytes(rb() for _ in range(NI))
    k0 = bytes(cpu.mem[kb:kb + 37 * 9]); fr0 = bytes(cpu.mem[fr:fr + 9])
    del evl[:]
    cpu.run(S['FRMIS'])
    sl = lambda k: hx(cpu.mem[kb + k * 37:kb + (k + 1) * 37])
    out.append('R %d %d %d %d %d %d %s %s | %s %d %s %s' % (st_ok[0], nomsl, delta, mstg, ttype, msar, hx(fr0), hx(k0), ';'.join(evl) or '-', cpu.mem[S['NOMSL']],
               hx(cpu.mem[fr:fr + 9]), ','.join(sl(k) for k in range(9))))

open('tests/vectors_equip.txt', 'w').write('\n'.join(out) + '\n')
print(len(out), 'vectores')
