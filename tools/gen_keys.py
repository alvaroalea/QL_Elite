#!/usr/bin/env python3
"""Genera tests/vectors_dk4.txt con el codigo ORIGINAL de DK4 (FREEZE) y DKS3 en el emulador de 6502: RDKEY devuelve una secuencia
de teclas (0 = ninguna), WSCAN, DELAY y BELL son stubs (BELL cuenta). Salida: las 9 opciones de configuracion (DAMP..DISK), DNOIZ,
como acaba (tecla '-' o ESC = DEATH2) y cuantas teclas se leyeron. Se comprueba con tests/test_dk4 (make test_dk4).
Uso: python3 tools/gen_keys.py [n] [semilla]"""
import sys, random, re
sys.path.insert(0, 'tools')
from frag6502 import *

n = int(sys.argv[1]) if len(sys.argv) > 1 else 2000
random.seed(int(sys.argv[2]) if len(sys.argv) > 2 else 48)
src = routine('DK4') + routine('DKS3')
# DKS3 va justo antes de U%/DOKEY en la fuente; DK4 acaba en RTS (DK2)
src += ['.TGINT', ' EQUB ' + ','.join(str(ord(c)) for c in 'DAXFYJKUT')]
src += ['.DAMP'] + [' EQUB 0'] * 8 + ['.DISK', ' EQUB 0', '.DNOIZ', ' EQUB 0']
src = [l for l in src if code(l) != 'SKIP 0' and not re.match(r'^(PRINT|SAVE)\b', code(l))]
cpu, S = build(src, None)
m = cpu.mem
print('stubs:', sorted(k for k, v in S.items() if 0x3E00 <= v < 0x3F00), file=sys.stderr)
class Stop(Exception): pass
st = {'keys': [], 'i': 0, 'bells': 0, 'end': None}
def ret(c): lo = c.pull(); hi = c.pull(); c.pc = (((hi << 8) | lo) + 1) & 0xFFFF
def rdkey(c):
    k = st['keys'][st['i']]; st['i'] += 1
    c.x = k; c.a = k; c.z = int(k == 0); c.n = k >> 7; ret(c)
def bell(c): st['bells'] += 1; ret(c)
def death2(c): st['end'] = 'esc'; raise Stop()
cpu.hooks[S['RDKEY']] = rdkey; cpu.hooks[S['BELL']] = bell; cpu.hooks[S['DEATH2']] = death2
D = S['DAMP']
def hx(b): return ''.join('%02x' % v for v in b)
KEYS = [ord(ch) for ch in 'DAXFYJKUTQS'] + [0, 0, ord('Z'), ord('1'), ord(' ')]
out = []
for _ in range(n):
    cfg = [random.choice([0, 0xFF]) for _ in range(9)]
    m[D:D + 9] = bytes(cfg); dn = random.choice([0, 0xFF]); m[S['DNOIZ']] = dn
    seq = [random.choice(KEYS) for _ in range(random.randrange(0, 12))]
    seq.append(random.choice([ord('-'), ord('-'), 0x1B]))
    st['keys'] = seq; st['i'] = 0; st['bells'] = 0; st['end'] = '-'
    try: cpu.run(S['FREEZE'])
    except Stop: pass
    out.append('K %s %02x %s : %s %02x %s %d' % (hx(cfg), dn, hx(seq), hx(m[D:D + 9]), m[S['DNOIZ']], st['end'], st['i']))
open('tests/vectors_dk4.txt', 'w').write('\n'.join(out) + '\n')
print(len(out), 'vectores')
