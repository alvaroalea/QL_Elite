#!/usr/bin/env python3
"""Generates tests/vectors_draw.txt with the ORIGINAL code in the 6502 emulator (the drawing primitives log what they are
asked, without a screen):
  T  TT15: cross with QQ19 and QQ11 (HLOIN and VLOIN log X1, X2, Y1 / X1, Y1, Y2)
  S  SIGHT: laser sight according to VIEW and LASER, with sightcol (also logs COL)
  N  NLIN2 (random A) and NLIN4 (graphic mode): HLOIN with COL
  B  BOMBEFF2 with real SOBOMB and CLICK (the 25 DORNDs of the sound), random QQ11 and DNOIZ: LOIN logs X1, Y1, X2, Y2
  E  DOEXP (with PTCLS, EXS1, DVID4, FMLTU...): CPIX logs X, A (y) and COL; heap XX19, INWK+31 and RAND on exit
Checked with tests/test_draw (make test_draw). Usage: python3 tools/gen_draw.py [n] [seed]"""
import sys, random, re, math
sys.path.insert(0, 'tools')
from frag6502 import *

n = int(sys.argv[1]) if len(sys.argv) > 1 else 2000
random.seed(int(sys.argv[2]) if len(sys.argv) > 2 else 91)
def hx(b): return ''.join('%02x' % v for v in b)
def noskip(ls):                                   # SKIP 0 outside; SKIP n -> n EQUB 0
    out = []
    for l in ls:
        m = re.match(r'^SKIP\s+(\d+)$', code(l))
        if m and int(m.group(1)) == 0: continue
        out.append(' EQUB ' + ', '.join(['0'] * int(m.group(1))) if m else l)
    return out
LOGS = {'log': 0x9000, 'logL': 0x9100, 'alogh': 0x9200}
def logtables(cpu):
    lg = [0x0803] + [int(0x2000 * math.log(i) / math.log(2) + 0.5) for i in range(1, 256)]
    for i in range(256):
        cpu.mem[0x9000 + i] = (lg[i] >> 8) & 255; cpu.mem[0x9100 + i] = lg[i] & 255
        cpu.mem[0x9200 + i] = int(2 ** ((i / 2 + 128) / 16) + 0.5) >> 8 & 255
def recorder(cpu, S, name, fields):
    """the stub of name logs (name, memory values of fields at the time of the call)"""
    def h(c):
        c.log.append((name,) + tuple(c.mem[S[f]] if isinstance(f, str) else f(c) for f in fields))
    cpu.hooks[S[name]] = h
out = []

# ---------------------------------------------------------------- T, S, N
src = noskip(routine('TT15') + routine('SIGHT') + routine('sightcol') + rng(nl('NLI4') - 1, nl('HLOIN2') - 1))
cpu, S = build(src, None)
print('stubs T/S/N:', sorted(k for k, v in S.items() if 0x3E00 <= v < 0x3F00), file=sys.stderr)
recorder(cpu, S, 'HLOIN', ['X1', 'X2', 'Y1', 'COL'])
recorder(cpu, S, 'VLOIN', ['X1', 'Y1', 'Y2', 'COL'])
def run(entry, a=0, **mem):
    cpu.log = []
    for k, v in mem.items(): cpu.mem[S[k]] = v
    cpu.a = a; cpu.run(S[entry])
    return ' '.join('%s %d %d %d %d' % ((('h' if e[0] == 'HLOIN' else 'v'),) + e[1:]) for e in cpu.log if e[0] in ('HLOIN', 'VLOIN'))
