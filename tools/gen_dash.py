#!/usr/bin/env python3
"""Generates tests/vectors_dash.txt with the ORIGINAL Dashboard group code in the 6502 emulator (real Maths; HLOIN, VLOIN, PIXEL and
CPIX log their arguments):
  D  full DIALS (DIS1, DIS2, DIS5, COMPAS, DOT, SP1, SP2, SPS1, SPS2, SPS4...) from a random previous state
  M  MSBAR (X, Y) and msblob with random mscol
  S  SCAN (INWK, TYPE)
  C  cntr, BUMP2 and REDU2 (X, A, DAMP, DJD, auto)
Checked with tests/test_dash (make test_dash). Requires ELITE_SRC. Usage: python3 tools/gen_dash.py [n] [seed]"""
import sys, random, re, os
sys.path.insert(0, 'tools')
from frag6502 import *
import math
n = int(sys.argv[1]) if len(sys.argv) > 1 else 2000
random.seed(int(sys.argv[2]) if len(sys.argv) > 2 else 49)
blocks = []
for i, l in enumerate(L):
    m = re.match(r'^;\s+Name: (\S+)', l)
    if not m: continue
    for j in range(i, min(i + 8, len(L))):
        c = re.match(r'^;\s+Category: (.*)$', L[j])
        if c:
            if c.group(1).strip().startswith('Maths'): blocks.append((i + 1, nx(i + 1)))
            break
code_ = []
for a, b in blocks:
    bl = preprocess(L[a - 1:b - 1])
    if any(re.match(r'^FOR\b', code(l)) for l in bl): continue
    bl = [l for l in bl if code(l) != 'SKIP 0']
    if any('PL2-1' in code(l) for l in bl):
        bl = [l.replace('PL2-1', 'PL2RTS') for l in bl] + ['.PL2RTS', ' RTS']
    code_ += bl

dash = rng(nl('DIALS') - 1, nl('ESCAPE') - 1) + routine('COMPAS') + routine('SP1') + routine('SP2') + [l.replace('COR1', 'DOTRTS') for l in routine('DOT')] + ['.DOTRTS', ' RTS'] + rng(nl('MSBARS') - 1, nl('HLOIN') - 1) + [' JMP HLOIN']
dash += rng(nl('msblob') - 1, nx(nl('msblob')) - 1) + rng(nl('MSBAR') - 1, nl('msloc') - 1) + ['.coolkey', ' CLC', ' RTS'] + rng(nl('msloc') - 1, nx(nl('msloc')) - 1) + rng(nl('SCAN') - 1, nl('HGR') - 1 if False else nx(nl('SCAN')) - 1)
dash += preprocess(L[nl('cntr') - 1:nx(nl('REDU2')) - 1])
def fix(lines):                                   # EQUD %binary or $hex or decimal -> 4 EQUB (little endian)
    out = []
    for l in lines:
        mm = re.match(r'^EQUD\s+([%$]?)([0-9A-Fa-f]+)$', code(l))
        if mm:
            v = int(mm.group(2), 2 if mm.group(1) == '%' else 16 if mm.group(1) == '$' else 10)
            out.append(' EQUB %d, %d, %d, %d' % (v & 255, v >> 8 & 255, v >> 16 & 255, v >> 24 & 255))
        else: out.append(l)
    return out
dash += fix(routine('scacol')) + ['.dials'] + [' EQUB 0'] * 13 + ['.mscol'] + [' EQUB 0'] * 4 + ['.DAMP', ' EQUB 0', '.DJD', ' EQUB 0', '.FLH', ' EQUB 0', '.COMC', ' EQUB 0']
code_ += [l for l in dash if code(l) != 'SKIP 0']
code_ = [re.sub(r'\s+AND\s+(?=\d)', ' & ', l) for l in code_]
cpu, S = build(code_, None, {'log': 0x2000, 'logL': 0x2100, 'alogh': 0x2200})
print('stubs:', sorted(k for k, v in S.items() if 0x3E00 <= v < 0x3F00), file=sys.stderr)
lg = [0x0803] + [int(0x2000 * math.log(i) / math.log(2) + 0.5) for i in range(1, 256)]
for i in range(256):
    cpu.mem[0x2000 + i] = (lg[i] >> 8) & 255; cpu.mem[0x2100 + i] = lg[i] & 255
    cpu.mem[0x2200 + i] = int(2 ** ((i / 2 + 128) / 16) + 0.5) >> 8 & 255
