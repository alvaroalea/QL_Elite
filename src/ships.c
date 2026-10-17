#include "ships.h"
#include "move.h"
#include "space.h"
#include "ship3d.h"
#include "state.h"
#include "emath.h"
#include "ship_math.h"
#include "universe.h"
#include "game.h"
#include "sun.h"
#include "combat.h"
#include "damage.h"
#include "tactics.h"
#include "dash.h"

static u8 heaps[NOSH][HEAP_BYTES];

/* E%: banderas NEWB iniciales por tipo (1..32) */
static const u8 e_newb[33] = {
    0, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x61, 0xA0, 0xA0, 0xA0, 0xA1, 0xA1, 0xC2,
    0x0C, 0x8C, 0x8C, 0x8C, 0x0C, 0x8C, 0x05, 0x8C, 0x8C, 0x8C, 0x82, 0x0C, 0x0C, 0x04, 0x04, 0x00
};

u8 ship_e_newb(u8 type) { return (u8)(type <= 32 ? e_newb[type] : 0); }

const u8 *ship_bp_for(u8 type)
{
    if (type == SHIPT_CORIOLIS && qq5 >= 10) return SHIP_BP(SHIPT_DODO);
    return SHIP_BP(type);
}

void ship_zinf(void)
{
    int i;
    for (i = 0; i < NI; i++) inwk[i] = 0;
    inwk[18] = 96; inwk[22] = 96;
    inwk[14] = (u8)(96 | 0x80);
}

static void put_slot(int n) { u8 *p = state_slot(n); int i; for (i = 0; i < NI; i++) p[i] = inwk[i]; }
static void get_slot(int n) { u8 *p = state_slot(n); int i; for (i = 0; i < NI; i++) inwk[i] = p[i]; }

int ship_new(u8 type)
{
    int n;
    const u8 *bp;
    for (n = 0; n < NOSH; n++) if (frin[n] == 0) break;
    if (n >= NOSH) return -1;
    if (!(type & 0x80)) {                              /* NW6: energia y banderas de linea del plano */
        bp = ship_bp_for(type);
        inwk[35] = bp[14];
        inwk[31] = (u8)(bp[19] & 7);
        inwk[36] = (u8)(inwk[36] | (e_newb[type] & 0x6F));
        many[type]++;
        if (type == SHIPT_ROCK_HERMIT || (type >= SHIPT_ESCAPE_POD && type < SHIPT_SHUTTLE + 2)) junk_count++;   /* JL..JH-1 y HER */
    }
    frin[n] = type;
    put_slot(n);
    heaps[n][0] = 0;
    return n;
}

/* KS2: los misiles que apuntaban a una ranura posterior a n pasan a la anterior; los que apuntaban a n se quedan sin blanco */
void ks2(int n)
{
    int j;
    u8 a, *p;
    for (j = 0; j < NOSH && frin[j]; j++) {
        if (frin[j] != SHIPT_MISSILE) continue;
        p = state_slot(j);
        a = p[32];
        if (!(a & 0x80)) continue;
        a = (u8)((a & 0x7F) >> 1);
        if (a < n) continue;
        if (a == n) { p[32] = 0; continue; }
        p[32] = (u8)(((a - 1) << 1) | 0x80);
    }
}

static void ks4(int n);

void ship_kill(int n)
{
    u8 type = frin[n];
    int j, i;
    if (mstg == n) {                                     /* el blanco de nuestro misil: TARGET LOST */
        abort_missile(MS_GREEN);
        mess(MESS_TARGET_LOST);
    }
    if (type == SHIPT_CORIOLIS) { ks4(n); return; }      /* KS4 */
    if (type == SHIPT_CONSTRICTOR) { tp = (u8)(tp | 0x02); tally[1]++; }   /* mision 1: Constrictor destruido */
    if (type < 128 && many[type]) many[type]--;
    if ((type == SHIPT_ROCK_HERMIT || (type >= SHIPT_ESCAPE_POD && type < SHIPT_SHUTTLE + 2)) && junk_count) junk_count--;
    for (j = n; j < NOSH - 1; j++) {
        frin[j] = frin[j + 1];
        for (i = 0; i < NI; i++) state_slot(j)[i] = state_slot(j + 1)[i];
        for (i = 0; i < HEAP_BYTES; i++) heaps[j][i] = heaps[j + 1][i];
    }
    frin[NOSH - 1] = 0;
    for (i = 0; i < NI; i++) state_slot(NOSH - 1)[i] = 0;
    for (i = 0; i < HEAP_BYTES; i++) heaps[NOSH - 1][i] = 0;
    ks2(n);
}

