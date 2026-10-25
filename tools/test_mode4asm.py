#!/usr/bin/env python3
"""Compara el ensamblador en MODO 4 (ql_line.asm: ql_line con _gfx_m4 = 1 y ql_line4; ql_ell.asm: ql_ellipse con hr, ql_hline, ql_vline, ql_xpix, ql_frame y
ql_clearcells con los codigos de color de modo 4; ql_ellrows.asm: ql_ellrows4), ejecutado con tools/m68k_mini.py, con las versiones en C
(tests/m4_dump.c, gfx_m4 = 1, sin ASM_LINE): los mismos bytes de pantalla. Uso: python3 tools/test_mode4asm.py [n]"""
import os, random, subprocess, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import m68k_mini

root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
n = int(sys.argv[1]) if len(sys.argv) > 1 else 400
SRC = '\n'.join(open(os.path.join(os.environ.get('ELITE_ASMDIR') or os.path.join(root, 'src'), f)).read() for f in ('ql_line.asm', 'ql_ell.asm', 'ql_ellrows.asm'))
rnd = random.Random(23)
SCR, BLK, OT, NT, M4SYM = 0x20000, 0x1D000, 0x1A000, 0x1B000, 0x1C000
CCM4 = [0, 15, 3, 15, 12, 15, 15, 15]          # gfx_ccode en modo 4 (negro, rojo y verde tal cual, el resto blanco)
GM = lambda cc: (0xAA if cc & 4 else 0) | (0x55 if cc & 8 else 0)
RM = lambda cc: (0xAA if cc & 2 else 0) | (0x55 if cc & 1 else 0)
HG = lambda cc: 0xFF if cc & 4 else 0          # a los 512 puntos: byte llano
HR = lambda cc: 0xFF if cc & 2 else 0

cases = []
for i in range(n * 2):                         # lineas
    x1, y1, x2, y2 = (rnd.randrange(256) for _ in range(4))
    k = i % 8
    if k == 0: y2 = y1
    elif k == 1: x2 = x1
    elif k == 2: y2 = max(0, min(255, y1 + (x2 - x1)))
    elif k == 3: y2 = max(0, min(255, y1 - (x2 - x1)))
    elif k == 4: x1, y1, x2, y2 = x2, y2, x1, y1
    elif k == 5: x2, y2 = x1, y1
    cases.append(('L', x1, y1, x2, y2, rnd.randrange(8)))
for c in range(8): cases.append(('L', 3, 5, 250, 200, c))
for (a, b, c, d) in ((0, 0, 255, 255), (255, 255, 0, 0), (0, 255, 255, 0), (255, 0, 0, 255), (0, 0, 255, 0), (0, 0, 0, 255), (255, 0, 255, 255)):
    cases.append(('L', a, b, c, d, 7))
for i in range(n):                             # elipses a los 512 puntos
    rx = rnd.randrange(1, 70) if i % 3 else rnd.randrange(1, 12)
    ry = rx + rx // 5 if i % 2 else rnd.randrange(1, 90)
    cx, cy = rnd.randrange(-40, 300), rnd.randrange(-40, 200)
    clip = (0, 0, 255, 159) if i % 4 else (0, 0, 255, 255)
    cases.append(('E', cx, cy, rx, ry, rnd.randrange(8)) + clip)
