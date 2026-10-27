#include "tactics.h"
#include "dockit.h"
#include "combat.h"
#include "damage.h"
#include "state.h"
#include "emath.h"
#include "ship_math.h"
#include "ship3d.h"
#include "shipdata.h"
#include "ships.h"
#include "space.h"
#include "sound.h"
#include "dash.h"

u8 ecma;                                         /* ECMA: count of the ECM that is sounding (anyone's); 0 = none */
u8 msar;                                         /* MSAR: the armed missile looks for a target */

#define SHIPT_SHU SHIPT_SHUTTLE
#define SHIPT_SH3 SHIPT_SIDEWINDER

/* ---------------------------------------------------------------- ECM */

/* ECBLB2: an ECM starts (ours or another ship's): ECMA = 32 and the E bulb changes */
void ecm_on(void)
{
    ecma = 32;
    dash_ecblb();
}

/* ECMOF */
void ecm_off(void)
{
    ecma = 0;
    ecmp = 0;
    dash_ecblb();
}

/* Part 3 (E key): with ECM and none other sounding, ECMP = $FF and ECBLB2 */
void ecm_key(void)
{
    if (!ecm_fitted) return;
    if (ecma) return;
    ecmp--;                                      /* DEC ECMP: 0 -> $FF */
    ecm_on();
}

/* Part 16 (MA16..MA66): ours uses energy (if it runs out, it switches off); the sound lasts ECMA frames */
void ecm_frame(void)
{
    if (ecmp) {
        if (dengy()) { ecm_off(); return; }      /* BEQ MA70 */
    }
    if (!ecma) return;
    snd_soblip((u8)(ecma | 192), 20);            /* LDY #20; ORA #192; TAX; JSR SOBLIP */
    ecma--;
    if (ecma == 0) ecm_off();
}

/* ---------------------------------------------------------------- our missiles */

void (*tac_msbar_hook)(u8 x, u8 col) = 0;        /* tests: each MSBAR (indicator, colour) */
void missile_bar(u8 col) { if (tac_msbar_hook) tac_msbar_hook(nomsl, col); dash_msbar(nomsl, col); }

/* ABORT2: missile target = slot (or $FF), missile indicator with colour col, MSAR = 0 */
void abort2(u8 slot, u8 col)
{
    mstg = slot;
    missile_bar(col);
    msar = 0;                                    /* MSBAR returns with Y = 0: STY MSAR */
}

void abort_missile(u8 col) { abort2(0xFF, col); }

/* Part 3: U (disarm), T (arm: look for target) and M (fire). Returns 1 if M jumped to the end of part 3 (BMI MA64) */
void missile_unarm(void)
{
    if (!nomsl) return;                          /* LDA KY15; AND NOMSL */
    abort_missile(MS_GREEN);
    snd_boop();
    msar = 0;
}

void missile_target(void)
{
    if (!(mstg & 0x80)) return;                  /* there is already a target */
    if (!nomsl) return;
    msar = 0xFF;                                 /* STA MSAR with the key value */
    missile_bar(MS_WHITE);
}

int missile_fire(void)
{
    if (mstg & 0x80) return 1;                   /* no target: BMI MA64 */
    frmis();
    return 0;
}

/* FRMIS: the missile leaves ahead (FRS1); the target gets angry; indicator to black, one missile less and hiss. Falls through to ANGRY with
   the A left by SOHISS (with sound off, the ABORT one: BLACK = 0) */
void frmis(void)
{
    u8 t;
    if (frs1(SHIPT_MISSILE) < 0) { fr1(); return; }
    t = mstg;
    zinf = state_slot(t);                        /* GINF */
    angry(frin[t]);
    abort_missile(MS_BLACK);
    nomsl--;
    snd_sohiss(120);
    angry(0);                                    /* falls through to ANGRY */
}

/* FR1: "MISSILE JAMMED" */
void fr1(void) { mess(MESS_JAMMED); }

/* SFRMIS: a ship launches a missile at us (SFS1 with AI $FE: with AI and aiming at us) */
void sfrmis(void)
{
    if (sfs1(0xFE, SHIPT_MISSILE) < 0) return;  /* BCC: it does not fit */
    mess(MESS_INCOMING);
    snd_sohiss(50);
}

/* Part 11: if the missile is looking for a target and the ship is in the sights, it locks on (beep and red indicator) */
void missile_lock(u8 slot)
{
    if (!msar) return;
    snd_beep();
    abort2(slot, MS_RED);
}

/* ---------------------------------------------------------------- TACTICS */

