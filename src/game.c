#include "game.h"
#include "cmdr.h"
#include "state.h"
#include "emath.h"

int cur_sys = 0;
int tgt_sys = 0;
uni_market mkt;
u8 game_visit_rnd = 0;

/* ---------------- dinero ---------------- */

unsigned long cash_get(void)
{
    return ((unsigned long)cash[0] << 24) | ((unsigned long)cash[1] << 16) | ((unsigned long)cash[2] << 8) | cash[3];
}

void cash_set(unsigned long v)
{
    cash[0] = (u8)(v >> 24); cash[1] = (u8)(v >> 16); cash[2] = (u8)(v >> 8); cash[3] = (u8)v;
}

int cash_spend(unsigned long v)
{
    unsigned long c = cash_get();
    if (c < v) return 0;
    cash_set(c - v);
    return 1;
}

void cash_earn(unsigned long v)
{
    unsigned long c = cash_get();
    c += v;
    if (c < v) c = 0xFFFFFFFFUL;
    cash_set(c);
}

/* ---------------- partida ---------------- */

static int find_system(u8 x, u8 y)
{
    int i;
    for (i = 0; i < UNI_NSYS; i++)
        if (uni_galaxy[i].x == x && uni_galaxy[i].y == y) return i;
    return uni_nearest(x, y);                        /* partida con coordenadas que no existen: el mas cercano */
}

static void make_market(void)
{
    uni_gen_market(qq26, &uni_galaxy[cur_sys], &mkt);
}

void game_set_target(int idx)
{
    tgt_sys = idx;
    qq9 = uni_galaxy[idx].x; qq10 = uni_galaxy[idx].y;
    qq8[1] = (u8)(jump_distance(idx) >> 8); qq8[0] = (u8)jump_distance(idx);
}

void game_target_only(int idx)
{
    tgt_sys = idx;
    qq8[1] = (u8)(jump_distance(idx) >> 8); qq8[0] = (u8)jump_distance(idx);
}

void game_enter_system(int idx, int fresh)
{
    const uni_system *s = &uni_galaxy[idx];
    int i;
    cur_sys = idx;
    qq0 = s->x; qq1 = s->y;
    qq3 = s->economy; qq4 = s->govtype; qq5 = s->techlev;
    qq6[0] = (u8)(s->productivity & 255); qq6[1] = (u8)(s->productivity >> 8);
    qq7[0] = s->population; qq7[1] = 0;
    if (fresh) {
        qq26 = game_visit_rnd ? game_visit_rnd : em_dornd();
        make_market();
        for (i = 0; i < NGOODS; i++) avl[i] = (u8)mkt.quantity[i];
    }
    game_set_target(idx);
}

void game_new(void)
{
    int i;
    state_clear();
    cmdr_default();
    gcnt = 0;
    uni_build_galaxy(1);
    uni_galaxy_seed(1, qq21);
    cur_sys = find_system(qq0, qq1);
    game_enter_system(cur_sys, 0);                   /* el comandante de fabrica trae su mercado (avl) y qq26 = 0 */
    make_market();
    for (i = 0; i < NGOODS; i++) mkt.quantity[i] = avl[i];
    qq12 = 0xFF;                                     /* atracado */
}

void game_after_load(void)
{
    int i;
    uni_build_galaxy(gcnt + 1);
    uni_galaxy_seed(gcnt + 1, qq21);
    cur_sys = find_system(qq0, qq1);
    game_enter_system(cur_sys, 0);
    make_market();
    for (i = 0; i < NGOODS; i++) mkt.quantity[i] = avl[i];     /* existencias tal como se guardaron */
    qq12 = 0xFF;
}

/* ---------------- comercio ---------------- */

int trade_hold_t(void)
{
    int i, t = 0;
    for (i = 0; i <= 12; i++) t += qq20[i];
    return t + tribble[1];
}

/* tnpr: toneladas (mercancias 0..12): cabe si cantidad + ocupado + 1 < CRGO (el +1 es el acarreo que deja CPX).
   kg/g (13..16): cantidad + lo que ya llevas < 200. */
int trade_room(int good)
{
    int room;
    if (good <= 12) room = (int)crgo - 2 - trade_hold_t();
    else room = 199 - (int)qq20[good];
    return room < 0 ? 0 : room;
}

