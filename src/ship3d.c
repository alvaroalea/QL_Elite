#include "ship3d.h"

u8 ztype;
const u8 *zxx0;
u8 *zxx19;
u8 *zinf;
u8 zxx4, zxx13, zxx17, zxx20, zcnt, zswap, zlsnum, zlsnum2;
u8 zxx18[9];
u8 xx3[258];
u8 ship_c_in;

static void default_loin(void) { gfx_line(zx15[0], zx15[1], zx15[2], zx15[3], C_WHITE, 1); }
void (*ship_loin_hook)(void) = default_loin;
void (*ship_doexp_hook)(void) = 0;
void (*ship_planet_hook)(void) = 0;

#define LOIN()    do { if (ship_loin_hook) ship_loin_hook(); } while (0)
#define YC2M1     (VIEW_CY * 2 - 1)         /* Y*2-1 = 135 */

/* puntero + desplazamiento de 16 bits con signo (los planos comparten datos con desplazamientos negativos) */
static const u8 *bp_ptr(u8 lo, u8 hi)
{
    long off = (long)hi * 256L + lo;
    if (off >= 32768L) off -= 65536L;
    return zxx0 + off;
}

/* ---------- LL51: producto del vector XX15 por la matriz XX16 ---------- */
void ship_ll51(void)
{
    u8 x = 0, y = 0, a;
    do {
        zQ = zx15[0]; zT = em_fmltu(zxx16[x]);
        zS = (u8)(zx15[1] ^ zxx16[x + 1]);
        zQ = zx15[2]; a = em_fmltu(zxx16[x + 2]); zQ = a; zR = zT;
        zT = em_ll38((u8)(zx15[3] ^ zxx16[x + 3]));
        zQ = zx15[4]; a = em_fmltu(zxx16[x + 4]); zQ = a; zR = zT;
        a = em_ll38((u8)(zx15[5] ^ zxx16[x + 5]));
        zx12[y] = a; zx12[y + 1] = zS;
        y = (u8)(y + 2); x = (u8)(x + 6);
    } while (x < 17);
}

/* ---------- monton de lineas ---------- */
void ship_lsclr(void)
{
    u8 y = zlsnum;
    while (y < zlsnum2) {
        zx15[0] = zxx19[y++]; zx15[1] = zxx19[y++]; zx15[2] = zxx19[y++]; zx15[3] = zxx19[y++];
        LOIN();
    }
    zxx19[0] = zlsnum;
}

void ship_lsput(void)
{
    u8 y = zlsnum, noold = (u8)(y >= zlsnum2), x;
    for (x = 3; x < 4; x--) zx12[x] = zx15[x];
    LOIN();
    zx15[0] = zxx19[y]; zxx19[y] = zx12[0]; y++;
    zx15[1] = zxx19[y]; zxx19[y] = zx12[1]; y++;
    zx15[2] = zxx19[y]; zxx19[y] = zx12[2]; y++;
    zx15[3] = zxx19[y]; zxx19[y] = zx12[3]; y++;
    zlsnum = y;
    if (noold) return;
    LOIN();
}

/* ---------- LL129, LL120, LL123, LL118: pendiente de la linea y recorte contra los bordes ---------- */
static u8 xr, yr;                              /* (Y X) devuelto por LL120/LL123 */

static u8 ll129(void)
{
    u8 a, old;
    int d;
    zQ = zx12[2];
    a = zS;
    if (a & 0x80) {
        d = 0 - (int)zR; zR = (u8)d;
        old = zS;
        zS = (u8)((old ^ 0xFF) + (d >= 0 ? 1 : 0));
        a = old;
    }
    return (u8)(a ^ zx12[3]);
}

