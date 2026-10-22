#!/usr/bin/env python3
"""Genera src/ql_lltab.asm: las tablas log, logL y alogh de src/math_tables.h intercaladas (4 bytes por entrada: log, logL, alogh, 0)
para que ql_ll9.asm las lea con UN registro base e indice i*4 (el modo indexado del 68000 solo tiene un desplazamiento de 8 bits)."""
import re, sys
src = open('src/math_tables.h').read()
def arr(name):
    m = re.search(r'em_%s\[256\]\s*=\s*\{(.*?)\};' % name, src, re.S)
    v = [int(x, 0) for x in re.findall(r'0x[0-9A-Fa-f]+|\d+', m.group(1))]
    assert len(v) == 256, (name, len(v))
    return v
log, logl, alogh = arr('log'), arr('logL'), arr('alogh')
out = ['; ql_lltab.asm - GENERADO por tools/gen_lltab.py (no editar): tablas log, logL y alogh intercaladas, 4 bytes por entrada (log, logL, alogh, 0).',
       '\t.globl\t_ql_lltab', '\t.data', '_ql_lltab:']
for i in range(256):
    out.append('\t.dc.b\t%d,%d,%d,0' % (log[i], logl[i], alogh[i]))
open('src/ql_lltab.asm', 'w').write('\n'.join(out) + '\n')
print('src/ql_lltab.asm:', len(out) - 4, 'entradas')
