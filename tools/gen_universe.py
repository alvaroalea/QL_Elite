#!/usr/bin/env python3
"""Genera tests/vectors_univ.txt ejecutando en el emulador de 6502 el codigo ORIGINAL de:
   G  las 8 galaxias: semillas de galaxia (G1 de Ghy), TT81, TT20/TT54, TT24 y cpl (nombre) de los 256 sistemas
   N  TT111 (sistema mas cercano a un punto, distancia QQ8 con SQUA2 y LL5) y jmp
   V  GVL (QQ26 con DORND y disponibilidad AVL) y TT151 (precio QQ24 y GC2)
   W  secuencias de NWSHP y KILLSHP (con KS2) sobre K%, FRIN, MANY, JUNK y el monton de lineas (SHIPS.bin de los binarios
      de referencia en $A300); ZINF
Requiere ELITE_SRC. Uso: python3 tools/gen_universe.py [n] [semilla]"""
import sys, random, re, os
sys.path.insert(0, 'tools')
from frag6502 import *

n = int(sys.argv[1]) if len(sys.argv) > 1 else 2000
random.seed(int(sys.argv[2]) if len(sys.argv) > 2 else 1985)
hx = lambda b: ''.join('%02x' % x for x in b)
def rb(): return random.randrange(256)
def pick(*v): return random.choice(v)
out = []
SRC = os.environ['ELITE_SRC']
REF = os.path.join(SRC, '..', '..', '4-reference-binaries', 'ib-disk')
QQ21_0 = [0x4A, 0x5A, 0x48, 0x02, 0x53, 0xB7]       # semillas de la galaxia 1 (JAMESON)

# ---------------------------------------------------------------- G: galaxias
gal = ['.GALX', ' LDX #5'] + rng(lab('G1'), lab('zZ')) + [' RTS']
math_ = rng(nl('SQUA2') - 1, nx(nl('MU11')) - 1) + rng(nl('LL5') - 1, nx(nl('LL5')) - 1)
uni = gal + routine('TT81') + routine('TT20') + routine('TT54') + routine('TT24') + routine('cpl') + routine('TT111') + math_ + routine('jmp')
cpu, S = build(uni, None)
names = []
cpu.hooks[S['TT27']] = lambda c: names.append(c.a)
QQ16 = []
for l in preprocess(L[nl('QQ16') - 1:nx(nl('QQ16')) - 1]):
    m = re.match(r'^\s*EQUS\s+"(..)"', l)
    if m: QQ16 += [ord(ch) for ch in m.group(1)]
def name():
    del names[:]
    cpu.mem[S['QQ17']] = 0xFF
    cpu.run(S['cpl'])
    s = ''
    for a in names:
        k = (a & 127) * 2; s += chr(QQ16[k]) + ('' if QQ16[k + 1] == ord('?') else chr(QQ16[k + 1]))
    return s
cpu.mem[S['QQ21']:S['QQ21'] + 6] = bytes(QQ21_0)
for g in range(8):
    if g: cpu.run(S['GALX'])
    out.append('S %d %s' % (g, hx(cpu.mem[S['QQ21']:S['QQ21'] + 6])))
    cpu.run(S['TT81'])
    for i in range(256):
        cpu.run(S['TT24'])
        m = cpu.mem
        out.append('G %d %d %s %d %d %d %d %d %d %s' % (g, i, hx(m[S['QQ15']:S['QQ15'] + 6]), m[S['QQ3']], m[S['QQ4']], m[S['QQ5']],
                   m[S['QQ6']], m[S['QQ7']], m[S['QQ7'] + 1], name()))
        cpu.run(S['TT20'])

# ---------------------------------------------------------------- N: TT111 y jmp
seeds = []
cpu.mem[S['QQ21']:S['QQ21'] + 6] = bytes(QQ21_0)
for g in range(8):
    seeds.append(bytes(cpu.mem[S['QQ21']:S['QQ21'] + 6])); cpu.run(S['GALX'])