/* entry: 0 = LL120 (multiplicar), 1 = LL123 (dividir); salta de una a otra segun T como el original */
static void ll120_123(int entry)
{
    u8 a, x, y, c, pa;
    unsigned s;
    int d;
    if (entry == 0) {
        zR = zx15[0];
        pa = ll129();
        if (zT != 0) goto ll121;
    } else {
        pa = ll129();
        if (zT != 0) goto ll122;
        goto ll121;
    }
ll122:
    x = 0; y = 0;
    c = (u8)(zS & 1); zS >>= 1; { u8 r0 = (u8)(zR & 1); zR = (u8)((zR >> 1) | (c << 7)); c = r0; }
    c = (u8)(zQ >> 7); zQ = (u8)(zQ << 1);
    if (!c) goto ll126;
ll125:
    s = (unsigned)x + zR; x = (u8)s; c = (u8)(s >> 8);
    s = (unsigned)y + zS + c; y = (u8)s;
ll126:
    c = (u8)(zS & 1); zS >>= 1; { u8 r0 = (u8)(zR & 1); zR = (u8)((zR >> 1) | (c << 7)); c = r0; }
    c = (u8)(zQ >> 7); zQ = (u8)(zQ << 1);
    if (c) goto ll125;
    if (zQ != 0) goto ll126;
    if (!(pa & 0x80)) goto ll133;
    xr = x; yr = y; return;
ll121:
    y = 0xFF; x = 0xFE;
ll130:
    c = (u8)(zR >> 7); zR = (u8)(zR << 1);
    { u8 c2 = (u8)(zS >> 7); zS = (u8)((zS << 1) | c); c = c2; }
    a = zS;
    if (!c) { if (a < zQ) { c = 0; goto ll132; } }
    /* LL131 */
    d = (int)a - zQ; zS = (u8)d; c = (u8)(d >= 0);
    d = (int)zR - 0 - (1 - c); zR = (u8)d; c = (u8)(d >= 0);
    c = 1;
ll132:
    { u8 c2 = (u8)(x >> 7); x = (u8)((x << 1) | c); c = c2; }
    { u8 c2 = (u8)(y >> 7); y = (u8)((y << 1) | c); c = c2; }
    if (c) goto ll130;
    if (pa & 0x80) { xr = x; yr = y; return; }
ll133:
    s = (unsigned)(u8)(x ^ 0xFF) + 1; x = (u8)s; c = (u8)(s >> 8);
    s = (unsigned)(u8)(y ^ 0xFF) + c; y = (u8)s;
    xr = x; yr = y;
}

static void ll118(void)
{
    u8 a, x;
    unsigned s;
    a = zx15[1];
    if (a & 0x80) {                                    /* x1 negativo: llevarlo a x = 0 */
        zS = a; ll120_123(0);
        s = (unsigned)xr + zx15[2]; zx15[2] = (u8)s;
        s = (unsigned)yr + zx15[3] + (s >> 8); zx15[3] = (u8)s;
        zx15[0] = 0; zx15[1] = 0; a = 0;
    }
    if (a != 0) {                                      /* LL119: x1 > 255: llevarlo a x = 255 */
        zS = a; zS--; ll120_123(0);
        s = (unsigned)xr + zx15[2]; zx15[2] = (u8)s;
        s = (unsigned)yr + zx15[3] + (s >> 8); zx15[3] = (u8)s;
        zx15[0] = 255; zx15[1] = 0;
    }
    a = zx15[3];                                       /* LL134 */
    if (a & 0x80) {                                    /* y1 negativo: llevarlo a y = 0 */
        zS = a; zR = zx15[2]; ll120_123(1);
        s = (unsigned)xr + zx15[0]; zx15[0] = (u8)s;
        s = (unsigned)yr + zx15[1] + (s >> 8); zx15[1] = (u8)s;
        zx15[2] = 0; zx15[3] = 0;
    }
    {                                                  /* LL135 */
        int d = (int)zx15[2] - VIEW_CY * 2;
        int c = (d >= 0);
        zR = (u8)d;
        d = (int)zx15[3] - 0 - (1 - c);
        zS = (u8)d;
        if (d < 0) return;                             /* BCC LL136 */
        ll120_123(1);
        s = (unsigned)xr + zx15[0]; zx15[0] = (u8)s;
        s = (unsigned)yr + zx15[1] + (s >> 8); zx15[1] = (u8)s;
        zx15[2] = YC2M1; zx15[3] = 0;
    }
    (void)x;
}

/* ---------- CLIP / CLIP2 ---------- */
u8 ship_clip(void)
{
    zswap = 0;
    return ship_clip2(zx15[5]);
}

