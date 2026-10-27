# Minimal assembler for the reference source syntax (".X" labels, EQUB/EQUW, P%, LO()/HI(), %bin, $hex)
import re
from cpu6502 import OPS, SIZE

def split_operand(op):
    op = op.strip()
    if op == '' : return 'imp', None
    if op.upper() == 'A': return 'acc', None
    if op.startswith('#'): return 'imm', op[1:]
    m = re.match(r'^\((.+),X\)$', op)
    if m: return 'izx', m.group(1)
    m = re.match(r'^\((.+)\),Y$', op)
    if m: return 'izy', m.group(1)
    m = re.match(r'^\((.+)\)$', op)
    if m: return 'ind', m.group(1)
    m = re.match(r'^(.+),X$', op)
    if m: return 'abx', m.group(1)
    m = re.match(r'^(.+),Y$', op)
    if m: return 'aby', m.group(1)
    return 'abs', op

def assemble(lines, org, syms, cpu, externs=None):
    """Assembles 'lines' into cpu.mem from org; adds the labels to syms. Returns the final address."""
    items = []
    for l in lines:
        t = re.sub(r';.*', '', l).strip()
        if not t or t in ('{', '}'): continue
        if re.match(r'^(PRINT|SAVE|CODE_|LOAD_|CLEAR|COPYBLOCK|IF|ENDIF|ELSE|ASSERT)\b', t) and not re.match(r'^[.]', t): continue
        items.append(t)
    unresolved = set()
    for pas in (1, 2):
        pc = org; out = []
        for t in items:
            m = re.match(r'^\.([A-Za-z_][A-Za-z_0-9%]*)$', t)
            if m:
                if pas == 1:
                    if m.group(1) in syms and syms[m.group(1)] != pc: raise Exception('etiqueta repetida ' + m.group(1))
                    syms[m.group(1)] = pc
                continue
            m = re.match(r'^([A-Za-z_][A-Za-z_0-9%]*)\s*=\s*(.+)$', t)
            if m:
                try: syms[m.group(1)] = syms.ev(m.group(2), pc)
                except Exception: pass
                continue
            m = re.match(r'^(EQUB|EQUW)\s+(.+)$', t)
            if m:
                for e in [x for x in re.split(r',(?![^(]*\))', m.group(2)) if x.strip()]:
                    try: v = syms.ev(e, pc)
                    except Exception:
                        if pas == 2: raise
                        v = 0
                    if m.group(1) == 'EQUB': cpu.mem[pc] = v & 255; pc += 1
                    else: cpu.mem[pc] = v & 255; cpu.mem[pc + 1] = (v >> 8) & 255; pc += 2
                continue
            m = re.match(r'^([A-Z]{3})\b\s*(.*)$', t)
            if not m or m.group(1) not in OPS: raise Exception('no entiendo: ' + t)
            mn, rest = m.group(1), m.group(2)
            mode, ex = split_operand(rest)
            val = None
            if ex is not None:
                try: val = syms.ev(ex, pc)
                except NameError as e:
                    miss = re.search(r"name '(.+?)' is not defined", str(e)).group(1)
                    if pas == 2: unresolved.add(miss.replace('_P', '%'))
                    val = 0x1000
                except Exception:
                    if pas == 2: raise Exception('simbolo sin resolver en: ' + t)
                    val = 0x1000
            d = OPS[mn]
            if mode == 'abs' and 'rel' in d: mode = 'rel'
            elif mode == 'abs' and 'zp' in d and val < 256 and 'abs' in d and not (mn in ('JMP', 'JSR')): mode = 'zp'
            elif mode == 'abx' and val < 256 and 'zpx' in d: mode = 'zpx'
            elif mode == 'aby' and val < 256 and 'zpy' in d: mode = 'zpy'
            if mode not in d: raise Exception('modo %s no valido en: %s' % (mode, t))
            sz = SIZE[mode]
            if pas == 2:
                cpu.mem[pc] = d[mode]
                if mode == 'rel':
                    off = val - (pc + 2)
                    if not -128 <= off <= 127: raise Exception('salto fuera de rango: ' + t)
                    cpu.mem[pc + 1] = off & 255
                elif sz == 2: cpu.mem[pc + 1] = val & 255
                elif sz == 3: cpu.mem[pc + 1] = val & 255; cpu.mem[pc + 2] = (val >> 8) & 255
            pc += sz
    if unresolved: raise Exception('SIN_RESOLVER ' + ' '.join(sorted(unresolved)))
    return pc
