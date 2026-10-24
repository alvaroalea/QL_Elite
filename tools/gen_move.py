#!/usr/bin/env python3
"""Genera tests/vectors_move.txt con el codigo ORIGINAL del grupo Moving en el emulador de 6502 (Maths real, SCAN y TACTICS
registran):
  T  MVT1 (INWK, A, X, R)           V  MVS4 (INWK, ALPHA, BETA, Y)       W  MVS5 (INWK, X, Y, RAT2)
  X  MVT6 (INWK, A, X, P+1, P+2)    M  MVEIT completo (planetas y soles por MV40, MV45, MVS4, MVS5, MVT1, MVT6, TIDY)
Se comprueba con tests/test_move (make test_move). Requiere ELITE_SRC. Uso: python3 tools/gen_move.py [n] [semilla]"""
import sys, random, re, os
sys.path.insert(0, 'tools')
from frag6502 import *
import math
n = int(sys.argv[1]) if len(sys.argv) > 1 else 3000
random.seed(int(sys.argv[2]) if len(sys.argv) > 2 else 45)
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
mv = [l for l in rng(nl('MVEIT') - 1, nl('PLUT') - 1)] + preprocess(L[nl('MVS5') - 1:nx(nl('MVS5')) - 1]) + preprocess(L[nl('MVT3') - 1:nx(nl('MVT3')) - 1])
code_ += [l for l in mv if code(l) != 'SKIP 0']
code_ = [re.sub(r'\s+AND\s+(?=\d)', ' & ', l) for l in code_]
cpu, S = build(code_, None, {'log': 0x2000, 'logL': 0x2100, 'alogh': 0x2200})
print('stubs:', sorted(k for k, v in S.items() if 0x3E00 <= v < 0x3F00), file=sys.stderr)
lg = [0x0803] + [int(0x2000 * math.log(i) / math.log(2) + 0.5) for i in range(1, 256)]
for i in range(256):
    cpu.mem[0x2000 + i] = (lg[i] >> 8) & 255; cpu.mem[0x2100 + i] = lg[i] & 255
    cpu.mem[0x2200 + i] = int(2 ** ((i / 2 + 128) / 16) + 0.5) >> 8 & 255
REF = os.path.join(elitesrc.SRC, '..', '..', '4-reference-binaries', 'ib-disk') if hasattr(elitesrc, 'SRC') else os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'ref', 'moxon-appleII', '4-reference-binaries', 'ib-disk')
ships = open(os.path.join(REF, 'SHIPS.bin'), 'rb').read()
NI = S['NI%']
m = cpu.mem
log = []
def scan(c): log.append('s' + ''.join('%02x' % v for v in m[S['INWK']:S['INWK'] + 9]) + '%02x%02x' % (m[S['INWK'] + 31], m[S['TYPE']]))
def tact(c): log.append('t')
m[S['SCAN']] = 0x60; m[S['TACTICS']] = 0x60
cpu.hooks[S['SCAN']] = scan; cpu.hooks[S['TACTICS']] = tact
SPEC = [0, 1, 2, 3, 0x1F, 0x20, 0x3F, 0x40, 0x5F, 0x60, 0x7F, 0x80, 0x81, 0xBF, 0xC0, 0xE0, 0xFF]
def B():
    return random.choice(SPEC) if random.random() < 0.35 else random.randrange(256)
def hx(b): return ''.join('%02x' % v for v in b)
def reset():
    for k in range(0, 0x100): m[k] = 0
    m[S['XX21']:S['XX21'] + len(ships)] = ships
def inwk():
    iw = bytearray(B() for _ in range(NI))
    if random.random() < 0.5: iw[31] &= 0x5F            # sin los bits 7 y 5 con frecuencia
    return iw
