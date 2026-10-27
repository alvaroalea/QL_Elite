#include "encounters.h"
#include "state.h"
#include "emath.h"
#include "ships.h"
#include "shipdata.h"
#include "game.h"
#include "universe.h"

/* enc_ev (EV) is defined in state.c: ZERO (ships_zero) sets it to 0 */
u8 enc_c;                                        /* carry with which it ends (used by the caller's next DORND) */

/* THERE: C = 1 if we are in the Constrictor's system (galaxy 2, which is 1 internally; system 144, 33) */
u8 there(void) { return (u8)(gcnt == 1 && qq0 == 144 && qq1 == 33); }

static u8 gov(void) { return qq4; }             /* gov: government of the current system (QQ4 on arrival; in the port QQ4 does not change when choosing) */

static u8 nwshp(u8 type) { return (u8)(ship_new(type) >= 0); }    /* NWSHP: C = 1 if it fits */

/* BAD: (slaves + narcotics) * 2 + firearms, with the 6502 carries; returns A and leaves its carry in *c */
static u8 bad(u8 *c)
{
    unsigned s;
    u8 a;
    s = (unsigned)qq20[3] + qq20[6];             /* CLC; ADC QQ20+6 */
    a = (u8)s;
    *c = (u8)(a >> 7);                           /* ASL A */
    a = (u8)(a << 1);
    s = (unsigned)a + qq20[10] + *c;             /* ADC QQ20+10 */
    *c = (u8)(s >> 8);
    return (u8)s;
}

/* Part 1 (MTT4): a trader with the position left by part 2 */
static void mtt4(u8 c)
{
    u8 a, x;
    em_rand_c = c;
    a = em_dornd();
    c = (u8)(a & 1);
    a = (u8)(a >> 1);                            /* LSR A */
    inwk[32] = a;                                /* no AI and no aggression */
    inwk[29] = a;
    x = (u8)(inwk[31] >> 7);
    inwk[31] = (u8)((inwk[31] << 1) | c);        /* ROL INWK+31 */
    c = x;
    inwk[27] = (u8)((a & 31) | 16);
    em_rand_c = c;
    a = em_dornd();
    if (!(a & 0x80)) {                           /* half of them go to dock at the station */
        a = (u8)(inwk[32] | 0xC0);
        inwk[32] = a;
        inwk[I_NEWB] = 0x10;
    }
    a = (u8)((a & 2) + SHIPT_COBRA_MK_3 + em_rand_c);           /* nodo: AND #2; ADC #CYL */
    if (a == SHIPT_ROCK_HERMIT) { enc_c = 1; return; }
    enc_c = nwshp(a);
}

/* Part 4 from nopl: pirates in a group, or a single ship (bounty hunter, or the Constrictor of mission 1) */
static void part4_nopl(u8 c)
{
    u8 a, y, t, xx13;
    em_rand_c = c;
    a = em_dornd();
    c = em_rand_c;
    if (gov() != 0) {                            /* in anarchy it always continues */
        if (a >= 90) { enc_c = 1; return; }      /* 65 %: nothing */
        if ((u8)(a & 7) < gov()) { enc_c = 0; return; }        /* the safer the system, the fewer ships */
        c = 1;
    }
    em_rand_c = c;                               /* LABEL_2: Ze */
    a = ships_ze();
    c = em_rand_c;
    if (a >= 100) {                              /* mt1: pirates (1 to 4) */
        a = (u8)(a & 3);
        enc_ev = a;
        xx13 = a;
        c = 1;
        do {                                     /* mt3 */
            em_rand_c = c;
            t = em_dornd();
            a = (u8)(em_dornd() & t & 7);
            a = (u8)(a + SHIPT_SIDEWINDER + em_rand_c);         /* ADC #PACK */
            c = nwshp(a);
            xx13--;
        } while (!(xx13 & 0x80));
        enc_c = c;
        return;
    }
    enc_ev++;
    y = (u8)((a & 3) + SHIPT_COBRA_MK_3_P);      /* ADC #CYL2 with C = 0 */
    c = there();
    if (c) {
        inwk[32] = 0xF9;                         /* ECM and aggression 60 */
        a = (u8)(tp & 3);
        c = (u8)(a & 1);
        a = (u8)(a >> 1);
        if (c && (u8)(a | many[SHIPT_CONSTRICTOR]) == 0) {      /* mission 1 in progress, not killed and no other Constrictor */
            enc_c = nwshp(SHIPT_CONSTRICTOR);    /* YESCON */
            return;
        }
    }
    inwk[I_NEWB] = 0x04;                         /* NOCON: hostile */
    em_rand_c = c;
    a = em_dornd();
    c = (u8)(a >= 200);
    inwk[32] = (u8)(((a << 1) | c) | 0xC0);      /* CMP #200; ROL A; ORA #%11000000 */
    enc_c = nwshp(y);
}

