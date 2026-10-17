#!/usr/bin/env python3
"""Genera tests/vectors_missions.txt ejecutando en el emulador de 6502 el codigo ORIGINAL de:
   T  DETOK (fichas extendidas de TKN1), DETOK3 (RUTOK) y TT27 (fichas normales de QQ18) con TT26/DASC (justificacion),
      CHPR, las fichas de salto (MT1..MT29), DT6/MTIN (al azar con DORND), cpl/ypl/cmn reales; letter, TT66, PAUSE2,
      DELAY, BEEP, NLIN4, CLYNS... registran
   P  PDESC con NRU% = 26 (ver missions.c) y con el NRU% = 0 del original (solo informa de cuantos casos cambian)
   M  la parte de las misiones de DOENTRY (BRIEF, DEBRIEF, BRIEF2, BRIEF3, DEBRIEF2 registran) y BRIEF2/BRIEF3/
      DEBRIEF2/DEBRIEF (TP, ENGY, TALLY, CASH con MCASH real; DETOK registra) y el bucle de BRIEF (LL9 y MVEIT registran INWK)
   E  partes 2 a 4 del bucle principal (aparicion de naves): Ze, ZINF, DORND, GTHG, THERE y BAD reales; NWSHP se imita
      (ranura libre de FRIN, MANY, JUNK, NEWB |= E% AND $6F, acarreo) y registra INWK
Requiere ELITE_SRC. Uso: python3 tools/gen_missions.py [n] [semilla]"""
import sys, random, re
sys.path.insert(0, 'tools')
from frag6502 import *

n = int(sys.argv[1]) if len(sys.argv) > 1 else 2000
random.seed(int(sys.argv[2]) if len(sys.argv) > 2 else 1985)
hx = lambda b: ''.join('%02x' % x for x in b)
def rb(): return random.randrange(256)
def pick(*v): return random.choice(v)
out = []

def tok_arrays():                                  # tablas de src/tokens_data.h (generadas por gen_tokens.py)
    h = open('src/tokens_data.h').read(); d = {}
    for m in re.finditer(r'static const u8 (\w+)\[\d+\] = \{(.*?)\};', h, re.S):
        d[m.group(1)] = [int(x) for x in re.findall(r'\d+', m.group(2))]
    return d
TK = tok_arrays()

def blk(a, b): return preprocess(L[a - 1:b - 1])
def named(*names): return sum((routine(x) for x in names), [])

text = blk(nl('MT27'), nl('QQ16') + 0) + routine('QQ16') + blk(nl('DTW1'), nx(nl('BELL'))) + \
       blk(nl('cmn'), nx(nl('ex'))) + routine('cpl') + routine('TT54') + routine('R5') + routine('clss') + \
       routine('CHPR') + ['.RR5', ' RTS'] + routine('MTIN') + blk(nl('BRIS'), nl('PAUSE2')) + routine('cleargrap') + dornd
text = [l for l in text if code(l) != 'SKIP 0']

def fix(lines):                                    # EQUS "XY" -> EQUB; "DTW7 = MT16 + 1" -> se quita (DTW7 no se usa aqui)
    o = []
    for l in lines:
        t = code(l)
        m = re.match(r'^EQUS\s+"(.*)"$', t)
        if m: o.append(' EQUB ' + ', '.join(str(ord(c)) for c in m.group(1))); continue
        if re.match(r'^DTW7\s*=', t): continue
        o.append(l)
    return o
text = fix(text)

# ---------------------------------------------------------------- T: fichas
cpu, S = build(text, None)
def load_tables(cpu):
    cpu.mem[S['QQ18']:S['QQ18'] + len(TK['tok_qq18'])] = bytes(TK['tok_qq18'])
    t = TK['tok_tkn1'] + TK['tok_rupla'] + TK['tok_rugal'] + TK['tok_rutok']
    cpu.mem[S['TKN1']:S['TKN1'] + len(t)] = bytes(t)
