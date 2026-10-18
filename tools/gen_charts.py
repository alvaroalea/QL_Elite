#!/usr/bin/env python3
"""Genera tests/vectors_charts.txt con el codigo ORIGINAL de las cartas en el emulador de 6502 (universo real: TT81, TT20,
TT54, TT111 con readdistnce, SQUA2, LL5; SCALEX/SCALEY/SCALEY2; TT14, TT15, TT103, TT105, TT126, TT128, NLIN5/NLIN2...).
Las primitivas registran sus argumentos: HLOIN (h X1 X2 Y1 COL), VLOIN (v X1 Y1 Y2 COL), PIXEL (p X A ZZ), CIRCLE2
(c K3 K4 K), SUN (s K3 K4 K) y cpl (real; registra n XC YC QQ15+3 QQ15+1 al entrar).
  G  TT22 (carta galactica)       L  TT23 (carta local)       M  TT16 (mover la mira)       H  hm (sistema mas cercano)
Se comprueba con tests/test_charts (make test_charts). Uso: python3 tools/gen_charts.py [n] [semilla]"""
import sys, random, re
sys.path.insert(0, 'tools')
from frag6502 import *

n = int(sys.argv[1]) if len(sys.argv) > 1 else 300
random.seed(int(sys.argv[2]) if len(sys.argv) > 2 else 23)
hx = lambda b: ''.join('%02x' % x for x in b)
src = []
for r in ('TT22', 'TT15', 'TT23', 'TT14', 'TT128', 'TT123', 'TT16', 'TT103', 'TT105', 'hm', 'TT81', 'TT20', 'TT54', 'TT111', 'cpl',   # TT22 cae en TT15 y TT16 en TT103
          'SCALEY', 'SCALEY2', 'SCALEX', 'DVLOIN', 'LL5'):
    src += routine(r)
src += rng(nl('SQUA2') - 1, nx(nl('MU11')) - 1) + rng(nl('NLI4') - 1, nl('HLOIN2') - 1)
src = [re.sub(r'\s+AND\s+(?=\d)', ' & ', l) for l in src if code(l) != 'SKIP 0']
src = [l for l in src if not re.match(r'^(PRINT|SAVE)\b', code(l)) and not re.match(r'^(CODE|LOAD)_\w+%\s*=', code(l))]
cpu, S = build(src, None)
stubs = sorted(k for k, v in S.items() if 0x3E00 <= v < 0x3F00)
print('stubs:', stubs, file=sys.stderr)
m = cpu.mem
def rec(name, f): cpu.hooks[S[name]] = lambda c: c.log.append(f(c))
rec('HLOIN', lambda c: 'h %d %d %d %d' % (m[S['X1']], m[S['X2']], m[S['Y1']], m[S['COL']]))
rec('VLOIN', lambda c: 'v %d %d %d %d' % (m[S['X1']], m[S['Y1']], m[S['Y2']], m[S['COL']]))
rec('PIXEL', lambda c: 'p %d %d %d' % (c.x, c.a, m[S['ZZ']]))
rec('CIRCLE2', lambda c: 'c %d %d %d' % (m[S['K3']], m[S['K4']], m[S['K']]))
rec('SUN', lambda c: 's %d %d %d' % (m[S['K3']], m[S['K4']], m[S['K']]))
rec('cpl', lambda c: 'n %d %d %d %d' % (m[S['XC']], m[S['YC']], m[S['QQ15'] + 3], m[S['QQ15'] + 1]))
def flf(c): c.a = 0
cpu.hooks[S['FLFLLS']] = flf

gal = [0x4A, 0x5A, 0x48, 0x02, 0x53, 0xB7]
def twist(b):                                        # G1 de Ghy: cada byte de QQ21 rota a la izquierda
    return [((v << 1) | (v >> 7)) & 255 for v in b]
