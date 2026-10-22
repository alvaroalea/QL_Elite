#!/usr/bin/env python3
"""Compara src/ql_ellrows.asm (ql_ellrows, ejecutada con tools/m68k_mini.py) con gfx_ell_update en C (tests/ellrows_dump): la misma pantalla
(partiendo de $FF) en miles de elipses con recorte, sin vieja o sin nueva, y las que apenas cambian. uso: test_ellrowsasm.py [casos] [semilla]"""
import os, re, subprocess, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import m68k_mini

root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
N = int(sys.argv[1]) if len(sys.argv) > 1 else 300
SEED = sys.argv[2] if len(sys.argv) > 2 else '1'
src = open(os.environ.get('ELLROWSASM') or os.path.join(root, 'src', 'ql_ellrows.asm')).read()
out = subprocess.run([os.path.join(root, 'tests', 'ellrows_dump'), str(N), SEED], capture_output=True, text=True, check=True).stdout.split('\n')
SCR, OT, NT, BLK = 0x20000, 0x1A000, 0x1B000, 0x1D000
bad = total = steps = 0
for line in out:
    if not line.startswith('CASE'): continue
    h = line.split()
    ocx, ocy, ory, ncx, ncy, nry, col, cly1, k = (int(x) for x in h[1:10])
    otab = bytes.fromhex(re.search(r'otab=([0-9a-f]*)', line).group(1)); ntab = bytes.fromhex(re.search(r'ntab=([0-9a-f]*)', line).group(1))
    want = {}
    for tok in line.split('out=')[1].split():
        a, b = tok.split(':'); want[int(a)] = int(b)
    top, bot = 32767, -32768
    if ory >= 0: top, bot = ocy - ory, ocy + ory
    if nry >= 0: top, bot = min(top, ncy - nry), max(bot, ncy + nry)
    top, bot = max(top, 0), min(bot, cly1)
    total += 1
    got = {}
    if top <= bot:
        cpu = m68k_mini.CPU(src)
        cpu.a[7] = 0x1F000
        cpu.mem[SCR:SCR + 32768] = bytes([0xFF]) * 32768
        cpu.mem[OT:OT + len(otab)] = otab; cpu.mem[NT:NT + len(ntab)] = ntab
        blk = [top, bot, ocx, ocy, ory, OT, ncx, ncy, nry, NT, 0xAA if col & 4 else 0, (0xAA if col & 2 else 0) | (0x55 if col & 1 else 0), SCR]
        for i, v in enumerate(blk): cpu.wr(BLK + 4 * i, 4, v & 0xFFFFFFFF)
        cpu.a[7] -= 4; cpu.wr(cpu.a[7], 4, BLK)
        steps += cpu.run('_ql_ellrows', max_steps=20000000)
        got = {j: cpu.mem[SCR + j] for j in range(32768) if cpu.mem[SCR + j] != 0xFF}
    if got != want:
        bad += 1
        if bad <= 5: print('DIFERENTE caso', k, 'old', (ocx, ocy, ory), 'new', (ncx, ncy, nry), 'asm', len(got), 'bytes, C', len(want))
print('%d casos (%d instrucciones), %d fallos' % (total, steps, bad))
sys.exit(1 if bad else 0)