load_tables(cpu)
ev = []
def logev(*a): ev.append(','.join(str(x) for x in a))
def on_letter(c):
    x = c.x - 12
    assert x % 7 == 0
    logev('L%d' % c.a, x // 7, c.y)
cpu.hooks[S['letter']] = on_letter
cpu.hooks[S['DOXC']] = lambda c: c.wr(S['XC'], c.a)
cpu.hooks[S['cleargrap']] = lambda c: logev('C')
cpu.hooks[S['TT66']] = lambda c: logev('T%d' % c.a)
cpu.hooks[S['PAUSE2']] = lambda c: logev('P')
cpu.hooks[S['DELAY']] = lambda c: logev('D%d' % c.y)
cpu.hooks[S['BEEP']] = lambda c: logev('B')
cpu.hooks[S['NLIN4']] = lambda c: logev('N')
cpu.hooks[S['CLYNS']] = lambda c: logev('Y')
for k in ('TT67', 'BPRNT', 'pr2'):                 # dinero, combustible, galaxia: no se usan en las fichas (casos descartados)
    cpu.hooks[S[k]] = (lambda k: lambda c: logev('X' + k))(k)

QQ16 = TK['tok_tkn2'][26:]
def name_of(seed):                                 # cpl en el emulador: las fichas de dos letras que imprime
    got = []
    c2 = CPU(); c2.mem[:] = cpu.mem; c2.hooks = {S['TT27']: lambda c: got.append(c.a) if 128 <= c.a < 160 else None}
    c2.mem[S['QQ15']:S['QQ15'] + 6] = bytes(seed); c2.mem[S['QQ17']] = 0xFF   # sin imprimir
    c2.run(S['cpl'])
    s = ''
    for a in got:
        k = (a & 127) * 2; s += chr(QQ16[k]) + ('' if QQ16[k + 1] == ord('?') else chr(QQ16[k + 1]))
    assert bytes(c2.mem[S['QQ15']:S['QQ15'] + 6]) == bytes(seed)
    return s

def ntok(tab):                                     # numero de fichas de la tabla (delimitador VE)
    return sum(1 for b in tab if b ^ 0x57 == 0)
NT1, NRT = ntok(TK['tok_tkn1']), ntok(TK['tok_rutok'])
skipped = []
VARS = ['QQ17', 'DTW1', 'DTW2', 'DTW3', 'DTW4', 'DTW5', 'DTW6', 'DTW8', 'XC', 'YC']
def tcase(kind, tok):
    st = {'QQ17': pick(0, 0x80, 0xC0, 0x40, 0x80, 0), 'DTW1': pick(0, 0x20), 'DTW2': pick(0, 0xFF, 0x80),
          'DTW3': 0, 'DTW4': pick(0, 0x80, 0x80, 0xC0), 'DTW5': 0,
          'DTW6': pick(0, 0x80), 'DTW8': pick(0xFF, 0xFF, 0xDF), 'XC': random.randrange(32), 'YC': random.randrange(24)}
    gc, mj = pick(0, 1, 2, rb() & 7), pick(0, 0, 0, 0xFF)
    rnd = [rb() for _ in range(4)]
    sel, cur = [rb() for _ in range(6)], [rb() for _ in range(6)]
    nm = ''.join(chr(random.randrange(65, 91)) for _ in range(random.randrange(1, 8)))
    for k, v in st.items(): cpu.mem[S[k]] = v
    cpu.mem[S['GCNT']] = gc; cpu.mem[S['MJ']] = mj; cpu.mem[S['text']] = 0
    cpu.mem[S['RAND']:S['RAND'] + 4] = bytes(rnd)
    cpu.mem[S['QQ15']:S['QQ15'] + 6] = bytes(sel); cpu.mem[S['QQ2']:S['QQ2'] + 6] = bytes(cur)
    cpu.mem[S['NAME']:S['NAME'] + 8] = bytes([ord(c) for c in nm] + [13] * (8 - len(nm)))
    iw = [rb() for _ in range(37)]; k3 = [rb() for _ in range(14)]
    cpu.mem[S['INWK']:S['INWK'] + 37] = bytes(iw); cpu.mem[S['K3']:S['K3'] + 14] = bytes(k3)
    del ev[:]
    cpu.a = tok; cpu.c = rb() & 1
    cpu.run(S[{'E': 'DETOK', 'R': 'DETOK3', 'S': 'TT27'}[kind]])
    fin = [cpu.mem[S[k]] for k in VARS]
    if any(e[0] == 'X' for e in ev): skipped.append(tok); return
    out.append('T %s %d %s %d %d %s %s %s %s %s %s %s | %s %s %s' % (kind, tok, ' '.join(str(st[k]) for k in VARS), gc, mj, hx(rnd), hx(iw), hx(k3),
               name_of(sel) or '-', (name_of(cur) or '-') if not mj else '-', nm, ';'.join(ev) or '-', ' '.join(map(str, fin + [cpu.mem[S['K3']]])),
               hx(cpu.mem[S['RAND']:S['RAND'] + 4]), hx(cpu.mem[0x100:0x100 + cpu.mem[S['DTW5']]]) or '-'))

for tok in range(1, NT1): tcase('E', tok)
for tok in range(1, NRT): tcase('R', tok)
for tok in list(range(2, 5)) + list(range(6, 256)): tcase('S', tok)
for _ in range(n):
    r = random.random()
    if r < 0.6: tcase('E', random.randrange(1, NT1))
    elif r < 0.8: tcase('R', random.randrange(1, NRT))
    else: tcase('S', pick(2, 3, 4, 6, 7, 8, 9, 10, 12, 13, random.randrange(10, 256)))
print('T:', len(out), 'casos; fichas TKN1', NT1, 'RUTOK', NRT, '; descartadas (dinero/combustible):', sorted(set(skipped)), file=sys.stderr)


# ---------------------------------------------------------------- P: PDESC
RUPLA, RUGAL = TK['tok_rupla'], TK['tok_rugal']
cases = []
for k in range(n):
    j = random.randrange(26)
    zz = RUPLA[j] if random.random() < 0.8 else rb()
    gc = (RUGAL[j] & 0x7F) if random.random() < 0.8 else pick(0, 1, 2, rb() & 7)
    cases.append((zz, gc, pick(0, 1, 2, 3, 5, rb()), pick(0, 0, 0, 1, 256), pick(0x80, 0x80, 0xFF, 0), [rb() for _ in range(4)],
                  [rb() for _ in range(6)]))
res = {}
for nru in (26, 0):
    cpu, S = build(fix(routine('PDESC')) + text, None, {'NRU%': nru})
    load_tables(cpu)
    cpu.hooks[S['letter']] = on_letter; cpu.hooks[S['DOXC']] = lambda c: c.wr(S['XC'], c.a)
    cpu.hooks[S['cleargrap']] = lambda c: logev('C'); cpu.hooks[S['TT66']] = lambda c: logev('T%d' % c.a)
    cpu.hooks[S['PAUSE2']] = lambda c: logev('P'); cpu.hooks[S['DELAY']] = lambda c: logev('D%d' % c.y)
    pd1 = []
    cpu.hooks[S['PD1']] = lambda c: pd1.append(1)
    diff = 0
    for k, (zz, gc, tp, q8, q12, rnd, q15) in enumerate(cases if nru else cases[:n // 4]):
        st = {'QQ17': 0x80, 'DTW1': 0x20, 'DTW2': 0xFF, 'DTW3': 0, 'DTW4': 0, 'DTW5': 0, 'DTW6': 0, 'DTW8': 0xFF, 'XC': 1, 'YC': 10}
        for kk, v in st.items(): cpu.mem[S[kk]] = v
        cpu.mem[S['ZZ']] = zz; cpu.mem[S['GCNT']] = gc; cpu.mem[S['TP']] = tp; cpu.mem[S['QQ12']] = q12
        cpu.mem[S['QQ8']] = q8 & 255; cpu.mem[S['QQ8'] + 1] = q8 >> 8; cpu.mem[S['RAND']:S['RAND'] + 4] = bytes(rnd)
        cpu.mem[S['QQ15']:S['QQ15'] + 6] = bytes(q15); cpu.mem[S['MJ']] = 0
        cpu.mem[S['INWK']:S['INWK'] + 37] = bytes(37); cpu.mem[S['K3']:S['K3'] + 14] = bytes(14)
        del ev[:]; del pd1[:]
        try: cpu.run(S['PDESC'], maxsteps=500000)
        except Exception: ev.append('CUELGUE'); pd1[:] = []
        r = (0 if pd1 else 1, ';'.join(ev) if not pd1 else '-', hx(cpu.mem[S['RAND']:S['RAND'] + 4]) if not pd1 else '-')
        if nru:
            res[k] = r
            out.append('P %d %d %d %d %d %s %s | %d %s %s' % ((zz, gc, tp, q8, q12, hx(rnd), name_of(q15) or '-') + r))
        elif r != res[k]: diff += 1
print('PDESC con el NRU%% = 0 del original: %d de %d casos cambian (texto de memoria ajena o cuelgue)' % (diff, n // 4), file=sys.stderr)

# ---------------------------------------------------------------- M: misiones (DOENTRY, BRIEF*, DEBRIEF*)
i0 = [k for k in range(lab('DOENTRY'), lab('EN4')) if code(L[k]) == 'LDA TP'][0]
doen = ['.DOEN'] + rng(i0, lab('EN4')) + ['.EN4', ' JMP BAY']
mis = rng(nl('BRIEF2') - 1, nx(nl('DEBRIEF')) - 1) + routine('MCASH')
cpu, S = build(doen + mis, None)
for _ in range(n):
    tp = pick(0, 1, 2, 3, 4, 6, 0x0A, 0x0E, 0x0B, rb(), rb() & 0x0F)
    t1, gc = pick(0, 1, 4, 5, 6, rb()), pick(0, 1, 2, 3, rb() & 7)
    q0, q1 = pick((215, 84), (63, 72), (144, 33), (rb(), rb()))
    cash = [rb() for _ in range(4)]
    cpu.mem[S['TP']] = tp; cpu.mem[S['TALLY'] + 1] = t1; cpu.mem[S['GCNT']] = gc; cpu.mem[S['QQ0']] = q0; cpu.mem[S['QQ1']] = q1
    cpu.mem[S['CASH']:S['CASH'] + 4] = bytes(cash); cpu.mem[S['ENGY']] = 1
    cpu.log = []
    cpu.run(S['DOEN'])
    who = ['BRIEF'] if any(x[0] == 'BRIEF' for x in cpu.log) else (['TOK'] if any(x[0] in ('DETOK', 'BRP') for x in cpu.log) else ['BAY'])
    # BRIEF y DEBRIEF van por stub (su codigo esta en mis: si se ejecuto, el log tiene DETOK)
    tok = [x[1] for x in cpu.log if x[0] in ('DETOK', 'BRP')]
    out.append('M %d %d %d %d %d %s | %s %d %d %d %s %s' % (tp, t1, gc, q0, q1, hx(cash), who[0] if who else '-', cpu.mem[S['TP']],
               cpu.mem[S['ENGY']], cpu.mem[S['TALLY'] + 1], hx(cpu.mem[S['CASH']:S['CASH'] + 4]), tok[0] if tok else '-'))

# BRIEF: LL9 y MVEIT registran (INWK 0, 3, 6, 7, 29, 30)
cpu, S = build(routine('BRIEF') + routine('ZINF') + ['.BRPS', ' JMP BRP'], None)
lg = []
cpu.hooks[S['LL9']] = lambda c: lg.append('%d.%d.%d.%d.%d.%d' % tuple(c.mem[S['INWK'] + k] for k in (0, 3, 6, 7, 29, 30)))
cpu.hooks[S['DOXC']] = lambda c: c.wr(S['XC'], c.a)
for _ in range(20):
    tp = rb(); cpu.mem[S['TP']] = tp; del lg[:]; cpu.log = []
    cpu.run(S['BRIEF'])
    nw = [x for x in cpu.log if x[0] == 'NWSHP']
    tok = [x[1] for x in cpu.log if x[0] in ('DETOK', 'BRP', 'BRPS')]
    out.append('B %d | %d %d %d %d %s' % (tp, cpu.mem[S['TP']], nw[0][1], tok[-1], len(lg), ';'.join(lg)))

# ---------------------------------------------------------------- E: aparicion de naves (partes 1 a 4)
i2 = [k for k in range(lab('ytq'), lab('MTT1')) if code(L[k]) == 'LDA MJ'][0]
enc = rng(lab('MTT4'), lab('TT100')) + ['.TT100', ' RTS', '.ytq', ' JMP MLOOP', '.ENC'] + rng(i2, lab('MLOOP')) + ['.MLOOP', ' RTS'] + \
      routine('Ze') + dornd + routine('ZINF') + routine('THERE') + routine('BAD') + routine('GTHG')
cpu, S = build(enc, None)
E_ = table('E%')
NOSH_ = S['NOSH']
made = []
def nwshp(c):
    t = c.a; fr = S['FRIN']
    x = next((k for k in range(NOSH_) if c.mem[fr + k] == 0), None)
    if x is None: made.append('%d:-' % t); c.c = 0; return
    c.mem[fr + x] = t
    if t == S['HER'] or S['JL'] <= t < S['JH']: c.mem[S['JUNK']] += 1
    c.mem[S['MANY'] + t] += 1
    c.mem[S['INWK'] + 36] |= E_[t - 1] & 0x6F
    iw = bytearray(c.mem[S['INWK']:S['INWK'] + 37]); iw[31] = 0; iw[35] = 0
    made.append('%d:%d:%s' % (t, x, hx(iw))); c.c = 1
cpu.hooks[S['NWSHP']] = nwshp
end = []
cpu.hooks[S['MLOOP']] = lambda c: end.append('MLOOP')
cpu.hooks[S['TT100']] = lambda c: end.append('TT100')
for _ in range(n * 4):
    rnd = [rb() for _ in range(4)]; c0 = rb() & 1
    mj = pick(0, 0, 0, 0, 0xFF); junk = pick(0, 0, 1, 2, 3); sspr = pick(0, 0, 0, 1)
    cops = pick(0, 0, 0, 1); con = pick(0, 0, 1); tp = pick(0, 1, 3, 8, 9, 0x0C, rb()); gc = pick(1, 1, 0, 2)
    q0, q1 = pick((144, 33), (144, 33), (rb(), rb())); gov = pick(0, 0, 1, 3, 7, rb() & 7)
    cargo = [pick(0, 0, 1, 5, 40, rb()) for _ in range(3)]; fist = pick(0, 0, 10, 40, 80, rb()); evc = pick(0, 0, 0xFF, 1, 2, 3)
    occ = [k for k in range(NOSH_) if random.random() < pick(0.1, 0.3, 0.6)]
    fr = S['FRIN']
    for k in range(NOSH_ + 1): cpu.mem[fr + k] = 0
    for k in occ: cpu.mem[fr + k] = 0x81 if k == 0 else 7
    for k in range(40): cpu.mem[S['MANY'] + k] = 0
    cpu.mem[S['MANY'] + S['SST']] = sspr; cpu.mem[S['MANY'] + S['COPS']] = cops; cpu.mem[S['MANY'] + S['CON']] = con
    for k, v in (('MJ', mj), ('JUNK', junk), ('TP', tp), ('GCNT', gc), ('QQ0', q0), ('QQ1', q1), ('gov', gov), ('FIST', fist), ('EV', evc)):
        cpu.mem[S[k]] = v
    cpu.mem[S['QQ20'] + 3] = cargo[0]; cpu.mem[S['QQ20'] + 6] = cargo[1]; cpu.mem[S['QQ20'] + 10] = cargo[2]
    cpu.mem[S['RAND']:S['RAND'] + 4] = bytes(rnd)
    del made[:]; del end[:]
    cpu.c = c0
    cpu.run(S['ENC'])
    out.append('E %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %s | %s %s %d %d %d %d %d' % (hx(rnd), c0, mj, junk, sspr, cops, con, tp, gc, q0, q1, gov,
               cargo[0], cargo[1], cargo[2], fist, evc, sum(1 << k for k in occ), ' '.join(made) or '-', end[0] if end else '-',
               hx(cpu.mem[S['RAND']:S['RAND'] + 4]), cpu.mem[S['EV']], cpu.mem[S['JUNK']], cpu.mem[S['MANY'] + S['COPS']],
               cpu.mem[S['MANY'] + S['CON']], cpu.c))


# ---------------------------------------------------------------- S: estacion (parte 14 del bucle de vuelo, KS4) y SOLAR
p14 = [k for k, l in enumerate(L) if re.match(r'^;\s+Name: Main flight loop \(Part 14 of 16\)', l)][0]
p15 = [k for k, l in enumerate(L) if re.match(r'^;\s+Name: Main flight loop \(Part 15 of 16\)', l)][0]
j14 = [k for k in range(p14, p15) if code(L[k]) == 'LDA MJ'][0]
frag = ['.P14'] + rng(j14, p15) + ['.MA93', ' RTS'] + routine('MAS1') + rng(nl('MAS2') - 1, nx(nl('MAS2')) - 1) + routine('MVT3') + \
       rng(nl('FAROF') - 1, nx(nl('FAROF2')) - 1) + routine('NWSPS') + ['.NWSHP', ' RTS'] + routine('NwS1')
math_ = rng(nl('MULT1') - 1, nx(nl('ADD')) - 1)
cpu, S = build(frag + math_, None, {'XX21': 0x2800, 'spasto': 0x2900, 'LSO': 0x2A00})
got = []
cpu.hooks[S['NWSHP']] = lambda c: got.append((c.a, bytes(c.mem[S['INWK']:S['INWK'] + 37])))
NI_ = S['NI%']
def sgn(r):
    v = random.randrange(-r, r + 1); m = abs(v)
    return [m & 255, (m >> 8) & 255, ((m >> 16) & 0x7F) | (0x80 if v < 0 else 0)]
for _ in range(n * 2):
    pl = sgn(pick(0x6000, 0x8000, 0xC000, 0xFFFF, 0x30000)) + sgn(pick(0x6000, 0x8000, 0xFFFF, 0x14000)) + sgn(pick(0x6000, 0x8000, 0xFFFF, 0x20000))
    nose = []
    for _k in range(9):
        v = random.choice([random.randrange(-0x6000, 0x6001), 0x6000, -0x6000, 0]); m = abs(v)
        nose += [m & 255, ((m >> 8) & 0x7F) | (0x80 if v < 0 else 0)]
    pl += nose + [rb() for _k in range(NI_ - 27)]
    iw = [rb() for _k in range(NI_)]
    mj, mc, sspr, tek = pick(0, 0, 0, 0, 0xFF), pick(0, 0, 0, 0, 32, 64, 5, rb()), pick(0, 0, 0, 0, 1), rb() & 15
    cpu.mem[S['K%']:S['K%'] + NI_] = bytes(pl); cpu.mem[S['INWK']:S['INWK'] + NI_] = bytes(iw)
    cpu.mem[S['MJ']] = mj; cpu.mem[S['MCNT']] = mc; cpu.mem[S['MANY'] + S['SST']] = sspr; cpu.mem[S['tek']] = tek
    cpu.mem[S['FRIN'] + 1] = 129
    del got[:]; cpu.log = []
    cpu.run(S['P14'])
    w = any(x[0] == 'WPLS' for x in cpu.log)
    out.append('S %s %s %d %d %d %d | %d %s' % (hx(pl), hx(iw), mj, mc, sspr, tek, int(w), ('%d:%s' % (got[0][0], hx(got[0][1]))) if got else '-'))

# KS4: KILLSHP de la estacion
cpu, S = build(routine('KS4') + routine('KILLSHP') + ['.NWSHP', ' RTS'] + routine('ZINF'), None)
cpu.hooks[S['FLFLLS']] = lambda c: setattr(c, 'a', 0)       # FLFLLS vuelve con A = 0
got = []
cpu.hooks[S['NWSHP']] = lambda c: got.append((c.a, bytes(c.mem[S['INWK']:S['INWK'] + 37])))
for _ in range(10):
    iw = [rb() for _k in range(37)]
    cpu.mem[S['INWK']:S['INWK'] + 37] = bytes(iw); cpu.mem[S['FRIN'] + 1] = S['SST']; cpu.mem[S['MANY'] + S['SST']] = 1
    cpu.mem[S['MSTG']] = 0xFF; del got[:]; cpu.log = []
    cpu.x = 1
    cpu.run(S['KILLSHP'])
    out.append('K %s | %d %d %d %d %s' % (hx(iw), cpu.mem[S['FRIN'] + 1], cpu.mem[S['MANY'] + S['SST']], int(any(x[0] == 'SPBLB' for x in cpu.log)),
               got[0][0], hx(got[0][1])))

# SOLAR (sin SOS1: msblob registra; NWSHP registra)
cpu, S = build(routine('SOLAR') + routine('SOS1') + routine('ZINF') + dornd, None)
got = []
cpu.hooks[S['NWSHP']] = lambda c: got.append('%d:%s' % (c.a, hx(c.mem[S['INWK']:S['INWK'] + 37])))
for _ in range(n):
    q15 = [rb() for _k in range(6)]; fist, tek = pick(0, 1, 40, 81, rb()), rb() & 15
    tr = [pick(0, 0, 0, rb()), pick(0, 0, rb() & 0x7F, rb())]; rnd = [rb() for _k in range(4)]; c0 = rb() & 1
    cpu.mem[S['QQ15']:S['QQ15'] + 6] = bytes(q15); cpu.mem[S['FIST']] = fist; cpu.mem[S['tek']] = tek
    cpu.mem[S['TRIBBLE']:S['TRIBBLE'] + 2] = bytes(tr); cpu.mem[S['RAND']:S['RAND'] + 4] = bytes(rnd)
    cpu.mem[S['QQ20']] = 9; cpu.mem[S['QQ20'] + 6] = 9
    cpu.mem[S['INWK']:S['INWK'] + 37] = bytes(37)
    del got[:]
    cpu.c = c0
    cpu.run(S['SOLAR'])
    out.append('O %s %d %d %s %s %d | %d %s %d %d %s %s' % (hx(q15), fist, tek, hx(tr), hx(rnd), c0, cpu.mem[S['FIST']],
               hx(cpu.mem[S['TRIBBLE']:S['TRIBBLE'] + 2]), cpu.mem[S['QQ20']], cpu.mem[S['QQ20'] + 6], hx(cpu.mem[S['RAND']:S['RAND'] + 4]), ' '.join(got)))

open('tests/vectors_missions.txt', 'w').write('\n'.join(out) + '\n')
print(len(out), 'vectores')
