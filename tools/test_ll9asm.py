#!/usr/bin/env python3
"""Compares src/ql_ll9.asm (ql_ll9_faces and ql_ll9_verts) with their C versions (ship_ll9_faces_c / ship_ll9_verts_c, dumped by
tests/ll9_dump) running the assembler text with tools/m68k_mini.py. usage: test_ll9asm.py [cases] [seed]  (without a real 68000:
it checks the algorithm, the addressing modes and the flags, not as68's encoding, which qcc validates on linking)."""
import re, subprocess, sys, os
sys.path.insert(0, os.path.dirname(__file__))
from m68k_mini import CPU

N = int(sys.argv[1]) if len(sys.argv) > 1 else 300
SEED = sys.argv[2] if len(sys.argv) > 2 else '1'
SRC = open('src/ql_ll9.asm').read() if not os.environ.get('LL9ASM') else open(os.environ['LL9ASM']).read()
TAB = open('src/ql_lltab.asm').read()

SYMS = {'_zxx19': 4, '_zlsnum': 1, '_zlsnum2': 1, '_zcnt2': 1, '_screen': 4, '_ship_dbg_skip': 1, '_clip_h': 2, '_ql_edge_v': 4, '_zxx0': 4, '_zxx4': 1, '_zxx17': 1, '_zxx20': 1, '_zcnt': 1, '_zT': 1, '_zxx16': 18, '_zxx18': 9, '_zx12': 6, '_zx15': 6,
        '_zk3': 16, '_xx3': 258, '_inwk': 37, '_view_yscale': 2, '_view_cy': 2}
BLOB_ADDR, TAB_ADDR, HEAP_ADDR = 0x4000, 0x8000, 0x9000

def build():
    cpu = CPU(SRC, mem_size=0x10000)
    addr = 0x1000
    for k, n in SYMS.items():
        cpu.syms[k] = addr; addr += (n + 1) & ~1
    cpu.syms['_ql_lltab'] = TAB_ADDR
    tab = bytearray()
    for m in re.finditer(r'\.dc\.b\s+([\d,]+)', TAB):
        tab += bytes(int(x) for x in m.group(1).split(','))
    assert len(tab) == 1024
    cpu.mem[TAB_ADDR:TAB_ADDR + 1024] = tab
    return cpu

out = subprocess.run(['./tests/ll9_dump', str(N), SEED], capture_output=True, text=True, check=True).stdout.split('\n')
blob = bytes.fromhex(out[0].split()[1])
cpu = build()
cpu.mem[BLOB_ADDR:BLOB_ADDR + len(blob)] = blob

LINES = []
def ql_line_native(c):
    sp = c.a[7]
    x1, y1, x2, y2, col, scr = (c.rd(sp + 4 * i, 4) for i in range(6))
    LINES.append((x1, y1, x2, y2)); assert col == 7
cpu.natives['_ql_line'] = ql_line_native

def fields(line):
    d = {}
    for m in re.finditer(r'(\w+)=([0-9a-f]+)', line): d[m.group(1)] = m.group(2)
    return d

def setb(sym, data, off=0):
    cpu.mem[cpu.syms[sym] + off:cpu.syms[sym] + off + len(data)] = data

def getb(sym, n): return bytes(cpu.mem[cpu.syms[sym]:cpu.syms[sym] + n])

