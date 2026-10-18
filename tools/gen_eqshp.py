#!/usr/bin/env python3
"""Genera tests/vectors_eqshp.txt con el codigo ORIGINAL de Equipment en el emulador de 6502: EQSHP (con eq, prx, refund, PRXS,
LCASH y MCASH reales; la pantalla, gnum y qv's TT217 son stubs que devuelven la eleccion):
  Q  EQSHP con el articulo elegido (0..13) y la vista del laser (0..3): final (ok / sin dinero / ya lo tienes / cancelado), dinero y
     equipo a la salida
  P  prx de cada articulo (precio de PRXS; el combustible depende de QQ14)
Se comprueba con tests/test_eqshp (make test_eqshp). Requiere ELITE_SRC. Uso: python3 tools/gen_eqshp.py [n] [semilla]"""
import sys, random, re
sys.path.insert(0, 'tools')
from frag6502 import *

n = int(sys.argv[1]) if len(sys.argv) > 1 else 3000
random.seed(int(sys.argv[2]) if len(sys.argv) > 2 else 47)
src = rng(nl('LCASH') - 1, nl('EQSHP') - 1) + rng(nl('EQSHP') - 1, nx(nl('PRXS')) - 1)
src = [l for l in src if code(l) != 'SKIP 0' and not re.match(r'^(PRINT|SAVE)\b', code(l))]
src = [re.sub(r'\s+AND\s+(?=\d)', ' & ', l) for l in src]
cpu, S = build(src, None)
m = cpu.mem
print('stubs:', sorted(k for k, v in S.items() if 0x3E00 <= v < 0x3F00), file=sys.stderr)
class Stop(Exception): pass
st = {'k': 0, 'v': 0, 'log': []}
def gnum(c):
    q = st['k']; m[S['R']] = q; c.a = q; c.c = 0; c.z = int(q == 0); c.n = q >> 7
    lo = c.pull(); hi = c.pull(); c.pc = (((hi << 8) | lo) + 1) & 0xFFFF
def tt217(c):
    c.a = c.x = 48 + st['v']
    lo = c.pull(); hi = c.pull(); c.pc = (((hi << 8) | lo) + 1) & 0xFFFF
def end(tag):
    def h(c): st['end'] = tag; raise Stop()
    return h
cpu.hooks[S['gnum']] = gnum; cpu.hooks[S['TT217']] = tt217
cpu.hooks[S['dn']] = end('ok'); cpu.hooks[S['dn2']] = end('dn2'); cpu.hooks[S['BAY']] = end('bay')
def ret(c): lo = c.pull(); hi = c.pull(); c.pc = (((hi << 8) | lo) + 1) & 0xFFFF
def doyc(c): m[S['YC']] = c.a; ret(c)
def doxc(c): m[S['XC']] = c.a; ret(c)
def incyc(c): m[S['YC']] = (m[S['YC']] + 1) & 255; ret(c)
cpu.hooks[S['DOYC']] = doyc; cpu.hooks[S['DOXC']] = doxc; cpu.hooks[S['INCYC']] = incyc
def prq(c): st['log'].append(c.a)
cpu.hooks[S['prq']] = prq
def msb(c): st['log'].append('m')
cpu.hooks[S['msblob']] = msb
def hx(b): return ''.join('%02x' % v for v in b)
POW, ARM, MLAS = S['POW'], S['Armlas'], S['Mlas']
LASERS = [0, POW, POW + 128, ARM, MLAS, 0x55, 0xFF]
EQ = ['QQ14', 'NOMSL', 'CRGO', 'ECM', 'BST', 'ESCP', 'BOMB', 'ENGY', 'DKCMP', 'GHYP']
def state():
    return ' '.join('%d' % m[S[v]] for v in EQ) + ' ' + hx(m[S['LASER']:S['LASER'] + 4]) + ' %08x' % int.from_bytes(m[S['CASH']:S['CASH'] + 4], 'big')
out = []
for _ in range(n):
    q14 = random.choice([70, 70, 0, 1, random.randrange(71)])
    vals = {'QQ14': q14, 'NOMSL': random.choice([0, 1, 3, 4, 4]), 'CRGO': random.choice([22, 37]), 'ECM': random.choice([0, 0xFF]),
            'BST': random.choice([0, 0xFF]), 'ESCP': random.choice([0, 0xFF]), 'BOMB': random.choice([0, 0x7F]), 'ENGY': random.choice([0, 1]),
            'DKCMP': random.choice([0, 0xFF]), 'GHYP': random.choice([0, 0xFF])}
    for k, v in vals.items(): m[S[k]] = v
    las = [random.choice(LASERS) for _ in range(4)]; m[S['LASER']:S['LASER'] + 4] = bytes(las)
    tek = random.randrange(15); m[S['tek']] = tek
    cash = random.choice([random.randrange(0, 2 ** 20), random.randrange(0, 200000), random.randrange(0, 2 ** 32), 0, 1, 299, 300, 3999, 4000, 59999, 60000, 0xFFFFFFFF])
    m[S['CASH']:S['CASH'] + 4] = cash.to_bytes(4, 'big')
    cnt = min(tek + 3, 14) if tek + 3 >= 12 or True else 0
    cnt = tek + 3 if tek + 3 < 12 else 14
    k = random.choice([0] + list(range(1, cnt + 1)))        # 0: cancelar (gnum devuelve 0)
    view = random.randrange(4); st['k'] = k; st['v'] = view; st['log'] = []; st['end'] = None
    pre = state()
    try: cpu.run(S['EQSHP'])
    except Stop: pass
    out.append('Q %d %d %d %d %s : %s %s : %s' % (tek, k, view, 1, pre, st['end'], ' '.join(str(x) for x in st['log']), state()))
for _ in range(200):
    q14 = random.randrange(71); m[S['QQ14']] = q14; m[S['PRXS']] = 0
    # prx lee PRXS; EQSHP lo pone: el combustible es (70 - QQ14) * 2
    m[S['PRXS']] = ((70 - q14) * 2) & 255
    res = []
    for it in range(14):
        cpu.a = it; cpu.run(S['prx']); res.append(cpu.x | (cpu.y << 8))
    out.append('P %d : %s' % (q14, ' '.join(map(str, res))))
open('tests/vectors_eqshp.txt', 'w').write('\n'.join(out) + '\n')
print(len(out), 'vectores')
