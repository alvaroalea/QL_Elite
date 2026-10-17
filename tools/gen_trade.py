#!/usr/bin/env python3
"""Genera tests/vectors_trade.txt ejecutando en el emulador de 6502 el codigo ORIGINAL de:
   tnpr (espacio en bodega), el tramo de compra de TT219 (tnpr + GCASH + LCASH + actualizacion de QQ20/AVL)
   y el tramo de venta de TT208 (QQ20 - R, GCASH, MCASH).
Las rutinas de pantalla se sustituyen por stubs; gnum devuelve la cantidad pedida. Requiere ELITE_SRC (ver README_ref.txt).
Uso: python3 tools/gen_trade.py [n] [semilla]"""
import sys, re, random
sys.path.insert(0, 'tools')
import elitesrc, orig
from cpu6502 import CPU
from asm6502 import assemble

FLAGS = {'_SOURCE_DISK': 1, '_IB_DISK': 0, '_4AM_CRACK': 0}

def preprocess(lines):
    """IF / ELIF / ELSE / ENDIF con las banderas de FLAGS (la disquetera de la fuente: _SOURCE_DISK)."""
    out = []; stack = []          # cada nivel: [activo_padre, ya_tomada, activo]
    def ev(e):
        e = re.sub(r'\bOR\b', ' or ', e); e = re.sub(r'\bAND\b', ' and ', e); e = re.sub(r'\bNOT\b', ' not ', e)
        e = re.sub(r'[A-Za-z_][A-Za-z_0-9]*', lambda m: str(FLAGS.get(m.group(0), 0)) if m.group(0) not in ('or', 'and', 'not') else m.group(0), e)
        return bool(eval(e, {'__builtins__': {}}, {}))
    for l in lines:
        t = re.sub(r';.*', '', l).strip()
        m = re.match(r'^(IF|ELIF)\s+(.+)$', t)
        if m and m.group(1) == 'IF':
            par = all(s[2] for s in stack); c = ev(m.group(2)); stack.append([par, c, par and c]); continue
        if m and m.group(1) == 'ELIF':
            s = stack[-1]; c = (not s[1]) and ev(m.group(2)); s[1] = s[1] or c; s[2] = s[0] and c; continue
        if t == 'ELSE': s = stack[-1]; s[2] = s[0] and not s[1]; s[1] = True; continue
        if t == 'ENDIF': stack.pop(); continue
        if all(s[2] for s in stack):
            code = l.split(';')[0]
            out.append(re.sub(r"'(.)'", lambda m: '"' + m.group(1) + '"', code))
    return out

L = elitesrc.read('elite-source.asm')
def seg(a, b): return L[a - 1:b - 1]
nl = orig.name_line

def build():
    buy = preprocess(seg(nl('TT219'), nl('TT208')))
    sell = preprocess(seg(nl('TT208'), nl('TT213')))
    # etiqueta de entrada de la venta: justo antes de "JSR gnum" que sigue a TT210
    k = max(i for i, l in enumerate(sell) if re.match(r'^\s*JSR gnum\b', l))
    sell.insert(k, '.SELLGO')
    tnpr = preprocess(seg(nl('tnpr'), orig.next_name_after(nl('tnpr'))))
    mult = preprocess(seg(nl('MU1'), orig.next_name_after(nl('MU1'))) + seg(nl('MULTU'), orig.next_name_after(nl('MU11'))))   # MU1 tiene su propia cabecera; MU11 es el cuerpo de MULTU
    gc = preprocess(seg(nl('GCASH'), nl('EQSHP')))
    lc = preprocess(seg(nl('LCASH'), nl('GCASH')))          # LCASH + MCASH
    stubs = []
    while True:
        S = elitesrc.parse_vars(); cpu = CPU()
        for i, n in enumerate(stubs):
            S[n] = 0x3E00 + i; cpu.mem[0x3E00 + i] = 0x60
        try:
            assemble(buy + sell + tnpr + mult + gc + lc, 0x4000, S, cpu)      # todo junto: hay referencias entre tramos
            return cpu, S
        except Exception as e:
            m = re.match(r'SIN_RESOLVER (.*)', str(e))
            if not m: raise
            new = [x for x in m.group(1).split() if x not in stubs]
            if not new: raise
            stubs += new

cpu, S = build()
cpu.mem[0x3EFF] = 0x60
RTS = 0x3EFF
V = lambda n: S[n]
state = {}

def gnum_hook(c):                                            # JSR gnum -> A = R = cantidad pedida, C = 0
    q = state['qty']; c.mem[V('R')] = q; c.a = q; c.c = 0; c.z = int(q == 0); c.n = q >> 7
    lo = c.pull(); hi = c.pull(); c.pc = (((hi << 8) | lo) + 1) & 0xFFFF
def stop(tag):
    def h(c): state['end'] = tag; c.pc = RTS
    return h
cpu.hooks[V('gnum')] = gnum_hook
cpu.hooks[V('TT222')] = stop('ok')
cpu.hooks[V('Tc')] = stop('fail')
cpu.hooks[V('TQ4')] = stop('TQ4')
cpu.hooks[V('TT212')] = stop('sold')