total = fails = 0
steps_f = steps_v = steps_e = deleg_total = 0
i = 1
while i < len(out) and out[i].startswith('CASE'):
    head = out[i].split(); kind, typ, off, voff, ys, cy, mode = (int(x) for x in head[1:8])
    inp = fields(out[i]); o = fields(out[i + 1]); i += 2
    dec = lambda n: int(inp[n])                          # numeric ones go in decimal on the line; hex ones go without 0x
    # the numeric header fields (zxx4, zxx17, zxx20) come out in decimal: they are re-read from the line
    nums = dict(re.findall(r'(zxx4|zxx17|zxx20|zcnt|zlsnum2)=(\d+)', out[i - 2]))
    setb('_zxx4', bytes([int(nums['zxx4'])])); setb('_zxx17', bytes([int(nums['zxx17'])])); setb('_zxx20', bytes([int(nums['zxx20'])]))
    if kind != 2:
        setb('_zxx16', bytes.fromhex(inp['zxx16'])); setb('_zxx18', bytes.fromhex(inp['zxx18']))
        setb('_inwk', bytes.fromhex(inp['inwk']))
    setb('_zk3', bytes.fromhex(inp['zk3']))
    setb('_zx12', bytes.fromhex(inp['zx12'])); setb('_zx15', bytes.fromhex(inp['zx15']))
    setb('_xx3', bytes.fromhex(inp['xx3'])); setb('_zcnt', bytes([0])); setb('_zT', bytes([0]))
    if kind != 2:
        pass
    zaddr = BLOB_ADDR + off
    if 'plan' in inp:                                    # own blueprint (mode 6): placed apart
        zaddr = 0x7000; cpu.mem[zaddr:zaddr + 48] = bytes.fromhex(inp['plan'])
    cpu.mem[cpu.syms['_zxx0']:cpu.syms['_zxx0'] + 4] = zaddr.to_bytes(4, 'big')
    cpu.a[7] = 0xF000
    if kind == 2:
        ok_raw = True
        setb('_zcnt', bytes([int(nums['zcnt'])])); setb('_zlsnum', bytes([1])); setb('_zlsnum2', bytes([int(nums['zlsnum2'])]))
        setb('_clip_h', cy.to_bytes(2, 'big'))   # in this type, the header carries clip_h in field 6
        setb('_zxx19', HEAP_ADDR.to_bytes(4, 'big')); cpu.mem[HEAP_ADDR:HEAP_ADDR + 256] = bytes.fromhex(inp['heap'])
        setb('_ship_dbg_skip', bytes([int(re.search(r'skip=(\d+)', out[i - 1]).group(1))])); setb('_screen', (0x20000).to_bytes(4, 'big'))
        LINES.clear(); elog = {}
        for m in re.finditer(r'([0-9a-f]{2})([01])([0-9a-f]{8})([0-9a-f]{16})', o['edges'] if 'edges' in o else ''):
            elog[int(m.group(1), 16)] = (int(m.group(2)), list(bytes.fromhex(m.group(3))), list(bytes.fromhex(m.group(4))))
        v = BLOB_ADDR + voff; cont = 0; ndeleg = 0
        while True:
            cpu.a[7] = 0xF000
            cpu.a[7] -= 4; cpu.wr(cpu.a[7], 4, cy); cpu.a[7] -= 4; cpu.wr(cpu.a[7], 4, cont); cpu.a[7] -= 4; cpu.wr(cpu.a[7], 4, v)
            steps_e += cpu.run('_ql_ll9_edges')
            if cpu.d[0] == 0: break
            ndeleg += 1                                  # edge to clip in C: what C did (ship_clip2 + ship_lsput) is applied in the reference
            idx = cpu.mem[cpu.syms['_zxx17']]
            acc, xy, raw = elog[idx]
            got_raw = list(cpu.mem[cpu.syms['_zx15']:cpu.syms['_zx15'] + 6]) + list(cpu.mem[cpu.syms['_zx12']:cpu.syms['_zx12'] + 2])
            if got_raw != raw:
                fails += 1; print('FALLO caso', total, 'arista devuelta al C con XX15/XX12 distintos', got_raw, raw); ok_raw = False
            if acc:
                y = cpu.mem[cpu.syms['_zlsnum']]; zl2 = cpu.mem[cpu.syms['_zlsnum2']]
                skip = cpu.mem[cpu.syms['_ship_dbg_skip']] & 2
                if not skip: LINES.append(tuple(xy))
                old = tuple(cpu.mem[HEAP_ADDR + y:HEAP_ADDR + y + 4])
                cpu.mem[HEAP_ADDR + y:HEAP_ADDR + y + 4] = bytes(xy)
                cpu.mem[cpu.syms['_zlsnum']] = (y + 4) & 255
                if y < zl2 and not skip: LINES.append(old)
            v = cpu.rd(cpu.syms['_ql_edge_v'], 4); cont = 1
        deleg_total += ndeleg
    elif kind == 0:
        cpu.a[7] -= 4; cpu.wr(cpu.a[7], 4, BLOB_ADDR + voff)
        steps_f += cpu.run('_ql_ll9_faces')
    else:
        cpu.a[7] -= 4; cpu.wr(cpu.a[7], 4, ys); cpu.a[7] -= 4; cpu.wr(cpu.a[7], 4, cy)
        steps_v += cpu.run('_ql_ll9_verts')
    total += 1
    if kind == 2:
        exp_lines = [tuple(bytes.fromhex(o['lines'][8 * j:8 * j + 8])) for j in range(len(o['lines']) // 8)] if 'lines' in o else []
        gotheap = bytes(cpu.mem[HEAP_ADDR:HEAP_ADDR + 256])
        ok = (LINES == exp_lines and gotheap == bytes.fromhex(o['heap']) and cpu.mem[cpu.syms['_zlsnum']] == int(re.search(r'zlsnum=(\d+)', out[i - 1]).group(1))
              and cpu.mem[cpu.syms['_zxx17']] == int(re.search(r'zxx17=(\d+)', out[i - 1]).group(1)))
        if not ok:
            fails += 1
            if fails <= 5: print('FALLO caso', total, 'aristas nave', typ, 'modo', mode, 'lineas asm', len(LINES), 'C', len(exp_lines), 'heap', gotheap == bytes.fromhex(o['heap']), 'zlsnum', cpu.mem[cpu.syms['_zlsnum']], 'vs', re.search(r'zlsnum=(\d+)', out[i - 1]).group(1))
        continue
    exp_zk3, exp_x18, exp_xx3 = bytes.fromhex(o['zk3']), bytes.fromhex(o['zxx18']), bytes.fromhex(o['xx3'])
    got = (getb('_zk3', 16), getb('_zxx18', 9), getb('_xx3', 258))
    ok = got == (exp_zk3, exp_x18, exp_xx3)
    if kind == 1:
        ok = ok and cpu.mem[cpu.syms['_zcnt']] == int(re.search(r'zcnt=(\d+)', out[i - 1]).group(1)) \
                and cpu.mem[cpu.syms['_zxx17']] == int(re.search(r'zxx17=(\d+)', out[i - 1]).group(1))
    if not ok:
        fails += 1
        if fails <= 5:
            print('FALLO caso', total, 'tipo', 'caras' if kind == 0 else 'vertices', 'nave', typ, 'modo', mode)
            for nm, g, e in (('zk3', got[0], exp_zk3), ('zxx18', got[1], exp_x18), ('xx3', got[2], exp_xx3)):
                if g != e:
                    j = next(j for j in range(len(g)) if g[j] != e[j]); print('  ', nm, 'primer byte distinto en', j, 'asm', g[j], 'C', e[j])
print('%d casos (caras %d pasos, vertices %d pasos, aristas %d pasos, %d aristas devueltas al C), %d fallos' % (total, steps_f, steps_v, steps_e, deleg_total, fails))
sys.exit(1 if fails else 0)
