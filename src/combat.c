#include "combat.h"
#include "state.h"
#include "emath.h"
#include "ship_math.h"
#include "ship3d.h"
#include "shipdata.h"
#include "ships.h"
#include "space.h"
#include "cmdr.h"
#include "sound.h"
#include "damage.h"
#include "tactics.h"
#include "lines.h"
#include "move.h"

u8 las, las2, lasct, lasx, lasy, dly;
u8 combat_view_on = 1;
void (*combat_mess_hook)(u8 token) = 0;
void (*combat_mess_off_hook)(void) = 0;

#define VIEW_X 128                       /* X: centro de la vista espacial de 256 x 136 */
#define VIEW_Y 68                        /* Y */
#define MLAS 50                          /* Mlas: potencia del laser de mineria */

/* KWL% y KWH%: lo que suma al TALLY cada tipo de nave (1..32), parte fraccionaria (/256) y entera */
static const u8 kwl[32] = {
    149, 0, 16, 10, 10, 6, 8, 10, 16, 17, 234, 170, 213, 0, 85, 26,
    85, 128, 85, 90, 85, 170, 50, 42, 21, 42, 64, 192, 170, 33, 85, 0
};
static const u8 kwh[32] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 2, 0, 5, 0
};

/* DOENTRY/RES2: al despegar desde la estacion todo esta lleno y frio */
void combat_reset(void)
{
    energy = 255; fsh = 255; ash = 255;
    gntmp = 0;
    las = las2 = lasct = 0;
    dly = 0;
    combat_view_on = 1;
}

/* TTX66 pone LAS2 = 0: la vista se ha borrado y LASLI2 no debe volver a dibujar (XOR) unas lineas que ya no estan */
void laser_forget(void) { las2 = 0; }

/* DENGY: DEC ENERGY; si llega a 0 se queda en 1. Devuelve la bandera Z del DEC */
u8 dengy(void)
{
    energy--;
    if (energy == 0) { energy++; return 1; }
    return 0;
}

/* SHD: INX; si da la vuelta, DEX y vuelve (escudo lleno); si no, cae en DENGY: cargar el escudo gasta 1 de energia */
u8 shd(u8 x)
{
    x++;
    if (x == 0) return 255;
    dengy();
    return x;
}

/* las: dos lineas desde (LASX, LASY) a (a, 2Y-1) y a (y, 2Y-1) con LL30 (= LOIN, XOR) */
static void las_pair(u8 a, u8 y)
{
    loin_draw(lasx, lasy, a, 2 * VIEW_Y - 1, LASER_COL);
    loin_draw(lasx, lasy, y, 2 * VIEW_Y - 1, LASER_COL);
}

void lasli2(void)
{
    if (!combat_view_on) return;         /* LDA QQ11; BNE RE1 */
    las_pair(32, 224);
    las_pair(48, 208);
}

void lasli(void)
{
    u8 a;
    a = em_dornd();
    lasy = (u8)((a & 7) + (VIEW_Y - 4) + (em_rand_c & 1));     /* AND #7; ADC #Y-4 (acarreo de DORND) */
    em_rand_c = 0;                                              /* el ADC no lleva: el siguiente DORND entra con C = 0 */
    a = em_dornd();
    lasx = (u8)((a & 7) + (VIEW_X - 4) + (em_rand_c & 1));     /* AND #7; ADC #X-4 */
    em_rand_c = 0;
    gntmp = (u8)(gntmp + 8);                                    /* LDA GNTMP; ADC #8 (el ADC anterior no lleva) */
    dengy();
    lasli2();
}

/* Parte 3 del bucle de vuelo (MA68): LAS = 0; si LASCT = 0, se pulsa el disparo, GNTMP < 242 y la vista tiene laser:
   LAS = LAS2 = potencia (bits 0-6), sonido, lineas, y LASCT = potencia AND %11111010 para los de pulsos (0 si bit 7) */
