#!/usr/bin/env python3
"""Genera tests/vectors_screen.txt con el codigo ORIGINAL de Drawing the screen en el emulador de 6502:
  T  TT66 (TTX66, TTX66K, HGR/TEXT reales; MT2, cleargrap, cleartext, BOX, FLFLLS, ee3 y TT27 registran; TT162 real)
  C  CLYNS (CHPR y clearrow registran)
  B  BOX (HLOIN registra X1, X2, Y1, COL; las esquinas de la fila 0 en $2001 y $2025)
Salidas: la secuencia registrada y QQ11, LSP, QQ17, DTW2, XC, YC, LAS2, DLY, de y text.
Se comprueba con tests/test_screen (make test_screen). Uso: python3 tools/gen_screen.py [n] [semilla]"""
import sys, random, re
sys.path.insert(0, 'tools')
from frag6502 import *

n = int(sys.argv[1]) if len(sys.argv) > 1 else 2000
random.seed(int(sys.argv[2]) if len(sys.argv) > 2 else 66)
src = []
for r in ('TT66', 'TTX66', 'TTX66K', 'HGR', 'TEXT', 'TT162', 'CLYNS'): src += routine(r)
src += [' JMP clearrow', '.DTW2', ' EQUB 0']          # CLYNS cae en clearrow (registra); DTW2 es un byte del codigo
src += ['.BOXR', ' JSR BOX', ' LDA $2001', ' LDX $2025', ' RTS']          # BOX esta en el bloque de TTX66K
src = [l for l in src if code(l) != 'SKIP 0' and not re.match(r'^(PRINT|SAVE)\b', code(l)) and not re.match(r'^(CODE|LOAD)_\w+%\s*=', code(l))]
cpu, S = build(src, None)
print('stubs:', sorted(k for k, v in S.items() if 0x3E00 <= v < 0x3F00), file=sys.stderr)
m = cpu.mem
def rec(name, f): cpu.hooks[S[name]] = lambda c: c.log.append(f(c))
rec('MT2', lambda c: 'm')
rec('cleargrap', lambda c: 'g')
rec('cleartext', lambda c: 'x')
rec('FLFLLS', lambda c: 'f')
rec('ee3', lambda c: 'e %d' % c.x)
rec('TT27', lambda c: 't %d %d %d' % (c.a, m[S['XC']], m[S['YC']]))
rec('CHPR', lambda c: 'c %d %d %d' % (c.a, m[S['XC']], m[S['YC']]))
rec('clearrow', lambda c: 'r %d' % c.y)
rec('HLOIN', lambda c: 'h %d %d %d %d' % (m[S['X1']], m[S['X2']], m[S['Y1']], m[S['COL']]))
def flf(c): c.a = 0; c.log.append('f')
cpu.hooks[S['FLFLLS']] = flf
rec('BOX', lambda c: 'b')
VARS = ('QQ11', 'LSP', 'QQ17', 'DTW2', 'XC', 'YC', 'LAS2', 'DLY', 'de', 'text')
def state(): return ' '.join('%d' % m[S[v]] for v in VARS)
out = []
for _ in range(n):
    for v in VARS: m[S[v]] = random.randrange(256)
    qq11 = random.choice([0, 0, 1, 13, 64, 128, 0x20, 4, 8, 16, 0xC0, 0xFF, random.randrange(256)])
    view = random.randrange(4); cnt = random.choice([0, 0, 0, 1, 15, 99, 255])
    m[S['VIEW']] = view; m[S['QQ22'] + 1] = cnt
    cpu.log = []
    cpu.a = qq11; cpu.run(S['TT66'])
    out.append('T %d %d %d : %s : %s' % (qq11, view, cnt, ' '.join(e for e in cpu.log if isinstance(e, str)), state()))
    for v in VARS: m[S[v]] = random.randrange(256)
    tx = random.choice([0, 0x80, 0xFF, 0x7F])
    m[S['text']] = tx
    cpu.log = []; cpu.run(S['CLYNS'])
    out.append('C %d : %s : %s' % (tx, ' '.join(e for e in cpu.log if isinstance(e, str)), ' '.join('%d' % m[S[v]] for v in ('XC', 'YC', 'DTW2', 'QQ17', 'DLY', 'de'))))
cpu.log = []; m[0x2001] = 0x55; m[0x2025] = 0x55; cpu.run(S['BOXR'])
out.append('B : %s : %d %d' % (' '.join(e for e in cpu.log if isinstance(e, str)), cpu.a, cpu.x))
open('tests/vectors_screen.txt', 'w').write('\n'.join(out) + '\n')
print(len(out), 'vectores')
