# Minimal 6502 emulator (documented instructions, no decimal mode) and assembler for the syntax of the
# reference source. Only to verify the ported routines against the original code.
import re

OPS = {
 'ADC': dict(imm=0x69, zp=0x65, zpx=0x75, abs=0x6D, abx=0x7D, aby=0x79, izx=0x61, izy=0x71),
 'AND': dict(imm=0x29, zp=0x25, zpx=0x35, abs=0x2D, abx=0x3D, aby=0x39, izx=0x21, izy=0x31),
 'ASL': dict(acc=0x0A, zp=0x06, zpx=0x16, abs=0x0E, abx=0x1E),
 'BCC': dict(rel=0x90), 'BCS': dict(rel=0xB0), 'BEQ': dict(rel=0xF0), 'BMI': dict(rel=0x30),
 'BNE': dict(rel=0xD0), 'BPL': dict(rel=0x10), 'BVC': dict(rel=0x50), 'BVS': dict(rel=0x70),
 'BIT': dict(zp=0x24, abs=0x2C),
 'CLC': dict(imp=0x18), 'CLD': dict(imp=0xD8), 'CLI': dict(imp=0x58), 'CLV': dict(imp=0xB8),
 'CMP': dict(imm=0xC9, zp=0xC5, zpx=0xD5, abs=0xCD, abx=0xDD, aby=0xD9, izx=0xC1, izy=0xD1),
 'CPX': dict(imm=0xE0, zp=0xE4, abs=0xEC), 'CPY': dict(imm=0xC0, zp=0xC4, abs=0xCC),
 'DEC': dict(zp=0xC6, zpx=0xD6, abs=0xCE, abx=0xDE), 'DEX': dict(imp=0xCA), 'DEY': dict(imp=0x88),
 'EOR': dict(imm=0x49, zp=0x45, zpx=0x55, abs=0x4D, abx=0x5D, aby=0x59, izx=0x41, izy=0x51),
 'INC': dict(zp=0xE6, zpx=0xF6, abs=0xEE, abx=0xFE), 'INX': dict(imp=0xE8), 'INY': dict(imp=0xC8),
 'JMP': dict(abs=0x4C, ind=0x6C), 'JSR': dict(abs=0x20),
 'LDA': dict(imm=0xA9, zp=0xA5, zpx=0xB5, abs=0xAD, abx=0xBD, aby=0xB9, izx=0xA1, izy=0xB1),
 'LDX': dict(imm=0xA2, zp=0xA6, zpy=0xB6, abs=0xAE, aby=0xBE),
 'LDY': dict(imm=0xA0, zp=0xA4, zpx=0xB4, abs=0xAC, abx=0xBC),
 'LSR': dict(acc=0x4A, zp=0x46, zpx=0x56, abs=0x4E, abx=0x5E), 'NOP': dict(imp=0xEA),
 'ORA': dict(imm=0x09, zp=0x05, zpx=0x15, abs=0x0D, abx=0x1D, aby=0x19, izx=0x01, izy=0x11),
 'PHA': dict(imp=0x48), 'PHP': dict(imp=0x08), 'PLA': dict(imp=0x68), 'PLP': dict(imp=0x28),
 'ROL': dict(acc=0x2A, zp=0x26, zpx=0x36, abs=0x2E, abx=0x3E),
 'ROR': dict(acc=0x6A, zp=0x66, zpx=0x76, abs=0x6E, abx=0x7E),
 'RTI': dict(imp=0x40), 'RTS': dict(imp=0x60),
 'SBC': dict(imm=0xE9, zp=0xE5, zpx=0xF5, abs=0xED, abx=0xFD, aby=0xF9, izx=0xE1, izy=0xF1),
 'SEC': dict(imp=0x38), 'SED': dict(imp=0xF8), 'SEI': dict(imp=0x78),
 'STA': dict(zp=0x85, zpx=0x95, abs=0x8D, abx=0x9D, aby=0x99, izx=0x81, izy=0x91),
 'STX': dict(zp=0x86, zpy=0x96, abs=0x8E), 'STY': dict(zp=0x84, zpx=0x94, abs=0x8C),
 'TAX': dict(imp=0xAA), 'TAY': dict(imp=0xA8), 'TSX': dict(imp=0xBA), 'TXA': dict(imp=0x8A),
 'TXS': dict(imp=0x9A), 'TYA': dict(imp=0x98),
}
DEC = {}
for n, d in OPS.items():
    for m, o in d.items(): DEC[o] = (n, m)