/* TA4, TA5 and TA20 (part 7): near us or with low aggression, turn towards the target (TA15); otherwise, move away */
static void ta4(void)
{
    u8 a;
    if (inwk[7] < 3 && ((inwk[1] | inwk[4]) & 0xFE) == 0) { ta15(); return; }
    em_rand_c = (u8)(inwk[7] >= 3);              /* TA5: the CMP #3 leaves the carry */
    a = (u8)(em_dornd() | 0x80);
    if (a >= inwk[32]) { ta15(); return; }
    sm_tas6();                                   /* TA20 */
    ta152((u8)(t_cnt ^ 0x80));
}

/* TA873 / TA353: the ship (missile) is destroyed; EXNO2 with X */
static void ta873(void) { inwk[31] = (u8)(inwk[31] | 0x80); }

/* TA19 and following (parts 4 to 7) */
static void ta19(void)
{
    u8 a, x, t, c;
    sm_tas2();
    t_cnt = sm_tas3(10);
    if (ztype == SHIPT_MISSILE) { sm_tas6(); ta152((u8)(t_cnt ^ 0x80)); return; }   /* TA20 */
    c = (u8)(ztype >= SHIPT_ANACONDA);           /* CMP #ANA */
    if (ztype == SHIPT_ANACONDA) {
        em_rand_c = 1;
        a = em_dornd();
        if (a >= 200) {                          /* the Anaconda releases a Worm or a Sidewinder */
            em_rand_c = 1;
            a = em_dornd();
            x = (a >= 100) ? SHIPT_WORM : SHIPT_SH3;
            sfs1(0xF1, x);
            return;
        }
        c = 0;
    }
    em_rand_c = c;                               /* TN7 */
    a = em_dornd();
    if (a >= 250) {
        em_rand_c = 1;
        a = em_dornd();
        inwk[29] = (u8)(a | 104);                /* a random roll */
    }
    /* TA7: energy */
    a = (u8)(zxx0[14] >> 1);
    if (a < inwk[35]) goto ta3l;                 /* more than half: attack with the laser */
    a = (u8)(a >> 2);
    c = 0;
    if (a < inwk[35]) goto ta3m;                 /* more than 1/8: perhaps a missile */
    em_rand_c = 1;
    a = em_dornd();
    if (a < 230) { c = 0; goto ta3m; }
    c = 1;
    if (!(ship_e_newb(ztype) & 0x80)) goto ta3m; /* LDA E%-1,X; BPL ta3: no escape pod */
    inwk[I_NEWB] = (u8)(inwk[I_NEWB] & 0xF0);
    zinf[I_NEWB] = inwk[I_NEWB];
    inwk[32] = 0;
    sescp();                                     /* abandons the ship */
    return;
ta3m:
    a = (u8)(inwk[31] & 7);                      /* missiles it has left */
    if (a == 0) goto ta3l;
    t = a;
    em_rand_c = c;
    a = (u8)(em_dornd() & 31);
    if (a >= t) goto ta3l;
    if (ecma) goto ta3l;
    inwk[31]--;
    if (ztype == SHIPT_THARGOID) { sfs1(inwk[32], SHIPT_THARGON); return; }
    sfrmis();
    return;
ta3l:
    /* TA3: laser */
    if (sm_mas4(0) & 0xE0) goto ta4;             /* far */
    x = t_cnt;
    if (x < 160) goto ta4;                       /* not aiming at us */
    a = (u8)(zxx0[19] & 0xF8);
    if (a == 0) goto ta4;                        /* no laser */
    inwk[31] = (u8)(inwk[31] | 0x40);            /* fires (LL9 draws the beam) */
    if (x < 163) goto ta4;                       /* misses */
    a = zxx0[19];
    if (oops((u8)(a >> 1), (u8)(a & 1))) return; /* LSR A; JSR OOPS: it hits us */
    inwk[28]--;
    if (ecma) return;                            /* BNE TA9-1 (RTS) */
    snd_lasnoise();
ta4:
    ta4();
}