void laser_fire(u8 view, u8 fire)
{
    u8 l;
    las = 0;
    if (lasct) return;
    if (!fire) return;
    if (gntmp >= 242) return;
    l = laser[view & 3];
    if (!l) return;
    las = las2 = (u8)(l & 0x7F);
    snd_lasnoise();
    lasli();
    lasct = (u8)((l & 0x80) ? 0 : (l & 0xFA));
}

/* Parte 16 (MA23): con LAS2 != 0 y LASCT < 8 se borran las lineas y LAS2 = 0. Con haz y militar (LASCT = 0) se borran en el
   mismo fotograma en que se dibujan: el parpadeo es el del original */
void laser_frame_end(void)
{
    if (las2 == 0) return;
    if (lasct >= 8) return;
    lasli2();
    las2 = 0;
}

/* MLOOP: el laser se enfria 1 por vuelta y LASCT baja de 2 en 2 (de 1 a 0 si es impar) */
void laser_cool(void)
{
    u8 x;
    if (gntmp) gntmp--;
    x = lasct;
    if (x) { x--; if (x) x--; lasct = x; }
}

/* HITCH: la nave de INWK (ya en los ejes de la vista, tras PLUT) esta delante (z positiva), no es planeta ni sol, no explota,
   x_hi = y_hi = 0 y x_lo^2 + y_lo^2 <= area de impacto del plano (bytes 1 y 2). Devuelve el acarreo */
u8 hitch(void)
{
    u8 s, r, x;
    unsigned u;
    if (inwk[8]) return 0;
    if (ztype & 0x80) return 0;
    if ((inwk[31] & 0x20) | inwk[1] | inwk[4]) return 0;
    s = em_squa2(inwk[0]); r = zP;
    x = em_squa2(inwk[3]);
    u = (unsigned)zP + r; r = (u8)u;                 /* SQUA2 sale con C = 0 */
    u = (unsigned)x + s + (u >> 8);
    if (u > 255) return 0;                           /* BCS TN10 */
    s = (u8)u;
    if (zxx0[2] != s) return (u8)(zxx0[2] > s);      /* CMP S; BNE HI1 */
    return (u8)(zxx0[1] >= r);                       /* CMP R */
}

/* AN2: la estacion (ranura 1) se vuelve hostil (NEWB bit 2) */
static void an2(void) { state_slot(1)[I_NEWB] = (u8)(state_slot(1)[I_NEWB] | 0x04); }

/* ANGRY: escribe en el bloque K% de la nave (INF), no en INWK */
void angry(u8 type)
{
    u8 *p = zinf;
    if (type == SHIPT_CORIOLIS) { an2(); return; }
    if (p[I_NEWB] & 0x20) an2();                     /* la protege la estacion */
    if (p[I_AI] == 0) return;                        /* sin IA: nada que enfadar */
    p[I_AI] = (u8)(p[I_AI] | 0x80);
    p[28] = 2;                                       /* acelera */
    p[30] = 4;                                       /* cabecea */
    if (ztype >= SHIPT_COBRA_MK_3) p[I_NEWB] = (u8)(p[I_NEWB] | 0x04);   /* LDA TYPE: la nave que se esta procesando */
}

/* EXNO2: TALLY(1 0) + fraccion; si sube el byte alto, "RIGHT ON COMMANDER!" */
void exno2(u8 type)
{
    unsigned s;
    if (type >= 1 && type <= 32) {
        s = (unsigned)tallyl + kwl[type - 1]; tallyl = (u8)s;
        s = (unsigned)tally[0] + kwh[type - 1] + (s >> 8); tally[0] = (u8)s;
        if (s > 255) { tally[1]++; mess(MESS_RIGHT_ON); }
    }
    snd_exno2();                                     /* SOEXPL con Y = 55 */
}

