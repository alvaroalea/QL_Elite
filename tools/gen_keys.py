#!/usr/bin/env python3
"""Generates tests/vectors_dk4.txt with the ORIGINAL DK4 (FREEZE) and DKS3 code in the 6502 emulator: RDKEY returns a sequence
of keys (0 = none), WSCAN, DELAY and BELL are stubs (BELL counts). Output: the 9 configuration options (DAMP..DISK), DNOIZ,
how it ends (key '-' or ESC = DEATH2) and how many keys were read. Checked with tests/test_dk4 (make test_dk4).
Usage: python3 tools/gen_keys.py [n] [seed]"""
import sys, random, re
sys.path.insert(0, 'tools')
from frag6502 import *

n = int(sys.argv[1]) if len(sys.argv) > 1 else 2000
random.seed(int(sys.argv[2]) if len(sys.argv) > 2 else 48)
src = routine('DK4') + routine('DKS3')
# DKS3 goes just before U%/DOKEY in the source; DK4 ends in RTS (DK2)
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
