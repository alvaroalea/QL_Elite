#!/usr/bin/env python3
"""Generates tests/vectors_tactics.txt by running the ORIGINAL TACTICS code (all 7 parts) in the 6502 emulator with everything
it uses real: DOCKIT/DCS1/GOPL/TA151, TAS1-6, VCSUB, SPS1, NORM, MVT3, MAD, MAS4, OOPS+OUCH, EXNO2 (with KWL%/KWH%),
SESCP/SFS1/SFS2, SFRMIS, ECBLB2, DORND... Only these log: NWSHP (type and INWK; returns with C = 1), MESS, LASNOISE, SOHISS,
EXNO3, SOEXPL, ECBLB and DEATH. High shields and energy so there is no death (DEATH does not return in the original).
Requires ELITE_SRC. Usage: python3 tools/gen_tactics.py [n] [seed]"""
import sys, random, math
sys.path.insert(0, 'tools')
from frag6502 import *

n = int(sys.argv[1]) if len(sys.argv) > 1 else 4000
random.seed(int(sys.argv[2]) if len(sys.argv) > 2 else 1985)
hx = lambda b: ''.join('%02x' % x for x in b)
def rb(): return random.randrange(256)
def pick(*v): return random.choice(v)

math_ = rng(nl('MULT1') - 1, nx(nl('ADD')) - 1)
code_ = rng(nl('TACTICS') - 1, lab('LL164')) + ['.LL164', 'RTS', '.yetanotherrts', 'RTS'] + routine('SFRMIS') + \
        rng(nl('SPS1') - 1, nx(nl('NORM')) - 1) + routine('MVT3') + math_ + sq + routine('TIS2') + routine('ZINF') + \
        rng(nl('LL28') - 1, nx(nl('LL28')) - 1) + routine('SPS3') + rng(nl('LL5') - 1, nx(nl('LL5')) - 1) + dornd + \
        routine('MAS4') + routine('OOPS') + ['.out', 'RTS'] + preprocess(L[nl('OUCH') - 1:nx(nl('ou3')) - 1]) + \
        routine('ECBLB2') + [' JMP ECBLB'] + routine('MVT1') + routine('MAS1') + routine('MAS2') + routine('MAS3') + \
        routine('EXNO2')
cpu, S = build(code_, None, {'log': 0x2000, 'logL': 0x2100, 'alogh': 0x2200, 'UNIV': 0x2400})
stubs = sorted(k for k, v in S.items() if 0x3E00 <= v < 0x3F00)
print('stubs:', stubs, file=sys.stderr)
lg = [0x0803] + [int(0x2000 * math.log(i) / math.log(2) + 0.5) for i in range(1, 256)]
for i in range(256):
    cpu.mem[0x2000 + i] = (lg[i] >> 8) & 255; cpu.mem[0x2100 + i] = lg[i] & 255
    cpu.mem[0x2200 + i] = int(2 ** ((i / 2 + 128) / 16) + 0.5) >> 8 & 255
cpu.mem[S['KWL%']:S['KWL%'] + 32] = bytes(table('KWL%')); cpu.mem[S['KWH%']:S['KWH%'] + 32] = bytes(table('KWH%'))
cpu.mem[S['E%']:S['E%'] + 32] = bytes(table('E%'))
if 'DNOIZ' in S: cpu.mem[S['DNOIZ']] = 0xFF                      # sound off: SOEXPL/SOHISS do not call DORND (like the port)
NI = S['NI%']
got = []
def nwshp(c):
    got.append((c.a, bytes(c.mem[S['INWK']:S['INWK'] + NI]))); c.c = 1
cpu.hooks[S['NWSHP']] = nwshp
cpu.hooks[S['MESS']] = lambda c: cpu.log.append(('MESS', c.a, 0, 0))
for k in ('LASNOISE', 'SOHISS', 'EXNO3', 'SOEXPL', 'ECBLB', 'DEATH', 'BEEP'):
    if k in S and S[k] >= 0x3E00 and S[k] < 0x3F00:
        cpu.hooks[S[k]] = (lambda nm: (lambda c: cpu.log.append((nm, c.a, 0, 0))))(k)

def coord(r):
    v = random.randrange(-r, r + 1); m = abs(v)
    return [m & 255, (m >> 8) & 255, ((m >> 16) & 0x7F) | (0x80 if v < 0 else 0)]
def vec():
    o = []
    for _ in range(3):
        v = random.choice([random.randrange(-0x6000, 0x6001), 0x6000, -0x6000, 0, random.randrange(-0x1000, 0x1000)])
        m = abs(v); o += [m & 255, ((m >> 8) & 0x7F) | (0x80 if v < 0 else 0)]
    return o
def ship(rr):
    s = coord(rr) + coord(rr) + coord(rr) + vec() + vec() + vec()
    return s + [rb() for _ in range(NI - len(s))]