for rx in (1, 2, 3, 5, 60, 120, 248):
    cases.append(('E', 128, 80, rx, (6 * rx + 2) // 5, 1, 0, 0, 255, 159))
for i in range(n):                             # tramos, columnas, puntos, celdas y rectangulos (puntos del juego de 2 puntos fisicos)
    x0, x1 = rnd.randrange(256), rnd.randrange(256)
    if i % 5 == 0: x1 = x0
    if x0 > x1: x0, x1 = x1, x0
    cases.append(('H', x0, x1, rnd.randrange(256), rnd.randrange(8)))
    x, y0, y1 = rnd.randrange(-5, 262), rnd.randrange(-20, 270), rnd.randrange(-20, 270)
    cases.append(('V', x, y0, y1, rnd.randrange(8)))
    cases.append(('P', rnd.randrange(-4, 260), rnd.randrange(-4, 260), rnd.randrange(8)))
    cases.append(('C', rnd.randrange(-3, 34), rnd.randrange(-3, 34), rnd.randrange(-2, 36), rnd.randrange(-2, 36)))
    hw, hh = rnd.randrange(0, 160), rnd.randrange(0, 110)
    cases.append(('F', rnd.randrange(-30, 290), rnd.randrange(-30, 200), hw, hh, rnd.randrange(8), 0, 0, 255, 159))
for i in range(n):                             # planeta fila a fila (ocx, ocy, orx, vieja?, ncx, ncy, nrx, nueva?, c, cly1)
    orx = 1 + rnd.randrange(248); nrx = orx if rnd.randrange(3) == 0 else 1 + rnd.randrange(248)
    if i % 4 == 0: orx, nrx = 1 + rnd.randrange(14), 1 + rnd.randrange(14)
    ocx, ocy = rnd.randrange(-120, 380), rnd.randrange(-160, 240)
    ncx = ocx if rnd.randrange(3) == 0 else ocx + rnd.randrange(9) - 4
    ncy = ocy if rnd.randrange(3) == 0 else ocy + rnd.randrange(9) - 4
    if i % 3 == 0: ocx, ocy = 128 + rnd.randrange(80) - 40, 80 + rnd.randrange(40) - 20; ncx, ncy = ocx + rnd.randrange(3) - 1, ocy + rnd.randrange(3) - 1
    om, nm = (0, 1) if i % 8 == 0 else ((1, 0) if i % 8 == 1 else (1, 1))
    cases.append(('R', ocx, ocy, orx, om, ncx, ncy, nrx, nm, 1 + rnd.randrange(7), 159 if rnd.randrange(2) else 135))

tmp = os.path.join(root, 'tests', 'tmp', 'mode4asm_cases.txt')
os.makedirs(os.path.dirname(tmp), exist_ok=True)
with open(tmp, 'w') as f:
    for c in cases: f.write(' '.join(str(x) for x in c) + '\n')
out = subprocess.run([os.path.join(root, 'tests', 'm4_dump'), tmp], capture_output=True, text=True, check=True).stdout.split('\n')

def push(cpu, args):
    for v in reversed(args):
        cpu.a[7] -= 4; cpu.wr(cpu.a[7], 4, v & 0xFFFFFFFF)

def mk():
    cpu = m68k_mini.CPU(SRC)
    cpu.a[7] = 0x1F000
    cpu.syms['_gfx_m4'] = M4SYM
    cpu.mem[M4SYM] = 1
    return cpu

bad = 0
for i, c in enumerate(cases):
    cpu = mk()
    full = c[0] in 'CR'
    if full: cpu.mem[SCR:SCR + 32768] = bytes([0xFF]) * 32768
    k = c[0]
    if k == 'L':
        push(cpu, list(c[1:]) + [SCR]); cpu.run('_ql_line')           # por el despachador (mira _gfx_m4)
    elif k == 'E':
        cx, cy, rx, ry, col, clx0, cly0, clx1, cly1 = c[1:]
        if rx < 0: rx = -rx
        if not (cx + rx < clx0 or cx - rx > clx1):
            cx, rx, clx0, clx1 = 2 * cx, 2 * rx, 2 * clx0, 2 * clx1 + 1
            if not (cy + ry < cly0 or cy - ry > cly1):
                cc = CCM4[col]
                a, b = 2 * rx, 2 * ry
                dx = 4 * (1 - a) * b * b; dy = 4 * a * a
                blk = [8 * a * a, 8 * b * b, HG(cc), HR(cc), clx0, cly0, clx1, cly1, dx, dy, 0, dx + dy, SCR, 2 * ry, cx - rx, cx + rx, cy, cy, 1]
                for j, v in enumerate(blk): cpu.wr(BLK + 4 * j, 4, v & 0xFFFFFFFF)
                push(cpu, [BLK]); cpu.run('_ql_ellipse', max_steps=8000000)
    elif k == 'F':
        cx, cy, hw, hh, col, clx0, cly0, clx1, cly1 = c[1:]
        cc = CCM4[col]
        blk = [cx - hw, cy - hh, cx + hw, cy + hh, clx0, cly0, clx1, cly1, GM(cc), RM(cc), SCR]
        for j, v in enumerate(blk): cpu.wr(BLK + 4 * j, 4, v & 0xFFFFFFFF)
        push(cpu, [BLK]); cpu.run('_ql_frame')
    elif k == 'C':
        col, row, nc, nr = c[1:]
        if col < 0: nc += col; col = 0
        if row < 0: nr += row; row = 0
        nc = min(nc, 32 - col); nr = min(nr, 32 - row)
        if nc > 0 and nr > 0:
            push(cpu, [col, row, nc, nr, SCR]); cpu.run('_ql_clearcells')
    elif k == 'P':
        x, y, col = c[1:]
        if 0 <= x <= 255 and 0 <= y <= 255:
            push(cpu, [x, y, CCM4[col], SCR]); cpu.run('_ql_xpix')
    elif k == 'V':
        x, y0, y1, col = c[1:]
        if y0 > y1: y0, y1 = y1, y0
        if not (x < 0 or x > 255 or y1 < 0 or y0 > 255):
            y0, y1 = max(y0, 0), min(y1, 255)
            push(cpu, [x, y0, y1, CCM4[col], SCR]); cpu.run('_ql_vline')
    elif k == 'H':
        x0, x1, y, col = c[1:]
        push(cpu, [x0, x1, y, CCM4[col], SCR]); cpu.run('_ql_hline')
    else:                                                              # R
        ocx, ocy, orx, om, ncx, ncy, nrx, nm, col, cly1 = c[1:]
        h = out[i].split()
        ory, nry = int(h[0]), int(h[1])
        otab = bytes.fromhex(h[2][2:]); ntab = bytes.fromhex(h[3][2:])
        ory_, nry_ = (ory if om else -1), (nry if nm else -1)
        top, bot = 32767, -32768
        if ory_ >= 0: top, bot = ocy - ory_, ocy + ory_
        if nry_ >= 0: top, bot = min(top, ncy - nry_), max(bot, ncy + nry_)
        top, bot = max(top, 0), min(bot, cly1)
        if top <= bot:
            cc = CCM4[col]
            cpu.mem[OT:OT + len(otab)] = otab; cpu.mem[NT:NT + len(ntab)] = ntab
            blk = [top, bot, 2 * ocx, ocy, ory_, OT, 2 * ncx, ncy, nry_, NT, GM(cc) and HG(cc) or HG(cc), HR(cc), SCR]
            for j, v in enumerate(blk): cpu.wr(BLK + 4 * j, 4, v & 0xFFFFFFFF)
            push(cpu, [BLK]); cpu.run('_ql_ellrows4', max_steps=30000000)
    base = 0xFF if full else 0
    got = {j: cpu.mem[SCR + j] for j in range(32768) if cpu.mem[SCR + j] != base}
    want = {}
    text = out[i].split('out=')[1] if k == 'R' else out[i]
    for tok in text.split():
        a, b = tok.split(':'); want[int(a)] = int(b)
    if got != want:
        bad += 1
        if bad <= 8: print('DIFERENTE', c, 'asm', len(got), 'bytes, C', len(want), 'bytes')
print('%d casos, %d fallos' % (len(cases), bad))
sys.exit(1 if bad else 0)
