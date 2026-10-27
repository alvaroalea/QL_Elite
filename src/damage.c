#include "damage.h"
#include "combat.h"
#include "state.h"
#include "emath.h"
#include "ship_math.h"
#include "ship3d.h"
#include "shipdata.h"
#include "space.h"
#include "game.h"
#include "sound.h"

u8 mess_de;
u8 player_dead;

/* QQ20+X: in the original ECM, BST, BOMB, ENGY and DKCMP come after the 17 commodities (OUCH walks them with X = 0..21) */
u8 *ouch_item(u8 x)
{
    if (x < 17) return &qq20[x];
    switch (x) {
    case 17: return &ecm_fitted;
    case 18: return &bst;
    case 19: return &bomb;
    case 20: return &engy;
    }
    return &dkcmp;
}

/* OUCH: 50 % of the time and only if X < 22 (9 %), if we carry that item and there is no other message: it is lost and reported */
void ouch(void)
{
    u8 a, x, *p;
    a = em_dornd();
    x = em_rand_x;
    if (a & 0x80) return;                        /* BMI out */
    if (x >= 22) return;                         /* CPX #22; BCS out */
    p = ouch_item(x);
    if (*p == 0) return;
    if (dly) return;
    mess_de = 3;                                 /* LDY #3; STY de: " DESTROYED" */
    *p = 0;                                      /* STA QQ20,X with A = DLY = 0 */
    if (x < 17) { mess((u8)(MESS_GOOD + x)); return; }   /* TXA; ADC #208 (C = 0) */
    if (x == 17) { mess(MESS_ECM); return; }     /* ou2 */
    if (x == 18) { mess(MESS_FUEL_SCOOPS); return; }     /* ou3 */
    mess((u8)(113 - 20 + x + 1));                /* ADC #113-20 with C = 1: 113..115 */
}

/* OOPS: the shield on the side of the ship that hits us (sign of z in its K% block) absorbs the damage; if it is not enough, the
   remainder comes out of energy. Energy at 0 or below: DEATH. If we survive without shield: EXNO3 and OUCH.
   The SBC of the original has no SEC: the incoming carry (c) is set by the caller */
u8 oops(u8 t, u8 c)
{
    int d;
    unsigned s;
    u8 a;
    if (!(zinf[8] & 0x80)) {                     /* ahead: front shield */
        d = (int)fsh - t - (1 - (c & 1));
        if (d >= 0) { fsh = (u8)d; return 0; }
        a = (u8)d;
        fsh = 0;
    } else {                                     /* OO1: behind, rear shield */
        d = (int)ash - t - (1 - (c & 1));
        if (d >= 0) { ash = (u8)d; return 0; }
        a = (u8)d;
        ash = 0;
    }
    s = (unsigned)a + energy;                    /* OO3: ADC ENERGY with C = 0 */
    energy = (u8)s;
    if (energy == 0 || s <= 255) { player_dead = 1; return 1; }    /* BEQ -> DEATH; BCS not taken -> DEATH */
    snd_exno3();
    em_rand_c = 1;                               /* the taken BCS leaves C = 1 for the DORND of OUCH (EXNO3 does not touch it in the test) */
    ouch();
    return 0;
}

