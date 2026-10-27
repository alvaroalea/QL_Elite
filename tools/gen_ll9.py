# Generates tests/vectors_ll9.txt by running the ORIGINAL routines (reference source) in the 6502 emulator.
# Requires the cloned source (ELITE_SRC). The generated file is used by tests/test_ll9.c.
import sys, random, math
sys.path.insert(0, 'tools')
import orig, elitesrc, gen_shipdata

random.seed(int(sys.argv[2]) if len(sys.argv) > 2 else 99)
cpu, S = orig.ll9_image()
order, blob, starts = gen_shipdata.blueprints()
BLOB = 0x7000; HEAP = 0x9000
cpu.mem[BLOB:BLOB + len(blob)] = blob
offs = [0] + [starts[n] for n in order]
A = lambda n: S[n]

def hx(b): return ''.join('%02x' % v for v in b)
def rb(n): return bytes(random.randrange(256) for _ in range(n))
SPEC = [0, 1, 2, 3, 0x7F, 0x80, 0x81, 0xFE, 0xFF]
def B(): return random.choice(SPEC) if random.random() < 0.3 else random.randrange(256)

def rotation():
    # random quaternion -> matrix; rows = nosev, roofv, sidev
    q = [random.gauss(0, 1) for _ in range(4)]; n = math.sqrt(sum(v * v for v in q)); w, x, y, z = [v / n for v in q]
    m = [[1 - 2 * (y * y + z * z), 2 * (x * y - z * w), 2 * (x * z + y * w)],
         [2 * (x * y + z * w), 1 - 2 * (x * x + z * z), 2 * (y * z - x * w)],
         [2 * (x * z - y * w), 2 * (y * z + x * w), 1 - 2 * (x * x + y * y)]]
    out = bytearray()
    for row in m:
        for v in row:
            mag = min(int(abs(v) * 96 * 256 + random.random()), 0x7FFF)
            hi = (mag >> 8) | (0x80 if v < 0 else 0)
            out += bytes([mag & 255, hi])
    return out

