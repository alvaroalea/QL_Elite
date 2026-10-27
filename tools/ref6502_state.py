# Reference: literal transcription (instruction by instruction, explicit carries) of the Apple II Elite routines
# that depend on the game state. Uses the already verified arithmetic transcriptions (ref6502*.py).
import sys
sys.path.insert(0, 'tools')
from ref6502 import adc, sbc, ror, lsr, mult1, add
from ref6502b import ll5, dvid4, dvidt, dvid3b

NI = 37
XC, YC = 128, 68          # original X and Y (view centre)

def dvid96(A):
    T = A & 0x80; A &= 0x7F; T1 = 254
    while True:
        A = (A << 1) & 255; c = 0
        if A >= 96: A -= 96; c = 1
        co = (T1 >> 7) & 1; T1 = ((T1 << 1) & 255) | c
        if not co: break
    return T1 | T

def tis2(A, Q):
    Y = A; A &= 0x7F
    if A >= Q: return (Y & 0x80) | 96
    T = 0xFE
    while True:
        A = (A << 1) & 255; c = 0
        if A >= Q: A -= Q; c = 1
        co = (T >> 7) & 1; T = ((T << 1) & 255) | c
        if not co: break
    t = T >> 2; r = (t >> 1) + t + (t & 1)
    return (Y & 0x80) | r

class M:
    def __init__(s):
        s.inwk = bytearray(NI); s.ks = bytearray(2 * NI)
        s.K = [0] * 4; s.K3 = bytearray(16); s.X15 = bytearray(6)
        s.P = 0; s.P1 = 0; s.P2 = 0
        s.Q = s.R = s.S = s.T = s.U = 0
        s.cash = bytearray(4)
        s.X = 0; s.Y = 0; s.C = 0

# ---- base routines already tested, with the M state ----
def squa2(m, A):                     # SQUA2: STA P, TAX, BNE MU11 (MU1 if A = 0); exits with C = 0
    m.P = A
    if A == 0: m.P = 0; return 0
    T = (A - 1) & 255; A = 0
    P = m.P; c = P & 1; P >>= 1
    for _ in range(8):
        if c: A, c = adc(A, T, c)
        A, c = ror(A, c); P, c = ror(P, c)
    m.P = P
    return A

def squa(m, A): return squa2(m, A & 0x7F)

def mult12(m, A):
    A, P = mult1(A, m.Q); m.P = P
    m.S = A; m.R = m.P
    return A

def mad(m, A):
    a, p = mult1(A, m.Q); m.P = p
    a, x = add(a, p, m.S, m.R)
    m.X = x
    return a

def tis1(m, X, A):
    m.Q = X; A ^= 0x80
    A = mad(m, A)
    return dvid96(A)

def dvidt_m(m, A):                   # (P+1 A) = (A P) / Q
    p1, a = dvidt(A, m.P, m.Q)
    m.P1 = p1
    return a

# ---- MVT3 ----
def mvt3(m, X):
    K = m.K; w = m.inwk
    A = K[3]; m.S = A; m.T = A & 0x80; A ^= w[X + 2]
    if not (A & 0x80):
        A = K[1]; A, c = adc(A, w[X], 0); K[1] = A
        A = K[2]; A, c = adc(A, w[X + 1], c); K[2] = A
        A = K[3]; A, c = adc(A, w[X + 2], c)
        A = (A & 0x7F) | m.T; K[3] = A; m.C = c
        return A
    A = m.S & 0x7F; m.S = A
    A = w[X]; A, c = sbc(A, K[1], 1); K[1] = A
    A = w[X + 1]; A, c = sbc(A, K[2], c); K[2] = A
    A = w[X + 2] & 0x7F; A, c = sbc(A, m.S, c)
    A = (A | 0x80) ^ m.T; K[3] = A
    if c: m.C = 1; return A
    A = 1; A, c = sbc(A, K[1], c); K[1] = A
    A = 0; A, c = sbc(A, K[2], c); K[2] = A
    A = 0; A, c = sbc(A, K[3], c)
    A = (A & 0x7F) | m.T; K[3] = A; m.C = c
    return A

def mas1(m, Y, X):
    K = m.K; w = m.inwk
    A = w[Y]; c = A >> 7; A = (A << 1) & 255; K[1] = A
    A = w[Y + 1]; n = A >> 7; A = ((A << 1) | c) & 255; c = n; K[2] = A
    A = 0; A, c = ror(A, c); K[3] = A
    A = mvt3(m, X)
    w[X + 2] = A
    w[X] = K[1]; w[X + 1] = K[2]
    return A & 0x7F

