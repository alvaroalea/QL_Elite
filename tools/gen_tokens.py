#!/usr/bin/env python3
"""Genera src/tokens_data.h con las tablas de texto del original, ensamblando las macros de la fuente:
   QQ18 (fichas normales, EOR RE), TKN1 (fichas extendidas, EOR VE), RUPLA/RUGAL/RUTOK (descripciones especiales),
   TKN2+QQ16 (fichas de dos letras, contiguas como en memoria: MT18 lee de las dos) y MTIN (fichas aleatorias).
Si existen los binarios de referencia (4-reference-binaries/ib-disk/WORDS.bin e IANTOK.bin) los compara byte a byte.
Requiere ELITE_SRC. Uso: python3 tools/gen_tokens.py"""
import os, re, sys
SRC = os.environ.get('ELITE_SRC', '')
RE_, VE = 0x23, 0x57

def lines(name): return open(os.path.join(SRC, name), encoding='latin-1').read().split('\n')
def code(l): return l.split(';')[0].strip()
S, D = lines('elite-source.asm'), lines('elite-data.asm')

def after(L, label):
    i = [k for k, l in enumerate(L) if code(l) == '.' + label][0] + 1
    return i

def pairs(L, label, n):                       # EQUS "XY" / EQUB a, b
    i = after(L, label); out = []
    while len(out) < 2 * n:
        t = code(L[i]); i += 1
        m = re.match(r'^EQUS\s+"(..)"$', t)
        if m: out += [ord(c) for c in m.group(1)]; continue
        m = re.match(r'^EQUB\s+(\d+)\s*,\s*(\d+)$', t)
        if m: out += [int(m.group(1)), int(m.group(2))]
    return out

TKN2 = pairs(S, 'TKN2', 13)                   # fichas 215..227
QQ16 = pairs(S, 'QQ16', 32)                   # fichas 128..159 (y 228..255 de ETWO a partir de la 14 de TKN2)
ETWOP = [(chr(TKN2[2 * i]), chr(TKN2[2 * i + 1])) for i in range(13)] + [(chr(QQ16[2 * i]), chr(QQ16[2 * i + 1])) for i in range(32)]
ETWOP[0] = ('-', '-')
MTIN = []
i = after(S, 'MTIN')
while len(MTIN) < 38:
    m = re.match(r'^EQUB\s+(\d+)$', code(S[i])); i += 1
    if m: MTIN.append(int(m.group(1)))

def ch(s):
    s = s.strip()
    m = re.match(r"^'(.)'$", s) or re.match(r'^"(.)"$', s)
    if not m: raise Exception('caracter? ' + s)
    c = m.group(1)
    return 39 if c == '`' else ord(c)

def num(s):
    s = s.strip().replace('$', '0x').replace('%', '0b')
    return int(s, 0)

def assemble(L, start, stop):
    out = []
    for l in L[start:stop]:
        t = code(l)
        if not t or t.startswith('.'): continue
        op, _, arg = t.partition(' ')
        args = [a.strip() for a in re.split(r",(?=(?:[^']*'[^']*')*[^']*$)", arg)] if arg else []
        if op == 'CHAR': out.append(ch(args[0]) ^ RE_)
        elif op == 'CONT': out.append(num(args[0]) ^ RE_)
        elif op == 'TWOK':
            k = (chr(ch(args[0])), chr(ch(args[1])))
            out.append((128 + [(chr(QQ16[2 * j]), chr(QQ16[2 * j + 1])) for j in range(32)].index(k)) ^ RE_)
        elif op in ('RTOK', 'TOKN'):
            n = num(args[0]); t2 = n + 160 if n <= 95 else (n - 114 if n >= 128 else n)
            out.append(t2 ^ (RE_ if op == 'RTOK' else VE))
        elif op == 'ECHR': out.append(ch(args[0]) ^ VE)
        elif op in ('EJMP', 'ETOK'): out.append(num(args[0]) ^ VE)
        elif op == 'ERND': out.append((num(args[0]) + 91) ^ VE)
        elif op == 'ETWO': out.append((215 + ETWOP.index((chr(ch(args[0])), chr(ch(args[1]))))) ^ VE)
        elif op == 'EQUB':
            for a in args: out.append(VE if a == 'VE' else num(a))
        elif op in ('IF', 'ELIF', 'ELSE', 'ENDIF', 'SKIP', 'PRINT', 'SAVE', 'INCBIN', 'ORG'): break
        else: raise Exception('macro? ' + t)
    return out

qq18 = assemble(D, after(D, 'QQ18'), after(D, 'SNE') - 1)
tkn1 = assemble(D, after(D, 'TKN1'), after(D, 'RUPLA') - 1)
rupla = assemble(D, after(D, 'RUPLA'), after(D, 'RUGAL') - 1)
rugal = assemble(D, after(D, 'RUGAL'), after(D, 'RUTOK') - 1)
rutok = assemble(D, after(D, 'RUTOK'), after(D, 'endian') - 1)
assert len(rupla) == len(rugal) == 26

ref = os.path.join(SRC, '..', '..', '4-reference-binaries', 'ib-disk')
if os.path.exists(os.path.join(ref, 'IANTOK.bin')):
    w = open(os.path.join(ref, 'WORDS.bin'), 'rb').read()
    ia = open(os.path.join(ref, 'IANTOK.bin'), 'rb').read()
    assert bytes(qq18) == w[:len(qq18)], 'QQ18 distinto del binario de referencia'
    assert len(tkn1) == 0xB1E and bytes(tkn1 + rupla + rugal + rutok) == ia[:len(tkn1) + 52 + len(rutok)], 'TKN1/RUTOK distintos'
    print('tablas iguales a WORDS.bin e IANTOK.bin', file=sys.stderr)

def arr(name, b, typ='u8'):
    s = 'static const %s %s[%d] = {\n' % (typ, name, len(b))
    for k in range(0, len(b), 20): s += '    ' + ', '.join('%d' % x for x in b[k:k + 20]) + ',\n'
    return s + '};\n'

h = '/* Generado por tools/gen_tokens.py desde elite-data.asm y elite-source.asm (Apple II): tablas de texto del\n' \
    '   original, en bruto (QQ18 con EOR RE = $23, TKN1 y RUTOK con EOR VE = $57). No editar. */\n'
h += '#define TOK_RE 0x%02X\n#define TOK_VE 0x%02X\n#define TOK_NRU %d\n' % (RE_, VE, len(rupla))
h += arr('tok_qq18', qq18) + arr('tok_tkn1', tkn1) + arr('tok_rutok', rutok)
h += arr('tok_rupla', rupla) + arr('tok_rugal', rugal)
h += '/* TKN2 (fichas 215..227) seguida de QQ16 (128..159), como en memoria */\n' + arr('tok_tkn2', TKN2 + QQ16)
h += arr('tok_mtin', MTIN)
open('src/tokens_data.h', 'w').write(h)
print('src/tokens_data.h: QQ18 %d, TKN1 %d, RUTOK %d bytes' % (len(qq18), len(tkn1), len(rutok)))
