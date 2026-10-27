#!/usr/bin/env python3
"""Generates tests/vectors_laser.txt by running the ORIGINAL laser and damage code in the 6502 emulator:
   H  HITCH                          (with real SQUA2/MU11)
   A  ANGRY                          (INF = K% slot 2; the station is slot 1)
   L  LASLI (+ real DORND, DENGY; LL30 logs the lines)
   F  part 3 firing (MA68..MA3; LASNOISE and LASLI log the call)
   M  MLOOP: cooling (GNTMP) and pulse count (LASCT)
   E  part 16 MA77..paen1 (real SHD): shields and energy
   D  damage MA47..MA8 (real SPIN, SPIN2, EXNO2, ANGRY, DORND; SFS1, EXNO, SOEXPL, MESS log)
   K  reward MA15..MAC1 (real MCASH; MESS and KS1 log)
   S  SFS1/SFS2 (+ real MVT1 and DORND; NWSHP logs INWK and the type)
Requires ELITE_SRC. Usage: python3 tools/gen_laser.py [n] [seed]"""
import sys, random
sys.path.insert(0, "tools")
from frag6502 import *
hx = lambda b: ''.join('%02x' % x for x in b)
n = int(sys.argv[1]) if len(sys.argv) > 1 else 3000
random.seed(int(sys.argv[2]) if len(sys.argv) > 2 else 1985)
out = []
def rb(): return random.randrange(256)
def pick(*v): return random.choice(v)

# ---------------------------------------------------------------- H: HITCH
cpu, S = build(rng(lab('HITCH'), lab('FRS1')) + sq, None)
for _ in range(n):
    iw = [rb() for _ in range(9)]
    if random.random() < 0.8: iw[1] = pick(0, 0, 0, 1); iw[4] = pick(0, 0, 0, 1); iw[8] = pick(0, 0, 0, 0x80, 1)
    if random.random() < 0.5: iw[0] = random.randrange(0, 60); iw[3] = random.randrange(0, 60)
    st = pick(0, 0, 0x20, 0x80, 0x08, rb())
    typ = pick(1, 7, 11, 16, 29, 31, 128, 130, rb())
    a1, a2 = rb(), pick(0, 1, 2, 4, 8, 16, rb())
    if random.random() < 0.4:                        # at the edge of the hit area
        sm = iw[0] * iw[0] + iw[3] * iw[3] + pick(0, 0, 1, -1, 256, -256)
        if 0 <= sm < 65536: a1, a2 = sm & 255, sm >> 8
    cpu.mem[S['INWK']:S['INWK'] + 9] = bytes(iw); cpu.mem[S['INWK'] + 31] = st; cpu.mem[S['TYPE']] = typ
    bp = 0x6000; cpu.mem[bp + 1] = a1; cpu.mem[bp + 2] = a2
    cpu.mem[S['XX0']] = bp & 255; cpu.mem[S['XX0'] + 1] = bp >> 8
    cpu.c = rb() & 1
    cpu.run(S['HITCH'])
    out.append('H %s %d %d %d %d %d' % (hx(iw), st, typ, a1, a2, cpu.c))

# ---------------------------------------------------------------- A: ANGRY
cpu, S = build(rng(lab('HITCH'), lab('FRS1')) + routine('ANGRY') + sq, None)
NI = 37
for _ in range(n):
    typ = pick(1, 2, 4, 5, 7, 10, 11, 16, 24, 29, 31, 32, rb() & 31)
    slot = [rb() for _ in range(NI)]
    if random.random() < 0.3: slot[32] = 0
    st36 = rb()
    k2 = S['K%'] + 2 * NI
    cpu.mem[k2:k2 + NI] = bytes(slot); cpu.mem[S['K%'] + NI + 36] = st36
    cpu.mem[S['INF']] = k2 & 255; cpu.mem[S['INF'] + 1] = k2 >> 8
    cpu.mem[S['TYPE']] = typ; cpu.a = typ
    cpu.run(S['ANGRY'])
    out.append('A %d %s %d %s %d' % (typ, hx(slot), st36, hx(cpu.mem[k2:k2 + NI]), cpu.mem[S['K%'] + NI + 36]))