def pos(kind):
    if kind == 'near':
        zh = random.randrange(1, 70); xh = random.randrange(0, max(1, zh)); yh = random.randrange(0, max(1, zh // 2 + 1))
        return bytes([random.randrange(256), xh, random.choice([0, 0x80]),
                      random.randrange(256), yh, random.choice([0, 0x80]),
                      random.randrange(256), zh, 0])
    if kind == 'tiny':
        return bytes([random.randrange(256), 0, random.choice([0, 0x80]), random.randrange(256), 0, random.choice([0, 0x80]),
                      random.randrange(1, 60), 0, 0])
    if kind == 'far':
        zh = random.randrange(40, 192); return bytes([random.randrange(256), random.randrange(0, 40), random.choice([0, 0x80]),
                                                      random.randrange(256), random.randrange(0, 40), random.choice([0, 0x80]),
                                                      random.randrange(256), zh, 0])
    return rb(9)

def make_inwk():
    k = random.random()
    p = pos('near' if k < 0.45 else 'tiny' if k < 0.60 else 'far' if k < 0.78 else 'rand')
    w = bytearray(37)
    w[0:9] = p
    w[9:27] = rotation() if random.random() < 0.7 else bytes(random.choice([0, 0, random.randrange(256)]) for _ in range(18))
    w[27:31] = rb(4)
    f = 0
    r = random.random()
    if random.random() < 0.35: f |= 0x08                        # line drawn before
    if random.random() < 0.12: f |= 0x20                        # exploding
    if random.random() < 0.10: f |= 0x40                        # firing laser
    if random.random() < 0.10: f |= 0x80                        # dead
    w[31] = f | random.randrange(8)
    w[32] = B(); w[33] = 0; w[34] = HEAP >> 8; w[35] = B()      # fixed XX19 (heap pointer)
    w[36] = 0x80 if random.random() < 0.08 else random.randrange(128)
    return w

out = []
def emit(op, ins, outs):
    out.append('%s in:%s out:%s' % (op, ','.join('%s=%s' % kv for kv in ins), ','.join('%s=%s' % kv for kv in outs)))

def reset_log(): cpu.stub_log[:] = []

def log_bytes():
    d = bytearray(); s = bytearray()
    n = 0
    for e in cpu.stub_log:
        if e[0] == 'LOIN': d += e[1]; n += 1
    ids = {'DOEXP': 1, 'PLANET': 2}
    sl = [ids[e[0]] for e in cpu.stub_log if e[0] in ids]
    return bytes([n]) + bytes(d), bytes([len(sl)] + sl)

def run_ll9(case_type, bp, w, heap, xx3, ks0, rand, ci, xx2):
    m = cpu.mem
    m[A('XX2'):A('XX2') + 16] = xx2
    m[A('INWK'):A('INWK') + 37] = w
    m[A('XX0')] = (BLOB + offs[bp]) & 255; m[A('XX0') + 1] = (BLOB + offs[bp]) >> 8
    m[A('INF')] = 0x00; m[A('INF') + 1] = 0x08
    m[0x800:0x800 + 37] = ks0
    m[A('TYPE')] = case_type
    m[HEAP:HEAP + 256] = heap
    m[0x100:0x200] = xx3[:256]
    m[A('RAND'):A('RAND') + 4] = rand
    cpu.c = ci; reset_log()
    cpu.run(A('LL9'))

N = int(sys.argv[1]) if len(sys.argv) > 1 else 3000
for _ in range(N):
    ty = random.randrange(1, 33); bp = ty
    ctype = ty | (0x80 if random.random() < 0.04 else 0)
    w = make_inwk()
    heap = bytearray(rb(256)); heap[0] = random.randrange(0, 60) & 0xFC
    xx3 = bytearray(rb(256)); ks0 = rb(37); rand = rb(4); ci = random.randrange(2); xx2 = rb(16)
    run_ll9(ctype, bp, w, heap, xx3, ks0, rand, ci, xx2)
    dl, sl = log_bytes()
    m = cpu.mem
    emit('ll9', [('inwk', hx(w)), ('ty', '%02x' % ctype), ('bp', '%02x' % bp), ('heap', hx(heap)), ('xx3', hx(xx3[:200])),
                 ('ks', hx(ks0)), ('rand', hx(rand)), ('ci', '%02x' % ci), ('xx2', hx(xx2))],
         [('inwk', hx(m[A('INWK'):A('INWK') + 37])), ('heap', hx(m[HEAP:HEAP + 128])), ('xx3', hx(m[0x100:0x100 + 200])),
          ('xx2', hx(m[A('XX2'):A('XX2') + 16])), ('ks', hx(m[0x800:0x800 + 37])), ('rand', hx(m[A('RAND'):A('RAND') + 4])),
          ('dlog', hx(dl)), ('slog', hx(sl))])

# --- line clipping: CLIP and CLIP2 ---
def coord():
    r = random.random()
    if r < 0.5: return random.randrange(256), random.choice([0, 0, 0, 1, 0xFF])
    if r < 0.65: return random.choice([0, 1, 134, 135, 136, 137, 255]), random.choice([0, 0, 1, 0xFF])
    if r < 0.8: return random.randrange(0, 160), 0
    return random.randrange(256), random.randrange(256)
for _ in range(N * 2):
    x15 = bytearray(6); x12 = bytearray(6)
    x15[0], x15[1] = coord(); x15[2], x15[3] = coord(); x15[4], x15[5] = coord()
    x12[0], x12[1] = coord()
    for o in range(2, 6): x12[o] = B()
    sw0 = random.randrange(256)
    which = random.choice(['clip', 'clip2'])
    m = cpu.mem
    m[A('XX15'):A('XX15') + 6] = x15; m[A('XX12'):A('XX12') + 6] = x12; m[A('SWAP')] = sw0
    m[A('XX13')] = 0
    cpu.a = 0
    if which == 'clip':
        cpu.run(A('CLIP')); ia = ''
    else:
        ia_v = x15[5]; cpu.a = ia_v
        # CLIP2 expects A = XX15+5; it is entered through CLIP2 with A loaded
        cpu.run(A('CLIP2'))
    emit(which, [('x15', hx(x15)), ('x12', hx(x12)), ('sw', '%02x' % sw0)],
         [('x15', hx(m[A('XX15'):A('XX15') + 6])), ('x12', hx(m[A('XX12'):A('XX12') + 6])), ('rc', '%02x' % cpu.c), ('sw', '%02x' % m[A('SWAP')])])

# --- LL51 y PROJ ---
for _ in range(N):
    x15 = bytearray(rb(6)); xx16 = rb(18)
    m = cpu.mem
    m[A('XX15'):A('XX15') + 6] = x15; m[A('XX16'):A('XX16') + 18] = xx16
    cpu.run(A('LL51'))
    emit('ll51', [('x15', hx(x15)), ('xx16', hx(xx16))], [('x12', hx(m[A('XX12'):A('XX12') + 6]))])
for _ in range(N):
    w = make_inwk(); k3 = rb(16)
    m = cpu.mem
    m[A('INWK'):A('INWK') + 37] = w; m[A('K3'):A('K3') + 16] = k3
    cpu.run(A('PROJ'))
    emit('proj', [('inwk', hx(w)), ('k3', hx(k3))],
         [('rc', '%02x' % cpu.c), ('ra', '%02x' % cpu.a), ('k3', hx(m[A('K3'):A('K3') + 16])), ('k', hx(m[A('K'):A('K') + 4]))])

open('tests/vectors_ll9.txt', 'w').write('\n'.join(out) + '\n')
print(len(out), 'vectores; planetas/explosiones y dibujos incluidos')