TYPES = [1, 1, 2, 2, 15, 30, 14, 29, 16, 11, 24, 26, 17, 9, 10, 3, 18, 20, 27]
out = []
for _ in range(n):
    typ = pick(*TYPES)
    iw = ship(pick(0x100, 0x400, 0x2000, 0x8000))
    iw[27] = pick(0, 10, 30, rb() & 63); iw[28] = rb(); iw[29] = rb(); iw[30] = rb()
    iw[31] = pick(0, 1, 2, 3, 4, 0x41, rb() & 0x47)
    iw[32] = pick(0xFF, 0x80, 0xC0, 0x81, 0xFE, rb() | 0x80)
    if typ == 1:                                                  # missile: at us or at a slot
        iw[32] = pick(0xFE, 0xFE, 0x80, 0x80, 0x82, 0x84, 0x86, 0x88, 0x8A, 0xC2 & 0xBF) | 0x80
        if random.random() < 0.4:
            for k in (0, 3, 6): iw[k] = pick(0, 0, rb())
            for k in (1, 4, 7): iw[k] = 0
            for k in (2, 5, 8): iw[k] &= 0x80
    iw[35] = rb(); iw[36] = pick(0, 1, 2, 4, 8, 16, 0x24, 0x84, rb() & 0x7F)
    slots = [ship(pick(0x400, 0x8000, 0x40000)) for _ in range(8)]
    if random.random() < 0.4:                                     # missile target stuck to it
        t = (iw[32] & 0x7F) >> 1
        if t < 8:
            slots[t][:9] = list(iw[:9])
            for k in (0, 3, 6): slots[t][k] ^= pick(0, 1, 8)
    slots[2] = list(iw)
    slots[1][36] = pick(0, 4, rb())
    for i in range(8): slots[i][31] = pick(0, 0x20, rb())
    bp = [rb() for _ in range(20)]
    bp[14] = pick(0, 70, 140, 250, rb()); bp[19] = pick(0, 0x10, 0x40, 0x92, rb())
    many_shu1, many_cops, many_thg, sspr = pick(0, 0, 1), pick(0, 2, 4), pick(0, 1), pick(0, 1)
    fist, ecma = pick(0, 0, 40, 80), pick(0, 0, 0, 5)
    rnd = [rb() for _ in range(4)]
    tl, t0, t1 = rb(), rb(), rb()
    for i in range(8): cpu.mem[S['K%'] + i * NI:S['K%'] + (i + 1) * NI] = bytes(slots[i])
    cpu.mem[S['INWK']:S['INWK'] + NI] = bytes(iw)
    k2 = S['K%'] + 2 * NI; cpu.mem[S['INF']] = k2 & 255; cpu.mem[S['INF'] + 1] = k2 >> 8
    for i in range(8):                                            # UNIV: address of each slot
        a = S['K%'] + i * NI; cpu.mem[S['UNIV'] + 2 * i] = a & 255; cpu.mem[S['UNIV'] + 2 * i + 1] = a >> 8
    bpa = 0x6000; cpu.mem[bpa:bpa + 20] = bytes(bp); cpu.mem[S['XX0']] = bpa & 255; cpu.mem[S['XX0'] + 1] = bpa >> 8
    for i in range(34): cpu.mem[S['MANY'] + i] = 0
    cpu.mem[S['MANY'] + 10] = many_shu1; cpu.mem[S['MANY'] + 16] = many_cops; cpu.mem[S['MANY'] + 29] = many_thg
    cpu.mem[S['MANY'] + 2] = sspr
    for k, v in (('TYPE', typ), ('FIST', fist), ('ECMA', ecma), ('FSH', 255), ('ASH', 255), ('ENERGY', 255),
                 ('TALLYL', tl), ('TALLY', t0), ('DLY', 0), ('RAT', 0), ('RAT2', 0), ('CNT', 0), ('CNT2', 0)):
        cpu.mem[S[k]] = v
    cpu.mem[S['TALLY'] + 1] = t1
    cpu.mem[S['RAND']:S['RAND'] + 4] = bytes(rnd)
    del got[:]; cpu.log = []
    cpu.x = typ; cpu.c = rb() & 1
    cpu.run(S['TACTICS'])
    lg_ = ','.join('%s:%d' % (x[0], x[1]) for x in cpu.log if x[0] in ('MESS', 'DEATH', 'ECBLB', 'LASNOISE')) or '-'
    nw = ';'.join('%d/%s' % (a, hx(b)) for a, b in got) or '-'
    out.append('T %d %s %s %s %d %d %d %d %d %d %s %d %d %d | %s %s %s %d %d %d %d %d %d %d %d %d %d %d %s %s' % (
        typ, hx(iw), hx(sum(slots, [])), hx(bp), many_shu1, many_cops, many_thg, sspr, fist, ecma, hx(rnd), tl, t0, t1,
        hx(cpu.mem[S['INWK']:S['INWK'] + NI]), hx(cpu.mem[S['K%']:S['K%'] + 8 * NI]), hx(cpu.mem[S['RAND']:S['RAND'] + 4]),
        cpu.mem[S['RAT']], cpu.mem[S['RAT2']], cpu.mem[S['CNT']], cpu.mem[S['CNT2']], cpu.mem[S['ECMA']],
        cpu.mem[S['FSH']], cpu.mem[S['ASH']], cpu.mem[S['ENERGY']], cpu.mem[S['TALLYL']], cpu.mem[S['TALLY']],
        cpu.mem[S['TALLY'] + 1], lg_, nw))