seeds = []
for g in range(8): seeds.append(gal); gal = twist(gal)
def systems(g):                                      # coordenadas de los 256 sistemas (TT81 + TT20)
    m[S['QQ21']:S['QQ21'] + 6] = bytes(seeds[g]); cpu.run(S['TT81']); out = []
    for i in range(256):
        out.append((m[S['QQ15'] + 3], m[S['QQ15'] + 1])); cpu.run(S['TT20'])
    return out
SYS = [systems(g) for g in range(8)]
out = []
def setup(g, cur, q9, q10, q14, qq11):
    m[S['QQ21']:S['QQ21'] + 6] = bytes(seeds[g])
    m[S['QQ0']], m[S['QQ1']] = SYS[g][cur]; m[S['QQ9']] = q9; m[S['QQ10']] = q10; m[S['QQ14']] = q14; m[S['QQ11']] = qq11
    m[S['SC'] + 1] = 0x2A                              # SC+1 (= INWK-1 en la pagina cero): nunca 0 tras dibujar
    cpu.log = []
for k in range(n):
    g = random.randrange(8); cur = random.randrange(256); q14 = random.choice([0, 1, 35, 70, random.randrange(71)])
    if random.random() < 0.5: q9, q10 = SYS[g][random.randrange(256)]
    else: q9, q10 = random.randrange(256), random.randrange(256)
    if random.random() < 0.3: q9, q10 = (SYS[g][cur][0] + random.randrange(-30, 31)) & 255, (SYS[g][cur][1] + random.randrange(-40, 41)) & 255
    setup(g, cur, q9, q10, q14, 64); cpu.run(S['TT22'])
    out.append('G %d %d %d %d %d : %s' % (g, cur, q9, q10, q14, ' '.join(e for e in cpu.log if isinstance(e, str))))
    setup(g, cur, q9, q10, q14, 128); cpu.run(S['TT23'])
    out.append('L %d %d %d %d %d : %s' % (g, cur, q9, q10, q14, ' '.join(e for e in cpu.log if isinstance(e, str))))
    for qq11 in (64, 128):
        dx = random.choice([0, 1, 255, 2, 254, random.randrange(256)]); dy = random.choice([0, 1, 255, 2, 254, random.randrange(256)])
        a9 = random.choice([q9, 0, 1, 254, 255]); a10 = random.choice([q10, 0, 1, 254, 255])
        setup(g, cur, a9, a10, q14, qq11); cpu.x = dx; cpu.y = dy; cpu.run(S['TT16'])
        out.append('M %d %d %d %d %d %d %d : %s : %d %d' % (qq11 >> 7, g, cur, a9, a10, dx, dy, ' '.join(e for e in cpu.log if isinstance(e, str)), m[S['QQ9']], m[S['QQ10']]))
        setup(g, cur, a9, a10, q14, qq11); cpu.run(S['hm'])
        out.append('H %d %d %d %d %d : %s : %d %d' % (qq11 >> 7, g, cur, a9, a10, ' '.join(e for e in cpu.log if isinstance(e, str)), m[S['QQ9']], m[S['QQ10']]))
# ---------------------------------------------------------------- F: HME2 (busqueda por nombre) con el motor de texto real
def blk(a, b): return preprocess(L[a - 1:b - 1])
def tok_arrays():
    h = open('src/tokens_data.h').read(); d = {}
    for mm in re.finditer(r'static const u8 (\w+)\[\d+\] = \{(.*?)\};', h, re.S):
        d[mm.group(1)] = [int(x) for x in re.findall(r'\d+', mm.group(2))]
    return d
TK = tok_arrays()
def fixs(lines):
    o = []
    for l in lines:
        t = code(l)
        mm = re.match(r'^EQUS\s+"(.*)"$', t)
        if mm: o.append(' EQUB ' + ', '.join(str(ord(ch)) for ch in mm.group(1))); continue
        if re.match(r'^DTW7\s*=', t): continue
        mm = re.match(r'^EQUD\s+\$([0-9A-Fa-f]+)$', t)
        if mm: v = int(mm.group(1), 16); o.append(' EQUB ' + ', '.join(str((v >> (8 * i)) & 255) for i in range(4))); continue
        if re.match(r'^(PRINT|SAVE)\b', t) or re.match(r'^(CODE|LOAD)_\w+%\s*=', t): continue
        o.append(l)
    return [x for x in o if code(x) != 'SKIP 0']
