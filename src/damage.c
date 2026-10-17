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

/* QQ20+X: en el original detras de las 17 mercancias vienen ECM, BST, BOMB, ENGY y DKCMP (OUCH los recorre con X = 0..21) */
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

/* OUCH: 50 % de las veces y solo si X < 22 (9 %), si llevamos ese articulo y no hay otro mensaje: se pierde y se avisa */
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
    *p = 0;                                      /* STA QQ20,X con A = DLY = 0 */
    if (x < 17) { mess((u8)(MESS_GOOD + x)); return; }   /* TXA; ADC #208 (C = 0) */
    if (x == 17) { mess(MESS_ECM); return; }     /* ou2 */
    if (x == 18) { mess(MESS_FUEL_SCOOPS); return; }     /* ou3 */
    mess((u8)(113 - 20 + x + 1));                /* ADC #113-20 con C = 1: 113..115 */
}

/* OOPS: el escudo del lado de la nave que nos golpea (signo de z en su bloque K%) absorbe el dano; si no llega, lo que
   falta sale de la energia. Energia a 0 o por debajo: DEATH. Si sobrevivimos sin escudo: EXNO3 y OUCH.
   El SBC del original no lleva SEC: el acarreo de entrada (c) lo pone quien llama */
u8 oops(u8 t, u8 c)
{
    int d;
    unsigned s;
    u8 a;
    if (!(zinf[8] & 0x80)) {                     /* delante: escudo delantero */
        d = (int)fsh - t - (1 - (c & 1));
        if (d >= 0) { fsh = (u8)d; return 0; }
        a = (u8)d;
        fsh = 0;
    } else {                                     /* OO1: detras, escudo trasero */
        d = (int)ash - t - (1 - (c & 1));
        if (d >= 0) { ash = (u8)d; return 0; }
        a = (u8)d;
        ash = 0;
    }
    s = (unsigned)a + energy;                    /* OO3: ADC ENERGY con C = 0 */
    energy = (u8)s;
    if (energy == 0 || s <= 255) { player_dead = 1; return 1; }    /* BEQ -> DEATH; BCS no tomado -> DEATH */
    snd_exno3();
    em_rand_c = 1;                               /* el BCS tomado deja C = 1 para el DORND de OUCH (EXNO3 no lo toca en la prueba) */
    ouch();
    return 0;
}

/* Partes 9 y 10 del bucle de vuelo. INWK esta en los ejes del espacio (antes de PLUT) */
int combat_collide(void)
{
    u8 type = ztype, a, c, b0, y;
    unsigned s;
    if (sm_mas4((u8)(inwk[31] & 0xA0)) != 0) return COL_NONE;     /* explotando o lejos (algun byte alto) */
    a = (u8)(inwk[0] | inwk[3] | inwk[6]);
    if (a & 0x80) return COL_NONE;               /* mas de 127 en algun eje */
    if (type & 0x80) return COL_NONE;            /* planeta o sol */
    if (type == SHIPT_CORIOLIS) goto isdk;
    if (a & 0xC0) return COL_NONE;               /* mas de 63 */
    if (type == SHIPT_MISSILE) return COL_NONE;  /* los misiles hacen su dano en TACTICS */
    if (!((bst & inwk[5]) & 0x80)) goto ma58;    /* sin recogedor o por encima de nosotros: choque */
    if (type == SHIPT_CANISTER) {                /* oily: contenido aleatorio 0..7 */
        em_rand_c = 1;                           /* CPX #OIL igual: C = 1 para DORND */
        a = (u8)(em_dornd() & 7);
    } else {
        b0 = zxx0[0];
        a = (u8)(b0 >> 4);                       /* LSR x4: el acarreo es el bit 3 */
        c = (u8)((b0 >> 3) & 1);
        if (a == 0) goto ma58;
        a = (u8)(a + 1 + c);                     /* ADC #1 */
    }
    /* slvy2: tnpr1 (una unidad de la mercancia A) */
    if (a > 16 || !trade_fits(a, 1)) {          /* no cabe: MA59 */
        snd_exno3();
        inwk[31] = (u8)(inwk[31] | 0x80);        /* MA60: se destruye */
        return COL_NONE;
    }
    y = a;
    s = (unsigned)qq20[y] + 1;                   /* LDY QQ29; ADC QQ20,Y (A = 1, C = 0) */
    qq20[y] = (u8)s;
    mess((u8)(y + MESS_GOOD + (s >> 8)));        /* TYA; ADC #208 con el acarreo de la suma */
    inwk[I_NEWB] = (u8)(inwk[I_NEWB] | 0x80);   /* recogida: se quita en la parte 12 */
    return COL_NONE;

isdk:
    if (state_slot(1)[I_NEWB] & 0x04) goto ma62;     /* estacion hostil */
    if (inwk[14] < 214) goto ma62;               /* angulo de llegada > 26 grados */
    sm_sps1();
    if (zx15[2] < 89) goto ma62;                 /* fuera del cono de 22 grados */
    if ((inwk[16] & 0x7F) < 80) goto ma62;       /* ranura demasiado inclinada */
    return COL_DOCK;                             /* GOIN: DOENTRY */
ma62:
    if (zDELTA >= 5) { player_dead = 1; return COL_DEATH; }
    zDELTA = 1;                                  /* MA67: frenazo y abolladura de 5 */
    if (oops(5, 0)) return COL_DEATH;            /* el CMP #5 dejo C = 0 */
    snd_exno3();
    return COL_NONE;
ma58:
    inwk[31] = (u8)(inwk[31] | 0x80);            /* la otra nave se destruye */
    c = (u8)(inwk[I_ENERGY] & 1);
    a = (u8)((inwk[I_ENERGY] >> 1) | 0x80);      /* SEC; ROR A: 128 + energia / 2 */
    if (oops(a, c)) return COL_DEATH;
    snd_exno3();
    return COL_NONE;
}

/* Parte 15 (MA22 y MA93..MA34): cada 32 fotogramas, en tres momentos distintos */
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
        if (sm_mas2m(0) != 0) return 0;                 /* el planeta esta lejos */
        a = sm_mas3(0);
        if (a == 0xFF && em_carry) return 0;            /* MAS3 desbordo (BCS MA23) */
        d = (int)a - 36 - 1;                            /* SBC #36 con C = 0 */
        if (d < 0) { player_dead = 1; return 1; }      /* MA28: nos estrellamos contra el planeta (ALTIT queda en 255) */
        zR = (u8)d;                                     /* Q es el que hubiera (el original no lo pone) */
        altit = em_ll5();                               /* Q = SQRT(R Q) */
        return 0;
    }
    if (a == 15) {
        if (dock_auto) mess(MESS_DOCK_ON);
        return 0;
    }
    if (a != 20) return 0;
    cabtmp = 30;
    if (sspr) return 0;                                 /* con la estacion no hay sol */
    if (sm_mas2(0, NI) != 0) return 0;                  /* sol (ranura 1) lejos */
    a = sm_mas3(NI);
    c = em_carry;
    s = (unsigned)(u8)(a ^ 0xFF) + 30 + c;              /* EOR #$FF; ADC #30 */
    cabtmp = (u8)s;
    if (s > 255) { player_dead = 1; return 1; }        /* BCS MA28: nos achicharramos */
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
