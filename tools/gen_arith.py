#!/usr/bin/env python3
"""Generates tests/vectors_arith.txt with the ORIGINAL Maths (Arithmetic) code in the 6502 emulator: each routine with random A, X, Y, C and
zero page (with many boundary values) and all its outputs (A, X, Y, C, P, P+1, Q, R, S, K, Y1). Routines: MULTU, MU11, SQUA,
SQUA2, MULT1, MULT12, MAD, ADD, TIS1, TIS2, DVID4, DV41, DV42, MLTU2, MULT3, MLU1, MLU2, LL38, DVID3B2 (and MU1, MU5, MU6 through them).
Checked with tests/test_arith (make test_arith). Requires ELITE_SRC. Usage: python3 tools/gen_arith.py [n] [seed]"""
import sys, random, re, math
sys.path.insert(0, 'tools')
from frag6502 import *
n = int(sys.argv[1]) if len(sys.argv) > 1 else 2000
random.seed(int(sys.argv[2]) if len(sys.argv) > 2 else 51)

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


cpu, S = build(code_, None, {'log': 0x2000, 'logL': 0x2100, 'alogh': 0x2200})
print('stubs:', sorted(k for k, v in S.items() if 0x3E00 <= v < 0x3F00), file=sys.stderr)
lg = [0x0803] + [int(0x2000 * math.log(i) / math.log(2) + 0.5) for i in range(1, 256)]
for i in range(256):
    cpu.mem[0x2000 + i] = (lg[i] >> 8) & 255; cpu.mem[0x2100 + i] = lg[i] & 255
    cpu.mem[0x2200 + i] = int(2 ** ((i / 2 + 128) / 16) + 0.5) >> 8 & 255
m = cpu.mem
def B():
    return random.choice([0, 1, 2, 3, 0x1F, 0x20, 0x3F, 0x40, 0x7E, 0x7F, 0x80, 0x81, 0xBF, 0xC0, 0xFE, 0xFF]) if random.random() < 0.35 else random.randrange(256)
def NZ():
    v = B()
    return v or 1
ZP = ['P', 'P+1', 'Q', 'R', 'S', 'T', 'U', 'T1']
def addr(nm): return S[nm[:-2]] + 1 if nm.endswith('+1') else S[nm]
out = []
OPS = ['MULTU', 'MU11', 'SQUA', 'SQUA2', 'MULT1', 'MULT12', 'MAD', 'ADD', 'TIS1', 'TIS2', 'DVID4', 'DV41', 'DV42', 'MLTU2', 'MULT3', 'MLU1', 'MLU2', 'LL38', 'DVID3B2']
for _ in range(n):
    for op in OPS:
        for k in range(0x100): m[k] = 0
        v = {nm: B() for nm in ZP}
        a, x, y, c = B(), B(), random.randrange(0, 20), random.randrange(2)
        z = [B(), B(), B()]
        delta = B(); sy = B()
        if op in ('MU11', 'MLU1') and False: pass
        if op == 'MU11': x = NZ()
        if op in ('DVID4', 'TIS2'): v['Q'] = NZ()                      # divisor 0: does not occur and depends on log[0] (varies between binaries)
        if op == 'DV41': a = NZ()
        if op == 'DV42':
            a = NZ(); m[S['SZ'] + y] = a
        if op == 'DVID3B2' and random.random() < 0.9: z[1] = NZ()
        for nm, val in v.items(): m[addr(nm)] = val
        m[S['DELTA']] = delta; m[S['INWK'] + 6], m[S['INWK'] + 7], m[S['INWK'] + 8] = z
        m[S['SY'] + y] = sy
        for k in range(4): m[S['K'] + k] = B()
        kin = [m[S['K'] + k] for k in range(4)]
        cpu.a, cpu.x, cpu.y, cpu.c = a, x, y, c
        try: cpu.run(S[op])
        except Exception as e:
            continue
        o = [cpu.a, cpu.x, cpu.y, cpu.c] + [m[addr(nm)] for nm in ZP] + [m[S['K'] + k] for k in range(4)] + [m[S['Y1']]]
        out.append('%s %d %d %d %d %s %d %d %d %d %d %d %d %d %d : %s' % (op, a, x, y, c, ' '.join(str(v[nm]) for nm in ZP), delta, sy, z[0], z[1], z[2], kin[0], kin[1], kin[2], kin[3], ' '.join(map(str, o))))
open('tests/vectors_arith.txt', 'w').write('\n'.join(out) + '\n')
print(len(out), 'vectores')