void ships_clear(void)
{
    int n, i;
    for (n = 0; n <= NOSH; n++) { frin[n] = 0; for (i = 0; i < NI; i++) state_slot(n)[i] = 0; }
    for (n = 0; n < NOSH; n++) for (i = 0; i < HEAP_BYTES; i++) heaps[n][i] = 0;
    for (n = 0; n <= NTY; n++) many[n] = 0;
    junk_count = 0;
}

/* Borra las lineas dibujadas de la nave de la ranura n (monton de lineas) */
static void erase_lines(int n)
{
    u8 *p = state_slot(n);
    if (!(p[I_STATE] & 0x08)) return;
    zxx19 = heaps[n];
    zlsnum = 1; zlsnum2 = heaps[n][0];
    ship_lsclr();
    p[I_STATE] = (u8)(p[I_STATE] & 0xF7);
}

/* Tras borrar la pantalla entera: ninguna nave figura como dibujada (sus lineas ya no estan) */
void ships_forget_lines(void)
{
    int n;
    for (n = 0; n < NOSH; n++) { state_slot(n)[I_STATE] = (u8)(state_slot(n)[I_STATE] & 0xF7); heaps[n][0] = 0; }
}

void ships_erase_all(void)
{
    int n;
    for (n = 0; n < NOSH; n++) if (frin[n] != 0 && !(frin[n] & 0x80)) erase_lines(n);
}

/* ---------------------------------------------------------------- SOLAR y NWSPS */

/* SOS1: el planeta (tipo 128, o 130 si el bit 1 del nivel tecnologico esta puesto) con los giros sin amortiguar */
static void sos1(void)
{
    inwk[29] = 127; inwk[30] = 127;
    ship_new((u8)((qq5 & 2) | 0x80));
}

/* NWSPS: la estacion en la posicion de INWK (IA $81, alabeo $FF, morro invertido con los tres NwS1) en la ranura 1 */
static void nwsps(void)
{
    inwk[32] = 0x81;
    inwk[30] = 0;
    inwk[I_NEWB] = 0;
    frin[1] = 0;                                         /* STX FRIN+1: la estacion sustituye al sol */
    inwk[29] = 0xFF;
    inwk[10] = (u8)(inwk[10] ^ 0x80); inwk[12] = (u8)(inwk[12] ^ 0x80); inwk[14] = (u8)(inwk[14] ^ 0x80);
    ship_new(SHIPT_CORIOLIS);                            /* Dodo si el nivel tecnologico es 10 o mas (ship_bp_for) */
}

/* TT110: al despegar, planeta delante (z_sign = 1) y estacion justo detras (z = -256) */
void ships_launch(void)
{
    ships_clear();
    ship_zinf();                                         /* RES2 acaba en ZINF */
    inwk[8]++;
    sos1();
    inwk[8] = 0x80;
    inwk[7]++;
    nwsps();
}

/* SOLAR: tras el hiperespacio; FIST a la mitad, Trumbles (si los hubiera), planeta delante y arriba a la derecha y sol detras */
void ships_arrive(void)
{
    const uni_system *sy = &uni_galaxy[cur_sys];
    u8 a, c, c2;
    unsigned v;
    ships_clear();
    if (tribble[0]) {                                    /* LDA TRIBBLE; BEQ nobirths */
        qq20[0] = 0; qq20[6] = 0;
        a = (u8)(em_dornd() & 15);
        v = (unsigned)a + tribble[0] + em_rand_c;        /* ADC TRIBBLE */
        a = (u8)(v | 4);
        c = (u8)(v >> 8);
        c2 = (u8)(a >> 7);
        tribble[0] = (u8)((a << 1) | c);                 /* ROL A */
        c = (u8)(tribble[1] >> 7);
        tribble[1] = (u8)((tribble[1] << 1) | c2);       /* ROL TRIBBLE+1 */
        if (tribble[1] & 0x80) tribble[1] = (u8)((tribble[1] >> 1) | (c << 7));   /* ROR TRIBBLE+1 */
    }
    c = (u8)(fist & 1);
    fist = (u8)(fist >> 1);                              /* LSR FIST */
    ship_zinf();
    a = (u8)((((sy->seed0 >> 8) & 3)) + 3 + c);          /* QQ15+1 AND 3; ADC #3 */
    inwk[8] = a;
    inwk[2] = (u8)(a >> 1); inwk[5] = (u8)(a >> 1);      /* ROR A (C = 0) */
    sos1();
    inwk[8] = (u8)((sy->goatsoupseed.b & 7) | 0x81);     /* QQ15+3 */
    inwk[2] = (u8)(sy->goatsoupseed.d & 3); inwk[1] = inwk[2];   /* QQ15+5 */
    inwk[29] = 0; inwk[30] = 0;
    ship_new(129);
}

