#!/usr/bin/env python3
"""Convierte el panel de instrumentos (dashboard) del volcado de pantalla del Apple II (A.SCREEN.bin, hires 280 x 192)
en un fichero para el QL: tira de 256 x 56 pixeles en modo 8 (lineas 136..191 de la pantalla, x de Elite 0..255).
El panel del original es un bitmap que se carga del disco; el juego del QL lo carga de <unidad>elite_dash.

Formato de salida (data/elite_dash): cabecera de 8 bytes 'E','L','Q','D', version 1, lineas 56, y0 hi, y0 lo (136); despues
56 lineas de 128 bytes en formato de pantalla del QL modo 8 (por palabra, 4 pixeles: byte par = verde y flash,
byte impar = rojo y azul).
Colores: el hires del Apple II da color por artefactos NTSC: un punto con vecino encendido es blanco; aislado: x par =
violeta (azul si el bit 7 del byte), x impar = verde (naranja si el bit 7). Violeta -> magenta, naranja ("RED" en Elite) -> rojo.
Uso: python3 tools/gen_dashboard.py [ruta A.SCREEN.bin] [salida]   (ELITE_SRC apunta a main-sources de la fuente de referencia)"""
import os, sys, glob

Y0, LINES, DOT0, WIDTH = 136, 56, 8, 256            # el pixel x de Elite es el punto x + 8 del Apple II
BLACK, BLUE, RED, MAGENTA, GREEN, CYAN, YELLOW, WHITE = range(8)

def find_default():
    src = os.environ.get('ELITE_SRC', '')
    cands = []
    if src: cands.append(os.path.join(src, '..', 'images', 'ib-disk', 'A.SCREEN.bin'))
    cands += glob.glob(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'ref', 'moxon-appleII', '1-source-files', 'images', 'ib-disk', 'A.SCREEN.bin'))
    for c in cands:
        if os.path.exists(c): return c
    raise SystemExit('no encuentro A.SCREEN.bin: pasa la ruta como primer argumento')

def dot_colors(data, y):
    base = (y & 7) * 0x400 + ((y >> 3) & 7) * 0x80 + (y >> 6) * 0x28
    dots, hi = [], []
    for b in data[base:base + 40]:
        for k in range(7):
            dots.append((b >> k) & 1); hi.append(b >> 7)
    out = []
    for x in range(280):
        if not dots[x]: out.append(BLACK); continue
        l = dots[x - 1] if x > 0 else 0
        r = dots[x + 1] if x < 279 else 0
        if l or r: out.append(WHITE)
        elif x % 2 == 0: out.append(BLUE if hi[x] else MAGENTA)
        else: out.append(RED if hi[x] else GREEN)
    return out

def ql_line(colors):
    """256 colores -> 128 bytes de modo 8 (como gfx_pixel de gfx.c)"""
    line = bytearray(128)
    for x, c in enumerate(colors):
        k = x & 3; s = 2 * (3 - k); w = (x >> 2) * 2
        g, r, b = (c >> 2) & 1, (c >> 1) & 1, c & 1
        line[w] |= g << (s + 1)                      # byte par: verde (bit s+1), flash (bit s)
        line[w + 1] |= (r << (s + 1)) | (b << s)     # byte impar: rojo (bit s+1), azul (bit s)
    return bytes(line)

def main():
    path = sys.argv[1] if len(sys.argv) > 1 else find_default()
    out = sys.argv[2] if len(sys.argv) > 2 else 'data/elite_dash'
    data = open(path, 'rb').read()
    assert len(data) == 8192, 'se esperaba una pagina hires de 8192 bytes'
    body = b''.join(ql_line(dot_colors(data, Y0 + i)[DOT0:DOT0 + WIDTH]) for i in range(LINES))
    os.makedirs(os.path.dirname(out) or '.', exist_ok=True)
    open(out, 'wb').write(b'ELQD' + bytes([1, LINES, Y0 >> 8, Y0 & 255]) + body)
    print('%s: %d bytes (%d lineas desde y=%d) desde %s' % (out, 8 + len(body), LINES, Y0, path))

if __name__ == '__main__':
    main()
