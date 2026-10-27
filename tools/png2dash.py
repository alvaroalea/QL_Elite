#!/usr/bin/env python3
"""QL instrument panel (cockpit) <-> PNG, to edit it with a PC drawing program.

  Convert a PNG into data/elite_dash:
      python3 tools/png2dash.py cockpit.png [data/elite_dash]
  Export the current elite_dash to PNG (to have the starting template):
      python3 tools/png2dash.py --export [data/elite_dash] [cockpit.png] [--scale K]

The panel is a strip of 256 x 56 pixels in QL mode 8 (screen lines 136..191), with the 8 QL colours:
    black (0,0,0)       blue (0,0,255)      red (255,0,0)      magenta (255,0,255)
    green (0,255,0)     cyan (0,255,255)    yellow (255,255,0) white (255,255,255)

The PNG can be any size that is a multiple of 256 x 56 (each QL pixel is a block of sx x sy PNG dots; the colour of the
block's centre is taken). On the QL mode 8 pixels are twice as wide as tall, so the default
export is 1024 x 112 (blocks of 4 x 2) so that on the PC it looks with the QL's proportion; with
--scale 1 it comes out 512 x 56 and with --scale 0 256 x 56 unscaled. The PNG can be palette or RGB/RGBA (transparency
is taken as black). Colours that are not exactly one of the 8 are brought to the nearest (each channel
>= 128 counts as on) and a warning says how many there were.

Careful when drawing: the game paints over the background, in XOR or erasing its area, the indicator bars (speed,
roll, pitch, energy, shields, fuel, altitude, temperatures), the compass, the radar (dots and sticks of the
ships, lines 146..190), the missile indicators (lines 185..188) and the S and E bulbs (lines 176..183). It is advisable to leave those
areas as in the exported template so they show up well.

Format of data/elite_dash: 8-byte header 'E','L','Q','D', version 1, number of lines (56), y0 high and low (136), and
56 lines of 128 bytes in QL mode 8 screen format (per word, 4 pixels: even byte = green (bit s+1) and flash
(bit s), odd byte = red (bit s+1) and blue (bit s), with s = 2 * (3 - k) for pixel k of the word).
Requires Pillow (pip install pillow)."""
import os, sys

Y0, LINES, WIDTH = 136, 56, 256
HEADER = b'ELQD' + bytes([1, LINES, Y0 >> 8, Y0 & 255])
# QL colour index: bit 2 = green, bit 1 = red, bit 0 = blue (like C_* in platform.h)
PALETTE = [(0, 0, 0), (0, 0, 255), (255, 0, 0), (255, 0, 255), (0, 255, 0), (0, 255, 255), (255, 255, 0), (255, 255, 255)]
NAMES = ['negro', 'azul', 'rojo', 'magenta', 'verde', 'cian', 'amarillo', 'blanco']

def need_pil():
    try:
        from PIL import Image
        return Image
    except ImportError:
        raise SystemExit('falta Pillow: pip install pillow')

def color_index(rgb):
    """(r, g, b) -> (QL index, exact?)"""
    r, g, b = rgb[:3]
    exact = (r, g, b) in PALETTE
    return ((g >= 128) << 2) | ((r >= 128) << 1) | (b >= 128), exact

def ql_line(colors):
    """256 colour indices -> 128 bytes of mode 8 (like gfx_pixel in gfx.c)"""
    line = bytearray(128)
    for x, c in enumerate(colors):
        k = x & 3; s = 2 * (3 - k); w = (x >> 2) * 2
        g, r, b = (c >> 2) & 1, (c >> 1) & 1, c & 1
        line[w] |= g << (s + 1)
        line[w + 1] |= (r << (s + 1)) | (b << s)
    return bytes(line)