/* KS4: la estacion se ha alejado (FAROF): en su ranura (la 1) aparece el sol, en y_sign = 6 */
static void ks4(int n)
{
    erase_lines(n);
    ship_zinf();
    sun_flflls();
    frin[1] = 0;
    sspr = 0;
    inwk[5] = 6;
    ship_new(129);
}

/* Parte 14 del bucle de vuelo (MA22..MA23S): cada 32 vueltas, fuera del hiperespacio y sin estacion: si estamos cerca del
   planeta (signos 0) y su punto planeta + 2 * morro esta a menos de 192 en cada eje, se borra el sol y aparece la estacion.
   Devuelve 1 si ha aparecido (SPBLB) */
int ships_part14(u8 mc)
{
    int i;
    if (mj) return 0;
    if (mc & 31) return 0;
    if (sspr) return 0;
    if (sm_mas2(0, 0)) return 0;
    for (i = 0; i <= 28; i++) inwk[i] = ks[i];           /* MAL4: INWK = planeta (29 bytes) */
    if (sm_mas1(9, 0)) return 0;                         /* x += nosev_x * 2 */
    if (sm_mas1(11, 3)) return 0;
    if (sm_mas1(13, 6)) return 0;
    if (inwk[1] > 192 || inwk[4] > 192 || inwk[7] > 192) return 0;    /* FAROF2 con 192 */
    sun_wpls();                                          /* WPLS: se borra el sol */
    nwsps();
    return 1;
}

/* ---------------------------------------------------------------- contactos de prueba */

int ships_spawn(u8 type)
{
    int n;
    const u8 *bp;
    u8 sg, sp;
    ship_zinf();
    inwk[0] = em_dornd(); inwk[1] = (u8)(em_dornd() % 2);   inwk[2] = (u8)(em_dornd() & 0x80);      /* entre 500 y 1500 unidades: */
    inwk[3] = em_dornd(); inwk[4] = (u8)(em_dornd() % 2);   inwk[5] = (u8)(em_dornd() & 0x80);      /* se ven como figuras de lineas */
    inwk[6] = em_dornd(); inwk[7] = (u8)(2 + em_dornd() % 4); inwk[8] = 0;
    bp = ship_bp_for(type);
    sp = bp[15] ? (u8)(bp[15] / 8) : 0;                  /* se acercan despacio para poder verlas */
    inwk[27] = sp;
    sg = (u8)(em_dornd() & 0x80);
    inwk[29] = (u8)(sg | 40);
    inwk[30] = (u8)((sg ^ 0x80) | 20);
    inwk[32] = (u8)(em_dornd() | 0x80);                  /* provisional: IA con agresividad al azar */
    if (type != SHIPT_ASTEROID) inwk[36] = 0x04;         /* y hostil, para probar TACTICS hasta que haya encuentros */
    n = ship_new(type);
    return n;
}

int ships_explode_nearest(void)
{
    int n, best = -1;
    unsigned bd = 0xFFFF, d;
    for (n = 0; n < NOSH; n++) {
        u8 t = frin[n], *p;
        if (t == 0) break;
        if ((t & 0x80) || t == SHIPT_CORIOLIS || t == SHIPT_DODO) continue;
        p = state_slot(n);
        if (p[I_STATE] & 0xA0) continue;
        d = (unsigned)p[7] + p[1] + p[4];
        if (d < bd) { bd = d; best = n; }
    }
    if (best >= 0) state_slot(best)[I_STATE] = (u8)(state_slot(best)[I_STATE] | 0x80);
    return best;
}

/* ---------------------------------------------------------------- SFS1, SFS2, SPIN, SPIN2 */