def mas2(m, A, Y): return (A | m.ks[Y + 2] | m.ks[Y + 5] | m.ks[Y + 8]) & 0x7F

def mas3(m, Y):
    A = squa2(m, m.ks[Y + 1]); m.R = A
    A = squa2(m, m.ks[Y + 4])
    A, c = adc(A, m.R, 0)
    if c: return 255
    m.R = A
    A = squa2(m, m.ks[Y + 7])
    A, c = adc(A, m.R, 0)
    if c: return 255
    return A

def mas4(m, A): return A | m.inwk[1] | m.inwk[4] | m.inwk[7]

# ---- TAS1 / VCSUB ----
def tas1(m, V, Y):                   # V = offset in ks
    K = m.K
    A = m.ks[V + Y] ^ 0x80; K[3] = A; Y -= 1
    K[2] = m.ks[V + Y]; Y -= 1
    K[1] = m.ks[V + Y]
    m.U = Y; X = m.U
    A = mvt3(m, X)
    Y = m.U
    m.K3[X + 2] = A; m.K3[X + 1] = K[2]; m.K3[X] = K[1]

def vcsub(m, V):
    tas1(m, V, 2); tas1(m, V, 5); tas1(m, V, 8)

# ---- NORM / TAS2 ----
def norm(m):
    x = m.X15
    m.R = squa(m, x[0]); m.Q = m.P
    m.T = squa(m, x[1])
    A, c = adc(m.P, m.Q, 0); m.Q = A
    A, c = adc(m.T, m.R, c); m.R = A
    m.T = squa(m, x[2])
    A, c = adc(m.P, m.Q, 0); m.Q = A
    A, c = adc(m.T, m.R, c); m.R = A
    m.Q = ll5(m.R, m.Q)
    for i in range(3): x[i] = tis2(x[i], m.Q)

def tas2(m):
    k = m.K3
    A = k[0] | k[3] | k[6] | 1; k[9] = A
    A = k[1] | k[4] | k[7]
    while True:
        c = k[9] >> 7; k[9] = (k[9] << 1) & 255
        n = A >> 7; A = ((A << 1) | c) & 255; c = n
        if c: break
        c = k[0] >> 7; k[0] = (k[0] << 1) & 255
        n = k[1] >> 7; k[1] = ((k[1] << 1) | c) & 255; c = n
        c = k[3] >> 7; k[3] = (k[3] << 1) & 255
        n = k[4] >> 7; k[4] = ((k[4] << 1) | c) & 255; c = n
        c = k[6] >> 7; k[6] = (k[6] << 1) & 255
        n = k[7] >> 7; k[7] = ((k[7] << 1) | c) & 255; c = n
        if not c: continue
        break
    x = m.X15
    A = k[1]; A, c = lsr(A); x[0] = A | k[2]
    A = k[4]; A, c = lsr(A); x[1] = A | k[5]
    A = k[7]; A, c = lsr(A); x[2] = A | k[8]
    norm(m)

# ---- TAS3 / TAS4 / TAS6 ----
def tas3(m, Y):
    w = m.inwk; x = m.X15
    m.Q = w[Y]; mult12(m, x[0])
    m.Q = w[Y + 2]; A = mad(m, x[1]); m.S = A; m.R = m.X
    m.Q = w[Y + 4]
    return mad(m, x[2])

def tas4(m, Y):
    k = m.ks; x = m.X15
    m.Q = k[NI + Y]; mult12(m, x[0])
    m.Q = k[NI + Y + 2]; A = mad(m, x[1]); m.S = A; m.R = m.X
    m.Q = k[NI + Y + 4]
    return mad(m, x[2])

def tas6(m):
    for i in range(3): m.X15[i] ^= 0x80

# ---- TIS3 / TIDY ----
def tis3(m, A, X, Y):
    w = m.inwk
    m.P2 = A
    m.Q = w[10 + X]; mult12(m, w[16 + X])
    m.Q = w[10 + Y]; A = mad(m, w[16 + Y])
    m.P = m.X
    Y = m.P2
    m.Q = w[10 + Y]
    A ^= 0x80
    return dvidt_m(m, A)