# ---------------------------------------------------------------- L: LASLI
cpu, S = build(rng(lab('LASLI'), nl('PDESC') - 1) + dornd + routine('DENGY'), None)
for _ in range(n):
    rnd = [rb() for _ in range(4)]; c = rb() & 1
    g = pick(0, 8, 100, 233, 241, rb()); e = pick(1, 2, 3, 128, 255, rb() or 1)
    cpu.mem[S['RAND']:S['RAND'] + 4] = bytes(rnd); cpu.mem[S['GNTMP']] = g; cpu.mem[S['ENERGY']] = e
    cpu.mem[S['QQ11']] = 0; cpu.c = c; cpu.log = []
    lines = []
    cpu.hooks[S['LL30']] = lambda cc: lines.append(tuple(cc.mem[S[v]] for v in ('X1', 'Y1', 'X2', 'Y2')))
    cpu.run(S['LASLI'])
    exp = [(cpu.mem[S['LASX']], cpu.mem[S['LASY']], x, 135) for x in (32, 224, 48, 208)]
    assert lines == exp, (lines, exp)          # LL30 always receives these four lines
    out.append('L %s %d %d %d %d %d %d %d %s' % (hx(rnd), c, g, e, cpu.mem[S['LASX']], cpu.mem[S['LASY']],
               cpu.mem[S['GNTMP']], cpu.mem[S['ENERGY']], hx(cpu.mem[S['RAND']:S['RAND'] + 4])))

# ---------------------------------------------------------------- F: firing (MA68..MA3)
cpu, S = build(rng(lab('MA68'), lab('MA3')) + ['RTS'], None)
for _ in range(n):
    lasct = pick(0, 0, 0, 1, 2, 10, rb()); ky7 = pick(0, 0xFF, 0xFF, 1); g = pick(0, 100, 241, 242, 243, rb())
    view = random.randrange(4)
    las = [pick(0, 15, 143, 151, 50, rb()) for _ in range(4)]
    l2 = rb()
    for k, v in (('LASCT', lasct), ('KY7', ky7), ('GNTMP', g), ('VIEW', view), ('LAS2', l2), ('LAS', rb()), ('DELTA', rb())):
        cpu.mem[S[k]] = v
    cpu.mem[S['LASER']:S['LASER'] + 4] = bytes(las); cpu.log = []
    cpu.run(S['MA68'])
    called = int(any(x[0] == 'LASLI' for x in cpu.log))
    out.append('F %d %d %d %d %s %d %d %d %d %d' % (lasct, ky7, g, view, hx(las), l2, cpu.mem[S['LAS']], cpu.mem[S['LAS2']],
               cpu.mem[S['LASCT']], called))

# ---------------------------------------------------------------- M: MLOOP
i0 = lab('MLOOP'); i1 = [k for k in range(i0, len(L)) if code(L[k]) == 'JSR DIALS'][0]
ml = [l for l in rng(i0, i1) if code(l) not in ('LDX #$FF', 'TXS')] + ['RTS']
cpu, S = build(ml, None)
for g in list(range(0, 256, 7)) + [0, 1, 255]:
    for lc in range(0, 256, 3):
        cpu.mem[S['GNTMP']] = g; cpu.mem[S['LASCT']] = lc
        cpu.run(S['MLOOP'])
        out.append('M %d %d %d %d' % (g, lc, cpu.mem[S['GNTMP']], cpu.mem[S['LASCT']]))

# ---------------------------------------------------------------- E: energy (MA77..paen1)
cpu, S = build(rng(lab('MA77'), lab('paen1')) + ['.paen1', 'RTS', '.MA22', 'RTS'] + routine('SHD') + routine('DENGY'), None)
for _ in range(n):
    e, a, f, eg = pick(0, 127, 128, 254, 255, rb()), pick(0, 254, 255, rb()), pick(0, 254, 255, rb()), pick(0, 1, 2, rb())
    mc = pick(0, 8, 16, rb())
    for k, v in (('ENERGY', e), ('ASH', a), ('FSH', f), ('ENGY', eg), ('MCNT', mc)): cpu.mem[S[k]] = v
    cpu.run(S['MA77'])
    out.append('E %d %d %d %d %d %d %d %d' % (e, a, f, eg, mc, cpu.mem[S['ENERGY']], cpu.mem[S['ASH']], cpu.mem[S['FSH']]))