/* SFS2: suma al eje X de la posicion (A * 2) con el signo de A (MVT1) */
static void sfs2(u8 a, u8 x)
{
    zR = (u8)(a << 1);
    mvt1((u8)(a & 0x80), x);
}

/* SFS1: la hija sale del bloque K% de la madre (INF), no de INWK; INWK se guarda y se restituye (el original usa XX3).
   Si la madre es la estacion, sale por la ranura a velocidad 32. Los restos (PLT..SPL) giran al azar y su velocidad es
   aleatoria (0..15: el X que deja DORND); el contador de alabeo es $7F con el signo del bit 7 del numero aleatorio (ROR con el acarreo de ASL) */
int sfs1(u8 ai, u8 type)
{
    u8 save[NI], a, c;
    int i, n;
    for (i = 0; i < NI; i++) { save[i] = inwk[i]; inwk[i] = zinf[i]; }
    if (ztype == SHIPT_CORIOLIS) {
        inwk[27] = 32;
        sfs2(inwk[10], 0);
        sfs2(inwk[12], 3);
        sfs2(inwk[14], 6);
    }
    inwk[32] = ai;
    inwk[29] = (u8)(inwk[29] & 0xFE);                    /* LSR; ASL */
    if (type >= SHIPT_PLATE && type <= SHIPT_SPLINTER) {
        em_rand_c = 1;                                   /* CMP #PLT con A >= PLT deja C = 1 para DORND */
        a = em_dornd();
        c = (u8)(a >> 7);
        inwk[30] = (u8)(a << 1);
        inwk[27] = (u8)(em_rand_x & 15);                 /* TXA: X lo ha dejado DORND (aleatorio), no es el tipo */
        inwk[29] = (u8)((c << 7) | 0x7F);                /* LDA #$FF; ROR A */
    }
    n = ship_new(type);
    em_rand_c = (u8)(n >= 0);                            /* NWSHP vuelve con C = 1 si hubo sitio (lo usa el DORND siguiente) */
    for (i = 0; i < NI; i++) inwk[i] = save[i];
    return n;
}

void spin2(u8 cnt, u8 type)
{
    while (cnt) { sfs1(0, type); cnt--; }
}

/* SPIN: la cantidad es (tipo AND byte 0 del plano AND 15): el TYA tapa el numero aleatorio (asi en el original) */
void spin(u8 type)
{
    u8 a = em_dornd();
    if (!(a & 0x80)) return;                             /* BPL oh */
    spin2((u8)(type & zxx0[0] & 15), type);
}

/* ---------------------------------------------------------------- Ze, fq1 y los restos de nuestra nave (DEATH, D1) */

/* Ze: INWK para una nave al azar a 25 unidades altas en cada eje, con IA y agresividad; cae en DORND2 y devuelve su A */
static u8 ze(void)
{
    u8 a, x, c;
    ship_zinf();
    a = em_dornd();
    inwk[2] = (u8)(a & 0x80);
    x = em_rand_x;
    inwk[5] = (u8)(x & 0x80);
    inwk[1] = inwk[4] = inwk[7] = 25;
    c = (u8)(x >= 245);                                  /* CMP #245; ROL A */
    inwk[32] = (u8)((u8)((x << 1) | c) | 0xC0);
    return em_dornd2();
}

/* fq1: morro hacia nosotros, velocidad DELTA * 2 + C, y NWSHP */
static int fq1(u8 type, u8 c)
{
    inwk[14] = 0x60;
    inwk[22] = (u8)(0x60 | 0x80);
    inwk[27] = (u8)((zDELTA << 1) | (c & 1));           /* LDA DELTA; ROL A */
    return ship_new(type);
}

/* FRS1: una nave nueva justo delante de nosotros, por debajo de la mira (y = -28, z = 14), con IA = MSTG * 2 | $80 */
int frs1(u8 type)
{
    u8 a, c;
    ship_zinf();
    inwk[3] = 28;
    inwk[6] = 14;                                        /* LSR A */
    inwk[5] = 0x80;
    a = mstg;
    c = (u8)(a >> 7);
    inwk[32] = (u8)((a << 1) | 0x80);                    /* ASL A; ORA #$80: el acarreo pasa a fq1 */
    return fq1(type, c);
}

