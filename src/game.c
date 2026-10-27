#include "game.h"
#include "cmdr.h"
#include "state.h"
#include "space.h"
#include "emath.h"

int cur_sys = 0;
int tgt_sys = 0;
uni_market mkt;
u8 game_visit_rnd = 0;

/* ---------------- money ---------------- */

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

/* ---------------- game ---------------- */

static int find_system(u8 x, u8 y)
{
    int i;
    for (i = 0; i < UNI_NSYS; i++)
        if (uni_galaxy[i].x == x && uni_galaxy[i].y == y) return i;
    return uni_nearest(x, y);                        /* game with coordinates that do not exist: the nearest one */
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

/* TT111: the system nearest (QQ9, QQ10) becomes the chosen one (and the crosshair is placed on it). With a chosen system, QQ9/QQ10
   are its coordinates: if another with a lower index is at distance 0 (a "twin", 38 of the 2,048 systems) the selection passes to
   that one, as in the original. NOTE (future): decide whether to do anything about the twins; today it is left as in the original */
void game_tt111(void) { game_set_target(uni_nearest(qq9, qq10)); }

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
    space_reset_all();                               /* TT170: RESET */
    cmdr_default();
    gcnt = 0;
    uni_build_galaxy(1);
    uni_galaxy_seed(1, qq21);
    cur_sys = find_system(qq0, qq1);
    game_enter_system(cur_sys, 0);                   /* the factory commander brings its market (avl) and qq26 = 0 */
    make_market();
    for (i = 0; i < NGOODS; i++) mkt.quantity[i] = avl[i];
    qq12 = 0xFF;                                     /* docked */
    game_tt111();                                    /* QU5: ping (game_enter_system) y TT111 */
}

void game_after_load(void)
{
    int i;
    uni_build_galaxy(gcnt + 1);
    uni_galaxy_seed(gcnt + 1, qq21);
    cur_sys = find_system(qq0, qq1);
    game_enter_system(cur_sys, 0);
    make_market();
    for (i = 0; i < NGOODS; i++) mkt.quantity[i] = avl[i];     /* stock as saved */
    qq12 = 0xFF;
    game_tt111();                                    /* QU5: ping y TT111 */
}

/* ---------------- trade ---------------- */

int trade_hold_t(void)
{
    int i, t = 0;
    for (i = 0; i <= 12; i++) t += qq20[i];
    return t + tribble[1];
}

/* tnpr: tonnes (commodities 0..12): fits if quantity + occupied + 1 < CRGO (the +1 is the carry left by CPX).
   kg/g (13..16): quantity + what you already carry < 200. */
int trade_room(int good)
{
    int room;
    if (good <= 12) room = (int)crgo - 2 - trade_hold_t();
    else room = 199 - (int)qq20[good];
    return room < 0 ? 0 : room;
}

/* tnpr of the original, transcribed: A = quantity; t (commodities 0..12): A = qty + C(=1, left by CPX) + QQ20[12..0] + TRIBBLE+1 with carry
   between additions; "does not fit" if A >= CRGO. kg/g: A = qty + QQ20[good] (C=0); "does not fit" if A >= 200. */
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
    while (n > 0 && !trade_fits(good, n)) n--;       /* with TRADE_FAITHFUL "fits" is not monotonic: search downwards */
    return n;
}

#ifdef DEBUG
int dbg_free = 0, dbg_range = 0;
#endif

int trade_buy(int good, unsigned qty)
{
    unsigned long cost;
    if (good < 0 || good >= NGOODS || qty == 0) return TR_NONE;
    if (qty > mkt.quantity[good]) return TR_NOSTOCK;
    if (!trade_fits(good, qty)) return TR_NOROOM;
    cost = (unsigned long)mkt.price[good] * qty;
#if TRADE_FAITHFUL
    cost &= 0xFFFFUL;                                 /* GCASH: (Y X) = price * quantity * 4 in 16 bits */
#endif
#ifdef DEBUG
    if (!dbg_free)
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
#ifdef DEBUG
    if (!dbg_free)
#endif
    qq20[good] = (u8)(qq20[good] - qty);
    {
        unsigned long amount = (unsigned long)mkt.price[good] * qty;
#if TRADE_FAITHFUL
        amount &= 0xFFFFUL;                           /* GCASH of the 6502: 16-bit amount (verified against the original) */
#endif
        cash_earn(amount);
    }
    return TR_OK;                                     /* as in the original, selling does not replenish the stock */
}

/* ---------------- equipment ---------------- */

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

/* refund: returns the price of the laser being replaced (POW=4, POW+128=5, military=12, other=13) */
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
    if (equip_is_laser(item)) {
        if (view < 0 || view > 3) return EQ_BAD;
        newlaser = (item == 4) ? LASER_POW : (item == 5) ? LASER_BEAM : (item == 12) ? LASER_MIL : LASER_MINING;
    }
    price = equip_price(item);
#ifdef DEBUG
    if (dbg_free) price = 0;                         /* DEBUG: everything free */
#endif
    if (!cash_spend(price)) return EQ_NOCASH;       /* eq: charges first (without money it warns even if you already have it) */
    switch (item) {                                  /* pres: if you already have it, MCASH returns what was charged */
    case 1: if (nomsl >= 4) goto present; break;
    case 2: if (crgo == 37) goto present; break;
    case 3: if (ecm_fitted) goto present; break;
    case 6: if (bst) goto present; break;
    case 7: if (escp) goto present; break;
    case 8: if (bomb) goto present; break;
    case 9: if (engy) goto present; break;
    case 10: if (dkcmp) goto present; break;
    case 11: if (ghyp) goto present; break;
    default: break;
    }
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
    default:                                         /* lasers: charges first and returns the old one afterwards, like EQSHP */
#ifdef DEBUG
        if (!dbg_free)
#endif
        cash_earn(laser_refund(laser[view]));
        laser[view] = newlaser;
        break;
    }
    return EQ_OK;
present:
    cash_earn(price);
    return EQ_PRESENT;
}

/* ---------------- commander ---------------- */

/* STATUS st4/st3: with high TALLY: >=25*256 Elite, >=10*256 Deadly, >=2*256 Dangerous, otherwise Competent;
   with TALLY < 256: X = 1 + number of shifts of (TALLY>>3) until zero ... (see the source) */
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

/* ---------------- hyperspace ---------------- */

unsigned jump_distance(int idx)
{
    uni_system here;
    if (!mj) return uni_distance(&uni_galaxy[cur_sys], &uni_galaxy[idx]);
    here = uni_galaxy[cur_sys];                     /* in hyperspace (MJP) the position is (QQ0, QQ1), not a system */
    here.x = qq0; here.y = qq1;
    return uni_distance(&here, &uni_galaxy[idx]);
}

int jump_possible(int idx)
{
    if (idx == cur_sys && !mj) return 0;
#ifdef DEBUG
    if (dbg_range) return 1;
#endif
    return jump_distance(idx) <= qq14;
}

int jump_do(int idx)
{
    unsigned d;
    if (!jump_possible(idx)) return 0;
    d = jump_distance(idx);
    qq14 = (u8)(qq14 >= d ? qq14 - d : 0);
    game_enter_system(idx, 1);
    mj = 0;                                        /* hyperspace is left on arriving at a system */
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
