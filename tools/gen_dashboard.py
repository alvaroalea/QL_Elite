#!/usr/bin/env python3
"""Converts the instrument panel (dashboard) from the Apple II screen dump (A.SCREEN.bin, hires 280 x 192)
into a file for the QL: a 256 x 56 pixel strip in mode 8 (screen lines 136..191, Elite x 0..255).
The original's panel is a bitmap loaded from disk; the QL game loads it from <drive>elite_dash.

Output format (data/elite_dash): 8-byte header 'E','L','Q','D', version 1, lines 56, y0 hi, y0 lo (136); then
56 lines of 128 bytes in QL mode 8 screen format (per word, 4 pixels: even byte = green and flash,
odd byte = red and blue).
Colours: Apple II hires gives colour by NTSC artefacts: a dot with a lit neighbour is white; isolated: even x =
violet (blue if the byte's bit 7), odd x = green (orange if bit 7). Violet -> magenta, orange ("RED" in Elite) -> red.
Usage: python3 tools/gen_dashboard.py [path A.SCREEN.bin] [output]   (ELITE_SRC points to main-sources of the reference source)"""
import os, sys, glob

Y0, LINES, DOT0, WIDTH = 136, 56, 8, 256            # Elite's pixel x is Apple II dot x + 8
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
    """256 colours -> 128 bytes of mode 8 (like gfx_pixel in gfx.c)"""
    line = bytearray(128)
    for x, c in enumerate(colors):
        k = x & 3; s = 2 * (3 - k); w = (x >> 2) * 2
        g, r, b = (c >> 2) & 1, (c >> 1) & 1, c & 1
        line[w] |= g << (s + 1)                      # even byte: green (bit s+1), flash (bit s)
        line[w + 1] |= (r << (s + 1)) | (b << s)     # odd byte: red (bit s+1), blue (bit s)
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
