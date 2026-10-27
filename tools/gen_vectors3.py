# Generates tests/vectors3.txt: one line per case, "op in:name=hex,... out:name=hex,..."
# The output comes from the literal 6502 transcription (tools/ref6502_state.py).
import sys, random
sys.path.insert(0, 'tools')
from ref6502_state import *
import ref6502_state as R

random.seed(33)
SPEC = [0, 1, 2, 3, 0x1F, 0x20, 0x3F, 0x40, 0x5F, 0x60, 0x7F, 0x80, 0x81, 0xBF, 0xC0, 0xE0, 0xFF]
def B():
    r = random.random()
    if r < 0.35: return random.choice(SPEC)
    return random.randrange(256)
def blob(n): return bytearray(B() for _ in range(n))
def hx(b): return ''.join('%02x' % v for v in b)
def h1(v): return '%02x' % (v & 255)

out = []
def emit(op, ins, outs):
    out.append('%s in:%s out:%s' % (op, ','.join('%s=%s' % kv for kv in ins), ','.join('%s=%s' % kv for kv in outs)))

def newm(**kw):
    m = M()
    for k, v in kw.items(): setattr(m, k, v)
    return m

N = 3000
for _ in range(N):
    # --- mvt3 ---
    w = blob(NI); K = [B() for _ in range(4)]; X = random.choice([0, 3, 6, 9, 12, 15])
    m = newm(inwk=bytearray(w), K=list(K)); a = mvt3(m, X)
    emit('mvt3', [('inwk', hx(w)), ('k', hx(bytes(K))), ('ix', h1(X))], [('k', hx(bytes(m.K))), ('ra', h1(a)), ('rc', h1(m.C))])
    # --- mas1 ---
    Y = random.choice([9, 11, 13, 15, 17, 19, 21, 23, 25]); X = random.choice([0, 3, 6])
    m = newm(inwk=bytearray(w), K=list(K)); a = mas1(m, Y, X)
    emit('mas1', [('inwk', hx(w)), ('k', hx(bytes(K))), ('iy', h1(Y)), ('ix', h1(X))], [('inwk', hx(m.inwk)), ('k', hx(bytes(m.K))), ('ra', h1(a))])
    # --- mas2, mas3, mas4 ---
    ks = blob(2 * NI); Y = random.choice([0, NI]); A = B()
    m = newm(ks=bytearray(ks)); a = mas2(m, A, Y)
    emit('mas2', [('ks', hx(ks)), ('ia', h1(A)), ('iy', h1(Y))], [('ra', h1(a))])
    m = newm(ks=bytearray(ks)); a = mas2(m, 0, Y)
    emit('mas2m', [('ks', hx(ks)), ('iy', h1(Y))], [('ra', h1(a))])
    ksm = bytearray(ks)
    if random.random() < 0.7:                      # small coordinates so as not to overflow every time
        for o in (1, 4, 7): ksm[Y + o] = random.randrange(0, 12) if random.random() < 0.8 else B()
    m = newm(ks=bytearray(ksm)); a = mas3(m, Y)
    emit('mas3', [('ks', hx(ksm)), ('iy', h1(Y))], [('ra', h1(a)), ('r', h1(m.R))])
    A = B()
    m = newm(inwk=bytearray(w)); a = mas4(m, A)
    emit('mas4', [('inwk', hx(w)), ('ia', h1(A))], [('ra', h1(a))])
    # --- vcsub / tas1 ---
    k3 = blob(16); V = random.choice([0, NI])
    m = newm(inwk=bytearray(w), ks=bytearray(ks), K3=bytearray(k3), K=list(K)); vcsub(m, V)
    emit('vcsub', [('inwk', hx(w)), ('ks', hx(ks)), ('k3', hx(k3)), ('k', hx(bytes(K))), ('iv', h1(V))],
         [('k3', hx(m.K3)), ('k', hx(bytes(m.K))), ('u', h1(m.U))])
    Y = random.choice([2, 5, 8])
    m = newm(inwk=bytearray(w), ks=bytearray(ks), K3=bytearray(k3), K=list(K)); tas1(m, V, Y)
    emit('tas1', [('inwk', hx(w)), ('ks', hx(ks)), ('k3', hx(k3)), ('k', hx(bytes(K))), ('iv', h1(V)), ('iy', h1(Y))],
         [('k3', hx(m.K3)), ('k', hx(bytes(m.K))), ('u', h1(m.U))])
    # --- tas2, norm ---
    k3 = blob(16)
    if random.random() < 0.3:
        for o in (1, 4, 7): k3[o] = random.randrange(0, 4)
    x15 = blob(6)
    m = newm(K3=bytearray(k3), X15=bytearray(x15)); tas2(m)
    emit('tas2', [('k3', hx(k3)), ('x15', hx(x15))], [('k3', hx(m.K3)), ('x15', hx(m.X15)), ('q', h1(m.Q))])
    m = newm(X15=bytearray(x15)); norm(m)
    emit('norm', [('x15', hx(x15))], [('x15', hx(m.X15)), ('q', h1(m.Q))])
    # --- tas3, tas4, tas6 ---
    Y = random.choice([0, 2, 6, 10, 16, 22, 32])
    m = newm(inwk=bytearray(w), X15=bytearray(x15)); a = tas3(m, Y)
    emit('tas3', [('inwk', hx(w)), ('x15', hx(x15)), ('iy', h1(Y))], [('ra', h1(a)), ('rx', h1(m.X)), ('s', h1(m.S)), ('r', h1(m.R))])
    m = newm(ks=bytearray(ks), X15=bytearray(x15)); a = tas4(m, Y)
    emit('tas4', [('ks', hx(ks)), ('x15', hx(x15)), ('iy', h1(Y))], [('ra', h1(a)), ('rx', h1(m.X)), ('s', h1(m.S)), ('r', h1(m.R))])
    m = newm(X15=bytearray(x15)); tas6(m)
    emit('tas6', [('x15', hx(x15))], [('x15', hx(m.X15))])
    # --- tis3 ---
    A = random.choice([0, 2, 4]); X = random.choice([0, 2, 4]); Y = random.choice([0, 2, 4])
    wq = bytearray(w)
    for o in (10, 12, 14): wq[o] = random.choice([0, 0, 1, 20, 60, 96, 0x80 | 96, 0x80 | 30, B()])
    if wq[10 + Y] == 0: wq[10 + Y] = 1
    m = newm(inwk=bytearray(wq)); a = tis3(m, A, X, Y)
    emit('tis3', [('inwk', hx(wq)), ('ia', h1(A)), ('ix', h1(X)), ('iy', h1(Y))], [('ra', h1(a)), ('p1', h1(m.P1))])
    # --- tidy: nearly orthonormal matrix with noise, and random matrices ---
    wt = bytearray(w)
    if random.random() < 0.6:
        for o in (10, 12, 14, 16, 18, 20): wt[o] = random.choice([0, 0, 0, 96, 0xE0, 20, 0x80 | 20, 70, B()])
    m = newm(inwk=bytearray(wt)); tidy(m)
    emit('tidy', [('inwk', hx(wt))], [('inwk', hx(m.inwk))])
    # --- sps ---
    m = newm(ks=bytearray(ks), K3=bytearray(k3)); sps1(m)
    emit('sps1', [('ks', hx(ks)), ('k3', hx(k3))], [('k3', hx(m.K3)), ('x15', hx(m.X15)), ('q', h1(m.Q))])
    m = newm(ks=bytearray(ks), K3=bytearray(k3)); sps4(m)
    emit('sps4', [('ks', hx(ks)), ('k3', hx(k3))], [('k3', hx(m.K3)), ('x15', hx(m.X15)), ('q', h1(m.Q))])
    X = random.choice([0, 3, 6])
    m = newm(ks=bytearray(ks), K3=bytearray(k3)); sps3(m, X)
    emit('sps3', [('ks', hx(ks)), ('k3', hx(k3)), ('ix', h1(X))], [('k3', hx(m.K3))])
    A = B()
    m = newm(); x, y = sps2(m, A)
    emit('sps2', [('ia', h1(A))], [('rx', h1(x)), ('ry', h1(y))])
    # --- pls6 / proj ---
    wp = bytearray(w)
    if random.random() < 0.8:
        wp[1] = random.randrange(0, 3); wp[4] = random.randrange(0, 3)       # small x_hi, y_hi
        wp[7] = random.randrange(1, 256); wp[2] &= 0x7F if random.random() < 0.7 else 0xFF
        wp[8] &= 0x7F if random.random() < 0.9 else 0xFF
    p0, p1, A = B(), B(), B()
    m = newm(inwk=bytearray(wp), P=p0, P1=p1); c, X = pls6(m, A)
    o = [('rc', h1(c)), ('k', hx(bytes(m.K)))]
    if X is not None: o.append(('rx', h1(X)))
    emit('pls6', [('inwk', hx(wp)), ('p', h1(p0)), ('p1', h1(p1)), ('ia', h1(A))], o)
    k3 = blob(16)
    m = newm(inwk=bytearray(wp), K3=bytearray(k3)); c = proj(m)
    emit('proj', [('inwk', hx(wp)), ('k3', hx(k3))], [('rc', h1(c)), ('k3', hx(m.K3)), ('k', hx(bytes(m.K)))])
    # --- money ---
    cs = blob(4); X, Y = B(), B()
    if random.random() < 0.5: cs[0] = 0; cs[1] = 0
    m = newm(cash=bytearray(cs)); c = lcash(m, X, Y)
    emit('lcash', [('cash', hx(cs)), ('ix', h1(X)), ('iy', h1(Y))], [('cash', hx(m.cash)), ('rc', h1(c))])
    m = newm(cash=bytearray(cs)); mcash(m, X, Y)
    emit('mcash', [('cash', hx(cs)), ('ix', h1(X)), ('iy', h1(Y))], [('cash', hx(m.cash))])
    p0, q = B(), B()
    m = newm(P=p0, Q=q); x, y = gcash(m)
    emit('gcash', [('p', h1(p0)), ('q', h1(q))], [('rx', h1(x)), ('ry', h1(y))])
    # --- scale ---
    A = B()
    a, c = scaley2(A); emit('scaley2', [('ia', h1(A))], [('ra', h1(a)), ('rc', h1(c))])
    a, c = scaley(A);  emit('scaley', [('ia', h1(A))], [('ra', h1(a)), ('rc', h1(c))])
    a, c = scalex(A);  emit('scalex', [('ia', h1(A))], [('ra', h1(a)), ('rc', h1(c))])

open('tests/vectors3.txt', 'w').write('\n'.join(out) + '\n')
print(len(out), 'vectores')
