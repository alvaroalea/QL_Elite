#!/usr/bin/env python3
"""Compares src/ql_line.asm (run with tools/m68k_mini.py) with loin_bres_c (C, tests/test_bres dump) over thousands of lines:
the same screen bytes. Usage: python3 tools/test_lineasm.py [n]"""
import os, random, subprocess, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import m68k_mini

root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
n = int(sys.argv[1]) if len(sys.argv) > 1 else 3000
src = open(os.path.join(root, 'src', 'ql_line.asm')).read()
rnd = random.Random(7)
cases = []
for i in range(n):
    x1, y1, x2, y2 = (rnd.randrange(256) for _ in range(4))
    k = i % 8
    if k == 0: y2 = y1
    elif k == 1: x2 = x1
    elif k == 2: y2 = max(0, min(255, y1 + (x2 - x1)))
    elif k == 3: y2 = max(0, min(255, y1 - (x2 - x1)))
    elif k == 4: x1, y1, x2, y2 = x2, y2, x1, y1
    elif k == 5: x2, y2 = x1, y1                        # a single dot
    cases.append((x1, y1, x2, y2, rnd.randrange(8)))
for c in range(8):                                      # the 8 colours on a fixed line, and the screen corners
    cases.append((3, 5, 250, 200, c))
for (a, b, c, d) in ((0, 0, 255, 255), (255, 255, 0, 0), (0, 255, 255, 0), (255, 0, 0, 255), (0, 0, 255, 0), (0, 0, 0, 255), (255, 0, 255, 255)):
    cases.append((a, b, c, d, 7))
tmp = os.path.join(root, 'tests', 'tmp', 'lineasm_cases.txt')
os.makedirs(os.path.dirname(tmp), exist_ok=True)
with open(tmp, 'w') as f:
    for c in cases: f.write('%d %d %d %d %d\n' % c)
exe = os.path.join(root, 'tests', 'test_bres')
out = subprocess.run([exe, 'dump', tmp], capture_output=True, text=True, check=True).stdout.split('\n')
bad = 0
SCR = 0x20000
for i, c in enumerate(cases):
    cpu = m68k_mini.run_line(src, *c, scr=SCR)
    got = {}
    for j in range(32768):
        v = cpu.mem[SCR + j]
        if v: got[j] = v
    want = {}
    for tok in out[i].split():
        a, b = tok.split(':'); want[int(a)] = int(b)
    if got != want:
        bad += 1
        if bad <= 5: print('DIFERENTE', c, 'asm', len(got), 'bytes, C', len(want), 'bytes')
    # must not touch anything outside the screen nor leave the stack register changed
    outside = [j for j in range(len(cpu.mem)) if cpu.mem[j] and not (SCR <= j < SCR + 32768) and not (0x1E000 <= j < 0x1F000)]
    if outside:
        bad += 1; print('escribe fuera de la pantalla', c, outside[:3])
print('%d lineas, %d fallos' % (len(cases), bad))
sys.exit(1 if bad else 0)
