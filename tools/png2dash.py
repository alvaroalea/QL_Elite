#!/usr/bin/env python3
"""Panel de instrumentos (cabina) del QL <-> PNG, para editarlo con un programa de dibujo del PC.

  Convertir un PNG en data/elite_dash:
      python3 tools/png2dash.py cabina.png [data/elite_dash]
  Exportar el elite_dash actual a PNG (para tener la plantilla de partida):
      python3 tools/png2dash.py --export [data/elite_dash] [cabina.png] [--scale K]

El panel es una tira de 256 x 56 pixeles en modo 8 del QL (lineas 136..191 de la pantalla), con los 8 colores del QL:
    negro (0,0,0)       azul (0,0,255)      rojo (255,0,0)      magenta (255,0,255)
    verde (0,255,0)     cian (0,255,255)    amarillo (255,255,0) blanco (255,255,255)

El PNG puede tener cualquier tamano multiplo de 256 x 56 (cada pixel del QL es un bloque de sx x sy puntos del PNG; se
toma el color del centro del bloque). En el QL los pixeles del modo 8 son el doble de anchos que de altos, asi que la
exportacion por defecto es de 1024 x 112 (bloques de 4 x 2) para que en el PC se vea con la proporcion del QL; con
--scale 1 sale de 512 x 56 y con --scale 0 de 256 x 56 sin escalar. El PNG puede ser de paleta o RGB/RGBA (la
transparencia se toma como negro). Los colores que no sean exactamente uno de los 8 se llevan al mas cercano (cada canal
>= 128 cuenta como encendido) y se avisa de cuantos hubo.

Cuidado al dibujar: el juego pinta encima del fondo, en XOR o borrando su zona, las barras de los indicadores (velocidad,
alabeo, cabeceo, energia, escudos, combustible, altitud, temperaturas), la brujula, el radar (puntos y palos de las naves,
lineas 146..190), los indicadores de misil (lineas 185..188) y las bombillas S y E (lineas 176..183). Conviene dejar esas
zonas como en la plantilla exportada para que se vean bien.

Formato de data/elite_dash: cabecera de 8 bytes 'E','L','Q','D', version 1, numero de lineas (56), y0 alto y bajo (136), y
56 lineas de 128 bytes en formato de pantalla del QL modo 8 (por palabra, 4 pixeles: byte par = verde (bit s+1) y flash
(bit s), byte impar = rojo (bit s+1) y azul (bit s), con s = 2 * (3 - k) para el pixel k de la palabra).
Requiere Pillow (pip install pillow)."""
import os, sys

Y0, LINES, WIDTH = 136, 56, 256
HEADER = b'ELQD' + bytes([1, LINES, Y0 >> 8, Y0 & 255])
# indice de color del QL: bit 2 = verde, bit 1 = rojo, bit 0 = azul (como C_* en platform.h)
PALETTE = [(0, 0, 0), (0, 0, 255), (255, 0, 0), (255, 0, 255), (0, 255, 0), (0, 255, 255), (255, 255, 0), (255, 255, 255)]
NAMES = ['negro', 'azul', 'rojo', 'magenta', 'verde', 'cian', 'amarillo', 'blanco']

def need_pil():
    try:
        from PIL import Image
        return Image
    except ImportError:
        raise SystemExit('falta Pillow: pip install pillow')

def color_index(rgb):
    """(r, g, b) -> (indice QL, exacto?)"""
    r, g, b = rgb[:3]
    exact = (r, g, b) in PALETTE
    return ((g >= 128) << 2) | ((r >= 128) << 1) | (b >= 128), exact

def ql_line(colors):
    """256 indices de color -> 128 bytes de modo 8 (como gfx_pixel de gfx.c)"""
    line = bytearray(128)
    for x, c in enumerate(colors):
        k = x & 3; s = 2 * (3 - k); w = (x >> 2) * 2
        g, r, b = (c >> 2) & 1, (c >> 1) & 1, c & 1
        line[w] |= g << (s + 1)
        line[w + 1] |= (r << (s + 1)) | (b << s)
    return bytes(line)

def ql_colors(line):
    """128 bytes de modo 8 -> 256 indices de color"""
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
            if sx > 1 or sy > 1:                     # avisa si el bloque no es de un solo color
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
