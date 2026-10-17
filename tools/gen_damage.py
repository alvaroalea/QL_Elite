#!/usr/bin/env python3
"""Genera tests/vectors_damage.txt ejecutando en el emulador de 6502 el codigo ORIGINAL de los danos a nuestra nave:
   O  OOPS (+ OUCH, ou2, ou3, DORND reales; DEATH, EXNO3 y MESS registran; MESS guarda tambien "de")
   C  partes 9 y 10 del bucle de vuelo (choques, recogida con tnpr1/tnpr, ISDK con SPS1, OOPS y OUCH reales;
      MESS, EXNO3, DEATH y DOENTRY registran)
   P  parte 15 (MA22..MA23: ENERGY LOW, altitud con MAS2/MAS3/LL5, cabina y combustible; MESS y DEATH registran)
   X  una vuelta de D1 de DEATH (Ze, DORND, ZINF y fq1 reales; NWSHP registra INWK y el tipo)
Los sonidos (EXNO3 -> SOEXPL) se sustituyen por stubs: en el original SOEXPL llama a DORND (el ruido) y gasta numeros
aleatorios cuando el sonido esta activado; el port no reproduce eso (ver Proyecto.md 10.3).
Requiere ELITE_SRC. Uso: python3 tools/gen_damage.py [n] [semilla]"""
import sys, random
sys.path.insert(0, 'tools')
from frag6502 import *

n = int(sys.argv[1]) if len(sys.argv) > 1 else 3000
random.seed(int(sys.argv[2]) if len(sys.argv) > 2 else 1985)
hx = lambda b: ''.join('%02x' % x for x in b)
def rb(): return random.randrange(256)
def pick(*v): return random.choice(v)
out = []

ouchr = ['.out', 'RTS'] + preprocess(L[nl('OUCH') - 1:nx(nl('ou3')) - 1])   # out (RTS) esta lejos, antes de me1
oopsr = routine('OOPS')

def log_hooks(cpu, S):
    """MESS registra (A, de); los demas stubs ya registran (nombre, A, X, Y)"""
    if 'MESS' in S:
        cpu.hooks[S['MESS']] = lambda c: c.log.append(('MESS', c.a, c.mem[S['de']], 0))

def log_tables(cpu, S):
    """tablas log/logL/alogh con las formulas de la fuente (como orig.ll9_image), donde los stubs las dejaron"""
    import math
    lg = [0x0803] + [int(0x2000 * math.log(i) / math.log(2) + 0.5) for i in range(1, 256)]
    for i in range(256):
        cpu.mem[S['log'] + i] = (lg[i] >> 8) & 255
        cpu.mem[S['logL'] + i] = lg[i] & 255
        cpu.mem[S['alogh'] + i] = int(2 ** ((i / 2 + 128) / 16) + 0.5) >> 8 & 255

def put_items(cpu, S, it): cpu.mem[S['QQ20']:S['QQ20'] + 22] = bytes(it)
def get_items(cpu, S): return bytes(cpu.mem[S['QQ20']:S['QQ20'] + 22])
def rand_items():
    it = [pick(0, 0, 1, 5, rb()) for _ in range(17)] + [pick(0, 0xFF, 1) for _ in range(5)]
    return it
def logs(cpu, names):
    l = [x for x in cpu.log if x[0] in names]
    return ','.join('%s:%d:%d' % (x[0], x[1], x[2]) for x in l) or '-'

# ---------------------------------------------------------------- O: OOPS
cpu, S = build(oopsr + ouchr + dornd, None); log_hooks(cpu, S)
NI = S['NI%']; k2 = S['K%'] + 2 * NI
for _ in range(5 * n):                               # OUCH pierde algo pocas veces: mas casos
    t, c = pick(5, 128, 200, 255, rb()), rb() & 1
    zs = pick(0, 0x80, rb())
    f, a, e = pick(0, 1, 5, 200, 255, rb()), pick(0, 1, 5, 200, 255, rb()), pick(1, 2, 50, 128, 255, rb())
    it = rand_items(); dl = pick(0, 0, 0, 5); rnd = [rb() for _ in range(4)]
    cpu.mem[k2 + 8] = zs; cpu.mem[S['INF']] = k2 & 255; cpu.mem[S['INF'] + 1] = k2 >> 8
    for k, v in (('FSH', f), ('ASH', a), ('ENERGY', e), ('DLY', dl), ('de', 0)): cpu.mem[S[k]] = v
    put_items(cpu, S, it); cpu.mem[S['RAND']:S['RAND'] + 4] = bytes(rnd); cpu.log = []
    cpu.a = t; cpu.c = c
    cpu.run(S['OOPS'])
    out.append('O %d %d %d %d %d %d %s %d %s %d %d %d %d %s %s %s' % (t, c, zs, f, a, e, hx(it), dl, hx(rnd),
               cpu.mem[S['FSH']], cpu.mem[S['ASH']], cpu.mem[S['ENERGY']], int(any(x[0] == 'DEATH' for x in cpu.log)),
               hx(get_items(cpu, S)), hx(cpu.mem[S['RAND']:S['RAND'] + 4]), logs(cpu, ('MESS', 'EXNO3'))))