# ---------------------------------------------------------------- D: damage (MA47..MA8)
kwl, kwh = table('KWL%'), table('KWH%')
dmg = rng(lab('MA47'), lab('MA8')) + ['.MA8', 'RTS'] + preprocess(L[nl('SPIN') - 1:nx(nl('SPIN')) - 1]) + dornd + \
      rng(lab('HITCH'), lab('FRS1')) + routine('ANGRY') + sq + rng(lab('SFS1'), lab('LL164')) + routine('MVT1') + routine('EXNO2')
cpu, S = build(dmg, None)
nw = [0]
def nwshp(cc):                                       # NWSHP: C = 1 if it fits (5 free slots in the C test)
    nw[0] += 1; cc.c = int(nw[0] <= 5)
cpu.hooks[S['NWSHP']] = (lambda f0: (lambda cc: (f0(cc), nwshp(cc))))(cpu.hooks[S['NWSHP']])
cpu.mem[S['KWL%']:S['KWL%'] + 32] = bytes(kwl); cpu.mem[S['KWH%']:S['KWH%'] + 32] = bytes(kwh)
for _ in range(n):
    typ = pick(2, 4, 5, 7, 7, 8, 10, 11, 16, 19, 24, 29, 31, 32, random.randrange(1, 33))
    las = pick(15, 15, 23, 50, 50, rb() & 127)
    e35 = pick(0, 1, 5, 15, 16, 23, 50, rb()); st = pick(0, 0x08, 0x03, rb() & 0x5F)
    newb = rb(); ai = pick(0, 0x80, rb()); b0 = rb()
    rnd = [rb() for _ in range(4)]; c = rb() & 1
    tl, t0, t1, st36 = rb(), rb(), rb(), rb()
    NIw = S['NI%']
    k2 = S['K%'] + 2 * NIw
    slot = [rb() for _ in range(NIw)]; slot[32] = ai; slot[36] = newb
    cpu.mem[k2:k2 + NIw] = bytes(slot); cpu.mem[S['K%'] + NIw + 36] = st36
    cpu.mem[S['INF']] = k2 & 255; cpu.mem[S['INF'] + 1] = k2 >> 8
    bp = 0x6000; cpu.mem[bp] = b0; cpu.mem[S['XX0']] = bp & 255; cpu.mem[S['XX0'] + 1] = bp >> 8
    for k, v in (('TYPE', typ), ('LAS', las), ('INWK', 0), ('TALLYL', tl), ('TALLY', t0), ('MSAR', 0)): cpu.mem[S[k]] = v
    cpu.mem[S['TALLY'] + 1] = t1
    cpu.mem[S['INWK'] + 35] = e35; cpu.mem[S['INWK'] + 31] = st; cpu.mem[S['INWK'] + 36] = newb
    cpu.mem[S['RAND']:S['RAND'] + 4] = bytes(rnd); cpu.c = c; cpu.log = []
    nw[0] = 0
    cpu.run(S['MA47'])
    log = [(x[0], x[1], x[2]) for x in cpu.log if x[0] in ('NWSHP', 'MESS', 'EXNO')]
    ls = ','.join('%s:%d:%d' % x for x in log) or '-'
    out.append('D %d %d %d %d %d %d %d %d %s %d %d %d %d %d %s %d %d %d %d %d %d %d %s %s' % (
        typ, las, e35, st, newb, ai, b0, st36, hx(rnd), c, tl, t0, t1, cpu.mem[S['LAS']],
        hx(slot), cpu.mem[S['INWK'] + 35], cpu.mem[S['INWK'] + 31], cpu.mem[S['TALLYL']], cpu.mem[S['TALLY']],
        cpu.mem[S['TALLY'] + 1], cpu.mem[S['K%'] + NIw + 36], 0, hx(cpu.mem[k2:k2 + NIw]), ls) + ' ' + hx(cpu.mem[S['RAND']:S['RAND'] + 4]))