for _ in range(n):
    g = random.randrange(8)
    x, y, x0, y0 = rb(), rb(), rb(), rb()
    cpu.mem[S['QQ21']:S['QQ21'] + 6] = seeds[g]
    cpu.mem[S['QQ9']] = x; cpu.mem[S['QQ10']] = y; cpu.mem[S['QQ0']] = x0; cpu.mem[S['QQ1']] = y0
    cpu.mem[S['ZZ']] = 0xEE; cpu.mem[S['QQ19']:S['QQ19'] + 6] = bytes(6)
    cpu.run(S['TT111'])
    m = cpu.mem
    r = '%d %s %d %d %d %d' % (m[S['ZZ']], hx(m[S['QQ15']:S['QQ15'] + 6]), m[S['QQ9']], m[S['QQ10']], m[S['QQ8']] | (m[S['QQ8'] + 1] << 8), m[S['QQ3']])
    cpu.run(S['jmp'])
    out.append('N %d %d %d %d %d | %s %d %d' % (g, x, y, x0, y0, r, cpu.mem[S['QQ0']], cpu.mem[S['QQ1']]))

# ---------------------------------------------------------------- V: GVL y TT151
def items(lines):                                  # macro ITEM price, factor, units, quantity, mask
    o = []
    for l in lines:
        m = re.match(r'^\s*ITEM\s+(.*)$', code(l))
        d = re.match(r'^EQUD\s+\$([0-9A-Fa-f]+)$', code(l))
        if d: v = int(d.group(1), 16); o.append(' EQUB ' + ', '.join(str((v >> (8 * i)) & 255) for i in range(4))); continue
        if not m: o.append(l); continue
        a = [x.strip() for x in m.group(1).split(',')]
        f = int(a[1]); u = {'"t"': 0, '"k"': 32}.get(a[2], 64)
        o.append(' EQUB %d, %d, %d, %d' % (int(a[0]), (128 if f < 0 else 0) + u + abs(f), int(a[3]), int(a[4].replace('%', '0b'), 0)))
    return o
mk = routine('GVL') + routine('var') + routine('TT151') + routine('TT162') + routine('GC2') + items(preprocess(L[nl('TENS') - 1:nx(nl('BPRNT')) - 1])) + dornd + items(preprocess(L[nl('QQ23') - 1:nx(nl('QQ23')) - 1]))
cpu, S = build(mk, None)
price = []
cpu.hooks[S['pr5']] = lambda c: price.append(c.x | (c.y << 8))
for _ in range(n):
    rnd = [rb() for _ in range(4)]; c0 = rb() & 1; eco = random.randrange(8)
    cpu.mem[S['RAND']:S['RAND'] + 4] = bytes(rnd); cpu.mem[S['QQ28']] = eco; cpu.mem[S['MJ']] = 0
    cpu.mem[S['AVL']:S['AVL'] + 17] = bytes([0x55] * 17)
    cpu.c = c0
    cpu.run(S['GVL'])
    q26 = cpu.mem[S['QQ26']]
    avl = bytes(cpu.mem[S['AVL']:S['AVL'] + 17])
    pr = []
    for it in range(17):
        del price[:]
        cpu.a = it; cpu.c = 0
        cpu.run(S['TT151'])
        pr.append(price[0] if price else -1)
    out.append('V %s %d %d | %d %s %s %s' % (hx(rnd), c0, eco, q26, hx(avl), hx(cpu.mem[S['RAND']:S['RAND'] + 4]), ','.join(map(str, pr))))

# ---------------------------------------------------------------- W: NWSHP, KILLSHP (KS2) y ZINF
ks = routine('NWSHP') + routine('GINF') + ['.UNIV'] + [' EQUW K%% + %d' % (i * 37) for i in range(9)] + routine('KS4') + routine('KILLSHP') + routine('KS2') + routine('KS3') + routine('ZINF')
cpu, S = build(ks, None)
cpu.hooks[S['ABORT']] = lambda c: c.wr(S['MSTG'], 0xFF)          # ABORT: MSTG = $FF
ships = open(os.path.join(REF, 'SHIPS.bin'), 'rb').read()
NI_, NOSH_ = S['NI%'], S['NOSH']
E_ = table('E%')
def reset():
    for k in range(0x800, 0xB60): cpu.mem[k] = 0
    cpu.mem[S['XX21']:S['XX21'] + len(ships)] = ships
    for k in range(NOSH_ + 1): cpu.mem[S['FRIN'] + k] = 0
    for k in range(40): cpu.mem[S['MANY'] + k] = 0
    cpu.mem[S['JUNK']] = 0; cpu.mem[S['MSTG']] = 0xFF; cpu.mem[S['TP']] = 0; cpu.mem[S['TALLY'] + 1] = 0
    cpu.mem[S['SLSP']] = S['LS%'] & 255; cpu.mem[S['SLSP'] + 1] = S['LS%'] >> 8
