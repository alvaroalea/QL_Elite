#!/usr/bin/env python3
"""Generates tests/vectors_geom.txt with the SAME operations and inputs as tools/gen_vectors3.py, but with the output of the
ORIGINAL code run in the 6502 emulator (instead of the Python transcription in tools/ref6502_state.py):
MVT3, MAS1, MAS2, MAS3, MAS4, VCSUB, TAS1, TAS2, NORM, TAS3, TAS4, TAS6, TIS3, TIDY, SPS1, SPS2, SPS3, SPS4, PLS6, PROJ,
LCASH, MCASH, GCASH, SCALEY, SCALEY2, SCALEX (and everything they use: MULT1, MAD, ADD, SQUA, LL5, LL28, DVID4...).
Checked with tests/test_state (make test_geom). Requires ELITE_SRC. Usage: python3 tools/gen_geom.py [n] [seed]"""
import sys, random, re
sys.path.insert(0, 'tools')
from frag6502 import *
import math

n = int(sys.argv[1]) if len(sys.argv) > 1 else 3000
random.seed(int(sys.argv[2]) if len(sys.argv) > 2 else 33)

# all the blocks of the Maths category, in source order
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
skipped = []
for a, b in blocks:
    bl = preprocess(L[a - 1:b - 1])
    if any(re.match(r'^FOR\b', code(l)) for l in bl): skipped.append(re.sub(r'^;\s+Name: ', '', L[a - 1]).strip()); continue   # tables
    bl = [l for l in bl if code(l) != 'SKIP 0']
    if any('PL2-1' in code(l) for l in bl):              # RTS just before PL2 (outside Maths): a local one
        bl = [l.replace('PL2-1', 'PL2RTS') for l in bl] + ['.PL2RTS', ' RTS']
    code_ += bl
print('tablas fuera:', skipped, file=sys.stderr)
extra = preprocess(L[nl('MVT3') - 1:nx(nl('MVT3')) - 1]) + preprocess(L[nl('PLS6') - 1:nx(nl('PLS6')) - 1]) + routine('PL21')   # PL21: SEC; RTS (a local RTS is not enough)
extra = [l.replace('PL2-1', 'PL2RTSB') for l in extra if code(l) != 'SKIP 0'] + ['.PL2RTSB', ' RTS']
code_ += extra
code_ = [re.sub(r'\s+AND\s+(?=\d)', ' & ', l) for l in code_]
cpu, S = build(code_, None, {'log': 0x2000, 'logL': 0x2100, 'alogh': 0x2200})
stubs = sorted(k for k, v in S.items() if 0x3E00 <= v < 0x3F00)
print('stubs:', stubs, file=sys.stderr)
lg = [0x0803] + [int(0x2000 * math.log(i) / math.log(2) + 0.5) for i in range(1, 256)]
for i in range(256):
    cpu.mem[0x2000 + i] = (lg[i] >> 8) & 255; cpu.mem[0x2100 + i] = lg[i] & 255
    cpu.mem[0x2200 + i] = int(2 ** ((i / 2 + 128) / 16) + 0.5) >> 8 & 255

NI = S['NI%']
SPEC = [0, 1, 2, 3, 0x1F, 0x20, 0x3F, 0x40, 0x5F, 0x60, 0x7F, 0x80, 0x81, 0xBF, 0xC0, 0xE0, 0xFF]
def B():
    r = random.random()
    if r < 0.35: return random.choice(SPEC)
    return random.randrange(256)
def blob(k): return bytearray(B() for _ in range(k))
def hx(b): return ''.join('%02x' % v for v in b)
def h1(v): return '%02x' % (v & 255)
out = []
def emit(op, ins, outs):
    out.append('%s in:%s out:%s' % (op, ','.join('%s=%s' % kv for kv in ins), ','.join('%s=%s' % kv for kv in outs)))

AREAS = {'inwk': ('INWK', NI), 'k': ('K', 4), 'ks': ('K%', 2 * NI), 'k3': ('K3', 16), 'x15': ('XX15', 6), 'cash': ('CASH', 4)}
ONE = {'p': 'P', 'p1': 'P+1', 'q': 'Q', 'r': 'R', 's': 'S', 'u': 'U', 't': 'T'}
def addr(nm): return S[nm[:-2]] + 1 if nm.endswith('+1') else S[nm]
def run(entry, a=0, x=0, y=0, c=0, **mem):
    """zeroes zero page and K%, sets the inputs and runs; returns an output reader"""
    for k in range(0, 0x100): cpu.mem[k] = 0
    for k in range(S['K%'], S['K%'] + 2 * NI): cpu.mem[k] = 0
    for k in range(S['CASH'], S['CASH'] + 4): cpu.mem[k] = 0
    for nm, v in mem.items():
        if nm in AREAS:
            ad, ln = AREAS[nm]; cpu.mem[S[ad]:S[ad] + ln] = bytes(v)
        elif nm in ONE: cpu.mem[addr(ONE[nm])] = v
        elif nm == 'v': cpu.mem[S['V']] = v & 255; cpu.mem[S['V'] + 1] = v >> 8
    cpu.a, cpu.x, cpu.y, cpu.c = a, x, y, c
    cpu.run(S[entry] if entry in S else entry)
    def get(nm):
        if nm in AREAS: ad, ln = AREAS[nm]; return hx(cpu.mem[S[ad]:S[ad] + ln])
        if nm in ONE: return h1(cpu.mem[addr(ONE[nm])])
        return h1({'ra': cpu.a, 'rx': cpu.x, 'ry': cpu.y, 'rc': cpu.c}[nm])
    return get