/* GTHG: un Thargoid agresivo (Ze, IA = $FF) y un Thargon en el mismo sitio */
u8 gthg(void)
{
    ze();
    inwk[32] = 0xFF;
    ship_new(SHIPT_THARGOID);
    return (u8)(ship_new(SHIPT_THARGON) >= 0);           /* JMP NWSHP: C = 1 si cabe */
}

u8 ships_ze(void) { return ze(); }

u8 *ships_heap(int n) { return heaps[n]; }

/* ESCAPE (la parte que mueve la nave): nuestra Cobra sale delante (FRS1 con CYL o, si no cabe, CYL2) y se aleja durante
   97 fotogramas (MVEIT + LL9 sobre INWK); luego se borra del radar. El resto (carga, FIST, cupula, combustible y GOIN) lo
   hace flight.c */
void ships_escape(void)
{
    int n;
    u8 type = SHIPT_COBRA_MK_3;
    ztype = type;                                        /* LDX #CYL; STX TYPE */
    n = frs1(type);
    if (n < 0) { type = SHIPT_COBRA_MK_3_P; n = frs1(type); }
    if (n < 0) return;
    inwk[27] = 8;
    inwk[30] = 194;
    inwk[32] = 194 >> 1;                                 /* LSR A: 97 vueltas */
    zinf = state_slot(n); zxx19 = heaps[n]; zxx0 = ship_bp_for(ztype); move_xsav = (u8)n;
    ships_hooks();
    do {
        mveit();
        ship_ll9();
        inwk[32]--;
    } while (inwk[32] != 0);
    space_scan(ztype);                                   /* JSR SCAN: lo quita del radar */
}

/* SESCP: la nave actual lanza una capsula de escape (SFS1 con IA %11111110) */
void sescp(void) { sfs1(0xFE, SHIPT_ESCAPE_POD); }

/* D1 de DEATH: restos (cargas o placas) que salen de donde estaba nuestra nave hasta llenar la ranura 4; algunos explotan */
int ships_death_debris_one(void)
{
    u8 a, x, c, t;
    int n;
    a = ze();
    x = em_rand_x;
    a = (u8)(a >> 2);
    inwk[0] = a;                                         /* LSR A; LSR A; STA INWK */
    inwk[1] = inwk[4] = inwk[7] = inwk[32] = 0;
    mcnt = 255;
    a = (u8)(a ^ 0x2A); inwk[3] = a;
    a = (u8)(a | 0x50); inwk[6] = a;                     /* z con el bit 6: nunca chocan con nosotros */
    a = (u8)(x & 0x8F); inwk[29] = a;
    lasct = 64;
    c = (u8)(a & 1); a = (u8)((a >> 1) | 0x80);          /* SEC; ROR A */
    inwk[30] = (u8)(a & 0x87);
    t = c ? SHIPT_PLATE : SHIPT_CANISTER;                /* BCC D3 (el plano de la placa existe) */
    n = fq1(t, c);
    em_rand_c = (u8)(n >= 0);                            /* NWSHP vuelve con C = 1 si hubo sitio */
    a = em_dornd();
    if (n >= 0) state_slot(n)[31] = (u8)(a & 0x80);      /* LDY #31; STA (INF),Y: unos explotan ya */
    return n;
}

void ships_death_debris(void)
{
    int guard = 0;
    do ships_death_debris_one(); while (frin[4] == 0 && ++guard < 64);
}

/* ---------------------------------------------------------------- PLANET */

void planet(void)
{
    u8 a;
    int sun;
    a = inwk[8];
    if (a >= 48) goto pl2;
    if ((u8)(a | inwk[7]) == 0) goto pl2;
    if (sm_proj()) goto pl2;
    zP1 = 96; zP = 0;
    em_dvid3b(0, (u8)(inwk[6] | 1), inwk[7], inwk[8]);   /* DVID3B2 */
    if (zK[1] != 0) zK[0] = 248;
    if (ztype & 1) { sun_draw(); return; }
    circle_wpls2();                                      /* PL9: borra el circulo anterior y dibuja el nuevo */
    circle_draw();
    return;
pl2:
    sun = ztype & 1;
    if (sun) sun_wpls(); else circle_wpls2();
}

/* ---------------------------------------------------------------- DOEXP */

static u8 e_u, e_tgt;
static u8 e_col;

