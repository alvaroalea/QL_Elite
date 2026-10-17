# Utilidades para ensamblar TRAMOS sueltos de la fuente original y ejecutarlos en el emulador (gen_laser.py, gen_damage.py).
# build() convierte en stubs que registran (nombre, A, X, Y) todo lo que no se resuelve; si una rama no llega a un stub,
# pone un RTS local al final. lab(nombre) = indice de la linea ".nombre"; rng(a, b) = lineas preprocesadas (IF/ELIF).
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
D = elitesrc.read('elite-data.asm')
nl = orig.name_line
nx = orig.next_name_after

def code(l): return re.sub(r';.*', '', l).strip()
def lab(name, start=0):
    for i in range(start, len(L)):
        if code(L[i]) == '.' + name: return i
    raise Exception('sin etiqueta ' + name)
def rng(a, b): return [re.sub(r'\s+AND\s+(?=\d)', ' & ', l) for l in preprocess(L[a:b])]
def routine(name): return preprocess(L[nl(name) - 1:nx(nl(name)) - 1])

def table(name):                          # EQUB de elite-data.asm a partir de .name
    i = [k for k, l in enumerate(D) if code(l) == '.' + name][0] + 1; out = []
    while len(out) < 32:
        t = code(D[i]); i += 1
        m = re.match(r'^EQUB\s+(.+)$', t)
        if m: out += [int(v.strip().replace('$', '0x'), 0) for v in m.group(1).split(',')]
    return out

def build(lines, logs, presets=None):
    """Ensambla; lo que no se resuelva pasa a ser un stub que registra (nombre, A, X, Y) y vuelve con RTS.
    presets: simbolos que se fijan antes (p. ej. tablas de datos con su propia direccion)."""
    stubs = []
    while True:
        S = elitesrc.parse_vars(); cpu = CPU(); cpu.log = []
        S.update(presets or {})
        for i, n in enumerate(stubs):
            S[n] = 0x3E00 + i; cpu.mem[0x3E00 + i] = 0x60
            cpu.hooks[0x3E00 + i] = (lambda nm: (lambda c: c.log.append((nm, c.a, c.x, c.y))))(n)
        try:
            assemble(lines, 0x4000, S, cpu)
            return cpu, S
        except Exception as e:
            m2 = re.match(r'salto fuera de rango: B.. (\w+)$', str(e))
            if m2 and not any(code(l) == '.' + m2.group(1) for l in lines):   # rama a algo de fuera: RTS local (sin registro)
                if m2.group(1) in stubs: stubs.remove(m2.group(1))
                lines = lines + ['.' + m2.group(1), 'RTS']; continue
            m = re.match(r'SIN_RESOLVER (.*)', str(e))
            if not m: raise
            new = [x for x in m.group(1).split() if x not in stubs]
            if not new: raise
            stubs += new

dornd = routine('DORND')
sq = preprocess(L[nl('SQUA') - 1:nx(nl('MU11')) - 1])