/* tnpr del original, transcrito: A = cantidad; t (mercancias 0..12): A = qty + C(=1, deja CPX) + QQ20[12..0] + TRIBBLE+1 con acarreo
   entre sumas; "no cabe" si A >= CRGO. kg/g: A = qty + QQ20[good] (C=0); "no cabe" si A >= 200. */
int trade_fits(int good, unsigned qty)
{
#if TRADE_FAITHFUL
    unsigned a = qty & 255, c, t;
    int x;
    if (good <= 12) {
        c = 1;
        for (x = 12; x >= 0; x--) { t = a + qq20[x] + c; c = t >> 8; a = t & 255; }
        t = a + tribble[1] + c;
        a = t & 255;
        return a < crgo;
    }
    a = (a + qq20[good]) & 255;
    return a < 200;
#else
    return (int)qty <= trade_room(good);
#endif
}

unsigned trade_price(int good) { return mkt.price[good]; }

unsigned trade_max_buy(int good)
{
    unsigned n = mkt.quantity[good];
    unsigned long afford;
    if (mkt.price[good] == 0) afford = 255; else afford = cash_get() / mkt.price[good];
    if (afford < n) n = (unsigned)afford;
    while (n > 0 && !trade_fits(good, n)) n--;       /* con TRADE_FAITHFUL "cabe" no es monotono: se busca hacia abajo */
    return n;
}

int trade_buy(int good, unsigned qty)
{
    unsigned long cost;
    if (good < 0 || good >= NGOODS || qty == 0) return TR_NONE;
    if (qty > mkt.quantity[good]) return TR_NOSTOCK;
    if (!trade_fits(good, qty)) return TR_NOROOM;
    cost = (unsigned long)mkt.price[good] * qty;
#if TRADE_FAITHFUL
    cost &= 0xFFFFUL;                                 /* GCASH: (Y X) = precio * cantidad * 4 en 16 bits */
#endif
    if (!cash_spend(cost)) return TR_NOCASH;
    qq20[good] = (u8)(qq20[good] + qty);
    mkt.quantity[good] -= qty;
    avl[good] = (u8)mkt.quantity[good];
    return TR_OK;
}

int trade_sell(int good, unsigned qty)
{
    if (good < 0 || good >= NGOODS || qty == 0 || qq20[good] == 0) return TR_NONE;
    if (qty > qq20[good]) return TR_NOSTOCK;
    qq20[good] = (u8)(qq20[good] - qty);
    {
        unsigned long amount = (unsigned long)mkt.price[good] * qty;
#if TRADE_FAITHFUL
        amount &= 0xFFFFUL;                           /* GCASH del 6502: importe de 16 bits (verificado contra el original) */
#endif
        cash_earn(amount);
    }
    return TR_OK;                                     /* como en el original, vender no repone las existencias */
}

/* ---------------- equipo ---------------- */

const char *eq_names[NEQUIP] = {
    "Fuel", "Missile", "Large Cargo Bay", "E.C.M.System", "Pulse Lasers", "Beam Lasers", "Fuel Scoops",
    "Escape Pod", "Energy Bomb", "Energy Unit", "Docking Computer", "Galactic Hyperdrive", "Military Lasers", "Mining Lasers"
};

static const unsigned eq_prices[NEQUIP] = { 0, 300, 4000, 6000, 4000, 10000, 5250, 10000, 9000, 15000, 10000, 50000U, 60000U, 8000 };

int equip_count(void)
{
    int n = (int)qq5 + 3;
    return (n < 12) ? n : NEQUIP;
}

unsigned equip_price(int item)
{
    if (item == 0) return (unsigned)((70 - (int)qq14) * 2);      /* PRXS+0 = (70 - QQ14) * 2 */
    return eq_prices[item];
}

int equip_is_laser(int item) { return item == 4 || item == 5 || item == 12 || item == 13; }

/* refund: devuelve el precio del laser que se sustituye (POW=4, POW+128=5, militar=12, otro=13) */
static unsigned laser_refund(u8 old)
{
    if (old == 0) return 0;
    if (old == LASER_POW) return eq_prices[4];
    if (old == LASER_BEAM) return eq_prices[5];
    if (old == LASER_MIL) return eq_prices[12];
    return eq_prices[13];
}

