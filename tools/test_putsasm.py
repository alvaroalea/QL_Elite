#!/usr/bin/env python3
"""Compara src/ql_text.asm (ql_puts, ejecutada con tools/m68k_mini.py) con su version en C (text.c): dibuja celdas seguidas mientras el
caracter sea imprimible (32..127) y no se llegue a maxn; cada linea r del glifo es pp ^ (fm[b] & dd), palabra larga en dst + 128 * r (la celda
siguiente, 4 bytes mas alla); devuelve cuantas dibujo. Tablas, cadenas, colores, maxn y direcciones aleatorios; no debe tocar nada mas de
la memoria ni cambiar la pila. Uso: python3 tools/test_putsasm.py [casos]"""
import os, random, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import m68k_mini

root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
n = int(sys.argv[1]) if len(sys.argv) > 1 else 2000
src = open(os.path.join(root, 'src', 'ql_text.asm')).read()
rnd = random.Random(11)
SCR, FM, FONT, STR, STK = 0x20000, 0x10000, 0x12000, 0x1C000, 0x1F000
bad = steps = 0
for k in range(n):
    cpu = m68k_mini.CPU(src)
    cpu.a[7] = STK
    fm = [rnd.getrandbits(32) for _ in range(256)]
    fi = [[4 * rnd.randrange(256) for _ in range(8)] for _ in range(96)]
    pp, dd = rnd.getrandbits(32), rnd.getrandbits(32)
    kind = k % 4
    L = rnd.randrange(0, 33)
    s = [rnd.randrange(32, 128) for _ in range(L)]
    if kind == 1 and L: s[rnd.randrange(L)] = rnd.choice([0, 10, 13, 8, 31, 128, 200, 255])     # un caracter que corta la tira
    if kind == 2: s = [32, 127, 32, 127][:L]                                                    # los extremos del rango
    s += [0, 65, 66]                                                                              # fin de cadena y lo que sigue
    maxn = rnd.choice([0, 1, 2, 5, 32, 100, rnd.randrange(0, 40)])
    ncell = 32
    dst = SCR + 1024 * rnd.randrange(31) + 4 * rnd.randrange(32 - min(maxn, 32) if maxn < 32 else 1)
    for i, v in enumerate(fm): cpu.wr(FM + 4 * i, 4, v)
    for g in range(96):
        for r in range(8): cpu.wr(FONT + 16 * g + 2 * r, 2, fi[g][r])
    cpu.mem[STR:STR + len(s)] = bytes(s)
    cpu.mem[SCR:SCR + 32768] = bytes(rnd.randrange(256) for _ in range(32768))
    for v in (maxn, dd, pp, FM, FONT, STR, dst):         # argumentos: dst, s, font, fm, pp, dd, maxn (el primero queda arriba)
        cpu.a[7] -= 4; cpu.wr(cpu.a[7], 4, v)
    sp0 = cpu.a[7]
    before = bytes(cpu.mem)
    steps += cpu.run('_ql_puts')
    want = bytearray(before)
    cnt = 0
    while cnt < maxn:
        c = s[cnt] - 32
        if c < 0 or c > 95: break
        for r in range(8):
            w = (pp ^ (fm[fi[c][r] >> 2] & dd)).to_bytes(4, 'big')
            o = dst + 4 * cnt + 128 * r
            want[o:o + 4] = w
        cnt += 1
    after = bytes(cpu.mem)
    if after[:0x1EF00] != bytes(want[:0x1EF00]) or after[0x1F000:] != bytes(want[0x1F000:]) or cpu.a[7] != sp0 or cpu.d[0] != cnt:
        bad += 1
        if bad <= 5: print('DIFERENTE caso', k, hex(dst), 'maxn', maxn, 'cuenta asm', cpu.d[0], 'C', cnt)
print('%d casos (%d instrucciones), %d fallos' % (n, steps, bad))
sys.exit(1 if bad else 0)