out = []
YS = (9, 15, 21)
for _ in range(n):
    # ---- MVT1
    reset(); iw = inwk(); m[S['INWK']:S['INWK'] + NI] = iw
    a, x, r = B(), random.choice([0, 3, 6]), B(); m[S['R']] = r
    cpu.a, cpu.x = a, x; cpu.run(S['MVT1'])
    out.append('T %s %d %d %d : %s' % (hx(iw), a, x, r, hx(m[S['INWK']:S['INWK'] + NI])))
    # ---- MVS4
    reset(); iw = inwk(); m[S['INWK']:S['INWK'] + NI] = iw
    al, be, y = B(), B(), random.choice(YS); m[S['ALPHA']] = al; m[S['BETA']] = be
    cpu.y = y; cpu.run(S['MVS4'])
    out.append('V %s %d %d %d : %s' % (hx(iw), al, be, y, hx(m[S['INWK']:S['INWK'] + NI])))
    # ---- MVS5
    reset(); iw = inwk(); m[S['INWK']:S['INWK'] + NI] = iw
    x, y = random.choice([(15, 9), (17, 11), (19, 13), (15, 21), (17, 23), (19, 25)]); r2 = random.choice([0, 0x80])
    m[S['RAT2']] = r2; cpu.x, cpu.y = x, y; cpu.run(S['MVS5'])
    out.append('W %s %d %d %d : %s %02x%02x' % (hx(iw), x, y, r2, hx(m[S['INWK']:S['INWK'] + NI]), m[S['K']], m[S['K'] + 1]))
    # ---- MVT6
    reset(); iw = inwk(); m[S['INWK']:S['INWK'] + NI] = iw
    a, x, p1, p2 = B(), random.choice([0, 3, 6]), B(), B(); m[S['P'] + 1] = p1; m[S['P'] + 2] = p2
    cpu.a, cpu.x = a, x; cpu.run(S['MVT6'])
    out.append('X %s %d %d %d %d : %d %d %d' % (hx(iw), a, x, p1, p2, cpu.a, m[S['P'] + 1], m[S['P'] + 2]))
    # ---- MV45
    reset(); iw = inwk(); m[S['INWK']:S['INWK'] + NI] = iw
    t = random.choice([1, 5, 7, 11, 16, 128, 129, 130]); de = random.choice([0, 1, 3, 12, 40, B()]); al, be = B(), B()
    m[S['TYPE']] = t; m[S['DELTA']] = de; m[S['ALPHA']] = al; m[S['BETA']] = be
    del log[:]
    cpu.run(S['MV45'])
    out.append('Y %d %d %d %d %s : %s : %s' % (t, de, al, be, hx(iw), ' '.join(log), hx(m[S['INWK']:S['INWK'] + NI])))
    # ---- MV40 hasta MV45 (se para al llegar a MV45)
    reset(); iw = inwk(); m[S['INWK']:S['INWK'] + NI] = iw
    al, be = B(), B(); m[S['ALPHA']] = al; m[S['BETA']] = be
    class Stop(Exception): pass
    def stop45(c): raise Stop()
    cpu.hooks[S['MV45']] = stop45
    try: cpu.run(S['MV40'])
    except Stop: pass
    del cpu.hooks[S['MV45']]
    out.append('Z %d %d %s : %s' % (al, be, hx(iw), hx(m[S['INWK']:S['INWK'] + NI])))
    # ---- MVEIT
    reset(); iw = inwk()
    t = random.choice([1, 3, 5, 7, 11, 15, 16, 17, 18, 19, 20, 22, 24, 31, S['MSL'] if 'MSL' in S else 1, 128, 129, 128 + 2, 130])
    if random.random() < 0.15: t = random.randrange(1, 31)
    if t >= 128: iw[31] &= 0x5F                         # un planeta no explota (sin plano: XX0 no apunta a nada)
    m[S['INWK']:S['INWK'] + NI] = iw
    m[S['TYPE']] = t; xs = B(); mc = B(); de = random.choice([0, 1, 3, 12, 40, B()])
    al, be, a1, b1 = B(), B(), random.choice([0, 1, 2, 3, B()]), random.choice([0, 1, 2, 3, B()])
    a2 = (B(), B()); b2 = (B(), B())
    for nm, v in (('XSAV', xs), ('MCNT', mc), ('DELTA', de), ('ALPHA', al), ('BETA', be), ('ALP1', a1), ('BET1', b1), ('ALP2', a2[0]), ('BET2', b2[0])): m[S[nm]] = v
    m[S['ALP2'] + 1] = a2[1]; m[S['BET2'] + 1] = b2[1]
    if t < 128:
        bp = S['XX21'] + 2 * t - 2; m[S['XX0']] = m[bp]; m[S['XX0'] + 1] = m[bp + 1]
    del log[:]
    cpu.run(S['MVEIT'])
    out.append('M %d %d %d %d %d %d %d %d %d %d %d %d %d %s : %s : %s' % (t, xs, mc, de, al, be, a1, a2[0], a2[1], b1, b2[0], b2[1], 0, hx(iw), ' '.join(log), hx(m[S['INWK']:S['INWK'] + NI])))
open('tests/vectors_move.txt', 'w').write('\n'.join(out) + '\n')
print(len(out), 'vectores')
