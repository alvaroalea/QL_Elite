# Genera src/font.h con la fuente del juego original (A.FLOWY.bin, la FONT del Apple II, en 1-source-files/fonts de la
# fuente de referencia): caracteres 32..127, 8 bytes cada uno, 7 puntos de ancho con el bit 0 a la izquierda (el bit 7 es
# el de paleta del Apple y no se usa). Aqui el bit 7 es el punto de la izquierda (para volcar directo).
# Uso: python3 tools/gen_font.py [A.FLOWY.bin] [src/font.h]   (por defecto, de ELITE_SRC/../fonts)
import os, sys
src = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.environ['ELITE_SRC'], '..', 'fonts', 'A.FLOWY.bin')
dst = sys.argv[2] if len(sys.argv) > 2 else 'src/font.h'
d = open(src, 'rb').read()
assert len(d) == 768, len(d)
def rev(b): return int('{:08b}'.format(b & 0x7F)[::-1], 2)
out = ["/* Generado por tools/gen_font.py desde A.FLOWY.bin (la fuente del Elite del Apple II, FONT). ASCII 32..127, bit7 = punto de la izquierda. */",
       "static const u8 font8x8[96][8] = {"]
for c in range(32, 128):
    v = [rev(x) for x in d[(c - 32) * 8:(c - 31) * 8]]
    out.append("    {%s}, /* %d %s */" % (','.join('0x%02X' % x for x in v), c, repr(chr(c)) if c < 127 else 'DEL'))
out.append("};")
open(dst, 'w').write('\n'.join(out) + '\n')
print(dst, 'desde', src)
