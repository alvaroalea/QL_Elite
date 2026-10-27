# Utilities to read the reference source (Mark Moxon, elite-source-code-apple-ii) ONLY to verify:
# symbol table (variables), assembly of routines with a minimal 6502 assembler and ship data.
# The source is not included in the project; it must be cloned (see tools/README_ref.txt) and ELITE_SRC pointed at it.
import os, re
SRC_DIR = os.environ.get('ELITE_SRC', os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'ref', 'moxon-appleII', '1-source-files', 'main-sources'))

def read(name):
    return open(os.path.join(SRC_DIR, name)).read().split('\n')

def conv(e):
    e = re.sub(r'%([01]+)', lambda m: str(int(m.group(1), 2)), e)       # binarios %0101
    e = re.sub(r'\$([0-9A-Fa-f]+)', lambda m: str(int('0x' + m.group(1), 16)), e)
    e = re.sub(r'&([0-9A-Fa-f]+)', lambda m: str(int('0x' + m.group(1), 16)), e)
    e = e.replace('<<', '<<').replace('>>', '>>')
    return e

class Syms(dict):
    def ev(self, expr, pc=None):
        e = expr.strip()
        e = re.sub(r'"(.)"', lambda m: str(ord(m.group(1))), e)
        e = conv(e)
        e = re.sub(r'\bLO\(', '_LO(', e); e = re.sub(r'\bHI\(', '_HI(', e)
        e = e.replace('P%', '_PC')
        e = re.sub(r"([A-Za-z_][A-Za-z_0-9]*)%", lambda m: m.group(1) + '_P', e)   # NI% -> NI_P
        env = {'INT': lambda v: int(v), '_LO': lambda v: v & 255, '_HI': lambda v: (v >> 8) & 255, '_PC': pc if pc is not None else 0}
        for k, v in self.items(): env[k.replace('%', '_P')] = v
        return int(eval(e, {'__builtins__': {}}, env))

def parse_vars(upto_marker='ORG CODE%'):
    """Walks the source up to the code and returns the symbols of variables and constants."""
    L = read('elite-source.asm')
    S = Syms(); pc = 0
    for l in L:
        t = re.sub(r';.*', '', l).strip()
        if not t: continue
        if t.startswith('ORG') and 'CODE%' in t: break
        m = re.match(r'^ORG\s+(.+)$', t)
        if m:
            try: pc = S.ev(m.group(1))
            except Exception: pass
            continue
        m = re.match(r'^\.([A-Za-z_][A-Za-z_0-9%]*)$', t)
        if m: S[m.group(1)] = pc; continue
        m = re.match(r'^([A-Za-z_][A-Za-z_0-9%]*)\s*=\s*(.+)$', t)
        if m:
            try: S[m.group(1)] = S.ev(m.group(2), pc)
            except Exception: pass
            continue
        m = re.match(r'^SKIP\s+(.+)$', t)
        if m:
            try: pc += S.ev(m.group(1), pc)
            except Exception: pass
            continue
        m = re.match(r'^(EQUB|EQUW|EQUD)\s+(.+)$', t)
        if m:
            n = len([x for x in m.group(2).split(',') if x.strip()])
            pc += n * {'EQUB': 1, 'EQUW': 2, 'EQUD': 4}[m.group(1)]
    return S

if __name__ == '__main__':
    S = parse_vars()
    for k in ['X1', 'XX15', 'XX1', 'INWK', 'XX19', 'NEWB', 'XX2', 'XX16', 'XX0', 'INF', 'V', 'XX12', 'XX18', 'XX17', 'XX13', 'TYPE', 'CNT', 'XX4', 'XX20', 'LSNUM', 'LSNUM2', 'XX3', 'SWAP', 'K%', 'FRIN', 'UNIV', 'K', 'K3', 'P', 'Q', 'R', 'S', 'T', 'U', 'RAND', 'NI%', 'X', 'Y', 'Y1', 'X2', 'Y2', 'T1', 'COL']:
        print(k, hex(S[k]) if k in S else 'NO')
