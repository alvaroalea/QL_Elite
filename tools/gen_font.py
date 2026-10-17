# Genera src/font.h desde font8x8_basic.h (dominio publico, D. Hepper). Caracteres 32..127,
# invirtiendo los bits: en el original el bit0 es el pixel izquierdo; aqui el bit7 (para volcar directo).
import re,sys
src=open(sys.argv[1]).read()
rows=re.findall(r'\{\s*((?:0x[0-9A-Fa-f]{2},?\s*){8})\}',src)
assert len(rows)==128,len(rows)
def rev(b): return int('{:08b}'.format(b)[::-1],2)
out=["/* Generado por tools/gen_font.py desde font8x8_basic.h (Public Domain, Daniel Hepper). ASCII 32..127, bit7 = pixel izquierdo. */",
     "static const u8 font8x8[96][8] = {"]
for c in range(32,128):
    v=[rev(int(x,16)) for x in re.findall(r'0x[0-9A-Fa-f]{2}',rows[c])]
    out.append("    {%s}, /* %d %s */"%(','.join('0x%02X'%x for x in v),c,repr(chr(c)) if c<127 else 'DEL'))
out.append("};")
open(sys.argv[2],'w').write('\n'.join(out)+'\n')