# ---------------------------------------------------------------- C: partes 9 y 10
i0 = [k for k in range(lab('MAL3'), len(L)) if code(L[k]) == 'BPL MAL3'][0] + 1
coll = ['.COLL'] + rng(i0, lab('MA26')) + ['.MA26', 'RTS']
extra = oopsr + ouchr + dornd + preprocess(L[nl('tnpr1') - 1:nx(nl('tnpr')) - 1]) + routine('MAS4') + \
        preprocess(L[nl('SPS1') - 1:nx(nl('NORM')) - 1]) + routine('SPS3') + sq + \
        preprocess(L[nl('LL5') - 1:nx(nl('LL5')) - 1]) + preprocess(L[nl('LL28') - 1:nx(nl('LL28')) - 1]) + routine('TIS2')
cpu, S = build(coll + extra, None, {'log': 0x2000, 'logL': 0x2100, 'alogh': 0x2200}); log_hooks(cpu, S)
log_tables(cpu, S)
bp = 0x6000
for _ in range(n):
    typ = pick(2, 2, 2, 5, 5, 4, 8, 3, 30, 1, 11, 16, 7, 129, random.randrange(1, 33))
    iw = [rb() for _ in range(NI)]
    for k in (1, 4, 7): iw[k] = pick(0, 0, 0, 0, 1)
    for k in (0, 3, 6): iw[k] = pick(random.randrange(64), random.randrange(64), random.randrange(128), rb())
    iw[31] = pick(0, 0, 0x08, 0x20, 0x80, rb() & 0x5F)
    scoop = typ in (3, 4, 5, 8, 30) and random.random() < 0.7               # recogidas: BST (QQ20+18) y la nave por debajo
    if scoop: iw[5] |= 0x80
    if typ == 2 and random.random() < 0.6: iw[14] = pick(214, 213, 255, random.randrange(200, 256)); iw[16] = pick(80, 79, 0xD0, rb())
    bst, delta = pick(0, 0xFF, 0xFF), pick(1, 4, 5, 12, 40)
    pl = [rb() for _ in range(9)]
    if random.random() < 0.7: pl[1], pl[4], pl[7] = random.randrange(4), random.randrange(4), random.randrange(4)
    st36 = pick(0, 4, rb()); b0 = pick(0x20, 0x80, 0xB0, 0xF0, 0x28, 0x88, 0xB8, 0, rb() & 0x77, rb() & 0xE7)   # mercancia <= 16 como en los planos reales
    f, a, e = pick(0, 5, 200, 255, rb()), pick(0, 5, 200, 255, rb()), pick(1, 2, 128, 255, rb())
    it = rand_items(); dl = pick(0, 0, 0, 5); rnd = [rb() for _ in range(4)]; c = rb() & 1
    if scoop: it[18] = 0xFF
    crgo = pick(20, 35, 2, rb()); trib = pick(0, 0, 1)
    cpu.mem[S['INWK']:S['INWK'] + NI] = bytes(iw); cpu.mem[k2:k2 + NI] = bytes(iw)
    cpu.mem[S['INF']] = k2 & 255; cpu.mem[S['INF'] + 1] = k2 >> 8
    cpu.mem[S['K%']:S['K%'] + 9] = bytes(pl); cpu.mem[S['K%'] + NI + 36] = st36
    cpu.mem[bp] = b0; cpu.mem[S['XX0']] = bp & 255; cpu.mem[S['XX0'] + 1] = bp >> 8
    for k, v in (('TYPE', typ), ('BST', bst), ('DELTA', delta), ('FSH', f), ('ASH', a), ('ENERGY', e), ('DLY', dl),
                 ('CRGO', crgo), ('de', 0)): cpu.mem[S[k]] = v
    cpu.mem[S['TRIBBLE']] = 0; cpu.mem[S['TRIBBLE'] + 1] = trib
    put_items(cpu, S, it); cpu.mem[S['RAND']:S['RAND'] + 4] = bytes(rnd); cpu.log = []
    cpu.c = c
    cpu.run(S['COLL'])
    out.append('C %d %s %d %d %s %d %d %d %d %d %s %d %s %d %d %d %d %d %d %d %d %s %s %s' % (
        typ, hx(iw), bst, delta, hx(pl), st36, b0, f, a, e, hx(it), dl, hx(rnd), c, crgo, trib,
        cpu.mem[S['INWK'] + 31], cpu.mem[S['INWK'] + 36], cpu.mem[S['DELTA']],
        cpu.mem[S['FSH']] * 65536 + cpu.mem[S['ASH']] * 256 + cpu.mem[S['ENERGY']],
        int(any(x[0] == 'DEATH' for x in cpu.log)) + 2 * int(any(x[0] == 'DOENTRY' for x in cpu.log)),
        hx(get_items(cpu, S)), hx(cpu.mem[S['RAND']:S['RAND'] + 4]), logs(cpu, ('MESS', 'EXNO3'))))