# ---------------------------------------------------------------- K: reward (MA15..MAC1)
i0 = lab('MA15'); i1 = lab('MAC1')
kil = rng(i0, i1) + ['.MAC1', 'RTS'] + preprocess(L[nl('LCASH') - 1:nl('GCASH') - 1])
cpu, S = build(kil, None)
for _ in range(n):
    newb, st, fist = rb(), pick(0xA0, 0xA8, 0x80, 0x20, rb()), rb()
    dly, mj = pick(0, 0, 0, 5), pick(0, 0, 0, 1)
    b10, b11 = pick(0, rb()), rb(); cash = pick(0, rb() << 8, random.randrange(2 ** 32), 0xFFFFFF00)
    cpu.mem[S['INWK'] + 36] = newb; cpu.mem[S['INWK'] + 31] = st; cpu.mem[S['FIST']] = fist
    cpu.mem[S['DLY']] = dly; cpu.mem[S['MJ']] = mj; cpu.mem[S['INWK'] + 35] = rb()
    k2 = S['K%'] + 2 * S['NI%']; cpu.mem[S['INF']] = k2 & 255; cpu.mem[S['INF'] + 1] = k2 >> 8
    bp = 0x6000; cpu.mem[bp + 10] = b10; cpu.mem[bp + 11] = b11; cpu.mem[S['XX0']] = bp & 255; cpu.mem[S['XX0'] + 1] = bp >> 8
    cpu.mem[S['CASH']:S['CASH'] + 4] = cash.to_bytes(4, 'big'); cpu.log = []
    cpu.run(S['MA15'])
    names = [x[0] for x in cpu.log]
    kill = int('KS1' in names); mess = [x[1] for x in cpu.log if x[0] == 'MESS']
    out.append('K %d %d %d %d %d %d %d %08x %d %d %08x %s' % (newb, st, fist, dly, mj, b10, b11, cash, kill, cpu.mem[S['FIST']],
               int.from_bytes(cpu.mem[S['CASH']:S['CASH'] + 4], 'big'), ','.join(map(str, mess)) or '-'))

# ---------------------------------------------------------------- S: SFS1
cpu, S = build(rng(lab('SFS1'), lab('LL164')) + dornd + routine('MVT1'), None)
NIw = S['NI%']
for _ in range(n):
    parent = pick(2, 2, 11, 16, 7, random.randrange(1, 33))
    child = pick(1, 4, 5, 8, 9, 30, random.randrange(1, 33))
    ai = rb()
    slot = [rb() for _ in range(NIw)]
    if random.random() < 0.5:
        for k in (1, 4, 7): slot[k] = random.randrange(0, 0x20)
    iw = [rb() for _ in range(NIw)]
    rnd = [rb() for _ in range(4)]; c = rb() & 1
    k2 = S['K%'] + 2 * NIw
    cpu.mem[k2:k2 + NIw] = bytes(slot); cpu.mem[S['INWK']:S['INWK'] + NIw] = bytes(iw)
    cpu.mem[S['INF']] = k2 & 255; cpu.mem[S['INF'] + 1] = k2 >> 8; cpu.mem[S['TYPE']] = parent
    cpu.mem[S['RAND']:S['RAND'] + 4] = bytes(rnd)
    got = []
    cpu.hooks[S['NWSHP']] = lambda cc: (got.append((cc.a, bytes(cc.mem[S['INWK']:S['INWK'] + NIw]))), setattr(cc, 'c', 1))
    cpu.a = ai; cpu.x = child; cpu.c = c
    cpu.run(S['SFS1'])
    assert bytes(cpu.mem[S['INWK']:S['INWK'] + NIw]) == bytes(iw)       # INWK restored
    out.append('S %d %d %d %s %s %d %d %s %s' % (parent, child, ai, hx(slot), hx(rnd), c, got[0][0], hx(got[0][1]),
               hx(cpu.mem[S['RAND']:S['RAND'] + 4])))

open('tests/vectors_laser.txt', 'w').write('\n'.join(out) + '\n')
print(len(out), 'vectores')