for _ in range(n):
    w = blob(NI); K = [B() for _ in range(4)]; X = random.choice([0, 3, 6, 9, 12, 15])
    g = run('MVT3', x=X, inwk=w, k=K)
    emit('mvt3', [('inwk', hx(w)), ('k', hx(bytes(K))), ('ix', h1(X))], [('k', g('k')), ('ra', g('ra')), ('rc', g('rc'))])
    Y = random.choice([9, 11, 13, 15, 17, 19, 21, 23, 25]); X = random.choice([0, 3, 6])
    g = run('MAS1', x=X, y=Y, inwk=w, k=K)
    emit('mas1', [('inwk', hx(w)), ('k', hx(bytes(K))), ('iy', h1(Y)), ('ix', h1(X))], [('inwk', g('inwk')), ('k', g('k')), ('ra', g('ra'))])
    ks = blob(2 * NI); Y = random.choice([0, NI]); A = B()
    g = run('MAS2', a=A, y=Y, ks=ks)
    emit('mas2', [('ks', hx(ks)), ('ia', h1(A)), ('iy', h1(Y))], [('ra', g('ra'))])
    g = run('m', y=Y, ks=ks)
    emit('mas2m', [('ks', hx(ks)), ('iy', h1(Y))], [('ra', g('ra'))])
    ksm = bytearray(ks)
    if random.random() < 0.7:
        for o in (1, 4, 7): ksm[Y + o] = random.randrange(0, 12) if random.random() < 0.8 else B()
    g = run('MAS3', y=Y, ks=ksm)
    emit('mas3', [('ks', hx(ksm)), ('iy', h1(Y))], [('ra', g('ra')), ('r', g('r'))])
    A = B()
    g = run('MAS4', a=A, inwk=w)
    emit('mas4', [('inwk', hx(w)), ('ia', h1(A))], [('ra', g('ra'))])
    k3 = blob(16); V = random.choice([0, NI])
    va = S['K%'] + V
    g = run('VCSUB', a=va >> 8, inwk=w, ks=ks, k3=k3, k=K, v=va)
    emit('vcsub', [('inwk', hx(w)), ('ks', hx(ks)), ('k3', hx(k3)), ('k', hx(bytes(K))), ('iv', h1(V))], [('k3', g('k3')), ('k', g('k')), ('u', g('u'))])
    Y = random.choice([2, 5, 8])
    g = run('TAS1', y=Y, inwk=w, ks=ks, k3=k3, k=K, v=va)
    emit('tas1', [('inwk', hx(w)), ('ks', hx(ks)), ('k3', hx(k3)), ('k', hx(bytes(K))), ('iv', h1(V)), ('iy', h1(Y))], [('k3', g('k3')), ('k', g('k')), ('u', g('u'))])
    k3 = blob(16)
    if random.random() < 0.3:
        for o in (1, 4, 7): k3[o] = random.randrange(0, 4)
    x15 = blob(6)
    g = run('TAS2', k3=k3, x15=x15)
    emit('tas2', [('k3', hx(k3)), ('x15', hx(x15))], [('k3', g('k3')), ('x15', g('x15')), ('q', g('q'))])
    g = run('NORM', x15=x15)
    emit('norm', [('x15', hx(x15))], [('x15', g('x15')), ('q', g('q'))])
    Y = random.choice([0, 2, 6, 10, 16, 22, 32])
    g = run('TAS3', y=Y, inwk=w, x15=x15)
    emit('tas3', [('inwk', hx(w)), ('x15', hx(x15)), ('iy', h1(Y))], [('ra', g('ra')), ('rx', g('rx')), ('s', g('s')), ('r', g('r'))])
    g = run('TAS4', y=Y, ks=ks, x15=x15)
    emit('tas4', [('ks', hx(ks)), ('x15', hx(x15)), ('iy', h1(Y))], [('ra', g('ra')), ('rx', g('rx')), ('s', g('s')), ('r', g('r'))])
    g = run('TAS6', x15=x15)
    emit('tas6', [('x15', hx(x15))], [('x15', g('x15'))])
    A = random.choice([0, 2, 4]); X = random.choice([0, 2, 4]); Y = random.choice([0, 2, 4])
    wq = bytearray(w)
    for o in (10, 12, 14): wq[o] = random.choice([0, 0, 1, 20, 60, 96, 0x80 | 96, 0x80 | 30, B()])
    if wq[10 + Y] == 0: wq[10 + Y] = 1
    g = run('TIS3', a=A, x=X, y=Y, inwk=wq)
    emit('tis3', [('inwk', hx(wq)), ('ia', h1(A)), ('ix', h1(X)), ('iy', h1(Y))], [('ra', g('ra')), ('p1', g('p1'))])
    wt = bytearray(w)
    if random.random() < 0.6:
        for o in (10, 12, 14, 16, 18, 20): wt[o] = random.choice([0, 0, 0, 96, 0xE0, 20, 0x80 | 20, 70, B()])
    g = run('TIDY', inwk=wt)
    emit('tidy', [('inwk', hx(wt))], [('inwk', g('inwk'))])
    g = run('SPS1', ks=ks, k3=k3)
    emit('sps1', [('ks', hx(ks)), ('k3', hx(k3))], [('k3', g('k3')), ('x15', g('x15')), ('q', g('q'))])
    g = run('SPS4', ks=ks, k3=k3)
    emit('sps4', [('ks', hx(ks)), ('k3', hx(k3))], [('k3', g('k3')), ('x15', g('x15')), ('q', g('q'))])
    X = random.choice([0, 3, 6])
    g = run('SPS3', x=X, ks=ks, k3=k3)
    emit('sps3', [('ks', hx(ks)), ('k3', hx(k3)), ('ix', h1(X))], [('k3', g('k3'))])
    A = B()
    g = run('SPS2', a=A)
    emit('sps2', [('ia', h1(A))], [('rx', g('rx')), ('ry', g('ry'))])
    wp = bytearray(w)
    if random.random() < 0.8:
        wp[1] = random.randrange(0, 3); wp[4] = random.randrange(0, 3)
        wp[7] = random.randrange(1, 256); wp[2] &= 0x7F if random.random() < 0.7 else 0xFF
        wp[8] &= 0x7F if random.random() < 0.9 else 0xFF
    p0, p1, A = B(), B(), B()
    g = run('PLS6', a=A, inwk=wp, p=p0, p1=p1)
    o = [('rc', g('rc')), ('k', g('k'))]
    kk = cpu.mem[S['K']:S['K'] + 4]
    if not ((kk[3] & 0x7F) | kk[2]): o.append(('rx', g('rx')))
    emit('pls6', [('inwk', hx(wp)), ('p', h1(p0)), ('p1', h1(p1)), ('ia', h1(A))], o)
    A = random.choice([192, 224, B()]); wf = bytearray(w)
    for o in (1, 4, 7):
        if random.random() < 0.5: wf[o] = (A + random.randrange(-2, 3)) & 255
    g = run('FAROF' if A == 224 and random.random() < 0.5 else 'FAROF2', a=A, inwk=wf)
    emit('farof2', [('inwk', hx(wf)), ('ia', h1(A))], [('rc', g('rc'))])
    k3 = blob(16)
    g = run('PROJ', inwk=wp, k3=k3)
    emit('proj', [('inwk', hx(wp)), ('k3', hx(k3))], [('rc', g('rc')), ('k3', g('k3')), ('k', g('k'))])
    cs = blob(4); X, Y = B(), B()
    if random.random() < 0.5: cs[0] = 0; cs[1] = 0
    g = run('LCASH', x=X, y=Y, cash=cs)
    emit('lcash', [('cash', hx(cs)), ('ix', h1(X)), ('iy', h1(Y))], [('cash', g('cash')), ('rc', g('rc'))])
    g = run('MCASH', x=X, y=Y, cash=cs)
    emit('mcash', [('cash', hx(cs)), ('ix', h1(X)), ('iy', h1(Y))], [('cash', g('cash'))])
    p0, q = B(), B()
    g = run('GCASH', p=p0, q=q)
    emit('gcash', [('p', h1(p0)), ('q', h1(q))], [('rx', g('rx')), ('ry', g('ry'))])
    A = B()
    for nm, ent in (('scaley2', 'SCALEY2'), ('scaley', 'SCALEY'), ('scalex', 'SCALEX')):
        g = run(ent, a=A)
        emit(nm, [('ia', h1(A))], [('ra', g('ra')), ('rc', g('rc'))])

open('tests/vectors_geom.txt', 'w').write('\n'.join(out) + '\n')
print(len(out), 'vectores')