/* Parts 9 and 10 of the flight loop. INWK is in space axes (before PLUT) */
int combat_collide(void)
{
    u8 type = ztype, a, c, b0, y;
    unsigned s;
    if (sm_mas4((u8)(inwk[31] & 0xA0)) != 0) return COL_NONE;     /* exploding or far (some high byte) */
    a = (u8)(inwk[0] | inwk[3] | inwk[6]);
    if (a & 0x80) return COL_NONE;               /* more than 127 on some axis */
    if (type & 0x80) return COL_NONE;            /* planet or sun */
    if (type == SHIPT_CORIOLIS) goto isdk;
    if (a & 0xC0) return COL_NONE;               /* more than 63 */
    if (type == SHIPT_MISSILE) return COL_NONE;  /* missiles do their damage in TACTICS */
    if (!((bst & inwk[5]) & 0x80)) goto ma58;    /* no scoop or above us: collision */
    if (type == SHIPT_CANISTER) {                /* oily: random contents 0..7 */
        em_rand_c = 1;                           /* CPX #OIL equal: C = 1 for DORND */
        a = (u8)(em_dornd() & 7);
    } else {
        b0 = zxx0[0];
        a = (u8)(b0 >> 4);                       /* LSR x4: the carry is bit 3 */
        c = (u8)((b0 >> 3) & 1);
        if (a == 0) goto ma58;
        a = (u8)(a + 1 + c);                     /* ADC #1 */
    }
    /* slvy2: tnpr1 (one unit of commodity A) */
    if (a > 16 || !trade_fits(a, 1)) {          /* does not fit: MA59 */
        snd_exno3();
        inwk[31] = (u8)(inwk[31] | 0x80);        /* MA60: it is destroyed */
        return COL_NONE;
    }
    y = a;
    s = (unsigned)qq20[y] + 1;                   /* LDY QQ29; ADC QQ20,Y (A = 1, C = 0) */
    qq20[y] = (u8)s;
    mess((u8)(y + MESS_GOOD + (s >> 8)));        /* TYA; ADC #208 with the carry from the sum */
    inwk[I_NEWB] = (u8)(inwk[I_NEWB] | 0x80);   /* scooped: removed in part 12 */
    return COL_NONE;

isdk:
    if (state_slot(1)[I_NEWB] & 0x04) goto ma62;     /* hostile station */
    if (inwk[14] < 214) goto ma62;               /* arrival angle > 26 degrees */
    sm_sps1();
    if (zx15[2] < 89) goto ma62;                 /* outside the 22-degree cone */
    if ((inwk[16] & 0x7F) < 80) goto ma62;       /* slot too tilted */
    return COL_DOCK;                             /* GOIN: DOENTRY */
ma62:
    if (zDELTA >= 5) { player_dead = 1; return COL_DEATH; }
    zDELTA = 1;                                  /* MA67: braking and a dent of 5 */
    if (oops(5, 0)) return COL_DEATH;            /* CMP #5 left C = 0 */
    snd_exno3();
    return COL_NONE;
ma58:
    inwk[31] = (u8)(inwk[31] | 0x80);            /* the other ship is destroyed */
    c = (u8)(inwk[I_ENERGY] & 1);
    a = (u8)((inwk[I_ENERGY] >> 1) | 0x80);      /* SEC; ROR A: 128 + energia / 2 */
    if (oops(a, c)) return COL_DEATH;
    snd_exno3();
    return COL_NONE;
}

/* Part 15 (MA22 and MA93..MA34): every 32 frames, at three different moments */
u8 damage_checks(u8 mc)
{
    u8 a, c;
    unsigned s;
    int d;
    if (mj) return 0;
    a = (u8)(mc & 31);
    if (a == 10) {
        if (energy <= 50) mess(MESS_ENERGY_LOW);         /* LDA #50; CMP ENERGY; BCC; ASL A */
        altit = 255;
        if (sm_mas2m(0) != 0) return 0;                 /* the planet is far */
        a = sm_mas3(0);
        if (a == 0xFF && em_carry) return 0;            /* MAS3 overflowed (BCS MA23) */
        d = (int)a - 36 - 1;                            /* SBC #36 with C = 0 */
        if (d < 0) { player_dead = 1; return 1; }      /* MA28: we crash into the planet (ALTIT stays at 255) */
        zR = (u8)d;                                     /* Q is what it would have been (the original does not set it) */
        altit = em_ll5();                               /* Q = SQRT(R Q) */
        return 0;
    }
    if (a == 15) {
        if (dock_auto) mess(MESS_DOCK_ON);
        return 0;
    }
    if (a != 20) return 0;
    cabtmp = 30;
    if (sspr) return 0;                                 /* with the station there is no sun */
    if (sm_mas2(0, NI) != 0) return 0;                  /* sun (slot 1) far */
    a = sm_mas3(NI);
    c = em_carry;
    s = (unsigned)(u8)(a ^ 0xFF) + 30 + c;              /* EOR #$FF; ADC #30 */
    cabtmp = (u8)s;
    if (s > 255) { player_dead = 1; return 1; }        /* BCS MA28: we are scorched */
    if (cabtmp < 224) return 0;
    if (!bst) return 0;
    a = (u8)(zDELTA >> 2);                              /* DELT4+1 */
    c = (u8)(a & 1);
    s = (unsigned)(a >> 1) + qq14 + c;                  /* LSR A; ADC QQ14 */
    a = (u8)s;
    if (a >= 70) a = 70;                                /* CMP #70; BCC; LDA #70 */
    qq14 = a;
    mess(MESS_SCOOP_ON);
    return 0;
}
