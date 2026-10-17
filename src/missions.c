#include "missions.h"
#include "detok.h"
#include "state.h"
#include "emath.h"
#include "ship_math.h"
#include "ship3d.h"
#include "shipdata.h"
#include "ships.h"
#include "move.h"
#include "space.h"
#include "sun.h"
#include "combat.h"
#include "damage.h"
#include "game.h"
#include "universe.h"
#include "text.h"
#include "input.h"
#include "sound.h"
#include "menu.h"
#include "config.h"

#define CONHIEGHT 80                             /* conhieght: hueco que se deja al Constrictor que gira */

/* ---------------------------------------------------------------- ganchos de pantalla del motor de fichas */

static u8 mis_ink = C_WHITE;

static void letter(u8 ch, u8 xc, u8 yc)
{
    if ((uptog & 0x80) && ch >= '[') ch = (u8)(ch - 0x20);      /* RR5: UPTOG, solo mayusculas (pantallas de texto) */
    txt_color(mis_ink, C_BLACK);
    txt_at(xc, yc);
    txt_putc(ch);
}

static void clss(void) { gfx_clear(C_BLACK); }
static void beep(void) { snd_bell(); }

static void pause2(void)                         /* PAUSE2: se suelta y se pulsa una tecla */
{
    int k;
    do k = in_wait(); while (k < 0 && !in_quit);
}

static void delay(u8 n)                          /* DELAY: unos 20 ms cada vuelta (un fotograma) */
{
    u8 i;
    if (in_script) return;
    for (i = 0; i < n; i++) in_poll();
}

/* nombres de sistema por TT27 (cpl y ypl): las letras pasan por TT27 una a una, como las fichas de dos letras */
static void name(u8 which)
{
    const char *s = uni_galaxy[which == 3 ? tgt_sys : cur_sys].name;
    u8 c;
    for (; *s; s++) {
        c = (u8)*s;
        if (c >= 'a' && c <= 'z') c = (u8)(c - 32);
        dt_tt27(c);
    }
}

/* TT66: borra la pantalla y pone la vista A (13 = el Constrictor girando; 1 = texto de la mision) */
void mis_tt66(u8 view)
{
    qq11 = view;
    dt_mt2();
    qq17 = 0x80;
    dtw2 = 0x80;
    dt_xc = 1; dt_yc = 1;
    gfx_clear(C_BLACK);                          /* TTX66K: texto (cleartext) o grafico (cleargrap; BOX no se dibuja) */
    if (view == 13) { sun_forget(); ships_forget_lines(); }       /* FLFLLS */
    laser_forget();                              /* LAS2 = 0 */
    dly = 0;
    mess_de = 0;
    dt_yc = 1;
}

void missions_hooks(void)
{
    dt_letter_hook = letter;
    dt_clss_hook = clss;
    dt_beep_hook = beep;
    dt_tt66_hook = mis_tt66;
    dt_pause2_hook = pause2;
    dt_delay_hook = delay;
    dt_name_hook = name;
}

/* ---------------------------------------------------------------- rutinas de las misiones */

void pas1(void) { mis_tt66(1); }                 /* PAS1 */
void mis_pause(void) { pause2(); pas1(); }       /* PAUSE */
void bris(void) { detok(216); delay(100); }      /* BRIS: INCOMING MESSAGE y 2 segundos */

static int brp(u8 a) { detok(a); return SC_STATUS; }      /* BRP: la ficha y BAY (pantalla de estado) */

int brief2(void) { tp = (u8)(tp | 0x04); return brp(11); }
int brief3(void) { tp = (u8)((tp & 0xF0) | 0x0A); return brp(222); }

int debrief2(void)
{
    tp = (u8)(tp | 0x04);                        /* mision 2 terminada */
    engy = 2;                                    /* unidad de energia de la marina */
    tally[1]++;
    return brp(223);
}

int debrief(void)
{
    tp = (u8)(tp & 0xFE);                        /* LSR TP; ASL TP */
    sm_mcash(50000 & 255, 50000 >> 8);           /* 5.000 CR */
    return brp(15);
}

void (*mis_ll9_hook)(void) = ship_ll9;
void (*mis_mveit_hook)(void) = mveit;

static void frame(void)                          /* LL9 y MVEIT del Constrictor de INWK */
{
    if (!in_script) in_poll();
    mis_ll9_hook();
    mis_mveit_hook();
}