# ---------------------------------------------------------------- KS2, missile keys (MA4..MA24) and ECM (MA16..MA66)
cpu2, S2 = build(routine('KS2'), None, {'UNIV': 0x2400})
for _ in range(n // 4):
    xx4 = random.randrange(8)
    fr = [pick(0, 1, 1, 11, 16) for _ in range(8)]
    if random.random() < 0.5: k = random.randrange(8); fr[k:] = [0] * (8 - k)
    ais = [pick(0x80, 0x82, 0x84, 0x86, 0x88, 0x8A, 0x8C, 0x8E, 0xFE, 0x00, rb()) for _ in range(8)]
    for i in range(10): cpu2.mem[S2['FRIN'] + i] = fr[i] if i < 8 else 0
    for i in range(8):
        a = S2['K%'] + i * NI; cpu2.mem[0x2400 + 2 * i] = a & 255; cpu2.mem[0x2401 + 2 * i] = a >> 8
        cpu2.mem[a + 32] = ais[i]
    cpu2.mem[S2['XX4']] = xx4
    cpu2.run(S2['KS2'])
    out.append('K %d %s %s %s' % (xx4, hx(fr), hx(ais), hx([cpu2.mem[S2['K%'] + i * NI + 32] for i in range(8)])))

keys = rng(lab('MA4'), lab('MA24')) + ['.MA24', 'RTS', '.MA64', 'RTS'] + routine('ABORT') + routine('ABORT2')
cpu3, S3 = build(keys, None)
cpu3.hooks[S3['MSBAR']] = lambda c: (c.log.append(('MSBAR', c.a, c.x, c.y)), setattr(c, 'y', 0))   # MSBAR returns with Y = 0
for _ in range(n // 4):
    ky = [pick(0, 0, 0xFF) for _ in range(3)]          # KY15 (U), KY14 (T), KY16 (M)
    nomsl, mstg, msar = pick(0, 1, 3, 4), pick(0xFF, 0xFF, 3), pick(0, 0xFF)
    for k, v in zip(('KY15', 'KY14', 'KY16'), ky): cpu3.mem[S3[k]] = v
    cpu3.mem[S3['NOMSL']] = nomsl; cpu3.mem[S3['MSTG']] = mstg; cpu3.mem[S3['MSAR']] = msar
    cpu3.log = []
    cpu3.run(S3['MA4'])
    lg_ = ','.join('%s:%d:%d' % (x[0], x[2], x[3]) for x in cpu3.log if x[0] in ('MSBAR', 'BOOP', 'FRMIS')) or '-'
    out.append('M %s %d %d %d %d %d %s' % (''.join('%02x' % v for v in ky), nomsl, mstg, msar, cpu3.mem[S3['MSTG']],
               cpu3.mem[S3['MSAR']], lg_))

ecmf = rng(lab('MA16'), lab('MA66')) + ['.MA66', 'RTS'] + routine('DENGY') + routine('ECMOF')
cpu4, S4 = build(ecmf, None)
for _ in range(n // 4):
    ecmp, ecma, en = pick(0, 0xFF), pick(0, 1, 2, 32, rb()), pick(1, 2, 3, 128, 255)
    cpu4.mem[S4['ECMP']] = ecmp; cpu4.mem[S4['ECMA']] = ecma; cpu4.mem[S4['ENERGY']] = en; cpu4.log = []
    cpu4.run(S4['MA16'])
    lg_ = ','.join('%s:%d:%d' % (x[0], x[2], x[3]) for x in cpu4.log if x[0] in ('SOBLIP', 'ECBLB')) or '-'
    out.append('E %d %d %d %d %d %d %s' % (ecmp, ecma, en, cpu4.mem[S4['ECMP']], cpu4.mem[S4['ECMA']], cpu4.mem[S4['ENERGY']], lg_))

open('tests/vectors_tactics.txt', 'w').write('\n'.join(out) + '\n')
print(len(out), 'vectores')