static u8 dornd0(void)                                  /* paso del generador con CLC delante (acarreo de entrada 0) */
{
    em_rand_c = 0;
    return em_dornd();
}

/* colores de las particulas: COL = (aleatorio AND 12) OR 16 -> 16, 20, 24 y 28 (azul, "rojo", "fuzzy" y otro en el Apple II) */
static u8 expl_col(u8 v)
{
    switch (v & 0x0C) {
    case 0:  return C_CYAN;                              /* el azul puro se ve poco sobre negro */
    case 4:  return C_RED;
    case 8:  return C_YELLOW;
    }
    return C_WHITE;
}

/* EXS1: (A X) = (S R) +/- (aleatorio * Q / 256); devuelve A (Z = 0 si queda fuera de la pantalla) y X en *xout */
static u8 exs1(u8 s_in, u8 *xout)
{
    u8 a, c, hi, t;
    unsigned s;
    int d, cc;
    zS = s_in;
    a = dornd0();
    c = em_rand_c;
    hi = (u8)(a >> 7);
    a = (u8)((a << 1) | c);                              /* ROL A */
    c = hi;
    if (!c) {
        a = em_fmltu(a);
        c = em_carry;
        s = (unsigned)a + zR + c;                        /* ADC R */
        *xout = (u8)s;
        c = (u8)(s >> 8);
        return (u8)(zS + c);                             /* LDA S; ADC #0 */
    }
    a = em_fmltu(a);                                     /* EX5 */
    t = a;
    d = (int)zR - t - (1 - (int)em_carry); cc = (d >= 0);
    *xout = (u8)d;
    return (u8)(zS - (1 - cc));
}

/* PTCLS: dibuja (XOR) la nube de particulas del monton; llamarla dos veces la borra */
static void ptcls(void)
{
    u8 a, x, y, cnt, rand1, k, xr, hi, y1, cnt2;
    zQ = zxx19[0];
    a = zxx19[1];
    if (a & 0x80) a = (u8)(a ^ 0xFF);
    a = (u8)(a >> 4);
    e_u = (u8)(a | 1);
    e_tgt = zxx19[2];
    rand1 = em_rand[1];
    y = 6;
    for (;;) {                                           /* EXL5 */
        for (x = 3; ; x--) { y++; zk3[x] = zxx19[y]; if (x == 0) break; }
        cnt = y;
        for (k = 3; k <= 6; k++) em_rand[k - 3] = (u8)(zxx19[k] ^ cnt);          /* EXL2: STA $FFFF,Y = RAND + Y - 3 */
        cnt2 = e_u;
        for (;;) {                                       /* EXL4 */
            a = dornd0();
            e_col = expl_col(a);
            zR = zk3[1];
            hi = exs1(zk3[0], &xr);
            if (hi == 0 && xr < 2 * 68 - 1) {            /* BNE EX11; CPX #2*Y-1; BCS EX11 */
                y1 = xr;
                zR = zk3[3];
                hi = exs1(zk3[2], &xr);
                if (hi == 0) gfx_cpix(xr, y1, e_col);    /* LDA Y1; JSR CPIX */
            } else {
                dornd0();                                /* EX11: un paso mas del generador */
            }
            cnt2--;                                      /* DEC CNT2; BPL EXL4 */
            if (cnt2 & 0x80) break;
        }
        y = cnt;
        if (y < e_tgt) continue;                         /* CPY TGT; BCC EXL5 */
        break;
    }
    em_rand[1] = rand1;                                  /* PLA; STA RAND+1 */
    em_rand[3] = state_slot(0)[6];                       /* LDA K%+6; STA RAND+3 */
}

static void ex2(void) { inwk[31] = (u8)(inwk[31] | 0xA0); }