def tidy(m):
    w = m.inwk; x = m.X15
    x[0] = w[10]; x[1] = w[12]; x[2] = w[14]
    norm(m)
    w[10] = x[0]; w[12] = x[1]; w[14] = x[2]
    Y = 4; A = x[0] & 0x60
    if A == 0:
        X = A                                   # TI1: TAX
        A = x[1] & 0x60
        if A == 0:                              # TI2
            A = Y; Y = 2
            w[20] = tis3(m, A, X, Y)
        else:
            w[18] = tis3(m, 2, X, Y)
    else:
        X = 2; A = 0
        w[16] = tis3(m, A, X, Y)
    x[0] = w[16]; x[1] = w[18]; x[2] = w[20]    # TI3
    norm(m)
    w[16] = x[0]; w[18] = x[1]; w[20] = x[2]
    m.Q = w[12]; mult12(m, w[20])
    w[22] = tis1(m, w[14], w[18]) ^ 0x80
    mult12(m, w[16])
    w[24] = tis1(m, w[10], w[20]) ^ 0x80
    mult12(m, w[18])
    w[26] = tis1(m, w[12], w[16]) ^ 0x80
    X = 14
    while X >= 0:
        w[9 + X] = 0; X -= 2

# ---- SPS ----
def sps3(m, X):
    k = m.ks
    m.K3[X] = k[1 + X]
    A = k[2 + X]; Y = A
    m.K3[X + 1] = A & 0x7F
    m.K3[X + 2] = Y & 0x80

def sps1(m):
    sps3(m, 0); sps3(m, 3); sps3(m, 6); tas2(m)

def sps4(m):
    for X in range(8, -1, -1): m.K3[X] = m.ks[NI + X]
    tas2(m)

def sps2(m, A):
    c = A >> 7; A = (A << 1) & 255; X = A
    A = 0; A, c = ror(A, c); Y = A
    m.Q = 20
    p, r = dvid4(X, m.Q); m.P = p
    X = m.P
    if Y & 0x80:
        Y = 0xFF; X = ((X ^ 0xFF) + 1) & 255
    else:
        Y = 0
    return X, Y

# ---- PLS6 / PROJ ----
def pls6(m, A):
    w = m.inwk
    m.P2 = A
    k = dvid3b(m.P, m.P1, A, w[6] | 1, w[7], w[8])
    m.K = list(k)
    K = m.K
    if ((K[3] & 0x7F) | K[2]) != 0: return 1, None
    X = K[1]
    if X >= 4: return 1, X
    if not (K[3] & 0x80): return 0, X
    A = K[0] ^ 0xFF; A, c = adc(A, 1, 0); K[0] = A
    A = X ^ 0xFF; A, c = adc(A, 0, c); X = A
    return 0, X

def proj(m):
    w = m.inwk
    m.P = w[0]; m.P1 = w[1]
    c, X = pls6(m, w[2])
    if c: return 1
    A, c = adc(m.K[0], XC, 0); m.K3[0] = A
    A, c = adc(X, 0, c); m.K3[1] = A
    m.P = w[3]; m.P1 = w[4]
    c, X = pls6(m, w[5] ^ 0x80)
    if c: return 1
    A, c = adc(m.K[0], YC, 0); m.K3[14] = A
    A, c = adc(X, 0, c); m.K3[15] = A
    return 0

# ---- money ----
def mcash(m, X, Y):
    k = m.cash
    A, c = adc(X, k[3], 0); k[3] = A
    A, c = adc(Y, k[2], c); k[2] = A
    A, c = adc(k[1], 0, c); k[1] = A
    A, c = adc(k[0], 0, c); k[0] = A
    return 0

def lcash(m, X, Y):
    k = m.cash
    A, c = sbc(k[3], X, 1); k[3] = A
    A, c = sbc(k[2], Y, c); k[2] = A
    A, c = sbc(k[1], 0, c); k[1] = A
    A, c = sbc(k[0], 0, c); k[0] = A
    if c: return 1
    return mcash(m, X, Y)

def gcash(m):
    A = squa_multu(m)
    P = m.P
    c = P >> 7; P = (P << 1) & 255
    n = A >> 7; A = ((A << 1) | c) & 255
    c = P >> 7; P = (P << 1) & 255
    A = ((A << 1) | c) & 255
    m.P = P
    return P, A                      # X, Y

def squa_multu(m):                   # MULTU: (A P) = P * Q  (Q = 0 -> 0)
    if m.Q == 0: m.P = 0; return 0
    p = m.P * m.Q
    m.P = p & 255
    return p >> 8

# ---- SCALEX / SCALEY ----
def scaley2(A):
    T3 = A
    A >>= 2
    A, c = sbc(A, T3, 1)
    A ^= 0xFF
    A, c = adc(A, 1, c)
    return A, c

def scaley(A): return scaley2(A >> 1)

def scalex(A):
    A, c = scaley2(A)
    return adc(A, 32, c)
