# Utilities to assemble loose STRETCHES of the original source and run them in the emulator (gen_laser.py, gen_damage.py).
# build() turns into stubs that log (name, A, X, Y) everything that is not resolved; if a branch does not reach a stub,
# it puts a local RTS at the end. lab(name) = index of the line ".name"; rng(a, b) = preprocessed lines (IF/ELIF).
import sys, re, random
sys.path.insert(0, 'tools')
import elitesrc, orig
from cpu6502 import CPU
from asm6502 import assemble
FLAGS = {'_SOURCE_DISK': 1, '_IB_DISK': 0, '_4AM_CRACK': 0}

def preprocess(lines):
    """IF / ELIF / ELSE / ENDIF with the FLAGS flags (the source's disk drive: _SOURCE_DISK)."""
    out = []; stack = []          # each level: [parent_active, already_taken, active]
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
def rng(a, b): return [re.sub(r'\s+AND\s+(?=\d)', ' & ', l) for l in preprocess(L[a:b]) if code(l) != 'SKIP 0']
def routine(name): return preprocess(L[nl(name) - 1:nx(nl(name)) - 1])

def table(name):                          # EQUB of elite-data.asm from .name
    i = [k for k, l in enumerate(D) if code(l) == '.' + name][0] + 1; out = []
    while len(out) < 32:
        t = code(D[i]); i += 1
        m = re.match(r'^EQUB\s+(.+)$', t)
        if m: out += [int(v.strip().replace('$', '0x').replace('%', '0b'), 0) for v in m.group(1).split(',')]
    return out

def build(lines, logs, presets=None):
    """Assembles; whatever is not resolved becomes a stub that logs (name, A, X, Y) and returns with RTS.
    presets: symbols set beforehand (e.g. data tables with their own address)."""
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
            if m2 and not any(code(l) == '.' + m2.group(1) for l in lines):   # branch to something outside: local RTS (no log)
                if m2.group(1) in stubs: stubs.remove(m2.group(1))
                lines = lines + ['.' + m2.group(1), 'RTS']; continue
            m = re.match(r'SIN_RESOLVER (.*)', str(e)) or re.match(r"name '(\w+)' is not defined", str(e))   # EQUW to routines outside
            if not m: raise
            new = [x for x in m.group(1).split() if x not in stubs]
            if not new: raise
            stubs += new

dornd = routine('DORND')
sq = preprocess(L[nl('SQUA') - 1:nx(nl('MU11')) - 1])