/* Parte 11 del bucle de vuelo, tras PLUT. Sin misiles aun: se omite el aviso de blanco del misil (MSAR) */
void combat_hit(void)
{
    u8 type = ztype;
    int d;
    if (!hitch()) return;                            /* BCC MA8 */
    missile_lock(move_xsav);                         /* LDA MSAR; BEQ MA47; JSR BEEP; LDX XSAV; LDY #RED; JSR ABORT2 */
    if (las == 0) return;                            /* MA47 */
    snd_exno();                                      /* EXNO: golpe */
    if (type == SHIPT_CORIOLIS) goto mad;            /* la estacion no sufre, pero se enfada */
    if (type >= SHIPT_CONSTRICTOR) {
        if (las != (LASER_MIL & 0x7F)) goto mad;     /* solo el militar la dana... */
        las = (u8)(las >> 2);                        /* ...a la cuarta parte (LSR LAS dos veces: queda asi este fotograma) */
    }
    d = (int)inwk[I_ENERGY] - las;                   /* BURN */
    if (d >= 0) { inwk[I_ENERGY] = (u8)d; goto mad; }
    inwk[31] = (u8)(inwk[31] | 0x80);                /* destruida: LL9 la hace explotar */
    /* el acarreo con que entra el primer DORND sale de las comparaciones del original (CMP #AST, CMP #Mlas) */
    if (type == SHIPT_ASTEROID) {
        if (las == MLAS) { em_rand_c = 1; spin2((u8)(em_dornd() & 3), SHIPT_SPLINTER); }
        else em_rand_c = (u8)(las >= MLAS);
    } else em_rand_c = (u8)(type >= SHIPT_ASTEROID);
    spin(SHIPT_PLATE);
    spin(SHIPT_CANISTER);
    exno2(type);
    inwk[I_ENERGY] = 0;                              /* el original guarda lo que deja A el sonido; la nave ya no cuenta */
mad:
    angry(type);
}

/* Parte 12 (MA15..KS1S). Con NEWB bit 7 (recogida o atracada) se quita sin mas. Al terminar la explosion: FIST |= bit 6 de
   NEWB (policia) y, si no hay mensaje en pantalla (DLY) ni estamos en el hiperespacio (MJ), la recompensa del plano (bytes
   10 y 11; nada si el byte 10 es 0) y el mensaje con el dinero */
int combat_killed(void)
{
    u8 x;
    if (inwk[I_NEWB] & 0x80) return 1;
    if (!(inwk[31] & 0x80) || !(inwk[31] & 0x20)) return 0;
    fist = (u8)(fist | (inwk[I_NEWB] & 0x40));
    if (dly | mj) return 1;
    if (ztype & 0x80) return 1;
    x = zxx0[10];
    if (x == 0) return 1;
    sm_mcash(x, zxx0[11]);
    mess(MESS_CASH);
    return 1;
}

/* Parte 16 (MA77..paen1): cada 8 fotogramas, con energia >= 128 se recargan los escudos y la energia sube ENGY + 1 */
void energy_tick(u8 mc)
{
    unsigned s;
    if (mc & 7) return;
    if (energy & 0x80) { ash = shd(ash); fsh = shd(fsh); }
    s = (unsigned)engy + energy + 1;                 /* SEC; LDA ENGY; ADC ENERGY */
    if (s <= 255) energy = (u8)s;                    /* BCS paen1 */
}

void mess(u8 token)
{
    if (combat_mess_hook) combat_mess_hook(token);
    dly = MESS_DLY;
    mess_de = 0;                                 /* " DESTROYED" vale para un solo mensaje (lo consumen MESS y su borrado) */
}

/* TT100: DEC DLY; al llegar a 0 se borra el mensaje; si pasa a negativo vuelve a 0 */
void dly_tick(void)
{
    if (dly == 0) return;
    dly--;
    if (dly == 0 && combat_mess_off_hook) combat_mess_off_hook();
}