int brief(void)
{
    int n;
    u8 x;
    tp = (u8)((tp & 0xFE) | 1);                  /* LSR TP; SEC; ROL TP: mision 1 en curso */
    bris();
    ship_zinf();
    ztype = SHIPT_CONSTRICTOR;
    n = ship_new(SHIPT_CONSTRICTOR);
    dt_xc = 1;
    inwk[7] = 1;                                 /* z_hi = 1 */
    mis_tt66(13);
    mcnt = 64;
    if (n >= 0) { zinf = state_slot(n); zxx19 = ships_heap(n); }
    zxx0 = ship_bp_for(SHIPT_CONSTRICTOR);
    move_xsav = 0;
    ships_hooks();
    do {                                         /* BRL1: gira 64 vueltas */
        inwk[29] = 0x7F;
        inwk[30] = 0x7F;
        frame();
        mcnt--;
    } while (mcnt && !in_quit);
    for (;;) {                                   /* BRL2: se aleja hacia arriba */
        if (in_quit) break;
        inwk[0] = (u8)(inwk[0] >> 1);
        if (++inwk[6] == 0) break;
        if (++inwk[6] == 0) break;
        x = (u8)(inwk[3] + 1);
        if (x >= CONHIEGHT) x = CONHIEGHT;
        inwk[3] = x;
        frame();
        mcnt--;
    }
    inwk[7]++;                                   /* BR2 */
    pas1();
    return brp(10);
}

/* DOENTRY (la parte de las misiones): que sesion toca (MIS_*). Mision 1 en las galaxias 1 y 2 con rango Competent;
   mision 2 en la galaxia 3 */
int missions_choose(void)
{
    u8 a = (u8)(tp & 3);
    if (a == 0) {
        if (tally[1] == 0) return MIS_NONE;
        if (gcnt >> 1) return MIS_NONE;
        return MIS_BRIEF;
    }
    if (a == 3) return MIS_DEBRIEF;              /* EN1 */
    if (gcnt != 2) return MIS_NONE;              /* EN2 */
    a = (u8)(tp & 0x0F);
    if (a == 0x02) return tally[1] < 5 ? MIS_NONE : MIS_BRIEF2;
    if (a == 0x06) return (qq0 == 215 && qq1 == 84) ? MIS_BRIEF3 : MIS_NONE;      /* EN3: en Ceerdi */
    if (a == 0x0A) return (qq0 == 63 && qq1 == 72) ? MIS_DEBRIEF2 : MIS_NONE;     /* EN5: en Birera con los planos */
    return MIS_NONE;                             /* EN4: BAY */
}

int missions_doentry(void)
{
    missions_hooks();
    space_reset();                               /* RES2 */
    ships_clear();
    zDELTA = 0;
    switch (missions_choose()) {
    case MIS_BRIEF:    return brief();
    case MIS_DEBRIEF:  return debrief();
    case MIS_BRIEF2:   return brief2();
    case MIS_BRIEF3:   return brief3();
    case MIS_DEBRIEF2: return debrief2();
    }
    return SC_STATUS;
}

/* PDESC (la parte de RUPLA/RUGAL/RUTOK): descripcion especial del sistema zz si estamos atracados en el. Devuelve 1 si la
   ha impreso (en dt_xc/dt_yc); 0 si toca la descripcion normal. RUPLA tiene 26 entradas: el original pone NRU% = 0 y
   recorre 256 posiciones de memoria (fallo que lo cuelga en algunos sistemas); aqui se recorren solo las 26 */
int pdesc(u8 zz)
{
    u8 y, g;
    if (qq8[0] | qq8[1]) return 0;
    if (!(qq12 & 0x80)) return 0;
    for (y = dt_nru(); y; y--) {                 /* PDL1 */
        if (dt_rupla(y) != zz) continue;
        g = dt_rugal(y);
        if ((u8)(g & 0x7F) != gcnt) continue;
        if (g & 0x80) detok2(176);               /* PD3 */
        else {
            if (!(tp & 1)) return 0;             /* solo durante la mision 1 */
            dt_mt14();
            detok2(1);                           /* LDA #1 (salta el LDA #176): DETOK2 con la ficha de salto 1 (MT1) */
        }
        detok3(y);
        detok(177);                              /* PD4 */
        return 1;
    }
    return 0;
}

void missions_set_ink(u8 c) { mis_ink = c; }