# ---------------------------------------------------------------- P: parte 15
p15 = ['.MA23S', 'RTS'] + rng(lab('MA22'), lab('MA23')) + ['.MA23', 'RTS'] + routine('MAS2') + routine('MAS3') + sq + \
      preprocess(L[nl('LL5') - 1:nx(nl('LL5')) - 1])
cpu, S = build(p15, None); log_hooks(cpu, S)
for _ in range(n):
    mj, mc = pick(0, 0, 0, 0, 1), pick(10, 10, 15, 20, 20, 20, 42, 52, rb() & 0xF7 or 1)
    e, au, sspr, bst = pick(50, 51, 10, 200), pick(0, 0, 1), pick(0, 0, 1), pick(0, 0xFF)
    delta, q14, cab = pick(0, 3, 12, 40), pick(0, 30, 69, 70), rb()
    q = rb()                                         # Q queda de antes: LL5 calcula SQRT(R Q) con el Q que hubiera
    pl = [rb() for _ in range(9)]; su = [rb() for _ in range(9)]
    for s_ in (pl, su):
        if random.random() < 0.7:
            for k in (2, 5, 8): s_[k] &= pick(0x80, 0x81, 0x80)
            for k in (1, 4, 7): s_[k] = pick(random.randrange(16), random.randrange(4), random.randrange(12), random.randrange(50, 140))
    cpu.mem[S['K%']:S['K%'] + 9] = bytes(pl); cpu.mem[S['K%'] + NI:S['K%'] + NI + 9] = bytes(su)
    for k, v in (('MJ', mj), ('MCNT', mc), ('ENERGY', e), ('auto', au), ('BST', bst), ('DELTA', delta), ('QQ14', q14),
                 ('CABTMP', cab), ('ALTIT', 77), ('Q', q)): cpu.mem[S[k]] = v
    cpu.mem[S['MANY'] + S['SST']] = sspr
    d4 = (delta & 3) << 6; cpu.mem[S['DELT4']] = d4; cpu.mem[S['DELT4'] + 1] = delta >> 2
    cpu.log = []
    cpu.run(S['MA22'])
    out.append('P %d %d %d %d %d %d %d %d %d %s %s %d %d %d %d %s %d' % (mj, mc, e, au, sspr, bst, delta, q14, cab, hx(pl), hx(su),
               cpu.mem[S['ALTIT']], cpu.mem[S['CABTMP']], cpu.mem[S['QQ14']], int(any(x[0] == 'DEATH' for x in cpu.log)),
               logs(cpu, ('MESS',)), q))

# ---------------------------------------------------------------- X: una vuelta de D1 (DEATH)
i1 = [k for k in range(lab('D1'), len(L)) if code(L[k]) == 'LDA FRIN+4'][0]
d1 = rng(lab('D1'), i1) + ['RTS'] + routine('Ze') + dornd + routine('ZINF') + rng(lab('fq1'), lab('FRMIS'))   # Ze cae en DORND2
cpu, S = build(d1, None)
xx21 = S.get('XX21')
for _ in range(n):
    rnd = [rb() for _ in range(4)]; c = rb() & 1; delta = pick(0, 12, 3, 40)
    got = []
    def nw(cc):
        got.append((cc.a, bytes(cc.mem[S['INWK']:S['INWK'] + NI])))
        k3 = S['K%'] + 3 * NI; cc.mem[S['INF']] = k3 & 255; cc.mem[S['INF'] + 1] = k3 >> 8; cc.c = 1
    cpu.hooks[S['NWSHP']] = nw
    if xx21 is not None: cpu.mem[xx21 - 1 + 2 * 4] = 0x12          # el plano de la placa existe
    cpu.mem[S['RAND']:S['RAND'] + 4] = bytes(rnd); cpu.mem[S['DELTA']] = delta; cpu.c = c
    cpu.run(S['D1'])
    k3 = S['K%'] + 3 * NI
    out.append('X %s %d %d %d %s %d %d %d %s' % (hx(rnd), c, delta, got[0][0], hx(got[0][1]), cpu.mem[k3 + 31],
               cpu.mem[S['MCNT']], cpu.mem[S['LASCT']], hx(cpu.mem[S['RAND']:S['RAND'] + 4])))

open('tests/vectors_damage.txt', 'w').write('\n'.join(out) + '\n')
print(len(out), 'vectores')
