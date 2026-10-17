#include "encounters.h"
#include "state.h"
#include "emath.h"
#include "ships.h"
#include "shipdata.h"
#include "game.h"
#include "universe.h"

u8 enc_ev;                                       /* EV: retraso de las naves extra */
u8 enc_c;                                        /* acarreo con que termina (lo usa el siguiente DORND del llamante) */

/* THERE: C = 1 si estamos en el sistema del Constrictor (galaxia 2, la 1 internamente; sistema 144, 33) */
u8 there(void) { return (u8)(gcnt == 1 && qq0 == 144 && qq1 == 33); }

static u8 gov(void) { return qq4; }             /* gov: gobierno del sistema actual (QQ4 al llegar; en el port QQ4 no cambia al elegir) */

static u8 nwshp(u8 type) { return (u8)(ship_new(type) >= 0); }    /* NWSHP: C = 1 si cabe */

/* BAD: (esclavos + narcoticos) * 2 + armas, con los acarreos del 6502; devuelve A y deja su acarreo en *c */
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

/* Parte 1 (MTT4): un comerciante con la posicion que dejo la parte 2 */
static void mtt4(u8 c)
{
    u8 a, x;
    em_rand_c = c;
    a = em_dornd();
    c = (u8)(a & 1);
    a = (u8)(a >> 1);                            /* LSR A */
    inwk[32] = a;                                /* sin IA ni agresividad */
    inwk[29] = a;
    x = (u8)(inwk[31] >> 7);
    inwk[31] = (u8)((inwk[31] << 1) | c);        /* ROL INWK+31 */
    c = x;
    inwk[27] = (u8)((a & 31) | 16);
    em_rand_c = c;
    a = em_dornd();
    if (!(a & 0x80)) {                           /* la mitad van a atracar en la estacion */
        a = (u8)(inwk[32] | 0xC0);
        inwk[32] = a;
        inwk[I_NEWB] = 0x10;
    }
    a = (u8)((a & 2) + SHIPT_COBRA_MK_3 + em_rand_c);           /* nodo: AND #2; ADC #CYL */
    if (a == SHIPT_ROCK_HERMIT) { enc_c = 1; return; }
    enc_c = nwshp(a);
}

/* Parte 4 desde nopl: piratas en grupo, o una nave suelta (cazarrecompensas, o el Constrictor de la mision 1) */
static void part4_nopl(u8 c)
{
    u8 a, y, t, xx13;
    em_rand_c = c;
    a = em_dornd();
    c = em_rand_c;
    if (gov() != 0) {                            /* en la anarquia siempre se sigue */
        if (a >= 90) { enc_c = 1; return; }      /* 65 %: nada */
        if ((u8)(a & 7) < gov()) { enc_c = 0; return; }        /* cuanto mas seguro el sistema, menos naves */
        c = 1;
    }
    em_rand_c = c;                               /* LABEL_2: Ze */
    a = ships_ze();
    c = em_rand_c;
    if (a >= 100) {                              /* mt1: piratas (de 1 a 4) */
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
    y = (u8)((a & 3) + SHIPT_COBRA_MK_3_P);      /* ADC #CYL2 con C = 0 */
    c = there();
    if (c) {
        inwk[32] = 0xF9;                         /* ECM y agresividad 60 */
        a = (u8)(tp & 3);
        c = (u8)(a & 1);
        a = (u8)(a >> 1);
        if (c && (u8)(a | many[SHIPT_CONSTRICTOR]) == 0) {      /* mision 1 en curso, sin matar y sin otro Constrictor */
            enc_c = nwshp(SHIPT_CONSTRICTOR);    /* YESCON */
            return;
        }
    }
    inwk[I_NEWB] = 0x04;                         /* NOCON: hostil */
    em_rand_c = c;
    a = em_dornd();
    c = (u8)(a >= 200);
    inwk[32] = (u8)(((a << 1) | c) | 0xC0);      /* CMP #200; ROL A; ORA #%11000000 */
    enc_c = nwshp(y);
}

/* Partes 2, 3 y 4 del bucle principal: cuando MCNT llega a 0 (cada 256 vueltas). c = acarreo con que se entra */
void encounters_tick(u8 c)
{
    u8 a, x, t;
    enc_c = c;
    if (mj) return;                              /* en el hiperespacio no aparece nada */
    em_rand_c = c;
    a = em_dornd();
    if (a >= 35 || junk_count >= 3) goto mtt1;   /* 87 %: ni asteroide ni carga */
    ship_zinf();
    inwk[7] = 38;                                /* lejos */
    em_rand_c = 0;                               /* CMP #3 con JUNK < 3 */
    a = em_dornd();
    x = em_rand_x;
    c = em_rand_c;
    inwk[0] = a;
    inwk[3] = x;
    inwk[2] = (u8)(a & 0x80);
    inwk[5] = (u8)(x & 0x80);
    inwk[1] = (u8)(c << 1);                      /* ROL INWK+1 dos veces: a 512 del centro si C */
    em_rand_c = 0;                               /* el segundo ROL deja C = 0 */
    a = em_dornd();
    x = em_rand_x;
    c = em_rand_c;
    if (em_rand_v) { mtt4(c); return; }          /* BVS MTT4: un comerciante (y TT100: no sigue) */
    inwk[29] = (u8)(a | 0x6F);                   /* alabeo al azar sin amortiguar */
    if (sspr) goto mtt1;
    a = x;
    if (!c) inwk[27] = (u8)((a & 31) | 16);      /* velocidad 16..31 */
    else inwk[30] = (u8)(a | 0x7F);              /* MTT2: cabeceo */
    em_rand_c = c;                               /* MTT3 */
    a = em_dornd();
    if (a >= 252) {                              /* 1,2 %: ermitano */
        inwk[32] = SHIPT_ROCK_HERMIT;            /* STA INWK+32 con A = HER: agresividad 15 */
        a = SHIPT_ROCK_HERMIT;
    } else {                                     /* thongs: asteroide (96 %) o carga (2 %) o roca (2 %) */
        c = (u8)(a >= 10);
        a = (u8)((a & 1) + SHIPT_CANISTER + c);
    }
    c = nwshp(a);                                /* whips */
mtt1:
    if (sspr) { enc_c = 1; return; }             /* en la zona de la estacion solo hay asteroides */
    /* parte 3: la policia, segun lo ilegal que llevamos */
    a = bad(&c);
    c = (u8)(a >> 7);
    a = (u8)(a << 1);                            /* ASL A */
    if (many[SHIPT_VIPER]) a = (u8)(a | fist);
    t = a;
    em_rand_c = c;
    a = ships_ze();
    c = em_rand_c;
    if (a == 136) {                              /* 0,4 %: Thargoide (fothg) y sigue en nopl */
        em_rand_c = 1;                           /* C = 1 (CMP #136 igual) para el DORND de Ze en GTHG */
        part4_nopl(gthg());
        return;
    }
    if (a < t) c = nwshp(SHIPT_VIPER);
    else c = 1;
    if (many[SHIPT_VIPER]) { enc_c = c; return; }
    /* parte 4 */
    enc_ev--;
    if (!(enc_ev & 0x80)) { enc_c = c; return; } /* aun no toca */
    enc_ev++;
    if ((tp & 0x0C) == 0x08) {                   /* mision 2 con los planos: Thargoides al 22 % */
        em_rand_c = 1;
        a = em_dornd();
        if (a >= 200) { em_rand_c = 1; c = gthg(); }    /* C = 1 (CMP #200) para Ze */
        else c = 0;
    } else c = (u8)((tp & 0x0C) >= 0x08);
    part4_nopl(c);
}
