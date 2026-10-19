#!/usr/bin/env python3
"""Compara src/ql_ell.asm (ql_ellipse y ql_hline, ejecutadas con tools/m68k_mini.py) con gfx_ellipse_xor y gfx_hline_xor en C
(tests/ell_dump.c): los mismos bytes de pantalla en miles de elipses (con recorte) y tramos. Uso: python3 tools/test_ellasm.py [n]"""
import os, random, subprocess, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import m68k_mini

root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
n = int(sys.argv[1]) if len(sys.argv) > 1 else 600
src = open(os.path.join(root, 'src', 'ql_ell.asm')).read()
rnd = random.Random(11)
SCR = 0x20000
cases = []
for i in range(n):
    rx = rnd.randrange(1, 70) if i % 3 else rnd.randrange(1, 12)
    ry = rx + rx // 5 if i % 2 else rnd.randrange(1, 90)
    cx, cy = rnd.randrange(-40, 300), rnd.randrange(-40, 200)
    clip = (0, 0, 255, 159) if i % 4 else (0, 0, 255, 255)
    cases.append(('E', cx, cy, rx, ry, rnd.randrange(8)) + clip)
for rx in (1, 2, 3, 5, 60, 120, 248):                      # las del juego, centradas
    cases.append(('E', 128, 80, rx, rx + rx // 5, 1, 0, 0, 255, 159))
for i in range(n):
    x0, x1 = rnd.randrange(256), rnd.randrange(256)
    if i % 5 == 0: x1 = x0
    if i % 5 == 1: x1 = min(255, x0 + rnd.randrange(0, 4))
    if x0 > x1: x0, x1 = x1, x0
    cases.append(('H', x0, x1, rnd.randrange(256), rnd.randrange(8)))
for i in range(n):                                          # columnas
    x, y0, y1 = rnd.randrange(-5, 262), rnd.randrange(-20, 270), rnd.randrange(-20, 270)
    if i % 5 == 0: y1 = y0
    cases.append(('V', x, y0, y1, rnd.randrange(8)))
for i in range(n * 2):                                      # puntos sueltos (tambien fuera de pantalla)
    cases.append(('P', rnd.randrange(-4, 260), rnd.randrange(-4, 260), rnd.randrange(8)))
for i in range(n):                                          # rectangulos de celdas de texto (pantalla llena de $FF; se cuentan los bytes distintos)
    cases.append(('C', rnd.randrange(-3, 34), rnd.randrange(-3, 34), rnd.randrange(-2, 36), rnd.randrange(-2, 36)))
for i in range(n):                                          # rectangulos con recorte (varios muy grandes, medio fuera)
    hw, hh = rnd.randrange(0, 160), rnd.randrange(0, 110)
    if i % 7 == 0: hw = 0
    if i % 11 == 0: hh = 0
    cx, cy = rnd.randrange(-30, 290), rnd.randrange(-30, 200)
    cases.append(('F', cx, cy, hw, hh, rnd.randrange(8), 0, 0, 255, 159))
for r in (8, 15, 30, 60, 120, 240):                         # los anillos del despegue (semialto = 5/8 del semiancho)
    cases.append(('F', 128, 80, r, max(1, r * 5 // 8), 1, 0, 0, 255, 159))
tmp = os.path.join(root, 'tests', 'tmp', 'ellasm_cases.txt')
os.makedirs(os.path.dirname(tmp), exist_ok=True)
with open(tmp, 'w') as f:
    for c in cases: f.write(' '.join(str(x) for x in c) + '\n')
exe = os.path.join(root, 'tests', 'ell_dump')
out = subprocess.run([exe, tmp], capture_output=True, text=True, check=True).stdout.split('\n')

def setup_ellipse(cpu, cx, cy, rx, ry, c, clx0, cly0, clx1, cly1):
    """el mismo preparado que gfx_ellipse_xor (C); None si el C no llega a dibujar con el asm"""
    if cx + rx < clx0 or cx - rx > clx1 or cy + ry < cly0 or cy - ry > clx1 * 0 + cly1: return None
    a, b = 2 * rx, 2 * ry
    dx = 4 * (1 - a) * b * b; dy = 4 * a * a
    blk = [8 * a * a, 8 * b * b, 0xAA if c & 4 else 0, (0xAA if c & 2 else 0) | (0x55 if c & 1 else 0),
           clx0, cly0, clx1, cly1, dx, dy, 0, dx + dy, SCR, 2 * ry, cx - rx, cx + rx, cy, cy]
    base = 0x1D000
    for i, v in enumerate(blk): cpu.wr(base + 4 * i, 4, v & 0xFFFFFFFF)
    return base

bad = 0
for i, c in enumerate(cases):
    cpu = m68k_mini.CPU(src)
    cpu.a[7] = 0x1F000
    if c[0] == 'C':
        cpu.mem[SCR:SCR + 32768] = bytes([0xFF]) * 32768
    if c[0] == 'E':
        base = setup_ellipse(cpu, *c[1:])
        if base is not None:
            cpu.a[7] -= 4; cpu.wr(cpu.a[7], 4, base)
            cpu.run('_ql_ellipse', max_steps=5000000)
    elif c[0] == 'F':
        cx, cy, hw, hh, col, clx0, cly0, clx1, cly1 = c[1:]
        blk = [cx - hw, cy - hh, cx + hw, cy + hh, clx0, cly0, clx1, cly1, 0xAA if col & 4 else 0,
               (0xAA if col & 2 else 0) | (0x55 if col & 1 else 0), SCR]
        base = 0x1D000
        for k, v in enumerate(blk): cpu.wr(base + 4 * k, 4, v & 0xFFFFFFFF)
        cpu.a[7] -= 4; cpu.wr(cpu.a[7], 4, base)
        cpu.run('_ql_frame')
    elif c[0] == 'C':
        col, row, nc, nr = c[1:]                                   # el recorte lo hace gfx_clear_cells (C) antes de llamar
        if col < 0: nc += col; col = 0
        if row < 0: nr += row; row = 0
        nc = min(nc, 32 - col); nr = min(nr, 32 - row)
        if nc > 0 and nr > 0:
            for v in reversed([col, row, nc, nr, SCR]):
                cpu.a[7] -= 4; cpu.wr(cpu.a[7], 4, v)
            cpu.run('_ql_clearcells')
    elif c[0] == 'P':
        x, y, col = c[1:]
        if 0 <= x <= 255 and 0 <= y <= 255:                        # el recorte lo hace gfx_xor_pixel (C) antes de llamar
            for v in reversed([x, y, col, SCR]):
                cpu.a[7] -= 4; cpu.wr(cpu.a[7], 4, v)
            cpu.run('_ql_xpix')
    elif c[0] == 'V':
        x, y0, y1, col = c[1:]
        if y0 > y1: y0, y1 = y1, y0
        if not (x < 0 or x > 255 or y1 < 0 or y0 > 255):          # el recorte lo hace gfx_vline_xor (C) antes de llamar
            y0, y1 = max(y0, 0), min(y1, 255)
            for v in reversed([x, y0, y1, col, SCR]):
                cpu.a[7] -= 4; cpu.wr(cpu.a[7], 4, v)
            cpu.run('_ql_vline')
    else:
        x0, x1, y, col = c[1:]
        for v in reversed([x0, x1, y, col, SCR]):
            cpu.a[7] -= 4; cpu.wr(cpu.a[7], 4, v)
        cpu.run('_ql_hline')
    got = {j: cpu.mem[SCR + j] for j in range(32768) if cpu.mem[SCR + j] != (0xFF if c[0] == 'C' else 0)}
    want = {}
    for tok in out[i].split():
        a, b = tok.split(':'); want[int(a)] = int(b)
    if got != want:
        bad += 1
        if bad <= 5: print('DIFERENTE', c, 'asm', len(got), 'bytes, C', len(want), 'bytes')
print('%d casos, %d fallos' % (len(cases), bad))
sys.exit(1 if bad else 0)