int equip_buy(int item, int view)
{
    unsigned price;
    u8 newlaser = 0;
    if (item < 0 || item >= NEQUIP) return EQ_BAD;
    /* estado "ya lo tienes" antes de cobrar (el original cobra y devuelve el dinero: el resultado es el mismo) */
    switch (item) {
    case 1: if (nomsl >= 4) return EQ_PRESENT; break;
    case 2: if (crgo == 37) return EQ_PRESENT; break;
    case 3: if (ecm_fitted) return EQ_PRESENT; break;
    case 6: if (bst) return EQ_PRESENT; break;
    case 7: if (escp) return EQ_PRESENT; break;
    case 8: if (bomb) return EQ_PRESENT; break;
    case 9: if (engy) return EQ_PRESENT; break;
    case 10: if (dkcmp) return EQ_PRESENT; break;
    case 11: if (ghyp) return EQ_PRESENT; break;
    default: break;
    }
    if (equip_is_laser(item)) {
        if (view < 0 || view > 3) return EQ_BAD;
        newlaser = (item == 4) ? LASER_POW : (item == 5) ? LASER_BEAM : (item == 12) ? LASER_MIL : LASER_MINING;
    }
    price = equip_price(item);
    if (!cash_spend(price)) return EQ_NOCASH;
    switch (item) {
    case 0: qq14 = 70; break;
    case 1: nomsl++; break;
    case 2: crgo = 37; break;
    case 3: ecm_fitted = 0xFF; break;
    case 6: bst = 0xFF; break;
    case 7: escp = 0xFF; break;
    case 8: bomb = 0x7F; break;
    case 9: engy = 1; break;
    case 10: dkcmp = 0xFF; break;
    case 11: ghyp = 0xFF; break;
    default:                                         /* lasers: cobra primero y devuelve el antiguo despues, como EQSHP */
        cash_earn(laser_refund(laser[view]));
        laser[view] = newlaser;
        break;
    }
    return EQ_OK;
}

/* ---------------- comandante ---------------- */

/* STATUS st4/st3: con TALLY alto: >=25*256 Elite, >=10*256 Deadly, >=2*256 Dangerous, si no Competent;
   con TALLY < 256: X = 1 + numero de desplazamientos de (TALLY>>3) hasta cero ... (ver la fuente) */
const char *rating_name(void)
{
    static const char *names[9] = { "Harmless", "Mostly Harmless", "Poor", "Average", "Above Average", "Competent", "Dangerous", "Deadly", "---- E L I T E ----" };
    int x;
    if (tally[1]) {
        x = 9;
        if (tally[1] < 25) { x = 8; if (tally[1] < 10) { x = 7; if (tally[1] < 2) x = 6; } }
    } else {
        unsigned a = tally[0] >> 2;
        x = 1;
        a >>= 1;
        while (a) { x++; a >>= 1; }
    }
    return names[x - 1];
}

const char *legal_name(void)
{
    if (fist == 0) return "Clean";
    return fist < 50 ? "Offender" : "Fugitive";
}

const char *laser_name(u8 v)
{
    if (v == LASER_POW) return "Pulse Laser";
    if (v == LASER_BEAM) return "Beam Laser";
    if (v == LASER_MIL) return "Military Laser";
    if (v == LASER_MINING) return "Mining Laser";
    return "Laser";
}

/* ---------------- hiperespacio ---------------- */

unsigned jump_distance(int idx)
{
    uni_system here;
    if (!mj) return uni_distance(&uni_galaxy[cur_sys], &uni_galaxy[idx]);
    here = uni_galaxy[cur_sys];                     /* en el hiperespacio (MJP) la posicion es (QQ0, QQ1), no un sistema */
    here.x = qq0; here.y = qq1;
    return uni_distance(&here, &uni_galaxy[idx]);
}

int jump_possible(int idx)
{
    if (idx == cur_sys && !mj) return 0;
    return jump_distance(idx) <= qq14;
}

int jump_do(int idx)
{
    unsigned d;
    if (!jump_possible(idx)) return 0;
    d = jump_distance(idx);
    qq14 = (u8)(qq14 - d);
    game_enter_system(idx, 1);
    mj = 0;                                        /* se sale del hiperespacio al llegar a un sistema */
    return 1;
}

int jump_galactic(void)
{
    int g;
    if (!ghyp) return 0;
    g = (int)gcnt + 1;
    game_enter_system(uni_galactic_jump(&g), 1);
    gcnt = (u8)(g - 1);
    uni_galaxy_seed(g, qq21);
    ghyp = 0;
    mj = 0;
    return 1;
}