/* DOEXP: una explosion mas: borra la nube anterior, la hace crecer y la dibuja (o termina la nave) */
void doexp(void)
{
    u8 a, t, c, y;
    unsigned s;
    if (inwk[31] & 0x40) ptcls();                        /* borra la nube anterior */
    t = inwk[6];
    a = inwk[7];
    if (a >= 32) { a = 0xFE; c = 1; }                    /* CMP #32; BCC; LDA #$FE; BNE yy (el acarreo queda a 1) */
    else {
        /* ASL T; ROL A; ASL T; ROL A; SEC; ROL A */
        c = (u8)(t >> 7); t = (u8)(t << 1);
        { u8 hi = (u8)(a >> 7); a = (u8)((a << 1) | c); c = hi; }
        c = (u8)(t >> 7); t = (u8)(t << 1);
        { u8 hi = (u8)(a >> 7); a = (u8)((a << 1) | c); c = hi; }
        { u8 hi = (u8)(a >> 7); a = (u8)((a << 1) | 1); c = hi; }
    }
    zQ = a;                                              /* yy: STA Q */
    s = (unsigned)zxx19[1] + 4 + c;                      /* LDA (XX19),Y; ADC #4 */
    if (s > 255) { ex2(); return; }
    a = (u8)s;
    zxx19[1] = a;
    em_dvid4(a);                                         /* (P R) = A * 256 / Q */
    a = zP;
    if (a >= 0x1C) a = 0xFE;
    else {                                               /* ASL R; ROL A (tres veces) */
        for (y = 0; y < 3; y++) { c = (u8)(zR >> 7); zR = (u8)(zR << 1); a = (u8)((a << 1) | c); }
    }
    zxx19[0] = a;                                        /* LABEL_1: DEY; STA (XX19),Y */
    inwk[31] = (u8)(inwk[31] & 0xBF);
    if (!(inwk[31] & 0x08)) return;                      /* TT48: la nube ya existe */
    y = zxx19[2];
    do { zxx19[y] = xx3[y - 7]; y--; } while (y != 6);   /* EXL1: copia los vertices de pantalla al monton */
    inwk[31] = (u8)(inwk[31] | 0x40);
    ptcls();
}

void ships_hooks(void)
{
    ship_doexp_hook = doexp;
    ship_planet_hook = planet;
    move_tactics_hook = tactics;
}

/* FAROF: 1 si alguna coordenada alta pasa de 224 */
static int farof(void) { return inwk[1] > 224 || inwk[4] > 224 || inwk[7] > 224; }

/* ---------------------------------------------------------------- bucle por nave (MAIN LOOP: MVEIT, PLUT, LL9, KILLSHP) */

u8 ships_step(u8 view)
{
    int n;
    u8 type, ev = 0, *sl;
    ships_hooks();
    for (n = 0; n < NOSH; ) {
        type = frin[n];
        if (type == 0) break;
        get_slot(n);
        sl = state_slot(n);
        ztype = type; zinf = sl; zxx19 = heaps[n]; move_xsav = (u8)n;
        if (!(type & 0x80)) {
            zxx0 = ship_bp_for(type);
            /* parte 4: la bomba de energia destruye todo menos la estacion, los Thargoides, el Constrictor y el Dodo */
            if ((bomb & 0x80) && type != SHIPT_CORIOLIS && type != SHIPT_THARGOID && type < SHIPT_CONSTRICTOR &&
                !(inwk[31] & 0x20)) {
                inwk[31] = (u8)(inwk[31] | 0x80);
                exno2(type);
            }
        }
        mveit();
        if (player_dead) return (u8)(ev | SHIPS_EV_DEATH);         /* TACTICS nos ha matado (OOPS: JMP DEATH) */
        put_slot(n);                                     /* MAL3 */
        if (!(type & 0x80)) {                            /* partes 9 y 10: choque, recogida o atraque */
            int col = combat_collide();
            if (col == COL_DOCK) return (u8)(ev | SHIPS_EV_DOCK);      /* JMP DOENTRY */
            if (col == COL_DEATH) return (u8)(ev | SHIPS_EV_DEATH);    /* JMP DEATH */
            if (inwk[I_NEWB] & 0x80) space_scan(type);   /* MA26: recogida: se borra del radar */
        }
        plut(view);
        if (!(type & 0x80)) combat_hit();                /* HITCH y danos del laser (parte 11) */
        ship_ll9();
        sl[35] = inwk[35];
        if (!(type & 0x80) && combat_killed()) { ship_kill(n); continue; }       /* recogida o fin de la explosion: recompensa */
        if (!(type & 0x80) && !farof()) { /* cerca: sigue */ }
        else if (!(type & 0x80)) {                       /* MAC1: FAROF con C = 0 (la estacion deja paso al sol: KS4) */
            if (type == SHIPT_CORIOLIS) ev |= SHIPS_EV_STATION;
            ship_kill(n);
            continue;
        }
        sl[31] = inwk[31];                               /* MA27 */
        n++;
    }
    return ev;
}
