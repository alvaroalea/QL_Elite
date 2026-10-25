#!/usr/bin/env python3
"""Interprete minimo de 68000 (solo las instrucciones que usa src/ql_line.asm) que ejecuta el TEXTO del ensamblador, para comprobar
la logica de la rutina sin un emulador de 68000 (aqui no hay qemu-m68k). No comprueba la codificacion de as68 (eso lo hace qcc al
enlazar): comprueba el algoritmo, los modos de direccionamiento, el orden de operandos y los indicadores (N, Z, V, C)."""
import re, sys

M32 = 0xFFFFFFFF

class CPU:
    def __init__(self, src, mem_size=0x40000):
        self.d = [0] * 8
        self.a = [0] * 8              # a7 = sp
        self.mem = bytearray(mem_size)
        self.n = self.z = self.v = self.c = self.x = 0
        self._ea_cache = {}
        self.prog, self.labels = [], {}
        self.syms = {}                # simbolos externos -> direccion (los pone el que usa la clase)
        self.natives = {}             # rutinas externas simuladas en Python (jsr)
        for raw in src.split('\n'):
            line = raw.split(';')[0].rstrip()
            if not line.strip(): continue
            while True:
                m = re.match(r'^([A-Za-z_][A-Za-z0-9_]*):\s*(.*)$', line)
                if not m: break
                self.labels[m.group(1)] = len(self.prog)
                line = m.group(2)
            line = line.strip()
            if not line or line.startswith('.'): continue
            parts = line.split(None, 1)
            op = parts[0].lower()
            args = [x.strip() for x in re.split(r',(?![^()]*\))', parts[1])] if len(parts) > 1 else []
            self.prog.append((op, args))

    # --- memoria
    def rd(self, addr, size):
        return int.from_bytes(self.mem[addr:addr + size], 'big')
    def wr(self, addr, size, val):
        self.mem[addr:addr + size] = (val & ((1 << (8 * size)) - 1)).to_bytes(size, 'big')

    # --- operandos
    @staticmethod
    def num(t):
        t = t.strip()
        neg = t.startswith('-')
        if neg: t = t[1:]
        v = int(t[1:], 16) if t.startswith('$') else int(t, 0)
        return -v if neg else v

    def ea_addr(self, t, size=4):
        t = t.strip()
        if t in self._ea_cache: return self._ea_cache[t]
        m = re.match(r'^-\((a[0-7]|sp)\)$', t)                                       # -(An): predecremento
        if m:
            r = self.reg(m.group(1)); self.a[r] = (self.a[r] - size) & M32
            self._ea_cache[t] = self.a[r]
            return self.a[r]
        m = re.match(r'^\((a[0-7]|sp)\)\+$', t)
        if m:
            r = self.reg(m.group(1)); addr = self.a[r] & M32
            self.a[r] = (self.a[r] + size) & M32
            self._ea_cache[t] = addr
            return addr
        m = re.match(r'^\((a[0-7]|sp)\)$', t)
        if m: return self.a[self.reg(m.group(1))] & M32
        m = re.match(r'^(-?\$?[0-9A-Fa-f]+)\((a[0-7]|sp)\)$', t)
        if m: return (self.a[self.reg(m.group(2))] + self.num(m.group(1))) & M32
        m = re.match(r'^(-?\$?[0-9A-Fa-f]*)\((a[0-7]|sp),(d[0-7])\.(w|l)\)$', t)      # d8(An,Dn.w): desplazamiento de 8 bits, indice con signo
        if m:
            disp = self.num(m.group(1)) if m.group(1) not in ('', '-') else 0
            assert -128 <= disp <= 127, 'desplazamiento indexado fuera de 8 bits: ' + t
            idx = self.d[int(m.group(3)[1])]
            idx = (idx & 0xFFFF) - 0x10000 if (m.group(4) == 'w' and idx & 0x8000) else (idx & 0xFFFF if m.group(4) == 'w' else idx)
            return (self.a[self.reg(m.group(2))] + disp + idx) & M32
        m = re.match(r'^([A-Za-z_][A-Za-z0-9_]*)(\+(\d+))?$', t)                        # simbolo absoluto (variables de C), con +N
        if m and m.group(1) in self.syms:
            return self.syms[m.group(1)] + (int(m.group(3)) if m.group(3) else 0)
        raise ValueError('modo no soportado: ' + t)

    @staticmethod
    def reg(name):
        return 7 if name == 'sp' else int(name[1])

    def get(self, t, size):
        t = t.strip()
        if t.startswith('#'): return self.num(t[1:]) & ((1 << (8 * size)) - 1)
        if re.match(r'^d[0-7]$', t): return self.d[int(t[1])] & ((1 << (8 * size)) - 1)
        if re.match(r'^(a[0-7]|sp)$', t): return self.a[self.reg(t)] & ((1 << (8 * size)) - 1)
        return self.rd(self.ea_addr(t, size), size)

    def put(self, t, size, val):
        t = t.strip()
        mask = (1 << (8 * size)) - 1
        if re.match(r'^d[0-7]$', t):
            i = int(t[1])
            self.d[i] = ((self.d[i] & ~mask) | (val & mask)) & M32
        elif re.match(r'^(a[0-7]|sp)$', t):
            self.a[self.reg(t)] = val & M32            # solo palabra larga (adda, lea, move.l a)
        else:
            self.wr(self.ea_addr(t, size), size, val)

    def flags_logic(self, val, size):
        bits = 8 * size
        val &= (1 << bits) - 1
        self.n = (val >> (bits - 1)) & 1; self.z = int(val == 0); self.v = self.c = 0

    def flags_add(self, a, b, r, size):
        bits = 8 * size; mask = (1 << bits) - 1
        a &= mask; b &= mask; res = r & mask
        self.n = (res >> (bits - 1)) & 1; self.z = int(res == 0)
        sa, sb, sr = a >> (bits - 1), b >> (bits - 1), res >> (bits - 1)
        self.v = int(sa == sb and sr != sa); self.c = int((a + b) > mask); self.x = self.c

    def flags_sub(self, dst, src, res, size):          # res = dst - src
        bits = 8 * size; mask = (1 << bits) - 1
        dst &= mask; src &= mask; r = res & mask
        self.n = (r >> (bits - 1)) & 1; self.z = int(r == 0)
        sd, ss, sr = dst >> (bits - 1), src >> (bits - 1), r >> (bits - 1)
        self.v = int(sd != ss and sr != sd); self.c = int(src > dst); self.x = self.c

    def cond(self, op):
        n, z, v, c = self.n, self.z, self.v, self.c
        return {'beq': z, 'bne': not z, 'bpl': not n, 'bmi': n, 'bcs': c, 'bcc': not c, 'bhi': (not c) and (not z),
                'bls': c or z, 'bge': n == v, 'blt': n != v, 'bgt': (not z) and n == v, 'ble': z or n != v, 'bra': 1}[op]

    @staticmethod
    def split_size(op, default):
        if '.' in op:
            base, sz = op.split('.')
            return base, {'b': 1, 'w': 2, 'l': 4, 's': 1}[sz]
        return op, default

    def regs_list(self, s):
        out = []
        for part in s.split('/'):
            if '-' in part:
                lo, hi = part.split('-')
                k = lo[0]
                for i in range(int(lo[1]), int(hi[1]) + 1): out.append((k, i))
            else: out.append((part[0], int(part[1])))
        order = [('d', i) for i in range(8)] + [('a', i) for i in range(8)]
        return [r for r in order if r in out]

    def run(self, entry, max_steps=2000000):
        pc = self.labels[entry]
        ret = 0xFFFFFFF0
        self.a[7] -= 4; self.wr(self.a[7], 4, ret)
        steps = 0
        while True:
            steps += 1
            if steps > max_steps: raise RuntimeError('demasiados pasos')
            op, args = self.prog[pc]; pc += 1
            self._ea_cache = {}
            base, size = self.split_size(op, 4)
            if base == 'rts':
                a = self.rd(self.a[7], 4); self.a[7] += 4
                if a == ret: return steps
                if 0x10000 <= a < 0x10000 + len(self.prog): pc = a - 0x10000; continue
                raise RuntimeError('rts a ' + hex(a))
            elif base == 'jsr':
                tgt = args[0].strip()
                if tgt in self.labels:
                    self.a[7] -= 4; self.wr(self.a[7], 4, pc + 0x10000); pc = self.labels[tgt]
                else:                                           # rutina externa simulada en Python (self.natives[nombre](cpu)): ve los argumentos desde a7
                    self.natives[tgt](self)
            elif base == 'bsr':
                self.a[7] -= 4; self.wr(self.a[7], 4, pc + 0x10000); pc = self.labels[args[0]]
            elif base == 'movem':
                if args[1].replace(' ', '') == '-(sp)':
                    for k, i in reversed(self.regs_list(args[0])):
                        self.a[7] -= 4; self.wr(self.a[7], 4, self.d[i] if k == 'd' else self.a[i])
                else:
                    for k, i in self.regs_list(args[1]):
                        val = self.rd(self.a[7], 4); self.a[7] += 4
                        if k == 'd': self.d[i] = val
                        else: self.a[i] = val
            elif base == 'move':
                v = self.get(args[0], size); self.put(args[1], size, v); self.flags_logic(v, size)
            elif base == 'moveq':
                v = self.num(args[0][1:]) & 0xFF
                v = v - 256 if v & 0x80 else v
                self.d[int(args[1][1])] = v & M32; self.flags_logic(v & M32, 4)
            elif base in ('add', 'sub', 'cmp'):
                s = self.get(args[0], size); d = self.get(args[1], size)
                if base == 'add':
                    r = d + s; self.put(args[1], size, r); self.flags_add(d, s, r, size)
                else:
                    r = d - s
                    if base == 'sub': self.put(args[1], size, r)
                    self.flags_sub(d, s, r, size)
            elif base == 'adda':
                self.a[self.reg(args[1])] = (self.a[self.reg(args[1])] + self.get(args[0], size)) & M32
            elif base == 'subq':
                if re.match(r'^(a[0-7]|sp)$', args[1].strip()):
                    self.a[self.reg(args[1])] = (self.a[self.reg(args[1])] - self.num(args[0][1:])) & M32
                else:
                    sv = self.num(args[0][1:]); d = self.get(args[1], size); r = d - sv
                    self.put(args[1], size, r); self.flags_sub(d, sv, r, size)
            elif base == 'addq':
                if re.match(r'^(a[0-7]|sp)$', args[1].strip()):
                    self.a[self.reg(args[1])] = (self.a[self.reg(args[1])] + self.num(args[0][1:])) & M32
                else:
                    s = self.num(args[0][1:]); d = self.get(args[1], size); r = d + s
                    self.put(args[1], size, r); self.flags_add(d, s, r, size)
            elif base in ('andi', 'ori', 'eori'):
                s = self.get(args[0], size); d = self.get(args[1], size)
                r = {'andi': s & d, 'ori': s | d, 'eori': s ^ d}[base]
                self.put(args[1], size, r); self.flags_logic(r, size)
            elif base in ('addx', 'subx'):
                s = self.get(args[0], size); d = self.get(args[1], size)
                if base == 'addx':
                    r = d + s + self.x; self.put(args[1], size, r)
                    bits = 8 * size; mask = (1 << bits) - 1
                    self.c = self.x = int(((d & mask) + (s & mask) + self.x) > mask)
                    res = r & mask
                    self.n = (res >> (bits - 1)) & 1
                    self.v = int(((d >> (bits - 1)) & 1) == ((s >> (bits - 1)) & 1) and self.n != ((d >> (bits - 1)) & 1))
                    if res: self.z = 0
                else:
                    r = d - s - self.x; self.put(args[1], size, r)
                    bits = 8 * size; mask = (1 << bits) - 1
                    self.c = self.x = int(((s & mask) + self.x) > (d & mask))
                    res = r & mask
                    self.n = (res >> (bits - 1)) & 1
                    self.v = int(((d >> (bits - 1)) & 1) != ((s >> (bits - 1)) & 1) and self.n != ((d >> (bits - 1)) & 1))
                    if res: self.z = 0
            elif base == 'divu':
                dv = self.get(args[0], 2); i = int(args[1][1]); dd = self.d[i] & M32
                assert dv != 0
                q, r = dd // dv, dd % dv
                if q > 0xFFFF: self.v = 1
                else:
                    self.d[i] = ((r & 0xFFFF) << 16) | q; self.v = 0
                    self.n = (q >> 15) & 1; self.z = int(q == 0); self.c = 0
            elif base == 'clr':
                self.put(args[0], size, 0); self.n = 0; self.z = 1; self.v = self.c = 0
            elif base in ('and', 'or', 'eor'):
                s = self.get(args[0], size); d = self.get(args[1], size)
                r = {'and': s & d, 'or': s | d, 'eor': s ^ d}[base]
                self.put(args[1], size, r); self.flags_logic(r, size)
            elif base in ('lsr', 'lsl'):
                if len(args) == 1: raise ValueError('lsr de memoria no soportado')
                cnt = self.num(args[0][1:]) if args[0].startswith('#') else self.d[int(args[0][1])] & 63
                v = self.get(args[1], size); bits = 8 * size; mask = (1 << bits) - 1
                c = 0
                for _ in range(cnt):
                    if base == 'lsr': c = v & 1; v >>= 1
                    else: c = (v >> (bits - 1)) & 1; v = (v << 1) & mask
                self.put(args[1], size, v); self.flags_logic(v, size)
                self.c = c if cnt else 0
                if cnt: self.x = c
            elif base == 'neg':
                d = self.get(args[0], size); r = -d
                self.put(args[0], size, r); self.flags_sub(0, d, r, size)
            elif base == 'tst':
                self.flags_logic(self.get(args[0], size), size)
            elif base == 'btst':
                bit = self.num(args[0][1:])
                if re.match(r'^d[0-7]$', args[1].strip()): v = self.get(args[1], 4); bit &= 31
                else: v = self.get(args[1], 1); bit &= 7           # memoria: un byte
                self.z = int(((v >> bit) & 1) == 0)
            elif base == 'exg':
                i, j = int(args[0][1]), int(args[1][1])
                self.d[i], self.d[j] = self.d[j], self.d[i]
            elif base == 'lea':
                self.a[self.reg(args[1])] = self.ea_addr(args[0])
            elif base == 'dbra' or base == 'dbf':
                i = int(args[0][1]); w = (self.d[i] - 1) & 0xFFFF
                self.d[i] = (self.d[i] & ~0xFFFF) | w
                if w != 0xFFFF: pc = self.labels[args[1]]
            elif base in ('bra', 'beq', 'bne', 'bpl', 'bmi', 'bcs', 'bcc', 'bhi', 'bls', 'bge', 'blt', 'bgt', 'ble'):
                if self.cond(base): pc = self.labels[args[0]]
            else:
                raise ValueError('instruccion no soportada: ' + op)

def run_line(src, x1, y1, x2, y2, c, scr=0x20000):
    cpu = CPU(src)
    cpu.a[7] = 0x1F000
    cpu.syms['_gfx_m4'] = 0x1C000                       # ql_line mira _gfx_m4 (0 = modo 8)
    args = [x1, y1, x2, y2, c, scr]
    for v in reversed(args):
        cpu.a[7] -= 4; cpu.wr(cpu.a[7], 4, v)
    cpu.run('_ql_line')
    return cpu

if __name__ == '__main__':
    print('usar tools/test_lineasm.py')