/* TA18: a missile */
static void ta18(void)
{
    u8 a, slot, *v;
    if (ecma) goto ta352;                        /* ECM makes them explode */
    a = inwk[32];
    if ((u8)(a << 1) & 0x80) goto ta34;          /* bit 6: it chases us */
    slot = (u8)((a & 0x7F) >> 1);
    v = state_slot(slot);
    sm_vcsub(v);                                 /* K3 = missile - target */
    if ((((zk3[2] | zk3[5] | zk3[8]) & 0x7F) | zk3[1] | zk3[4] | zk3[7]) != 0) goto ta64;
    if (inwk[32] == 0x82) goto ta352;            /* target = slot 1 (the station): it explodes outright */
    a = v[31];
    if (!(a & 0x20)) v[31] = (u8)(a | 0x80);     /* BIT M32+1 ($20): if it is not exploding, destroys it */
    if ((inwk[0] | inwk[3] | inwk[6]) == 0) {    /* TA35: and if we were on top of it, it catches us */
        if (oops(80, (u8)(inwk[32] >= 0x82))) return;
    }
    exno2((u8)((inwk[32] & 0x7F) >> 1));         /* TA87: EXNO2 with X = the target's slot (so in the original) */
    ta873();
    return;
ta352:
    if ((inwk[0] | inwk[3] | inwk[6]) == 0) {
        if (oops(80, 1)) return;
    }
    exno2(SHIPT_PLATE);                          /* TA872: LDX #PLT */
    ta873();
    return;
ta34:
    if (sm_mas4(0) != 0) goto tn4;               /* still far: keep chasing us */
    ta873();
    snd_exno3();
    oops(250, 0);                                /* it catches us */
    return;
ta64:
    em_rand_c = sm_c;                            /* carry from VCSUB */
    a = em_dornd();
    if (a < 16 && (v[32] & 1)) { ecm_on(); return; }   /* M32: the target has ECM and uses it */
    ta19();
    return;
tn4:
    { int i; for (i = 0; i <= 8; i++) zk3[i] = inwk[i]; }
    ta19();
}

/* TACTICS (parts 1 to 7). X = TYPE */
void tactics(void)
{
    u8 x = ztype, a, c;
    t_rat = 3;
    t_rat2 = 4;
    t_cnt2 = 22;
    if (x == SHIPT_MISSILE) { ta18(); return; }
    if (x == SHIPT_CORIOLIS) {                   /* the station launches shuttles or, if angry, police */
        if (inwk[I_NEWB] & 0x04) {
            em_rand_c = 1;
            a = em_dornd();
            if (a < 240) return;
            if (many[SHIPT_VIPER] >= 4) return;
            sfs1(0xF1, SHIPT_VIPER);             /* TN6 */
            return;
        }
        if (many[SHIPT_SHU + 1]) return;
        em_rand_c = 1;
        a = em_dornd();
        if (a < 253) return;
        x = (u8)((a & 1) + SHIPT_SHU - 1 + 1);   /* AND #1; ADC #SHU-1 (C = 1) */
        sfs1(0xF1, x);
        return;
    }
    if (x == SHIPT_ROCK_HERMIT) {                /* the rock releases a ship */
        em_rand_c = 1;
        a = em_dornd();
        if (a < 200) return;
        inwk[32] = 0;
        inwk[I_NEWB] = 0x24;
        x = (u8)((a & 3) + SHIPT_SH3 + 1);       /* AND #3; ADC #SH3 (C = 1) */
        sfs1(0xF1, x);
        inwk[32] = 0;
        return;
    }
    if (inwk[35] < zxx0[14]) inwk[35]++;         /* TA17: recharges its energy */
    if (x == SHIPT_THARGON && many[SHIPT_THARGOID] == 0) {   /* TA21: with no mother ship it stays still */
        inwk[32] = (u8)(inwk[32] & 0xFE);
        inwk[27] = (u8)(inwk[27] >> 1);
        return;
    }
    em_rand_c = (u8)(x >= SHIPT_THARGON);        /* TA14: the CPX #TGL leaves the carry */
    em_dornd();
    a = inwk[I_NEWB];
    c = (u8)(a & 1); a = (u8)(a >> 1);
    if (c && em_rand_x >= 50) return;            /* trader: almost never does anything */
    c = (u8)(a & 1); a = (u8)(a >> 1);           /* TN1: bounty hunter */
    if (c && fist >= 40) {
        inwk[I_NEWB] = (u8)(inwk[I_NEWB] | 0x04);
        a = (u8)(inwk[I_NEWB] >> 2);
    }
    c = (u8)(a & 1); a = (u8)(a >> 1);           /* TN2: hostile? */
    if (!c) {
        if ((a >> 1) & 1) dockit(); else gopl(); /* not hostile: dock or go to the planet */
        return;
    }
    c = (u8)(a & 1);                             /* TN3: pirate near the station: calms down */
    if (c && sspr) inwk[32] = (u8)(inwk[32] & 0x81);
    { int i; for (i = 0; i <= 8; i++) zk3[i] = inwk[i]; }   /* TN4 */
    ta19();
}