def B(): return random.choice([0, 1, 2, 7, 10, 16, 20, 33, 34, 35, 100, 128, 200, 220, 221, 222, 223, 224, 254, 255]) if random.random() < 0.4 else random.randrange(256)
for _ in range(n):
    qq11 = random.choice([0, 0x80, 0x40, 1, 0xC0])
    x, y, sz = B(), B(), random.choice([7, 16, B()])
    cpu.mem[S['QQ19']] = x; cpu.mem[S['QQ19'] + 1] = y; cpu.mem[S['QQ19'] + 2] = sz
    out.append('T %d %d %d %d : %s' % (qq11 >> 7, x, y, sz, run('TT15', QQ11=qq11, COL=8)))
    view = random.randrange(4); las = [random.choice([0, 15, 143, 151, 50, 0x80 | 50, B()]) for _ in range(4)]
    for i in range(4): cpu.mem[S['LASER'] + i] = las[i]
    out.append('S %d %s : %s' % (view, ' '.join(map(str, las)), run('SIGHT', VIEW=view, QQ11=0)))
    a = B()
    out.append('N 2 %d : %s' % (a, run('NLIN2', a=a)))
    out.append('N 4 0 : %s' % run('NLIN4', text=0))

# ---------------------------------------------------------------- B
src = noskip(rng(nl('BOMBOFF') - 1, nx(nl('BOMBTBY')) - 1) + routine('SOBOMB') + routine('CLICK') + routine('DORND')) + ['.DNOIZ', ' EQUB 0']
cpu, S = build(src, None)
print('stubs B:', sorted(k for k, v in S.items() if 0x3E00 <= v < 0x3F00), file=sys.stderr)
recorder(cpu, S, 'LOIN', ['X1', 'Y1', 'X2', 'Y2'])
for _ in range(n):
    rnd = [random.randrange(256) for _ in range(4)]; qq11 = random.choice([0, 0, 0, 1, 0x80]); dn = random.choice([0, 0, 0xFF])
    c = random.randrange(2); tbx = [random.randrange(256) for _ in range(10)]; tby = [random.randrange(3, 131) for _ in range(10)]
    cpu.mem[S['RAND']:S['RAND'] + 4] = bytes(rnd); cpu.mem[S['QQ11']] = qq11; cpu.mem[S['DNOIZ']] = dn
    cpu.mem[S['BOMBTBX']:S['BOMBTBX'] + 10] = bytes(tbx); cpu.mem[S['BOMBTBY']:S['BOMBTBY'] + 10] = bytes(tby)
    cpu.log = []; cpu.c = c; cpu.run(S['BOMBEFF2'])
    lo = ' '.join('%d %d %d %d' % e[1:] for e in cpu.log if e[0] == 'LOIN')
    out.append('B %s %d %d %d %s %s : %s %s %s : %s' % (hx(rnd), qq11, dn, c, hx(tbx), hx(tby), hx(cpu.mem[S['RAND']:S['RAND'] + 4]),
               hx(cpu.mem[S['BOMBTBX']:S['BOMBTBX'] + 10]), hx(cpu.mem[S['BOMBTBY']:S['BOMBTBY'] + 10]), lo))

# ---------------------------------------------------------------- E
maths = []
for i, l in enumerate(L):
    m = re.match(r'^;\s+Name: (\S+)', l)
    if not m: continue
    for j in range(i, min(i + 8, len(L))):
        cc = re.match(r'^;\s+Category: (.*)$', L[j])
        if cc:
            if cc.group(1).strip().startswith('Maths (Arithmetic)'):
                bl = preprocess(L[i:nx(i + 1) - 1])
                if not any(re.match(r'^FOR\b', code(x)) for x in bl): maths += bl
            break