def put_hold(h): cpu.mem[V('QQ20'):V('QQ20') + 17] = bytes(h)
def get_hold(): return bytes(cpu.mem[V('QQ20'):V('QQ20') + 17])
def put_cash(v): cpu.mem[V('CASH'):V('CASH') + 4] = v.to_bytes(4, 'big')
def get_cash(): return int.from_bytes(cpu.mem[V('CASH'):V('CASH') + 4], 'big')
hx = lambda b: ''.join('%02x' % x for x in b)

n = int(sys.argv[1]) if len(sys.argv) > 1 else 6000
random.seed(int(sys.argv[2]) if len(sys.argv) > 2 else 1985)

def rand_hold(crgo, reachable):
    """bodega: en partidas reales las toneladas (0..12) suman <= crgo-2 y kg/g <= 199"""
    h = [0] * 17
    if reachable:
        room = crgo - 2
        for i in random.sample(range(13), random.randrange(0, 5)):
            v = random.randrange(0, room + 1) if random.random() < 0.7 else random.choice([0, 1, room // 2])
            v = min(v, room - sum(h[:13])); h[i] = max(v, 0)
        if random.random() < 0.3:                       # a veces casi llena
            i = random.randrange(13); h[i] = max(0, room - sum(h[:13]) + h[i] - random.choice([0, 0, 1, 2]))
        for i in range(13, 17):
            if random.random() < 0.4: h[i] = random.choice([0, 1, 50, 150, 198, 199, random.randrange(200)])
    else:
        h = [random.choice([0, random.randrange(256)]) for _ in range(17)]
    return h

out = []
for i in range(n):
    reach = random.random() < 0.8
    crgo = random.choice([22, 22, 22, 37, 37, 21, 23, 36]) if reach else random.randrange(256)
    g = random.randrange(17)
    h = rand_hold(crgo, reach)
    trib = random.choice([0, 0, 0, 1, 2]) if reach else random.randrange(256)
    avl = random.randrange(64) if reach else random.randrange(256)
    price = random.randrange(256)
    qty = random.randrange(0, avl + 1) if reach else random.randrange(256)
    if reach and random.random() < 0.3 and avl > 0: qty = random.choice([avl, 1, max(0, avl - 1)])
    # ---- tnpr
    put_hold(h); cpu.mem[V('TRIBBLE') + 1] = trib; cpu.mem[V('CRGO')] = crgo; cpu.mem[V('QQ29')] = g
    cpu.a = qty; cpu.c = random.randrange(2)
    cpu.run(V('tnpr'))
    out.append('N %d %d %s %d %d %d %d %d' % (reach, g, hx(h), trib, crgo, qty, cpu.c, cpu.a))
    # ---- compra
    put_hold(h); cpu.mem[V('TRIBBLE') + 1] = trib; cpu.mem[V('CRGO')] = crgo; cpu.mem[V('QQ29')] = g
    cpu.mem[V('AVL') + g] = avl; cpu.mem[V('QQ25')] = avl; cpu.mem[V('QQ24')] = price
    cash = random.choice([random.randrange(0, 2 ** 20), random.randrange(0, 2 ** 31), price * qty * 4, price * qty * 4 - 1, price * qty * 4 + 1, 0, 1000]) & 0xFFFFFFFF
    put_cash(cash); state['qty'] = qty; state['end'] = None
    cpu.mem[V('QQ17')] = 0
    cpu.run(V('TT223K'))
    y = cpu.y if state['end'] == 'fail' else 0
    out.append('B %d %d %d %d %d %s %d %d %d %s %d %d %08x' % (reach, g, qty, avl, price, hx(h), trib, crgo, cash, 'x', 0 if state['end'] == 'ok' else 1, y, get_cash()) + ' ' + hx(get_hold()) + ' %d' % cpu.mem[V('AVL') + g])
    # ---- venta
    h2 = rand_hold(crgo, reach)
    hold_g = h2[g]
    q2 = random.randrange(0, hold_g + 1) if reach or random.random() < 0.5 else random.randrange(256)
    if hold_g and random.random() < 0.3: q2 = random.choice([hold_g, 1])
    put_hold(h2); cpu.mem[V('QQ29')] = g; cpu.mem[V('QQ24')] = price; cpu.mem[V('QQ25')] = hold_g
    cash = random.choice([random.randrange(0, 2 ** 20), random.randrange(0, 2 ** 31), 0, 1000, 0xFFFFFFFF - 5, 0xFFFF0000]) & 0xFFFFFFFF
    put_cash(cash); state['qty'] = q2; state['end'] = None
    cpu.mem[V('YY') if 'YY' in S else 0] = cpu.mem[V('YY') if 'YY' in S else 0]
    cpu.run(V('SELLGO'))
    out.append('S %d %d %d %d %s %08x %08x' % (reach, g, q2, price, hx(h2), cash, get_cash()) + ' ' + hx(get_hold()))

open('tests/vectors_trade.txt', 'w').write('\n'.join(out) + '\n')
print(len(out), 'vectores')
