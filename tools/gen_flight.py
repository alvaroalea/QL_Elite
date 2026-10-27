#!/usr/bin/env python3
"""Generates tests/vectors_flight.txt with the ORIGINAL Flight group code in the 6502 emulator:
  U  PLUT (random INWK and VIEW 0..3)
  E  ESCAPE (RES2, FRS1, MVEIT, LL9, SCAN and GOIN log): calls and variables on exit
  J  MJP (mis-jump): calls and variables
  L  TT110 (launch): calls and variables
Checked with tests/test_flight (make test_flight). Requires ELITE_SRC. Usage: python3 tools/gen_flight.py [n] [seed]"""
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
src.append(' RTS')                                 # PLUT ends in LO2: RTS
cpu, S = build(src, None)
print('stubs PLUT:', sorted(k for k, v in S.items() if 0x3E00 <= v < 0x3F00), file=sys.stderr)
m = cpu.mem; NI = S['NI%']
for _ in range(n):
    iw = bytearray(B() for _ in range(NI)); v = random.randrange(4)
    m[S['INWK']:S['INWK'] + NI] = iw; m[S['VIEW']] = v
    cpu.run(S['PLUT'])
    out.append('U %d %s : %s' % (v, hx(iw), hx(m[S['INWK']:S['INWK'] + NI])))
# ---------------------------------------------------------------- J: MJP (real RES2, ZERO, ZINF and ECMOF; TT66, LL164, GTHG, LOOK1 and ECBLB log)
FLAGS['_IB_DISK'] = 1                            # MJP1 of the IB version (until there are three Thargoids)
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
# ---------------------------------------------------------------- L: TT110 (real RES2, SOS1, NWSPS, BAD and DORND; NWSHP logs INWK)
def rb(): return random.randrange(256)
dornd_ = routine('DORND')
src = routine('TT110') + routine('SOS1') + routine('NWSPS') + ['.NWSHP', ' RTS'] + routine('NwS1') + routine('RES2') + routine('ZINF') + routine('ZERO') + routine('ECMOF') + routine('BAD') + dornd_
src = [l for l in src if code(l) != 'SKIP 0']
cpu, S = build(src, None, {'XX21': 0x2800, 'spasto': 0x2900, 'LSO': 0x2A00})
print('stubs TT110:', sorted(k for k, v in S.items() if 0x3E00 <= v < 0x3F00), file=sys.stderr)
m = cpu.mem; NI_ = S['NI%']
got = []
cpu.hooks[S['NWSHP']] = lambda c: got.append('%d:%s' % (c.a, hx(m[S['INWK']:S['INWK'] + NI_])))
for _ in range(n // 2):
    for k in range(0x100): m[k] = 0
    tek, fist = rb() & 15, random.choice([0, 1, 40, 81, rb()])
    cargo = [random.choice([0, 0, 1, 5]) for _k in range(3)]; rnd = [rb() for _k in range(4)]; c0 = rb() & 1
    m[S['tek']] = tek; m[S['FIST']] = fist; m[S['QQ12']] = 0xFF; m[S['QQ20'] + 3], m[S['QQ20'] + 6], m[S['QQ20'] + 10] = cargo
    m[S['RAND']:S['RAND'] + 4] = bytes(rnd); m[S['DELTA']] = rb()
    m[S['K%']:S['K%'] + 2 * NI_] = bytes(2 * NI_); m[0x2900] = 0x11; m[0x2901] = 0x11
    m[0x2800 + 2 * 32 - 2] = 0x22; m[0x2800 + 2 * 32 - 1] = 0x22; m[0x2802] = 0; m[0x2803] = 0
    del got[:]; cpu.log = []; cpu.c = c0
    cpu.run(S['TT110'])
    dodo = {0x2222: 1, 0x1111: 0}.get(m[0x2802] | (m[0x2803] << 8), -1)
    out.append('L %d %d %s %s %d : %d %d %d %s %d : %s' % (tek, fist, ''.join('%02x' % v for v in cargo), hx(rnd), c0, m[S['FIST']], m[S['DELTA']], m[S['QQ12']], hx(m[S['RAND']:S['RAND'] + 4]), dodo, ' '.join(got)))
open('tests/vectors_flight.txt', 'w').write('\n'.join(out) + '\n')
print(len(out), 'vectores')