m = cpu.mem
NI = S['NI%']
log = []
def rec(name, f):
    cpu.hooks[S[name]] = lambda c: log.append(f(c))
rec('HLOIN', lambda c: 'h %d %d %d %d' % (m[S['X1']], m[S['X2']], m[S['Y1']], m[S['COL']]))
rec('VLOIN', lambda c: 'v %d %d %d %d' % (m[S['X1']], m[S['Y1']], m[S['Y2']], m[S['COL']]))
rec('PIXEL', lambda c: 'p %d %d %d' % (c.x, c.a, m[S['ZZ']]))
def cpix(c):
    log.append('c %d %d %d' % (c.x, c.a, m[S['COL']])); m[S['Y1']] = c.a        # CPIX starts with STA Y1
cpu.hooks[S['CPIX']] = cpix
def hx(b): return ''.join('%02x' % v for v in b)
SPEC = [0, 1, 2, 3, 0x1F, 0x20, 0x3F, 0x40, 0x5F, 0x60, 0x7F, 0x80, 0x81, 0xBF, 0xC0, 0xE0, 0xFF]
def B():
    return random.choice(SPEC) if random.random() < 0.35 else random.randrange(256)
COLS = [S[k] for k in ('VIOLET', 'GREEN', 'WHITE', 'BLUE', 'RED')]
def dstate():
    return hx(m[S['dials']:S['dials'] + 13]) + ' ' + hx(m[S['dialc']:S['dialc'] + 13]) + ' %d %d %d' % (m[S['COMX']], m[S['COMY']], m[S['COMC']])
def reset():
    for k in range(0, 0x100): m[k] = 0
out = []
for _ in range(n):
    # ---- DIALS
    reset()
    nd = random.choice([0, 0, 1, 2, 3, 4])
    for k in range(13):
        m[S['dials'] + k] = random.choice([0, 31, 1, random.randrange(32)]) if k not in (1, 2) else random.choice([0, 224 - 8 + random.randrange(17)])
        m[S['dialc'] + k] = random.choice([0] + COLS)
    m[S['COMX']], m[S['COMY']], m[S['COMC']] = B(), B(), random.choice([0, 0x60, 0x30])
    for nm in ('DELTA', 'ALP1', 'BET1', 'ENERGY', 'FSH', 'ASH', 'QQ14', 'ALTIT', 'CABTMP', 'GNTMP', 'MCNT'): m[S[nm]] = B()
    m[S['ALP2'] + 1] = B(); m[S['BET2']] = B(); m[S['FLH']] = random.choice([0, 0xFF])
    m[S['SSPR']] = random.choice([0, 0, 1])
    for k in range(S['K%'], S['K%'] + 2 * NI): m[k] = B()
    if random.random() < 0.5:
        for k in (1, 4, 7): m[S['K%'] + k] = random.randrange(64)          # reasonable positions
    ks = hx(m[S['K%']:S['K%'] + 2 * NI])
    inp = ' '.join('%d' % m[S[nm]] for nm in ('DELTA', 'ALP1', 'BET1', 'ENERGY', 'FSH', 'ASH', 'QQ14', 'ALTIT', 'CABTMP', 'GNTMP', 'MCNT')) + ' %d %d %d %d' % (m[S['ALP2'] + 1], m[S['BET2']], m[S['FLH']], m[S['SSPR']])
    pre = dstate()
    del log[:]
    cpu.run(S['DIALS'])
    out.append('D %s %s %s : %s : %s' % (inp, pre, ks, ' | '.join(log), dstate()))
    # ---- MSBAR
    reset()
    for k in range(4): m[S['mscol'] + k] = random.choice([0] + COLS)
    x, y = random.randrange(1, 5), random.choice([0] + COLS)
    pre = hx(m[S['mscol']:S['mscol'] + 4])
    del log[:]
    cpu.x, cpu.y = x, y; cpu.run(S['MSBAR'])
    out.append('M %d %d %s : %s : %s' % (x, y, pre, ' | '.join(log), hx(m[S['mscol']:S['mscol'] + 4])))
    reset()
    for k in range(4): m[S['mscol'] + k] = random.choice([0] + COLS)
    nm_ = random.randrange(0, 5); m[S['NOMSL']] = nm_
    pre = hx(m[S['mscol']:S['mscol'] + 4])
    del log[:]
    cpu.run(S['msblob'])
    out.append('B %d %s : %s : %s' % (nm_, pre, ' | '.join(log), hx(m[S['mscol']:S['mscol'] + 4])))
    # ---- SCAN
    reset()
    iw = bytearray(B() for _ in range(NI))
    if random.random() < 0.6: iw[31] |= 0x10
    if random.random() < 0.5:
        for k in (1, 4, 7): iw[k] &= 0x3F
    t = random.choice([random.randrange(1, 34), 128, 129, 130])
    m[S['INWK']:S['INWK'] + NI] = iw; m[S['TYPE']] = t
    del log[:]
    cpu.run(S['SCAN'])
    out.append('S %d %s : %s' % (t, hx(iw), ' | '.join(log)))
    # ---- cntr, BUMP2, REDU2
    reset()
    for nm, v in (('DAMP', random.choice([0, 0xFF])), ('DJD', random.choice([0, 0xFF])), ('auto', random.choice([0, 0, 1]))): m[S[nm]] = v
    x0, a0 = B(), B()
    cpu.x = x0; cpu.run(S['cntr'])
    out.append('C %d %d %d %d : %d' % (x0, m[S['DAMP']], m[S['DJD']], m[S['auto']], cpu.x))
    cpu.x, cpu.a = x0, a0; cpu.run(S['BUMP2'])
    out.append('U %d %d %d : %d' % (x0, a0, m[S['DJD']], cpu.x))
    cpu.x, cpu.a = x0, a0; cpu.run(S['REDU2'])
    out.append('R %d %d %d : %d' % (x0, a0, m[S['DJD']], cpu.x))
