#!/usr/bin/env python3
"""Genera tests/vectors_flight.txt con el codigo ORIGINAL del grupo Flight en el emulador de 6502:
  U  PLUT (INWK al azar y VIEW 0..3)
  E  ESCAPE (RES2, FRS1, MVEIT, LL9, SCAN y GOIN registran): llamadas y variables a la salida
  J  MJP (mis-jump): llamadas y variables
  L  TT110 (despegue): llamadas y variables
Se comprueba con tests/test_flight (make test_flight). Requiere ELITE_SRC. Uso: python3 tools/gen_flight.py [n] [semilla]"""
import sys, random, re
sys.path.insert(0, 'tools')
from frag6502 import *
n = int(sys.argv[1]) if len(sys.argv) > 1 else 1000
random.seed(int(sys.argv[2]) if len(sys.argv) > 2 else 50)
def hx(b): return ''.join('%02x' % v for v in b)
SPEC = [0, 1, 2, 3, 0x1F, 0x20, 0x3F, 0x40, 0x5F, 0x60, 0x7F, 0x80, 0x81, 0xBF, 0xC0, 0xE0, 0xFF]
def B(): return random.choice(SPEC) if random.random() < 0.35 else random.randrange(256)
out = []
# ---------------------------------------------------------------- U: PLUT
src = [l for l in routine('PLUT') if code(l) != 'SKIP 0']
src = [re.sub(r'PU2-1', 'PU2RTS', l) for l in src]
i2 = [i for i, l in enumerate(src) if code(l) == '.PU2'][0]
j = max(i for i in range(i2) if code(src[i]))
assert code(src[j]) == 'RTS'
src.insert(j, '.PU2RTS')
src.append(' RTS')                                 # PLUT acaba en LO2: RTS
cpu, S = build(src, None)
print('stubs PLUT:', sorted(k for k, v in S.items() if 0x3E00 <= v < 0x3F00), file=sys.stderr)
m = cpu.mem; NI = S['NI%']
for _ in range(n):
    iw = bytearray(B() for _ in range(NI)); v = random.randrange(4)
    m[S['INWK']:S['INWK'] + NI] = iw; m[S['VIEW']] = v
    cpu.run(S['PLUT'])
    out.append('U %d %s : %s' % (v, hx(iw), hx(m[S['INWK']:S['INWK'] + NI])))
# ---------------------------------------------------------------- J: MJP (RES2, ZERO, ZINF y ECMOF reales; TT66, LL164, GTHG, LOOK1 y ECBLB registran)
FLAGS['_IB_DISK'] = 1                            # MJP1 de la version IB (hasta tener tres Thargoides)
src = [l for l in routine('MJP') + routine('RES2') + routine('ZINF') + routine('ZERO') + routine('ECMOF') if code(l) != 'SKIP 0']
cpu, S = build(src, None)
FLAGS['_IB_DISK'] = 0
print('stubs MJP:', sorted(k for k, v in S.items() if 0x3E00 <= v < 0x3F00), file=sys.stderr)
m = cpu.mem
log = []
cpu.hooks[S['TT66']] = lambda c: log.append('T%d' % c.a)
cpu.hooks[S['LL164']] = lambda c: log.append('L')
def gthg(c): log.append('G'); m[S['MANY'] + S['THG']] = (m[S['MANY'] + S['THG']] + 1) & 255
cpu.hooks[S['GTHG']] = gthg
cpu.hooks[S['LOOK1']] = lambda c: log.append('V%d' % c.x)
cpu.hooks[S['SPBLB']] = lambda c: log.append('S')
cpu.hooks[S['ECBLB']] = lambda c: log.append('E')
cpu.hooks[S['WPSHPS']] = lambda c: log.append('W')
for _ in range(n // 4):
    q1, mc, ecma, sspr = B(), random.choice([0, 0, 1, 2, 3, 4]), random.choice([0, 0, 77]), random.choice([0, 0, 1])
    m[S['QQ1']] = q1; m[S['MANY'] + S['THG']] = mc; m[S['ECMA']] = ecma; m[S['SSPR']] = sspr; m[S['MJ']] = B(); m[S['NOSTM']] = B()
    del log[:]
    cpu.y = B(); cpu.run(S['MJP'])
    out.append('J %d %d %d %d : %s : %d %d %d %d %d' % (q1, mc, ecma, sspr, ' '.join(log), m[S['MJ']], m[S['NOSTM']], m[S['QQ1']], m[S['MANY'] + S['THG']], m[S['ECMA']]))
open('tests/vectors_flight.txt', 'w').write('\n'.join(out) + '\n')
print(len(out), 'vectores')
