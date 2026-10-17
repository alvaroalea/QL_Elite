# Construye una imagen 6502 con las rutinas ORIGINALES de la fuente de referencia (ensambladas con asm6502)
# para comparar contra los puertos en C. Las rutinas externas se sustituyen por "stubs" con RTS que registran
# la llamada. Requiere la fuente de referencia clonada (variable ELITE_SRC).
import sys, re
sys.path.insert(0, 'tools')
import elitesrc
from cpu6502 import CPU
from asm6502 import assemble

def src_range(a, b):
    return elitesrc.read('elite-source.asm')[a - 1:b]

def name_line(name, nth=0):
    L = elitesrc.read('elite-source.asm'); k = 0
    for i, l in enumerate(L):
        if re.match(r'^;\s+Name: %s\b' % re.escape(name), l):
            if k == nth: return i + 1
            k += 1
    raise Exception('no existe ' + name)

def build(ranges, stubs=(), org=0x4000):
    """ranges: lista de (linea_ini, linea_fin) de elite-source.asm; stubs: nombres externos a sustituir."""
    S = elitesrc.parse_vars(); cpu = CPU()
    stub_addr = 0x3F00
    cpu.stub_log = []
    for n in stubs:
        S[n] = stub_addr; cpu.mem[stub_addr] = 0x60
        def mk(name, addr):
            def h(c): c.stub_log.append(name)
            return h
        cpu.hooks[stub_addr] = mk(n, stub_addr); stub_addr += 1
    pc = org
    for a, b in ranges:
        pc = assemble(src_range(a, b), pc, S, cpu)
    return cpu, S

if __name__ == '__main__':
    L = elitesrc.read('elite-source.asm')
    cpu, S = build([(name_line('SHPPT'), name_line('MVEIT') - 1)], stubs=sys.argv[1:])
    print('ok, fin', hex(0), 'LL9 en', hex(S['LL9']))

def next_name_after(line):
    L = elitesrc.read('elite-source.asm')
    for i in range(line, len(L)):
        if re.match(r'^;\s+Name: ', L[i]): return i + 1
    return len(L)

def ll9_image(extra_stubs=()):
    """Imagen con LL9 y todo lo que usa, mas tablas log/alogh. Devuelve (cpu, simbolos)."""
    S = elitesrc.parse_vars(); cpu = CPU()
    # tablas matematicas con las formulas de la fuente (indice 0 de log/logL como en el binario original)
    import math
    log = [0x0803] + [int(0x2000 * math.log(i) / math.log(2) + 0.5) for i in range(1, 256)]
    for i in range(256):
        cpu.mem[0x2000 + i] = (log[i] >> 8) & 255
        cpu.mem[0x2100 + i] = log[i] & 255
        cpu.mem[0x2200 + i] = int(2 ** ((i / 2 + 128) / 16) + 0.5) >> 8 & 255
    S['log'] = 0x2000; S['logL'] = 0x2100; S['alogh'] = 0x2200
    stub_addr = 0x3F00; cpu.stub_log = []
    for n in ['DOEXP', 'LOIN', 'PLANET'] + list(extra_stubs):
        S[n] = stub_addr; cpu.mem[stub_addr] = 0x60
        cpu.hooks[stub_addr] = (lambda name: (lambda c: c.stub_log.append((name, bytes(c.mem[0x6E:0x72]))))) (n)
        stub_addr += 1
    pc = 0x4000; allines = []
    for rng in [(name_line('FMLTU'), next_name_after(name_line('FMLTU'))),
                 (name_line('DORND'), next_name_after(name_line('DORND'))),
                 (name_line('DVID3B2'), next_name_after(name_line('DVID3B2') + 60) - 1),
                 src_range(name_line('PROJ'), name_line('PL2') - 1) + ['.PL2'],
                 ['.PL21', 'SEC', 'RTS'] + src_range(name_line('PLS6'), next_name_after(name_line('PLS6')) - 1),
                 (name_line('SHPPT'), name_line('MVEIT') - 1)]:
        allines += rng if isinstance(rng, list) else src_range(rng[0], rng[1])
    assemble(allines, pc, S, cpu)
    return cpu, S

if __name__ == '__main__' and len(sys.argv) == 1:
    cpu, S = ll9_image(); print('LL9', hex(S['LL9']), 'LL51', hex(S['LL51']))