open('tests/vectors_dash.txt', 'w').write('\n'.join(out) + '\n')
print(len(out), 'vectores')

# ---------------------------------------------------------------- E and S bulbs (BULB + letter2 on the hires screen)
def fixd(lines):
    o = []
    for l in lines:
        mm = re.match(r'^EQUD\s+([%$])([0-9A-Fa-f]+)$', code(l))
        if mm:
            v = int(mm.group(2), 2 if mm.group(1) == '%' else 16)
            o.append(' EQUB %d, %d, %d, %d' % (v & 255, v >> 8 & 255, v >> 16 & 255, v >> 24 & 255))
        else: o.append(l)
    return o
lt = [l for l in preprocess(L[lab('letter2'):lab('letter2') + 400]) if code(l)]
end = [i for i, l in enumerate(lt) if code(l) == 'RTS'][0]
bl = rng(nl('ECBLB') - 1, nl('MSBAR') - 1) + lt[:end + 1]
for t in ('SCTBL', 'SCTBH'): bl += routine(t)
bl = fixd([l for l in bl if code(l) != 'SKIP 0'])
bl += ['.SCTBX1'] + [' EQUB %d' % ((i + 8) % 7) for i in range(256)]
bl += ['.SCTBX2'] + [' EQUB %d' % ((i + 8) // 7) for i in range(256)]
cpu, S = build(bl, None)
print('stubs bulb:', sorted(k for k, v in S.items() if 0x3E00 <= v < 0x3F00), file=sys.stderr)
m = cpu.mem
sctbl = m[S['SCTBL']:S['SCTBL'] + 24]; sctbh = m[S['SCTBH']:S['SCTBH'] + 24]
def rowaddr(y): return sctbl[y >> 3] + 256 * sctbh[y >> 3] + 0x400 * (y & 7)
def snap():
    d = {}
    for y in range(192):
        b = rowaddr(y)
        for col in range(40): d[(y, col)] = m[b + col] & 0x7F
    return d
for ent in ('ECBLB', 'SPBLB'):
    for rnd in (False, True):
        for a in range(0x2000, 0x4000): m[a] = random.randrange(256) if rnd else 0
        a0 = snap(); cpu.run(S[ent]); a1 = snap()
        ch = []
        for (y, col), v in a1.items():
            for bit in range(7):
                if (v ^ a0[(y, col)]) >> bit & 1:
                    gx = col * 7 + bit - 8
                    if 0 <= gx < 256: ch.append((gx, y))
        out.append('%s %d : %s' % ('E' if ent == 'ECBLB' else 'P', int(rnd), ' '.join('%02x%02x' % p for p in sorted(ch))))
open('tests/vectors_dash.txt', 'w').write('\n'.join(out) + '\n')
print(len(out), 'vectores')