def ql_colors(line):
    """128 bytes of mode 8 -> 256 colour indices"""
    out = []
    for x in range(WIDTH):
        k = x & 3; s = 2 * (3 - k); w = (x >> 2) * 2
        g = (line[w] >> (s + 1)) & 1
        r = (line[w + 1] >> (s + 1)) & 1
        b = (line[w + 1] >> s) & 1
        out.append((g << 2) | (r << 1) | b)
    return out

def png_to_dash(png, out):
    Image = need_pil()
    im = Image.open(png)
    if im.mode in ('RGBA', 'LA', 'PA') or (im.mode == 'P' and 'transparency' in im.info):
        bg = Image.new('RGBA', im.size, (0, 0, 0, 255))
        bg.alpha_composite(im.convert('RGBA'))
        im = bg
    im = im.convert('RGB')
    w, h = im.size
    if w % WIDTH or h % LINES or w == 0 or h == 0:
        raise SystemExit('%s: %d x %d no es multiplo de %d x %d' % (png, w, h, WIDTH, LINES))
    sx, sy = w // WIDTH, h // LINES
    px = im.load()
    inexact, mixed, used = 0, 0, [0] * 8
    body = bytearray()
    for y in range(LINES):
        row = []
        for x in range(WIDTH):
            c, ok = color_index(px[x * sx + sx // 2, y * sy + sy // 2])
            if not ok: inexact += 1
            if sx > 1 or sy > 1:                     # warns if the block is not a single colour
                c0 = px[x * sx, y * sy]
                if any(px[x * sx + i, y * sy + j] != c0 for i in range(sx) for j in range(sy)): mixed += 1
            used[c] += 1
            row.append(c)
        body += ql_line(row)
    os.makedirs(os.path.dirname(out) or '.', exist_ok=True)
    open(out, 'wb').write(HEADER + bytes(body))
    print('%s: %d bytes desde %s (%d x %d, bloques de %d x %d)' % (out, len(HEADER) + len(body), png, w, h, sx, sy))
    print('colores: ' + ', '.join('%s %d' % (NAMES[i], used[i]) for i in range(8) if used[i]))
    if inexact: print('aviso: %d pixeles no eran uno de los 8 colores del QL y se han llevado al mas cercano' % inexact)
    if mixed: print('aviso: %d bloques de %d x %d no eran de un solo color (se ha usado el centro)' % (mixed, sx, sy))

def dash_to_png(dash, png, scale):
    Image = need_pil()
    data = open(dash, 'rb').read()
    if data[:4] != b'ELQD' or len(data) < 8 or data[4] != 1:
        raise SystemExit('%s: no es un fichero elite_dash' % dash)
    lines = data[5]
    if len(data) < 8 + lines * 128: raise SystemExit('%s: fichero corto' % dash)
    sx, sy = (2 * scale, scale) if scale > 0 else (1, 1)
    im = Image.new('P', (WIDTH * sx, lines * sy))
    im.putpalette([v for c in PALETTE for v in c] + [0] * (768 - 24))
    px = im.load()
    for y in range(lines):
        cols = ql_colors(data[8 + y * 128:8 + (y + 1) * 128])
        for x, c in enumerate(cols):
            for j in range(sy):
                for i in range(sx):
                    px[x * sx + i, y * sy + j] = c
    im.save(png)
    print('%s: %d x %d (bloques de %d x %d) desde %s' % (png, WIDTH * sx, lines * sy, sx, sy, dash))

def main():
    args = sys.argv[1:]
    scale = 2
    if '--scale' in args:
        i = args.index('--scale'); scale = int(args[i + 1]); del args[i:i + 2]
    if args and args[0] in ('-h', '--help') or not args:
        print(__doc__); return
    if args[0] == '--export':
        dash = args[1] if len(args) > 1 else 'data/elite_dash'
        png = args[2] if len(args) > 2 else 'cabina.png'
        dash_to_png(dash, png, scale)
    else:
        png_to_dash(args[0], args[1] if len(args) > 1 else 'data/elite_dash')

if __name__ == '__main__':
    main()