/* Parts 2, 3 and 4 of the main loop: when MCNT reaches 0 (every 256 iterations). c = carry on entry */
void encounters_tick(u8 c)
{
    u8 a, x, t;
    enc_c = c;
    if (mj) return;                              /* in hyperspace nothing appears */
    em_rand_c = c;
    a = em_dornd();
    if (a >= 35 || junk_count >= 3) goto mtt1;   /* 87 %: neither asteroid nor cargo */
    ship_zinf();
    inwk[7] = 38;                                /* far */
    em_rand_c = 0;                               /* CMP #3 with JUNK < 3 */
    a = em_dornd();
    x = em_rand_x;
    c = em_rand_c;
    inwk[0] = a;
    inwk[3] = x;
    inwk[2] = (u8)(a & 0x80);
    inwk[5] = (u8)(x & 0x80);
    inwk[1] = (u8)(c << 1);                      /* ROL INWK+1 twice: at 512 from the centre if C */
    em_rand_c = 0;                               /* the second ROL leaves C = 0 */
    a = em_dornd();
    x = em_rand_x;
    c = em_rand_c;
    if (em_rand_v) { mtt4(c); return; }          /* BVS MTT4: a trader (and TT100: it does not continue) */
    inwk[29] = (u8)(a | 0x6F);                   /* random undamped roll */
    if (sspr) goto mtt1;
    a = x;
    if (!c) inwk[27] = (u8)((a & 31) | 16);      /* speed 16..31 */
    else inwk[30] = (u8)(a | 0x7F);              /* MTT2: pitch */
    em_rand_c = c;                               /* MTT3 */
    a = em_dornd();
    if (a >= 252) {                              /* 1.2 %: hermit */
        inwk[32] = SHIPT_ROCK_HERMIT;            /* STA INWK+32 with A = HER: aggression 15 */
        a = SHIPT_ROCK_HERMIT;
    } else {                                     /* thongs: asteroid (96 %) or cargo (2 %) or rock (2 %) */
        c = (u8)(a >= 10);
        a = (u8)((a & 1) + SHIPT_CANISTER + c);
    }
    c = nwshp(a);                                /* whips */
mtt1:
    if (sspr) { enc_c = 1; return; }             /* in the station zone there are only asteroids */
    /* part 3: the police, according to how illegal our cargo is */
    a = bad(&c);
    c = (u8)(a >> 7);
    a = (u8)(a << 1);                            /* ASL A */
    if (many[SHIPT_VIPER]) a = (u8)(a | fist);
    t = a;
    em_rand_c = c;
    a = ships_ze();
    c = em_rand_c;
    if (a == 136) {                              /* 0.4 %: Thargoid (fothg) and continues in nopl */
        em_rand_c = 1;                           /* C = 1 (CMP #136 equal) for the DORND of Ze in GTHG */
        part4_nopl(gthg());
        return;
    }
    if (a < t) c = nwshp(SHIPT_VIPER);
    else c = 1;
    if (many[SHIPT_VIPER]) { enc_c = c; return; }
    /* parte 4 */
    enc_ev--;
    if (!(enc_ev & 0x80)) { enc_c = c; return; } /* not yet */
    enc_ev++;
    if ((tp & 0x0C) == 0x08) {                   /* mission 2 with the plans: Thargoids at 22 % */
        em_rand_c = 1;
        a = em_dornd();
        if (a >= 200) { em_rand_c = 1; c = gthg(); }    /* C = 1 (CMP #200) for Ze */
        else c = 0;
    } else c = (u8)((tp & 0x0C) >= 0x08);
    part4_nopl(c);
}
