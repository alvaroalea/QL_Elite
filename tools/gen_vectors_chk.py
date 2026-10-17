#!/usr/bin/env python3
"""Vectores para CHECK y CHECK2 del comandante: transcripcion literal del 6502 (acarreo explicito en cada ADC/ROR).
Formato de cada linea: <77 bytes en hex> <CHECK> <CHECK2>.  Los bloques incluyen el comandante de fabrica."""
import random, sys
NT = 76

def adc(a, m, c):
    t = a + m + c
    return t & 255, t >> 8

def check(b):                      # .CHECK: LDX #NT%-3 ; CLC ; TXA ; QUL2: ADC NA%+7,X ; EOR NA%+8,X ; DEX ; BNE
    x = NT - 3; c = 0; a = x
    while True:
        a, c = adc(a, b[x - 1], c)
        a ^= b[x]
        x -= 1
        if x == 0: break
    return a

def check2(b):                     # .CHECK2: ... QU2L2: STX T ; EOR T ; ROR A ; ADC ; EOR ; DEX ; BNE
    x = NT - 3; c = 0; a = x
    while True:
        a ^= x
        c, a = a & 1, (a >> 1) | (c << 7)       # ROR A
        a, c = adc(a, b[x - 1], c)
        a ^= b[x]
        x -= 1
        if x == 0: break
    return a

default = [0, 20, 173, 0x4A, 0x5A, 0x48, 0x02, 0x53, 0xB7, 0, 0, 3, 0xE8, 70, 0, 0, 15, 0, 0, 0, 0, 0, 22] + [0] * 17 + \
          [0] * 7 + [0, 0, 0, 0, 3, 0] + [16, 15, 17, 0, 3, 28, 14, 0, 0, 10, 0, 17, 58, 7, 9, 8, 0] + [0, 0, 0, 128, 0xAA, 0x27, 0x03]
assert len(default) == 77, len(default)
assert check(default) == 0x03 and check2(default) == 0x27     # constantes de fabrica de la fuente
rnd = random.Random(1985)
out = []
blocks = [default] + [[0] * 77, [255] * 77]
for i in range(3000):
    blocks.append([rnd.randrange(256) for _ in range(77)])
for b in blocks:
    out.append(" ".join(["".join("%02x" % v for v in b), "%02x" % check(b), "%02x" % check2(b)]))
open("tests/vectors_chk.txt", "w").write("\n".join(out) + "\n")
print(len(out), "vectores")
