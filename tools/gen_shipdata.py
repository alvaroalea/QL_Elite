# Extrae los planos de las naves (cabecera, vertices, aristas, caras) de elite-data.asm y genera src/shipdata.c.
# Los datos se calculan evaluando EQUB/EQUW y las macros VERTEX/EDGE/FACE; no se copia ningun comentario.
import sys, re
sys.path.insert(0, 'tools')
import elitesrc

def parse():
    L = elitesrc.read('elite-data.asm')
    start = next(i for i, l in enumerate(L) if l.strip() == '.SHIP_MISSILE')
    end = next(i for i, l in enumerate(L) if i > start and l.strip().startswith('PRINT "SHIPS"'))
    # tabla XX21 (orden de tipos)
    xi = next(i for i, l in enumerate(L) if l.strip() == '.XX21')
    order = []
    for l in L[xi + 1:xi + 60]:
        m = re.match(r'^\s*EQUW\s+(SHIP_\w+)', l)
        if m: order.append(m.group(1))
        elif order and not l.strip().startswith(';') and l.strip(): break
    # separar bloques: cada plano empieza en una etiqueta .SHIP_X que no acaba en _VERTICES/_EDGES/_FACES
    blocks = {}; cur = None; lines = []
    for l in L[start:end]:
        t = re.sub(r';.*', '', l).strip()
        m = re.match(r'^\.(SHIP_\w+)$', t)
        if m and not re.search(r'_(VERTICES|EDGES|FACES)$', m.group(1)):
            cur = m.group(1); blocks[cur] = []
        if cur and t: blocks[cur].append(t)
    return order, blocks

def expr(e, labels, base):
    e = e.strip()
    S = elitesrc.Syms()
    for k, v in labels.items(): S[k] = v
    return S.ev(e)

def assemble_all(order, blocks):
    """Ensambla todos los planos en un unico espacio (algunos comparten aristas/caras de otro plano)."""
    seq = [(n, blocks[n]) for n in blocks]
    def run(labels, emit):
        out = bytearray(); starts = {}
        for name, lines in seq:
            starts[name] = len(out)
            for t in lines:
                m = re.match(r'^\.(\w+)$', t)
                if m:
                    if not emit: labels[m.group(1)] = len(out)
                    continue
                m = re.match(r'^(EQUB|EQUW)\s+(.+)$', t)
                if m:
                    for e in re.split(r',(?![^(]*\))', m.group(2)):
                        if not e.strip(): continue
                        try: v = expr(e, labels, 0)
                        except Exception:
                            if emit: raise
                            v = 0
                        if m.group(1) == 'EQUB': out.append(v & 255)
                        else: out += bytes([v & 255, (v >> 8) & 255])
                    continue
                m = re.match(r'^(VERTEX|EDGE|FACE)\s+(.+)$', t)
                if m:
                    a = [int(x) for x in m.group(2).replace(' ', '').split(',')]
                    if m.group(1) == 'VERTEX':
                        x, y, z, f1, f2, f3, f4, vis = a
                        sg = (128 if x < 0 else 0) + (64 if y < 0 else 0) + (32 if z < 0 else 0) + vis
                        out += bytes([abs(x), abs(y), abs(z), sg & 255, (f1 + (f2 << 4)) & 255, (f3 + (f4 << 4)) & 255])
                    elif m.group(1) == 'EDGE':
                        v1, v2, f1, f2, vis = a
                        out += bytes([vis, f1 + (f2 << 4), (v1 << 2) & 255, (v2 << 2) & 255])
                    else:
                        nx, ny, nz, vis = a
                        sg = (128 if nx < 0 else 0) + (64 if ny < 0 else 0) + (32 if nz < 0 else 0) + vis
                        out += bytes([sg & 255, abs(nx), abs(ny), abs(nz)])
                    continue
                if re.match(r'^(IF|ENDIF|ELSE|CLEAR|ORG)\b', t): continue
                raise Exception('linea no reconocida en %s: %s' % (name, t))
        return out, starts
    labels = {}
    run(labels, False)
    out, starts = run(labels, True)
    return bytes(out), starts

def blueprints():
    """Devuelve (orden_de_tipos, blob, inicio_de_cada_plano)."""
    order, blocks = parse()
    blob, starts = assemble_all(order, blocks)
    return order, blob, starts

if __name__ == '__main__':
    order, blob, starts = blueprints()
    assert len(order) == 32, len(order)
    o = ['/* Planos de las naves de Elite (Apple II) en un solo bloque: cada plano tiene cabecera de 20 bytes,',
         '   vertices (6), aristas (4) y caras (4); algunos comparten aristas/caras con otro plano mediante',
         '   desplazamientos relativos de 16 bits (por eso van juntos y en este orden).',
         '   Generado por tools/gen_shipdata.py desde la fuente de referencia; no editar a mano. */',
         '#include "shipdata.h"', '', 'const u8 ship_blob[%d] = {' % len(blob)]
    for k in range(0, len(blob), 16): o.append('    ' + ','.join('0x%02X' % v for v in blob[k:k + 16]) + ',')
    o += ['};', '', '/* desplazamiento del plano de cada tipo (1..32) dentro de ship_blob */',
          'const unsigned short ship_off[SHIP_TYPES + 1] = { 0,']
    o += ['    %d,  /* %d: %s */' % (starts[n], i + 1, n) for i, n in enumerate(order)] + ['};', '']
    open('src/shipdata.c', 'w').write('\n'.join(o))
    open('src/shipdata.h', 'w').write('#ifndef SHIPDATA_H\n#define SHIPDATA_H\n#include "platform.h"\n#define SHIP_TYPES 32\n'
        '#define SHIP_BLOB_SIZE %dL\n'
        '/* ship_blob: todos los planos; ship_off[tipo] (1..32): desplazamiento del plano del tipo dentro del bloque */\n'
        'extern const u8 ship_blob[%d];\nextern const unsigned short ship_off[SHIP_TYPES + 1];\n'
        '#define SHIP_BP(t) (ship_blob + ship_off[t])\n'
        '/* tipos de nave (nombres de la fuente, sin el prefijo SHIP_) */\n%s'
        '#endif\n' % (len(blob), len(blob), ''.join('#define SHIPT_%s %d\n' % (n[5:], i + 1) for i, n in enumerate(order))))
    print('naves:', len(order), 'bytes:', len(blob))