u8 ship_clip2(u8 a)
{
    u8 x, c, t;
    int d, cc;
    x = YC2M1;
    a |= zx12[1];
    if (a != 0) goto ll107;
    if (x < zx12[0]) goto ll107;
    x = 0;
ll107:
    zxx13 = x;
    a = (u8)(zx15[1] | zx15[3]);
    if (a != 0) goto ll83;
    if (YC2M1 < zx15[2]) goto ll83;
    if (zxx13 != 0) goto ll108;
ll146:
    zx15[1] = zx15[2]; zx15[2] = zx15[4]; zx15[3] = zx12[0];
    return 0;
ll109:
    return 1;
ll108:
    zxx13 >>= 1;
ll83:
    if (!(zxx13 & 0x80)) goto ll115;
    if ((zx15[1] & zx15[5]) & 0x80) goto ll109;
    if ((zx15[3] & zx12[1]) & 0x80) goto ll109;
    x = (u8)(zx15[1] - 1); a = x;
    x = (u8)(zx15[5] - 1); zx12[2] = x; a |= zx12[2];
    if (!(a & 0x80)) goto ll109;
    cc = (zx15[2] >= VIEW_CY * 2);
    a = (u8)(zx15[3] - (1 - cc)); zx12[2] = a;
    cc = (zx12[0] >= VIEW_CY * 2);
    a = (u8)(zx12[1] - (1 - cc)); a |= zx12[2];
    if (!(a & 0x80)) goto ll109;
ll115:
    /* TYA; PHA: Y no se usa en este puerto */
    d = (int)zx15[4] - zx15[0]; zx12[2] = (u8)d; cc = (d >= 0);
    d = (int)zx15[5] - zx15[1] - (1 - cc); zx12[3] = (u8)d;
    d = (int)zx12[0] - zx15[2]; zx12[4] = (u8)d; cc = (d >= 0);
    d = (int)zx12[1] - zx15[3] - (1 - cc); zx12[5] = (u8)d;
    zS = (u8)(zx12[5] ^ zx12[3]);
    if (zx12[5] & 0x80) {
        d = 0 - (int)zx12[4]; zx12[4] = (u8)d; cc = (d >= 0);
        d = 0 - (int)zx12[5] - (1 - cc); zx12[5] = (u8)d;
    }
    a = zx12[3];
    if (a & 0x80) {
        d = 0 - (int)zx12[2]; zx12[2] = (u8)d; cc = (d >= 0);
        d = 0 - (int)a - (1 - cc); a = (u8)d;
    }
ll111:
    x = a;
    if (x != 0) goto ll112;
    x = zx12[5];
    if (x == 0) goto ll113;
ll112:
    c = (u8)(a & 1); a >>= 1;
    t = (u8)(zx12[2] & 1); zx12[2] = (u8)((zx12[2] >> 1) | (c << 7));
    c = (u8)(zx12[5] & 1); zx12[5] >>= 1;
    t = (u8)(zx12[4] & 1); zx12[4] = (u8)((zx12[4] >> 1) | (c << 7));
    (void)t;
    goto ll111;
ll113:
    zT = x;
    if (zx12[2] >= zx12[4]) {                          /* CMP XX12+4: C si XX12+2 >= XX12+4 */
        zQ = zx12[2]; em_ll28(zx12[4]);
    } else {
        zQ = zx12[4]; em_ll28(zx12[2]);
        zT--;
    }
    /* LL116 */
    zx12[2] = zR; zx12[3] = zS;
    a = zxx13;
    if (a != 0 && !(a & 0x80)) goto llx117;            /* BEQ LL138; BPL LLX117 */
    ll118();                                           /* LL138 */
    if (!(zxx13 & 0x80)) goto ll124;
    if ((zx15[1] | zx15[3]) != 0) goto ll137;          /* LL117 */
    if (zx15[2] >= VIEW_CY * 2) goto ll137;
llx117:
    x = zx15[0]; zx15[0] = zx15[4]; zx15[4] = x;
    a = zx15[5]; x = zx15[1]; zx15[5] = x; zx15[1] = a;
    x = zx15[2]; zx15[2] = zx12[0]; zx12[0] = x;
    a = zx12[1]; x = zx15[3]; zx12[1] = x; zx15[3] = a;
    ll118();
    zswap--;
ll124:
    goto ll146;
ll137:
    return 1;
}