src = noskip(maths + ['.TT48', ' RTS'] + rng(nl('DOEXP') - 1, nx(nl('DOEXP')) - 1))   # TT48 (an outside RTS) just before, within branch range
src = [re.sub(r'\s+AND\s+(?=\d)', ' & ', l) for l in src]
cpu, S = build(src, None, LOGS)
logtables(cpu)
print('stubs E:', sorted(k for k, v in S.items() if 0x3E00 <= v < 0x3F00), file=sys.stderr)
recorder(cpu, S, 'CPIX', [lambda c: c.x, lambda c: c.a, 'COL'])
NI = S['NI%']; HEAP = 0x7000
for _ in range(n):
    w = bytearray(random.randrange(256) for _ in range(NI))
    w[7] = random.choice([0, 1, 2, 5, 10, 31, 32, 40, 200]) if random.random() < 0.8 else w[7]
    w[31] = random.randrange(256)
    heap = bytearray(random.randrange(256) for _ in range(64))
    heap[1] = random.choice([random.randrange(256), random.randrange(240, 256), random.randrange(0, 20)])
    heap[2] = 6 + 4 * random.randrange(1, 10)            # byte 7 of the blueprint (4 * vertices + 6): with 6 EXL1 would wrap around Y
    xx3 = bytearray(random.randrange(256) for _ in range(64))
    for k in range(0, 64, 2):
        if random.random() < 0.6: xx3[k + 1] = 0                       # x_hi / y_hi = 0: inside the screen
    k6 = random.randrange(256); rnd = [random.randrange(256) for _ in range(4)]
    cpu.mem[S['INWK']:S['INWK'] + NI] = bytes(w); cpu.mem[HEAP:HEAP + 64] = bytes(heap)
    cpu.mem[S['XX3']:S['XX3'] + 64] = bytes(xx3); cpu.mem[S['K%'] + 6] = k6
    cpu.mem[S['XX19']] = HEAP & 255; cpu.mem[S['XX19'] + 1] = HEAP >> 8
    cpu.mem[S['RAND']:S['RAND'] + 4] = bytes(rnd)
    cpu.log = []; cpu.c = random.randrange(2); cpu.run(S['DOEXP'])
    px = ' '.join('%d %d %d' % e[1:] for e in cpu.log if e[0] == 'CPIX')
    out.append('E %s %s %s %d %s : %02x %s %s : %s' % (hx(w), hx(heap), hx(xx3), k6, hx(rnd), cpu.mem[S['INWK'] + 31],
               hx(cpu.mem[HEAP:HEAP + 64]), hx(cpu.mem[S['RAND']:S['RAND'] + 4]), px))

# ---------------------------------------------------------------- U: LAUN and LL164 (real sounds with their DORNDs; CIRCLE2 logs)
src = noskip(routine('LAUN') + routine('HFS2') + routine('LL164') + routine('SOHISS') + routine('SOBLIP') + routine('DORND')) + ['.DNOIZ', ' EQUB 0']
cpu, S = build(src, None)
print('stubs U:', sorted(k for k, v in S.items() if 0x3E00 <= v < 0x3F00), file=sys.stderr)
cpu.hooks[S['SOHISS']] = lambda c: c.log.append('h %d' % c.y)
cpu.hooks[S['SOBLIP']] = lambda c: c.log.append('b %d %d' % (c.x, c.y))
cpu.hooks[S['CIRCLE2']] = lambda c: c.log.append('c %d %d %d %d' % (c.mem[S['K3']], c.mem[S['K4']], c.mem[S['K']], c.mem[S['STP']]))
for _ in range(max(1, n // 20)):
    for ent in ('LAUN', 'LL164'):
        rnd = [random.randrange(256) for _ in range(4)]; dn = random.choice([0, 0, 0xFF]); c = 1     # with C = 0 the LL164 series drops by 11 the first time
        cpu.mem[S['RAND']:S['RAND'] + 4] = bytes(rnd); cpu.mem[S['DNOIZ']] = dn
        cpu.log = []; cpu.c = c; cpu.run(S[ent])
        lo = ' '.join(e for e in cpu.log if isinstance(e, str))
        out.append('U %s %s %d %d : %s : %s' % (ent, hx(rnd), dn, c, hx(cpu.mem[S['RAND']:S['RAND'] + 4]), lo))

# ---------------------------------------------------------------- K: scacol table (bytes assembled from the source)
src = noskip(routine('scacol'))
src = [l for l in src if not re.match(r'^EQUD', code(l))]
cpu, S = build(src, None)
out.append('K : ' + ' '.join(str(cpu.mem[S['scacol'] + i]) for i in range(34)))

open('tests/vectors_draw.txt', 'w').write('\n'.join(out) + '\n')
print(len(out), 'vectores')
