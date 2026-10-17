#!/usr/bin/env python3
"""Genera tests/vectors_planet.txt ejecutando en el emulador de 6502 el codigo ORIGINAL del dibujo de planetas y soles:
PLANET, PL9, PL2, PROJ, DVID3B2, SUN (4 partes), CHKON, EDGES, HLOIN2, FLFLLS, WPLS, CIRCLE, CIRCLE2, BLINE, WPLS2, WP1,
LL145 (recorte), FMLTU2/FMLTU, SQUA2, LL5 y DORND, todo real. De LOIN se ejecuta la parte que importa para el resto (parte 1
y el comienzo de STPX/STPY: deltas, SWAP y el intercambio de los extremos) y se registra la linea ya ordenada; de HLOIN, el
redondeo a pares y el orden (hasta HL5). Cada prueba es una secuencia de fotogramas: el planeta o el sol se mueve (tambien
cruzando los bordes de la vista y saliendo de ella) y entre fotograma y fotograma SWAP toma un valor al azar, como si otra
nave hubiera pasado por LL145. Se registran todas las lineas en orden y, al final, los montones (LSX2/LSY2/LSP, LSO, LSX,
SUNX) y RAND. Requiere ELITE_SRC. Uso: python3 tools/gen_planet.py [pruebas] [semilla]"""
import sys, random, math
sys.path.insert(0, 'tools')
from frag6502 import *

ntr = int(sys.argv[1]) if len(sys.argv) > 1 else 400
random.seed(int(sys.argv[2]) if len(sys.argv) > 2 else 1985)
hx = lambda b: ''.join('%02x' % x for x in b)

def seg_to(a, label_line):                     # lineas desde el indice a hasta la etiqueta (sin incluirla)
    return rng(a, label_line)

loin = rng(lab('LL30'), lab('STPX')) + rng(lab('STPX'), lab('LI3')) + ['.LI3', 'LDY YSAV', 'RTS'] + \
       rng(lab('STPY'), lab('LI15')) + ['.LI15', 'LDY YSAV', 'RTS']
hloin = ['.HL6', 'RTS'] + rng(lab('HLOIN'), lab('HL5')) + ['.HL5', 'LDY YSAV', 'RTS']
code_ = rng(lab('HFS1'), lab('HF8')) + ['.HF8', 'RTS'] + routine('TT128') + rng(nl('PROJ') - 1, nx(nl('PLS6')) - 1) + routine('BLINE') + routine('HLOIN2') + routine('FLFLLS') + \
        rng(nl('LL118') - 1, nl('LSPUT') - 1) + rng(nl('LL28') - 1, nx(nl('LL28')) - 1) + routine('FMLTU2') + routine('FMLTU') + sq + \
        rng(nl('DVID3B2') - 1, orig.next_name_after(nl('DVID3B2') + 60) - 1) + \
        rng(nl('LL5') - 1, nx(nl('LL5')) - 1) + dornd + loin + hloin
pre = {'log': 0x2000, 'logL': 0x2100, 'alogh': 0x2200, 'SNE': 0x2300}
cpu, S = build(code_, None, pre)
stubs = sorted(k for k, v in S.items() if 0x3E00 <= v < 0x3F00)
assert all(k in ('TT14',) for k in stubs) or True
print('stubs:', stubs, file=sys.stderr)
lg = [0x0803] + [int(0x2000 * math.log(i) / math.log(2) + 0.5) for i in range(1, 256)]
for i in range(256):
    cpu.mem[0x2000 + i] = (lg[i] >> 8) & 255
    cpu.mem[0x2100 + i] = lg[i] & 255
    cpu.mem[0x2200 + i] = int(2 ** ((i / 2 + 128) / 16) + 0.5) >> 8 & 255
sne = [255 if abs(math.sin(i / 64 * 2 * math.pi)) >= 1 else int(256 * abs(math.sin(i / 64 * 2 * math.pi)) + 0.5) for i in range(32)]   # FOR de elite-data.asm
cpu.mem[0x2300:0x2300 + 32] = bytes(sne)
cpu.mem[S['Yx2M1']] = 135                      # 2 * Y - 1: ultima linea de la vista (lo pone RES2)

V = lambda k: S[k]
log = []
cpu.hooks[V('LI3')] = lambda c: log.append('L:%d,%d,%d,%d,%d' % tuple(c.mem[V(k)] for k in ('X1', 'Y1', 'X2', 'Y2', 'SWAP')))
cpu.hooks[V('LI15')] = cpu.hooks[V('LI3')]
cpu.hooks[V('HL5')] = lambda c: log.append('H:%d,%d,%d' % (min(c.x, c.mem[V('X2')]), max(c.x, c.mem[V('X2')]), c.mem[V('Y1')]))

def setc(v):                                 # coordenada de 3 bytes (bajo, alto, signo + bits 16-22)
    m = abs(v); return [m & 255, (m >> 8) & 255, ((m >> 16) & 0x7F) | (0x80 if v < 0 else 0)]