def state():
    m = cpu.mem; s = hx(m[S['FRIN']:S['FRIN'] + NOSH_ + 1]) + ' ' + hx(m[S['MANY']:S['MANY'] + 33]) + ' %d %d %d %d ' % (
        m[S['JUNK']], m[S['TP']], m[S['TALLY'] + 1], m[S['MSTG']])
    blk = []
    for k in range(NOSH_):
        b = bytearray(m[S['K%'] + k * NI_:S['K%'] + (k + 1) * NI_])
        if m[S['FRIN'] + k] == 0: b = bytearray(NI_)             # ranura libre: lo que quede en K% no cuenta
        blk.append(hx(b))
    return s + ','.join(blk) + ' %d' % (m[S['SLSP']] | (m[S['SLSP'] + 1] << 8))
fails = 0
for _ in range(n // 4):
    reset()
    ops = []
    for step in range(random.randrange(3, 14)):
        if random.random() < 0.65 or not any(cpu.mem[S['FRIN'] + k] for k in range(NOSH_)):
            t = pick(random.randrange(1, 33), 5, 7, 11, 16, 17, 31, 15, 2, 3)
            iw = [rb() for _ in range(NI_)]
            cpu.mem[S['INWK']:S['INWK'] + NI_] = bytes(iw)
            cpu.a = t
            cpu.run(S['NWSHP'])
            ops.append('n%d:%s:%d' % (t, hx(iw), cpu.c))
            if not cpu.c: fails += 1
        else:
            occ = [k for k in range(NOSH_) if cpu.mem[S['FRIN'] + k]]
            x = random.choice(occ)
            if cpu.mem[S['FRIN'] + x] == S['SST']: continue
            if random.random() < 0.3: cpu.mem[S['MSTG']] = random.choice(occ)
            ops.append('k%d:%d' % (x, cpu.mem[S['MSTG']]))
            t = cpu.mem[S['FRIN'] + x]                # quien llama (KS1, en el bucle de vuelo) tiene INF y XX0 de esa nave
            inf = S['K%'] + x * NI_
            cpu.mem[S['INF']] = inf & 255; cpu.mem[S['INF'] + 1] = inf >> 8
            cpu.mem[S['XX0']] = cpu.mem[S['XX21'] + 2 * t - 2]; cpu.mem[S['XX0'] + 1] = cpu.mem[S['XX21'] + 2 * t - 1]
            cpu.x = x
            cpu.run(S['KILLSHP'])
    out.append('W %s | %s' % (' '.join(ops), state()))
print('W: NWSHP sin sitio (monton o ranuras):', fails, file=sys.stderr)
for _ in range(20):
    iw = [rb() for _ in range(NI_)]
    cpu.mem[S['INWK']:S['INWK'] + NI_] = bytes(iw)
    cpu.run(S['ZINF'])
    out.append('Z %s | %s' % (hx(iw), hx(cpu.mem[S['INWK']:S['INWK'] + NI_])))

# ---------------------------------------------------------------- F: fuente (FONT del Apple II, la que dibuja letter)
fnt = open(os.path.join(SRC, '..', 'fonts', 'A.FLOWY.bin'), 'rb').read()
for c in range(32, 128): out.append('F %d %s' % (c, hx(fnt[(c - 32) * 8:(c - 31) * 8])))

open('tests/vectors_univ.txt', 'w').write('\n'.join(out) + '\n')
print(len(out), 'vectores')