SIZE = dict(imp=1, acc=1, imm=2, zp=2, zpx=2, zpy=2, abs=3, abx=3, aby=3, ind=3, izx=2, izy=2, rel=2)

class CPU:
    def __init__(s):
        s.mem = bytearray(65536); s.a = s.x = s.y = 0; s.sp = 0xFF; s.pc = 0
        s.c = s.z = s.n = s.v = s.i = s.d = 0
        s.hooks = {}; s.steps = 0
    def rd(s, a): return s.mem[a & 0xFFFF]
    def wr(s, a, v): s.mem[a & 0xFFFF] = v & 255
    def rd16(s, a): return s.mem[a & 0xFFFF] | (s.mem[(a + 1) & 0xFFFF] << 8)
    def setnz(s, v): v &= 255; s.z = int(v == 0); s.n = v >> 7; return v
    def push(s, v): s.wr(0x100 + s.sp, v); s.sp = (s.sp - 1) & 255
    def pull(s): s.sp = (s.sp + 1) & 255; return s.rd(0x100 + s.sp)
    def ea(s, mode):
        p = s.pc + 1
        if mode == 'imm': return p
        if mode == 'zp': return s.rd(p)
        if mode == 'zpx': return (s.rd(p) + s.x) & 255
        if mode == 'zpy': return (s.rd(p) + s.y) & 255
        if mode == 'abs': return s.rd16(p)
        if mode == 'abx': return (s.rd16(p) + s.x) & 0xFFFF
        if mode == 'aby': return (s.rd16(p) + s.y) & 0xFFFF
        if mode == 'izx':
            z = (s.rd(p) + s.x) & 255; return s.rd(z) | (s.rd((z + 1) & 255) << 8)
        if mode == 'izy':
            z = s.rd(p); return ((s.rd(z) | (s.rd((z + 1) & 255) << 8)) + s.y) & 0xFFFF
        raise Exception('modo ' + mode)
    def run(s, start, stop_sp=0xFF, maxsteps=2000000):
        """Runs a routine as a JSR to 'start' until it returns (RTS with the initial stack)."""
        s.sp = 0xFF; s.push(0xFF); s.push(0xFE)               # sentinel return address $FFFF
        s.pc = start; s.steps = 0
        while True:
            if s.pc == 0xFFFF: return
            h = s.hooks.get(s.pc)
            if h: h(s)
            s.step(); s.steps += 1
            if s.steps > maxsteps: raise Exception('bucle infinito en %04X' % s.pc)
    def branch(s, cond, ):
        off = s.rd(s.pc + 1); off = off - 256 if off > 127 else off
        s.pc = (s.pc + 2 + (off if cond else 0)) & 0xFFFF
    def step(s):
        op = s.mem[s.pc]
        if op not in DEC: raise Exception('opcode %02X en %04X' % (op, s.pc))
        n, m = DEC[op]
        if m == 'rel':
            cond = {'BCC': not s.c, 'BCS': s.c, 'BEQ': s.z, 'BNE': not s.z, 'BMI': s.n, 'BPL': not s.n,
                    'BVC': not s.v, 'BVS': s.v}[n]
            s.branch(cond); return
        if n == 'JMP':
            a = s.rd16(s.pc + 1)
            if m == 'ind': a = s.rd(a) | (s.rd((a & 0xFF00) | ((a + 1) & 0xFF)) << 8)
            s.pc = a; return
        if n == 'JSR':
            r = (s.pc + 2) & 0xFFFF; s.push(r >> 8); s.push(r & 255); s.pc = s.rd16(s.pc + 1); return
        if n == 'RTS':
            lo = s.pull(); hi = s.pull(); s.pc = ((hi << 8 | lo) + 1) & 0xFFFF; return
        size = SIZE[m]
        if m in ('imp', 'acc'):
            if n == 'ASL' or n == 'LSR' or n == 'ROL' or n == 'ROR': s.a = s.shift(n, s.a)
            else: s.implied(n)
            s.pc = (s.pc + size) & 0xFFFF; return
        a = s.ea(m)
        if n in ('LDA', 'LDX', 'LDY'):
            v = s.setnz(s.rd(a))
            if n == 'LDA': s.a = v
            elif n == 'LDX': s.x = v
            else: s.y = v
        elif n == 'STA': s.wr(a, s.a)
        elif n == 'STX': s.wr(a, s.x)
        elif n == 'STY': s.wr(a, s.y)
        elif n == 'AND': s.a = s.setnz(s.a & s.rd(a))
        elif n == 'ORA': s.a = s.setnz(s.a | s.rd(a))
        elif n == 'EOR': s.a = s.setnz(s.a ^ s.rd(a))
        elif n == 'ADC':
            v = s.rd(a); t = s.a + v + s.c
            s.v = int(((s.a ^ t) & (v ^ t) & 0x80) != 0); s.c = int(t > 255); s.a = s.setnz(t)
        elif n == 'SBC':
            v = s.rd(a) ^ 255; t = s.a + v + s.c
            s.v = int(((s.a ^ t) & (v ^ t) & 0x80) != 0); s.c = int(t > 255); s.a = s.setnz(t)
        elif n in ('CMP', 'CPX', 'CPY'):
            r = {'CMP': s.a, 'CPX': s.x, 'CPY': s.y}[n]; t = r - s.rd(a)
            s.c = int(t >= 0); s.setnz(t)
        elif n == 'BIT':
            v = s.rd(a); s.z = int((s.a & v) == 0); s.n = v >> 7; s.v = (v >> 6) & 1
        elif n in ('ASL', 'LSR', 'ROL', 'ROR'): s.wr(a, s.shift(n, s.rd(a)))
        elif n == 'INC': s.wr(a, s.setnz(s.rd(a) + 1))
        elif n == 'DEC': s.wr(a, s.setnz(s.rd(a) - 1))
        else: raise Exception('instruccion ' + n)
        s.pc = (s.pc + size) & 0xFFFF
    def shift(s, n, v):
        if n == 'ASL': s.c = v >> 7; r = (v << 1) & 255
        elif n == 'LSR': s.c = v & 1; r = v >> 1
        elif n == 'ROL': c = s.c; s.c = v >> 7; r = ((v << 1) | c) & 255
        else: c = s.c; s.c = v & 1; r = (v >> 1) | (c << 7)
        return s.setnz(r)
    def implied(s, n):
        if n == 'CLC': s.c = 0
        elif n == 'SEC': s.c = 1
        elif n == 'CLD': s.d = 0
        elif n == 'SED': s.d = 1
        elif n == 'CLI': s.i = 0
        elif n == 'SEI': s.i = 1
        elif n == 'CLV': s.v = 0
        elif n == 'INX': s.x = s.setnz(s.x + 1)
        elif n == 'INY': s.y = s.setnz(s.y + 1)
        elif n == 'DEX': s.x = s.setnz(s.x - 1)
        elif n == 'DEY': s.y = s.setnz(s.y - 1)
        elif n == 'TAX': s.x = s.setnz(s.a)
        elif n == 'TAY': s.y = s.setnz(s.a)
        elif n == 'TXA': s.a = s.setnz(s.x)
        elif n == 'TYA': s.a = s.setnz(s.y)
        elif n == 'TSX': s.x = s.setnz(s.sp)
        elif n == 'TXS': s.sp = s.x
        elif n == 'PHA': s.push(s.a)
        elif n == 'PLA': s.a = s.setnz(s.pull())
        elif n == 'PHP': s.push(s.c | s.z << 1 | s.i << 2 | s.d << 3 | 0x30 | s.v << 6 | s.n << 7)
        elif n == 'PLP':
            p = s.pull(); s.c = p & 1; s.z = (p >> 1) & 1; s.i = (p >> 2) & 1; s.d = (p >> 3) & 1; s.v = (p >> 6) & 1; s.n = p >> 7
        elif n == 'NOP': pass
        else: raise Exception('implicita ' + n)
