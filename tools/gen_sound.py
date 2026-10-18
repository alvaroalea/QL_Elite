#!/usr/bin/env python3
"""Genera tests/vectors_sound.txt con el codigo ORIGINAL del grupo Sound en el emulador de 6502 (DORND real):
EXNO, EXNO3, SOEXPL (Y), SOHISS (X, Y), SOBOMB, BEEP, BOOP, SOBEEP (X, Y), SOBLIP (X, Y) y LASNOISE, con DNOIZ, RAND y
acarreo al azar. Lo comprobable del sonido es lo que cambia el juego: cuantos DORND se gastan, RAND y el acarreo a la salida.
Se comprueba con tests/test_sound_orig (make test_soundorig). Uso: python3 tools/gen_sound.py [n] [semilla]"""
import sys, random, re
sys.path.insert(0, 'tools')
from frag6502 import *

n = int(sys.argv[1]) if len(sys.argv) > 1 else 300
random.seed(int(sys.argv[2]) if len(sys.argv) > 2 else 40)
src = [l for l in rng(nl('EXNO') - 1, nl('COLD') - 1)] + routine('DORND') + ['.DNOIZ', ' EQUB 0']
src = [l for l in src if not re.match(r'^(PRINT|SAVE)\b', code(l))]
cpu, S = build(src, None)
print('stubs:', sorted(k for k, v in S.items() if 0x3E00 <= v < 0x3F00), file=sys.stderr)
cnt = [0]
cpu.hooks[S['DORND']] = lambda c: cnt.__setitem__(0, cnt[0] + 1)
def hx(b): return ''.join('%02x' % v for v in b)
out = []
for _ in range(n):
    for ent in ('EXNO', 'EXNO3', 'SOEXPL', 'SOHISS', 'SOBOMB', 'BEEP', 'BOOP', 'SOBEEP', 'SOBLIP', 'LASNOISE'):
        rnd = [random.randrange(256) for _ in range(4)]
        dn = random.choice([0, 0, 0x7F, 0xFF, 0x80])
        x = random.choice([0, 1, 2, 255, random.randrange(256)]); y = random.choice([0, 1, 2, 15, 40, 120, 210, random.randrange(256)])
        if ent in ('SOEXPL', 'SOHISS') and y > 120 and random.random() < 0.5: y = random.randrange(1, 60)   # acotar el tiempo
        c = random.randrange(2)
        cpu.mem[S['RAND']:S['RAND'] + 4] = bytes(rnd); cpu.mem[S['DNOIZ']] = dn
        cpu.x = x; cpu.y = y; cpu.a = random.randrange(256); cpu.c = c
        cnt[0] = 0
        cpu.run(S[ent], maxsteps=10**9)
        out.append('%s %s %d %d %d %d : %d %s %d' % (ent, hx(rnd), dn, c, x, y, cnt[0], hx(cpu.mem[S['RAND']:S['RAND'] + 4]), cpu.c))
open('tests/vectors_sound.txt', 'w').write('\n'.join(out) + '\n')
print(len(out), 'vectores')