text = blk(nl('MT27'), nl('QQ16') + 0) + routine('QQ16') + blk(nl('DTW1'), nx(nl('BELL'))) + \
       blk(nl('cmn'), nx(nl('ex'))) + routine('cpl') + routine('TT20') + routine('TT54') + routine('R5') + routine('clss') + \
       routine('CHPR') + ['.RR5', ' RTS'] + routine('MTIN') + routine('cleargrap') + dornd + routine('HME2') + routine('TT81')
cpu, S = build(fixs(text), None)
print('stubs F:', sorted(k for k, v in S.items() if 0x3E00 <= v < 0x3F00), file=sys.stderr)
m = cpu.mem
m[S['QQ18']:S['QQ18'] + len(TK['tok_qq18'])] = bytes(TK['tok_qq18'])
t = TK['tok_tkn1'] + TK['tok_rupla'] + TK['tok_rugal'] + TK['tok_rutok']
m[S['TKN1']:S['TKN1'] + len(t)] = bytes(t)
cpu.hooks[S['DOXC']] = lambda c: c.wr(S['XC'], c.a)
term = []
def mt26(c):                                         # lectura de la linea: el texto en INWK+5 terminado en CR
    for i, ch in enumerate(term): c.mem[S['INWK'] + 5 + i] = ch
    c.mem[S['INWK'] + 5 + len(term)] = 13
cpu.hooks[S['MT26']] = mt26
res = []
cpu.hooks[S['HME5']] = lambda c: res.append(c.mem[S['XX20']])
QQ16 = TK['tok_tkn2'][26:]
for k in range(n):
    g = random.randrange(8)
    m[S['QQ21']:S['QQ21'] + 6] = bytes(seeds[g])
    i = random.randrange(256)
    m[S['QQ21']:S['QQ21'] + 6] = bytes(seeds[g]); cpu.run(S['TT81'])
    for _ in range(i): cpu.run(S['TT20'])
    got = []; c2 = CPU(); c2.mem[:] = m; c2.hooks = {S['TT27']: lambda c: got.append(c.a) if 128 <= c.a < 160 else None}
    c2.mem[S['QQ17']] = 0xFF; c2.run(S['cpl'])
    nm = ''
    for a in got:
        kk = (a & 127) * 2; nm += chr(QQ16[kk]) + ('' if QQ16[kk + 1] == ord('?') else chr(QQ16[kk + 1]))
    r = random.random()
    if r < 0.15: nm = nm[:-1]
    elif r < 0.3: nm = nm + random.choice('AEX')
    elif r < 0.4: nm = nm.lower()
    elif r < 0.5: nm = ''.join(random.choice('ABCDEFGHIJKLMNOPQRSTUVWXYZ') for _ in range(random.randrange(1, 9)))
    term = [ord(ch) for ch in nm.upper()] if random.random() < 0.85 else [ord(ch) for ch in nm]
    del res[:]
    m[S['QQ17']] = 0xC0                                # minusculas: el estado tras imprimir un titulo (con $80 no encuentra el sistema 0; con 0 o $40, ninguno)
    m[S['QQ9']] = random.randrange(256); m[S['QQ10']] = random.randrange(256)
    m[S['QQ12']] = 1; m[S['QQ11']] = 64
    cpu.log = []; cpu.run(S['HME2'])
    out.append('F %d %s : %d' % (g, ''.join(chr(ch) for ch in term), res[0] if res else -1))

open('tests/vectors_charts.txt', 'w').write('\n'.join(out) + '\n')
print(len(out), 'vectores')