/* ---------- SHPPT: nave lejana, un punto ---------- */
static void shpt(u8 a)
{
    unsigned s;
    zx15[1] = a; zx15[3] = a;
    zx15[0] = zk3[0];
    s = (unsigned)zk3[0] + 3;
    zx15[2] = (s > 255) ? 255 : (u8)s;
    ship_lsput();
}

void ship_shppt(void)
{
    u8 a;
    sm_proj();
    a = (u8)(sm_a | zk3[1]);
    if (a == 0 && zk4[0] < VIEW_CY * 2 - 2) {
        shpt(zk4[0]);
        shpt((u8)(zk4[0] + 1));
        inwk[31] |= 0x08;
    } else {
        inwk[31] &= 0xF7;
    }
    ship_lsclr();
}

/* ---------- LL9 ---------- */
static void ee51(void)
{
    if ((inwk[31] & 0x08) == 0) return;
    inwk[31] &= 0xF7;
    ship_lsclr();
}

void ship_ll9(void)
{
    u8 a, c, x, y, t, c0;
    int d, cc;
    unsigned s;
    const u8 *v;

    if (ztype & 0x80) { if (ship_planet_hook) ship_planet_hook(); return; }      /* LL25 */
    zxx4 = 31; zlsnum = 1;
    zlsnum2 = (inwk[31] & 0x08) ? zxx19[0] : 0;
    a = inwk[36];
    if (a & 0x80) goto ee51_ret;
    if (inwk[31] & 0x20) goto ee28;
    if (!(inwk[31] & 0x80)) goto ee28;
    inwk[31] = (u8)((inwk[31] | 0x20) & 0x3F);
    zinf[28] = 0; zinf[30] = 0;
    c0 = (inwk[31] & 0x08) ? 1 : ship_c_in;              /* C al volver de EE51 (LSCLR deja C = 1) */
    ee51();
    zxx19[1] = 18;
    zxx19[2] = zxx0[7];
    em_rand_c = c0;
    zxx19[3] = em_dornd();
    for (y = 4; y <= 6; y++) { em_rand_c = 0; zxx19[y] = em_dornd(); }
ee28:
    a = inwk[8];
    if (a & 0x80) goto ll14;                              /* BPL LL10 no tomado */
    goto ll10;
ll14:
    if (!(inwk[31] & 0x20)) goto ee51_ret;
    inwk[31] &= 0xF7;
    if (ship_doexp_hook) ship_doexp_hook();
    return;
ee51_ret:
    ee51();
    return;
ll10:
    if (inwk[7] >= 192) goto ll14;
    if ((((unsigned)inwk[1] << 8) | inwk[0]) >= (((unsigned)inwk[7] << 8) | inwk[6])) goto ll14;
    if ((((unsigned)inwk[4] << 8) | inwk[3]) >= (((unsigned)inwk[7] << 8) | inwk[6])) goto ll14;
    x = zxx0[6];
    xx3[x] = 255; xx3[x + 1] = 255;
    t = inwk[6]; a = inwk[7];
    c = (u8)(a & 1); a >>= 1; t = (u8)((t >> 1) | (c << 7));
    c = (u8)(a & 1); a >>= 1; t = (u8)((t >> 1) | (c << 7));
    c = (u8)(a & 1); a >>= 1; t = (u8)((t >> 1) | (c << 7));
    c = (u8)(a & 1); a >>= 1;
    if (a != 0) goto ll13;
    a = (u8)((t >> 1) | (c << 7)); a >>= 3;
    zxx4 = a;
    goto ll17;
ll13:
    if (zxx0[13] >= inwk[7]) goto ll17;
    if (inwk[31] & 0x20) goto ll17;
    ship_shppt();
    return;
ll17:
    for (x = 0; x < 6; x++) {
        zxx16[x] = inwk[21 + x]; zxx16[6 + x] = inwk[15 + x]; zxx16[12 + x] = inwk[9 + x];
    }
    zQ = 197;
    for (y = 16; y < 128; y = (u8)(y - 2)) {
        c = (u8)(zxx16[y] >> 7);
        a = (u8)((zxx16[y + 1] << 1) | c);
        em_ll28(a);
        zxx16[y] = zR;
    }
    for (x = 0; x < 9; x++) zxx18[x] = inwk[x];
    zk3[15] = 255;                                         /* XX2+15 */
    if (!(inwk[31] & 0x20)) goto ee29;
    x = (u8)(zxx0[12] >> 2);
    for (;;) { xx2[x] = 255; if (x == 0) break; x--; }
    zxx4 = 0;                                              /* INX; STX XX4 (X pasa de $FF a 0) */
    goto ll42;
ee29:
    a = zxx0[12];
    if (a == 0) goto ll42;
    zxx20 = a;
    x = zxx0[18];
    a = zxx18[7];
    while (a != 0) {                                       /* LL90 */
        x++;
        c = (u8)(zxx18[4] & 1); zxx18[4] >>= 1; zxx18[3] = (u8)((zxx18[3] >> 1) | (c << 7));
        c = (u8)(zxx18[1] & 1); zxx18[1] >>= 1; zxx18[0] = (u8)((zxx18[0] >> 1) | (c << 7));
        c = (u8)(a & 1); a >>= 1; zxx18[6] = (u8)((zxx18[6] >> 1) | (c << 7));
    }
    zxx17 = x;                                             /* LL91 */
    zx15[5] = zxx18[8]; zx15[0] = zxx18[0]; zx15[1] = zxx18[2]; zx15[2] = zxx18[3];
    zx15[3] = zxx18[5]; zx15[4] = zxx18[6];
    ship_ll51();
    zxx18[0] = zx12[0]; zxx18[2] = zx12[1]; zxx18[3] = zx12[2];
    zxx18[5] = zx12[3]; zxx18[6] = zx12[4]; zxx18[8] = zx12[5];
    v = bp_ptr(zxx0[4], zxx0[17]);
    y = 0;
ll86:
    zx12[1] = v[y];
    if ((v[y] & 0x1F) >= zxx4) goto ll87;
    xx2[y >> 2] = 255;
    y = (u8)(y + 4);
    goto ll88;
ll87:
    zx12[3] = (u8)(zx12[1] << 1); zx12[5] = (u8)(zx12[3] << 1);
    y++; zx12[0] = v[y];
    y++; zx12[2] = v[y];
    y++; zx12[4] = v[y];
    x = zxx17;
    if (x < 4) goto ll92;
    zx15[0] = zxx18[0]; zx15[1] = zxx18[2]; zx15[2] = zxx18[3];
    zx15[3] = zxx18[5]; zx15[4] = zxx18[6]; zx15[5] = zxx18[8];
    goto ll89;
ovflw:
    zxx18[0] >>= 1; zxx18[6] >>= 1; zxx18[3] >>= 1;
    x = 1;
ll92:
    zx15[0] = zx12[0]; zx15[2] = zx12[2];
    a = zx12[4];
    {
        u8 n = x;
        while (n > 0) { zx15[0] >>= 1; zx15[2] >>= 1; a >>= 1; n--; }
    }
    zR = a; zS = zx12[5]; zQ = zxx18[6];
    a = em_ll38(zxx18[8]);
    if (em_carry) goto ovflw;
    zx15[4] = a; zx15[5] = zS;
    zR = zx15[0]; zS = zx12[1]; zQ = zxx18[0];
    a = em_ll38(zxx18[2]);
    if (em_carry) goto ovflw;
    zx15[0] = a; zx15[1] = zS;
    zR = zx15[2]; zS = zx12[3]; zQ = zxx18[3];
    a = em_ll38(zxx18[5]);
    if (em_carry) goto ovflw;
    zx15[2] = a; zx15[3] = zS;
ll89:
    zQ = zx12[0]; a = em_fmltu(zx15[0]); zT = a;
    zS = (u8)(zx12[1] ^ zx15[1]);
    zQ = zx12[2]; a = em_fmltu(zx15[2]); zQ = a; zR = zT;
    zT = em_ll38((u8)(zx12[3] ^ zx15[3]));
    zQ = zx12[4]; a = em_fmltu(zx15[4]); zQ = a; zR = zT;
    a = em_ll38((u8)(zx15[5] ^ zx12[5]));
    x = (u8)(y >> 2);
    if (!(zS & 0x80)) a = 0;
    xx2[x] = a;
    y++;
ll88:
    if (y >= zxx20) goto ll42;
    goto ll86;
ll42:
    t = zxx16[2]; x = zxx16[3]; zxx16[2] = zxx16[6]; zxx16[3] = zxx16[7]; zxx16[6] = t; zxx16[7] = x;
    t = zxx16[4]; x = zxx16[5]; zxx16[4] = zxx16[12]; zxx16[5] = zxx16[13]; zxx16[12] = t; zxx16[13] = x;
    t = zxx16[10]; x = zxx16[11]; zxx16[10] = zxx16[14]; zxx16[11] = zxx16[15]; zxx16[14] = t; zxx16[15] = x;
    zxx20 = zxx0[8];
    v = zxx0 + 20;
    y = 0; zcnt = 0;
ll48:
    zxx17 = y;
    zx15[0] = v[y]; y++;
    zx15[2] = v[y]; y++;
    zx15[4] = v[y]; y++;
    t = v[y]; zT = t;
    if ((t & 0x1F) < zxx4) goto ll50;
    y++; a = v[y]; zP = a;
    if (xx2[a & 0x0F] != 0) goto ll49;
    if (xx2[a >> 4] != 0) goto ll49;
    y++; a = v[y]; zP = a;
    if (xx2[a & 0x0F] != 0) goto ll49;
    if (xx2[a >> 4] != 0) goto ll49;
    goto ll50;
ll49:
    zx15[1] = t; zx15[3] = (u8)(t << 1); zx15[5] = (u8)(zx15[3] << 1);
    ship_ll51();
    zx15[2] = inwk[2];
    if (((inwk[2] ^ zx12[1]) & 0x80)) goto ll52;
    s = (unsigned)zx12[0] + inwk[0]; zx15[0] = (u8)s;
    s = (unsigned)inwk[1] + (s >> 8); zx15[1] = (u8)s;
    goto ll53;
ll52:
    d = (int)inwk[0] - zx12[0]; zx15[0] = (u8)d; cc = (d >= 0);
    d = (int)inwk[1] - 0 - (1 - cc); zx15[1] = (u8)d; cc = (d >= 0);
    if (cc) goto ll53;
    zx15[1] = (u8)(zx15[1] ^ 0xFF);
    d = 1 - (int)zx15[0] - 1; zx15[0] = (u8)d; cc = (d >= 0);   /* LDA #1; SBC XX15 con C = 0 */
    if (cc) zx15[1]++;
    zx15[2] ^= 0x80;
ll53:
    zx15[5] = inwk[5];
    if (((inwk[5] ^ zx12[3]) & 0x80)) goto ll54;
    s = (unsigned)zx12[2] + inwk[3]; zx15[3] = (u8)s;
    s = (unsigned)inwk[4] + (s >> 8); zx15[4] = (u8)s;
    goto ll55;
ll54:
    d = (int)inwk[3] - zx12[2]; zx15[3] = (u8)d; cc = (d >= 0);
    d = (int)inwk[4] - 0 - (1 - cc); zx15[4] = (u8)d; cc = (d >= 0);
    if (cc) goto ll55;
    zx15[4] = (u8)(zx15[4] ^ 0xFF);
    s = (unsigned)(u8)(zx15[3] ^ 0xFF) + 1; zx15[3] = (u8)s; cc = (int)(s >> 8);
    zx15[5] ^= 0x80;
    if (cc) zx15[4]++;
ll55:
    if (zx12[5] & 0x80) goto ll56;
    s = (unsigned)zx12[4] + inwk[6]; zT = (u8)s;
    zU = (u8)((unsigned)inwk[7] + (s >> 8));
    goto ll57;
ll56:
    d = (int)inwk[6] - zx12[4]; zT = (u8)d; cc = (d >= 0);
    d = (int)inwk[7] - 0 - (1 - cc); zU = (u8)d; cc = (d >= 0);
    if (!cc) goto ll140;
    if (zU != 0) goto ll57;
    if (zT >= 4) goto ll57;
ll140:
    zU = 0; zT = 4;
ll57:
    if ((zU | zx15[1] | zx15[4]) == 0) goto ll60;
    c = (u8)(zx15[1] & 1); zx15[1] >>= 1; zx15[0] = (u8)((zx15[0] >> 1) | (c << 7));
    c = (u8)(zx15[4] & 1); zx15[4] >>= 1; zx15[3] = (u8)((zx15[3] >> 1) | (c << 7));
    c = (u8)(zU & 1); zU >>= 1; zT = (u8)((zT >> 1) | (c << 7));
    goto ll57;
ll60:
    zQ = zT;
    if (zx15[0] < zQ) goto ll69;
    em_ll61(zx15[0]);
    goto ll69b;
ll69:
    em_ll28(zx15[0]);
ll69b:
    x = zcnt;
    if (zx15[2] & 0x80) goto ll62;
    s = (unsigned)zR + 128; xx3[x] = (u8)s; x++;
    xx3[x] = (u8)((unsigned)zU + (s >> 8));
    goto ll66;
ll62:
    d = 128 - (int)zR; xx3[x] = (u8)d; cc = (d >= 0); x++;
    xx3[x] = (u8)(0 - (int)zU - (1 - cc));
ll66:
    y = x;                                                 /* TXA; PHA */
    zU = 0; zQ = zT;
    if (zx15[3] < zQ) goto ll67;
    em_ll61(zx15[3]);
    goto ll68;
ll67:
    em_ll28(zx15[3]);
ll68:
    x = y; x++;
    if (zx15[5] & 0x80) goto ll70;
    d = VIEW_CY - (int)zR; xx3[x] = (u8)d; cc = (d >= 0); x++;
    xx3[x] = (u8)(0 - (int)zU - (1 - cc));
    goto ll50;
ll70:
    s = (unsigned)VIEW_CY + zR; xx3[x] = (u8)s; x++;
    xx3[x] = (u8)((unsigned)zU + (s >> 8));
ll50:
    s = (unsigned)zcnt + 4; zcnt = (u8)s;
    s = (unsigned)zxx17 + 6; y = (u8)s;
    if (s > 255) goto ll72;
    if (y >= zxx20) goto ll72;
    goto ll48;
ll72:
    if (!(inwk[31] & 0x20)) goto ee31;
    inwk[31] |= 0x08;
    if (ship_doexp_hook) ship_doexp_hook();
    return;
ee31:
    zxx20 = zxx0[9];
    inwk[31] |= 0x08;
    y = 0; zxx17 = 0;
    if (!(inwk[31] & 0x40)) goto ll170;
    inwk[31] &= 0xBF;
    y = zxx0[6];
    x = xx3[y]; zx15[0] = x; x++;
    if (x == 0) goto ll170;
    x = xx3[y + 1]; zx15[1] = x; x++;
    if (x == 0) goto ll170;
    zx15[2] = xx3[y + 2]; zx15[3] = xx3[y + 3];
    zx15[4] = 0; zx15[5] = 0; zx12[1] = 0;
    zx12[0] = inwk[6];
    if (inwk[2] & 0x80) zx15[4]--;
    if (ship_clip()) goto ll170;
    ship_lsput();
ll170:
    v = bp_ptr(zxx0[3], zxx0[16]);
    zcnt = zxx0[5];
ll75:
    y = 0;
    if (v[0] < zxx4) goto ll78;
    a = v[1]; zP = a;
    if (xx2[a & 0x0F] != 0) goto ll79;
    if (xx2[a >> 4] == 0) goto ll78;
ll79:
    x = v[2];
    zx15[0] = xx3[x]; zx15[1] = xx3[x + 1]; zx15[2] = xx3[x + 2]; zx15[3] = xx3[x + 3];
    x = v[3];
    zx15[4] = xx3[x]; zx12[0] = xx3[x + 2]; zx12[1] = xx3[x + 3]; zx15[5] = xx3[x + 1];
    if (ship_clip2(zx15[5])) goto ll78;
    ship_lsput();
ll78:
    if (zlsnum >= zcnt) goto ll81;
    v += 4;
    zxx17++;
    if (zxx17 < zxx20) goto ll75;
ll81:
    ship_lsclr();
    (void)c0; (void)c; (void)cc;
}
