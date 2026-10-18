#!/usr/bin/env python3
"""Genera tests/vectors_jameson.txt: los bytes de NA2% (el comandante JAMESON de fabrica, Q% = FALSE) ensamblados desde la fuente
original, que copia JAMESON en NA%. Se comprueba con tests/test_jameson (make test_jameson). Requiere ELITE_SRC."""
import sys, re
sys.path.insert(0, 'tools')
from frag6502 import *
raw = [l.replace('Q%', '0') for l in L[6731:6871]]
src = [l for l in preprocess(raw) if code(l) != 'SKIP 0']
src = [re.sub(r'\s+AND\s+(?=\S)', ' & ', l) for l in src]
out = []
for l in src:
    m = re.match(r'^\s*EQUS\s+"(.*)"\s*$', l)
    d = re.match(r'^\s*EQUD\s+(\S+)\s*$', l)
    if m: out.append(' EQUB ' + ','.join(str(ord(c)) for c in m.group(1)))
    elif d:
        v = int(d.group(1).replace('$', '0x'), 0); out.append(' EQUB ' + ','.join(str((v >> (8 * i)) & 255) for i in range(4)))
    else:
        k = re.match(r'^\s*SKIP\s+(\d+)\s*$', l)
        out += [' EQUB 0'] * int(k.group(1)) if k else [l]
cpu, S = build(out, None)
b = bytes(cpu.mem[S['NA2%']:S['NAEND%']])
open('tests/vectors_jameson.txt', 'w').write(b.hex() + '\n')
print(len(b), 'bytes:', b.hex())
