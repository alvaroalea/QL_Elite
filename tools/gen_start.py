#!/usr/bin/env python3
"""Genera tests/vectors_start.txt con el codigo ORIGINAL de Start and end en el emulador de 6502:
  R  RESET (ZERO real; SPBLB, ECMOF y WPSHPS registran)
  S  RES2  (idem)
  F  DFAULT (CHECK y CHECK2 reales; con sumas de comprobacion buenas o malas)
Se comprueba con tests/test_start (make test_start). Uso: python3 tools/gen_start.py [n] [semilla]"""
import sys, random, re
sys.path.insert(0, 'tools')
from frag6502 import *

n = int(sys.argv[1]) if len(sys.argv) > 1 else 1000
random.seed(int(sys.argv[2]) if len(sys.argv) > 2 else 170)
src = []
for r in ('RESET', 'ZERO'): src += routine(r)
src = [l for l in src if code(l) != 'SKIP 0' and not re.match(r'^(PRINT|SAVE)\b', code(l)) and not re.match(r'^(CODE|LOAD)_\w+%\s*=', code(l))]
# RESET cae en RES2 (el bloque de RES2 va justo detras); RES2 acaba con WPSHPS (stub)
src = routine('RESET')
src = [l for l in src if code(l) != 'SKIP 0']
src += routine('RES2') + routine('ZINF') + routine('ZERO') + routine('ECMOF')
cpu, S = build(src, None)
m = cpu.mem
print('stubs:', sorted(k for k, v in S.items() if 0x3E00 <= v < 0x3F00), file=sys.stderr)
def lg(c): return ' '.join('%s' % e[0] for e in c.log)
VARS = ['FRIN', 'MANY', 'JUNK', 'auto', 'ECMP', 'MJ', 'CABTMP', 'LAS2', 'MSAR', 'VIEW', 'LASCT', 'GNTMP', 'HFX', 'EV', 'DLY', 'de']
SPAN = 'FRIN'
zr = range(S['FRIN'], S['de'] + 1)
OUT = [102, 103, 104, 105, 106, 107, 108] + [S[v] for v in ('JSTGY', 'QQ12', 'FSH', 'ASH', 'ENERGY', 'ALP2', 'BET2', 'JSTX', 'JSTY', 'MCNT', 'DELTA', 'ALPHA', 'ALP1', 'NOSTM', 'LSX2', 'LSY2', 'MSTG', 'dontclip', 'Yx2M1', 'ECMA')] + [S['ALP2'] + 1, S['BET2'] + 1, S['BET1'], S['BETA'], S['FSH'] + 1, S['FSH'] + 2, S['SSPR']]
OUT = sorted(set(OUT) | set(zr) | set(range(S['INWK'], S['INWK'] + 37)) | {S['SLSP'], S['SLSP'] + 1})
def randmem():
    for a in OUT: m[a] = random.choice([0, 255, 128, random.randrange(256)])
    m[S['SSPR']] = random.choice([0, 0, 1, 2])
    m[S['ECMA']] = random.choice([0, 0, 1, 77])
    m[S['MSTG']] = random.randrange(256)
out = []
for k in range(n):
    for name, ent in (('R', 'RESET'), ('S', 'RES2')):
        randmem()
        before = [m[a] for a in OUT]
        cpu.log = []
        cpu.run(S[ent])
        out.append('%s %s : %s : %s' % (name, ' '.join('%d' % v for v in before), lg(cpu), ' '.join('%d' % m[a] for a in OUT)))
# DFAULT con CHECK y CHECK2 reales: NA% (8 de nombre + 76) con suma buena; CHK2 bien o mal (marca de datos alterados)
src2 = routine('DFAULT') + routine('CHECK') + routine('CHECK2')
cpu2, S2 = build(src2, None)
m2 = cpu2.mem
NA = S2['NA%']; NAME = S2['NAME']
print('DFAULT: NA%%=%d NAME=%d CHK=%d CHK2=%d CHK3=%d COK=%d' % (NA, NAME, S2['CHK'], S2['CHK2'], S2['CHK3'], S2['COK']), file=sys.stderr)
for k in range(n):
    for i in range(84): m2[NA + i] = random.randrange(256)
    for i in list(range(47 + 8, 51 + 8)) + [20 + 8, 21 + 8]: m2[NA + i] = 0     # bytes sin uso (EQUW 0 tras los lasers, EQUD 0 tras ESCP)
    for i in range(84): m2[NAME + i] = random.randrange(256)       # estado actual cualquiera: se pisa
    # CHECK real sobre NA% para obtener CHK y CHK3 buenos
    cpu2.run(S2['CHECK']); m2[S2['CHK']] = cpu2.a
    chk = cpu2.a
    cpu2.run(S2['CHECK2']); m2[S2['CHK3']] = cpu2.a
    m2[S2['CHK2']] = (chk ^ 0xA9) if random.random() < 0.5 else random.randrange(256)    # CHK2
    cok0 = random.randrange(256); m2[S2['COK']] = cok0
    blk = [m2[NA + 8 + i] for i in range(74)] + [m2[S2['CHK2']], m2[S2['CHK3']], m2[S2['CHK']]]
    m2[S2['QQ11']] = 0x55
    cpu2.run(S2['DFAULT'])
    after = [m2[NAME + 8 + i] for i in range(74)]
    assert m2[S2['QQ11']] == 0
    out.append('F %s : %s' % (' '.join('%d' % v for v in blk), ' '.join('%d' % v for v in after)))
open('tests/vectors_start.txt', 'w').write('\n'.join(out) + '\n')
inv = {}
for k, v in S.items():
    if v < 0x3E00 and k not in inv.values() and (v not in inv): inv[v] = k
def nm(a):
    if S['INWK'] <= a < S['INWK'] + 37: return 'I%d' % (a - S['INWK'])
    if a in (S['SLSP'], S['SLSP'] + 1): return {S['SLSP']: 'SLSPL', S['SLSP'] + 1: 'SLSPH'}[a]
    if S['FRIN'] <= a <= S['de']: return 'Z%d' % (a - S['FRIN'])
    return {S['JSTGY']: 'JSTGY', S['QQ12']: 'QQ12', S['FSH']: 'FSH', S['FSH'] + 1: 'ASH', S['FSH'] + 2: 'ENERGY', S['ALP2']: 'ALP2', S['ALP2'] + 1: 'ALP2H',
            S['BET2']: 'BET2', S['BET2'] + 1: 'BET2H', S['JSTX']: 'JSTX', S['JSTY']: 'JSTY', S['MCNT']: 'MCNT', S['DELTA']: 'DELTA', S['ALPHA']: 'ALPHA',
            S['ALP1']: 'ALP1', S['NOSTM']: 'NOSTM', S['LSX2']: 'LSX2', S['LSY2']: 'LSY2', S['MSTG']: 'MSTG', S['dontclip']: 'dontclip',
            S['Yx2M1']: 'Yx2M1', S['ECMA']: 'ECMA', S['BET1']: 'BET1', S['BETA']: 'BETA', S['QQ22']: 'QQ22', S['QQ22'] + 1: 'QQ22H'}.get(a, 'X%d' % a)
open('tests/vectors_start.addr', 'w').write(' '.join(nm(a) for a in OUT) + '\n')
print(len(out), 'vectores', len(OUT), 'direcciones')