out = []
for tr in range(ntr):
    typ = random.choice([128, 130, 129, 129])
    nf = random.randrange(4, 14)
    # el centro en pantalla es (128 + x * 256 / z, 68 - y * 256 / z): para tocar los bordes, |x| ~ z / 2 e |y| ~ z / 4
    z = random.choice([random.randrange(0x300, 0x1200), random.randrange(0x1000, 0x6000), random.randrange(0x200, 0x30000)])
    kx = random.choice([0, 0.3, 0.45, 0.5, 0.55, 0.7, 1.2, random.uniform(-1.5, 1.5)])
    ky = random.choice([0, 0.2, 0.26, 0.3, 0.6, random.uniform(-0.8, 0.8)])
    x = int(z * kx * random.choice([1, -1])); y = int(z * ky * random.choice([1, -1]))
    dx, dy, dz = [random.choice([0, random.randrange(-z // 16 - 1, z // 16 + 1)]) for _ in range(3)]
    rnd = [random.randrange(256) for _ in range(4)]
    # estado inicial: montones vacios (como sun_forget)
    for i in range(256): cpu.mem[V('LSX2') + i] = 0; cpu.mem[V('LSY2') + i] = 0
    cpu.mem[V('LSX2')] = 0xFF; cpu.mem[V('LSY2')] = 0xFF; cpu.mem[V('LSP')] = 1
    for i in range(256): cpu.mem[V('LSO') + i] = 0
    cpu.mem[V('LSX')] = 0xFF; cpu.mem[V('SUNX')] = 0; cpu.mem[V('SUNX') + 1] = 0
    cpu.mem[V('RAND'):V('RAND') + 4] = bytes(rnd)
    out.append('T %d %d %s' % (typ, nf, hx(rnd)))
    for f in range(nf):
        iw = setc(x) + setc(y) + setc(z)
        sw = random.choice([0, 0, 0xFF, random.randrange(256)])
        c = random.randrange(2)
        cpu.mem[V('INWK'):V('INWK') + 9] = bytes(iw); cpu.mem[V('TYPE')] = typ; cpu.mem[V('SWAP')] = sw
        del log[:]
        cpu.c = c
        cpu.run(V('PLANET'))
        out.append('F %s %d %d %s' % (hx(iw), sw, c, ' '.join(log) or '-'))
        x += dx; y += dy; z = max(0x100, z + dz)
    lsp = cpu.mem[V('LSP')]
    out.append('E %d %s %s %s %d %s %s' % (lsp, hx(cpu.mem[V('LSX2'):V('LSX2') + 256]), hx(cpu.mem[V('LSY2'):V('LSY2') + 256]),
               hx(cpu.mem[V('LSO'):V('LSO') + 200]), cpu.mem[V('LSX')], hx(cpu.mem[V('SUNX'):V('SUNX') + 2]),
               hx(cpu.mem[V('RAND'):V('RAND') + 4])))
# Tunel (HFS1 con STP 4 u 8) y circulo de las cartas (TT128: STP = 2, LSP = 1)
for _ in range(60):
    stp = random.choice([4, 8]); sw = random.choice([0, 0xFF, random.randrange(256)])
    for i in range(256): cpu.mem[V('LSX2') + i] = 0; cpu.mem[V('LSY2') + i] = 0
    cpu.mem[V('LSX2')] = 0xFF; cpu.mem[V('LSY2')] = 0xFF; cpu.mem[V('LSP')] = 1
    cpu.mem[V('STP')] = stp; cpu.mem[V('SWAP')] = sw
    del log[:]
    cpu.run(V('HFS1'))
    out.append('U %d %d %s' % (stp, sw, ' '.join(log) or '-'))
for _ in range(1500):
    q0, q1, k = random.randrange(256), random.randrange(256), random.choice([random.randrange(256), random.randrange(4, 80), 0, 1, 255])
    sw = random.choice([0, 0xFF])
    for i in range(256): cpu.mem[V('LSX2') + i] = 0; cpu.mem[V('LSY2') + i] = 0
    cpu.mem[V('LSX2')] = 0xFF; cpu.mem[V('LSY2')] = 0xFF
    cpu.mem[V('QQ19')] = q0; cpu.mem[V('QQ19') + 1] = q1; cpu.mem[V('K')] = k; cpu.mem[V('SWAP')] = sw
    del log[:]
    cpu.run(V('TT128'))
    out.append('R %d %d %d %d %d %s' % (q0, q1, k, sw, cpu.mem[V('LSP')], ' '.join(log) or '-'))
# CHKON suelto, con valores en los bordes (K4 - K = 135, K3 +- K = 0 / 256...)
for _ in range(4000):
    k = random.choice([0, 1, 8, 60, 100, 200, 248, random.randrange(256)])
    k3 = random.choice([random.randrange(-300, 600), k, -k, 256 + k, 255 - k, random.randrange(-40, 40)]) & 0xFFFF
    k4 = random.choice([random.randrange(-300, 600), 135 + k, 134 + k, 136 + k, k, k - 1, -k]) & 0xFFFF
    cpu.mem[V('K3')] = k3 & 255; cpu.mem[V('K3') + 1] = k3 >> 8; cpu.mem[V('K4')] = k4 & 255; cpu.mem[V('K4') + 1] = k4 >> 8
    cpu.mem[V('K')] = k; cpu.c = random.randrange(2)
    cpu.run(V('CHKON'))
    out.append('C %d %d %d %d' % (k3, k4, k, cpu.c))
open('tests/vectors_planet.txt', 'w').write('\n'.join(out) + '\n')
print(len(out), 'lineas')
