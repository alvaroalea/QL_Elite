/* Own tests of space.c and dash.c (they do not compare with the original: see docs/PENDIENTES.md). */
#include <stdio.h>
#include <string.h>
#include "space.h"
#include "dash.h"
#include "state.h"
#include "emath.h"
#include "game.h"
#include "keys.h"
#include "stars.h"
#include "ships.h"
#include "move.h"
#include "ship3d.h"
#include "chart.h"
#include "sun.h"
#include "damage.h"
#include "shipdata.h"

static int fails, total;
static void check(int ok, const char *what) { total++; if (!ok) { fails++; printf("FALLO: %s\n", what); } }

static struct controls none(void) { struct controls c; memset(&c, 0, sizeof c); return c; }

static unsigned rnd_s = 777;
static unsigned rnd(void) { rnd_s = rnd_s * 25173u + 13849u; return (rnd_s >> 4) & 0xFFFF; }

static void set_pos(u8 *s, int ax, long x, long y, long z)    /* position in units: ax = 0 */
{
    long v[3]; int i;
    v[0] = x; v[1] = y; v[2] = z;
    for (i = 0; i < 3; i++) {
        long m = v[i] < 0 ? -v[i] : v[i];
        s[3 * i] = (u8)(m & 255); s[3 * i + 1] = (u8)((m >> 8) & 255); s[3 * i + 2] = (u8)(((m >> 16) & 0x7F) | (v[i] < 0 ? 0x80 : 0));
    }
    (void)ax;
}

int main(void)
{
    struct controls c;
    int i, k;
    long j;
    u8 snap[32768];
    plat_init();
    state_clear();
    game_new();

    /* cntr: one step towards 128, except with damping off */
    check(space_cntr(128) == 128, "cntr: el centro no se mueve");
    check(space_cntr(140) == 139 && space_cntr(129) == 128, "cntr: baja hacia 128");
    check(space_cntr(100) == 101 && space_cntr(127) == 128, "cntr: sube hacia 128");
    damp = 0xFF; check(space_cntr(140) == 140, "cntr: sin amortiguacion no se mueve"); damp = 0;

    /* BUMP2 / REDU2 */
    djd = 0;
    check(space_bump2(128, 20) == 148 && space_bump2(250, 20) == 255, "bump2 sube y satura en 255");
    check(space_bump2(60, 20) == 128, "bump2 desde la mitad baja se detiene en el centro (DJD = 0)");
    djd = 1; check(space_bump2(60, 20) == 80, "bump2 con DJD != 0 no se detiene en el centro"); djd = 0;
    check(space_redu2(128, 20) == 108 && space_redu2(5, 20) == 1, "redu2 baja y se detiene en 1");
    check(space_redu2(200, 20) == 128, "redu2 desde la mitad alta se detiene en el centro (DJD = 0)");
    djd = 1; check(space_redu2(200, 20) == 180, "redu2 con DJD != 0"); djd = 0;

    /* controls: roll and pitch values after one frame (transcription of the main loop) */
    space_reset(); jstx = 128;
    c = none(); c.roll_left = 1; space_controls(&c);
    check(jstx == 146 && alp2[0] == 0 && alp1 == 2 && alpha == 2, "alabeo a la izquierda: JSTX 146, ALPHA 2");
    space_reset(); jstx = 128;
    c = none(); c.roll_right = 1; space_controls(&c);
    check(jstx == 110 && alp2[0] == 0x80 && alp1 == 2 && alpha == 0x82, "alabeo a la derecha: JSTX 110, ALPHA con signo");
    space_reset(); jstx = 128;
    for (i = 0; i < 40; i++) { c = none(); c.roll_left = 1; space_controls(&c); }
    check(jstx > 128 && alp1 > 2, "alabeo sostenido: crece");
    c = none();
    for (i = 0; i < 400; i++) space_controls(&c);
    check(jstx == 128 && alp1 == 0 && (alpha & 0x7F) == 0, "al soltar la tecla el giro vuelve al centro");
    space_reset(); jstx = 128;
    c = none(); c.speed_up = 1;
    for (i = 0; i < 100; i++) space_controls(&c);
    check(zDELTA == 40, "la velocidad llega a 40 y no pasa");
    c = none(); c.slow_down = 1;
    for (i = 0; i < 100; i++) space_controls(&c);
    check(zDELTA == 1, "la velocidad baja hasta 1 y no llega a 0");

    /* planet movement: z drops DELTA per frame (MV45/MVT1) */
    space_reset();
    memset(state_slot(0), 0, NI); frin[0] = 128;
    set_pos(state_slot(0), 0, 0, 0, 0x1000L);
    alpha = 0; beta = 0; zDELTA = 10;
    for (i = 0; i < 5; i++) space_step();
    check(state_slot(0)[6] == 0xCE && state_slot(0)[7] == 0x0F && state_slot(0)[8] == 0, "planeta: z = 0x1000 - 5 * 10");
    memset(state_slot(0), 0, NI);
    set_pos(state_slot(0), 0, 0, 0, 5L);
    space_step();
    check(state_slot(0)[6] == 5 && state_slot(0)[7] == 0 && (state_slot(0)[8] & 0x80), "planeta: cruzar z = 0 deja z = -5");

    /* rotation by the roll: y' = y - x * ALPHA / 256 (positive ALPHA = bit 7 clear) */
    memset(state_slot(0), 0, NI);
    set_pos(state_slot(0), 0, 0x2000L, 0, 0x4000L);
    alpha = 20; beta = 0; zDELTA = 0;
    space_step();
    {
        long y = (long)state_slot(0)[3] | ((long)state_slot(0)[4] << 8);
        check((state_slot(0)[5] & 0x80) && y >= 638 && y <= 642, "alabeo: y baja x * 20 / 256 = 640");
    }
    /* altitude: every 32 frames (MCNT mod 32 = 10, the value M% sees; MCNT drops afterwards, in TT100), only if the planet is near */
    memset(state_slot(0), 0, NI);
    set_pos(state_slot(0), 0, 0, 0, 0x7800L);
    alpha = 0; beta = 0; zDELTA = 0; mcnt = 10; altit = 255;
    space_step();
    check(altit >= 69 && altit <= 71, "altitud: z_hi = 120 -> (hi(120^2) - 37) = 19 -> sqrt(19 * 256 + Q) = 69..71 (Q es el que hubiera)");
    memset(state_slot(0), 0, NI);
    set_pos(state_slot(0), 0, 0, 0, 0x5000L);
    mcnt = 10;
    check((space_step() & SPACE_EV_DEATH) && player_dead && altit == 255, "altitud: dentro del planeta, muerte (DEATH; ALTIT queda en 255)");
    player_dead = 0;
    memset(state_slot(0), 0, NI);
    set_pos(state_slot(0), 0, 0, 0, 0x30000L);
    mcnt = 10; space_step();
    check(altit == 255, "altitud: lejos = 255");

    /* launch (TT110): planet ahead (z_sign = 1) and station just behind (z = -256); speed 12 */
    space_launch();
    check(sspr == 1 && frin[1] == SHIPT_CORIOLIS && state_slot(1)[8] == 0x80 && state_slot(1)[7] == 1 && state_slot(1)[6] == 0 &&
          state_slot(0)[8] == 1 && state_slot(0)[7] == 0 && zDELTA == 12, "despegue: estacion detras (z = -256), planeta delante (z = 65536), DELTA = 12");
    /* backwards, far from the planet: the station is removed on passing 224 * 256 (FAROF) and the sun appears in its slot (KS4) */
    zDELTA = 40; alpha = 0; beta = 0; state_slot(0)[8] = 0x81;            /* planet behind: part 14 does not put it back */
    for (i = 0; i < 2000 && sspr; i++) { space_step(); mcnt--; }
    check(sspr == 0 && frin[1] == 129 && state_slot(1)[5] == 6, "al alejarse la estacion se quita y aparece el sol (y_sign = 6)");

    /* panel: drawing in XOR leaves the screen the same as drawing from scratch with the last state */
    for (k = 0; k < 2; k++) {
        if (k == 1) { game_new(); }
        gfx_clear(C_BLACK);
        dash_show();
        space_launch();
        for (i = 0; i < 150; i++) {
            zDELTA = (u8)(1 + rnd() % 40); energy = (u8)rnd(); fsh = (u8)rnd(); ash = (u8)rnd(); qq14 = (u8)(rnd() % 71);
            altit = (u8)rnd(); cabtmp = (u8)rnd(); gntmp = (u8)rnd();
            alp1 = (u8)(rnd() % 32); bet1 = (u8)(rnd() % 9); alp2[1] = (u8)((rnd() & 1) << 7); bet2[0] = (u8)((rnd() & 1) << 7);
            mcnt = (u8)rnd(); flh = (u8)((rnd() & 1) ? 0xFF : 0);
            set_pos(state_slot(0), 0, (long)(rnd() % 4000) - 2000, (long)(rnd() % 4000) - 2000, (long)(rnd() % 6000) - 3000);
            if (rnd() & 1) { sspr = 1; set_pos(state_slot(1), 0, (long)(rnd() % 400) - 200, (long)(rnd() % 400) - 200, (long)(rnd() % 400) - 200); }
            else sspr = 0;
            dash_update();
        }
        memcpy(snap, screen, 32768);
        gfx_clear(C_BLACK);
        dash_show();                                   /* repaints the background and resets the memories */
        dash_update();                                 /* with the last state */
        {
            int bad = 0;
            for (j = 0; j < 32768L; j++) if (snap[j] != screen[j]) { bad = 1; break; }
            check(!bad, k == 0 ? "panel: animar = dibujar desde cero (sin fichero de fondo: panel minimo)" : "panel: animar = dibujar desde cero (2)");
        }
    }

    /* radar (SCAN): position of the dot and the stick, colour, and that what was drawn is fully erased */
    game_new(); gfx_clear(C_BLACK); dash_show(); space_launch();
    {
        u8 base[32768];
        int s2, k2;
        space_step();                                      /* draws the station */
        space_scanner_clear();
        memcpy(base, screen, 32768);
        gfx_clear(C_BLACK); dash_show();
        memcpy(base, screen, 32768);
        for (i = 0; i < 4; i++) { u8 *p; s2 = space_spawn(SHIPT_VIPER); check(s2 == 2 + i, "contacto en la siguiente ranura libre"); (void)p; }
        check(space_spawn(SHIPT_VIPER) >= 0 || 1, "(ranuras)");
        zDELTA = 10; alpha = 0; beta = 0;
        for (k2 = 0; k2 < 120; k2++) { alpha = (u8)(k2 % 7); beta = (u8)(k2 % 3); space_step(); }
        space_scanner_clear();
        {
            int bad = 0;
            for (j = 136L * 128L; j < 192L * 128L; j++) if (base[j] != screen[j]) { bad = 1; break; }      /* only the panel (the radar) */
            check(!bad, "radar: tras mover los contactos y borrar el radar el panel queda como estaba");
        }
    }
    /* exact positions: ahead and behind */
    game_new(); gfx_clear(C_BLACK); space_launch();
    for (i = 0; i <= NOSH; i++) frin[i] = 0;
    sspr = 0; many[SHIPT_VIPER] = 0;
    memset(state_slot(2), 0, NI);
    set_pos(state_slot(2), 0, 10L * 256, 0, 0);            /* x = +10 (high units), y = 0, z = 0 */
    frin[2] = SHIPT_VIPER; state_slot(2)[I_STATE] = 0x10;  /* already drawn: space_scan draws it now */
    for (i = 0; i < NI; i++) inwk[i] = state_slot(2)[i];
    space_scan(SHIPT_VIPER);
    check(gfx_get(132, 164) == C_CYAN && gfx_get(133, 164) == C_CYAN && gfx_get(134, 164) == C_CYAN && gfx_get(135, 164) == C_BLACK, "radar: cabeza de 2 puntos (132, 133) y palo de 1 punto (134) en y = 164 para x = +10, y = z = 0");
    check(gfx_get(131, 164) == C_BLACK && gfx_get(136, 164) == C_BLACK && gfx_get(134, 163) == C_BLACK && gfx_get(135, 165) == C_BLACK, "radar: el punto no se sale de ahi (palo de 1 punto)");
    gfx_clear(C_BLACK);
    set_pos(inwk, 0, 0, 0, -(0x20L * 256));                /* behind: z = -32 (high units) */
    inwk[I_STATE] = 0x10;
    space_scan(SHIPT_VIPER);
    check(gfx_get(122, 172) == C_CYAN && gfx_get(123, 172) == C_CYAN && gfx_get(124, 172) == C_CYAN && gfx_get(125, 172) == C_BLACK, "radar: detras (z negativo) el punto baja 8 lineas (y = 172)");
    gfx_clear(C_BLACK);
    inwk[I_STATE] = 0; space_scan(SHIPT_VIPER);
    check(gfx_get(124, 172) == C_BLACK, "radar: sin el bit 4 de INWK+31 no se dibuja");
    inwk[I_STATE] = 0x10; space_scan(128);
    check(gfx_get(124, 172) == C_BLACK, "radar: los planetas (tipo con el bit 7) no salen");

    /* stardust: drawing in XOR leaves the screen the same as redrawing all the particles, in the 4 views, and the signs are restored */
    for (k = 0; k < 4; k++) {
        u8 a0, a1, b0, b1;
        game_new(); space_reset();
        gfx_clear(C_BLACK);
        zDELTA = 12;
        stars_set_speed(zDELTA);
        stars_new();
        for (i = 0; i < 150; i++) {
            c = none();
            c.roll_left = (u8)((rnd() & 3) == 0); c.roll_right = (u8)((rnd() & 3) == 1);
            c.pitch_up = (u8)((rnd() & 3) == 0); c.pitch_down = (u8)((rnd() & 3) == 2);
            space_controls(&c);
            a0 = alpha; a1 = alp2[0]; b0 = bet2[0]; b1 = bet2[1];
            stars_set_speed(zDELTA);
            stars_update((u8)k);
            check(alpha == a0 && alp2[0] == a1 && bet2[0] == b0 && bet2[1] == b1, "polvo: STARS2 restaura ALPHA, ALP2 y BET2");
        }
        memcpy(snap, screen, 32768);
        gfx_clear(C_BLACK);
        stars_draw_all();
        {
            int bad = 0;
            for (j = 0; j < 32768L; j++) if (snap[j] != screen[j]) { bad = 1; break; }
            check(!bad, "polvo: animar = redibujar todas las particulas en su sitio");
        }
    }
    /* FLIP: twice is the identity */
    {
        u8 sx0[NOST + 1], sy0[NOST + 1];
        game_new(); gfx_clear(C_BLACK); stars_new();
        memcpy(sx0, sx, sizeof sx0); memcpy(sy0, sy, sizeof sy0);
        gfx_clear(C_BLACK); stars_flip(); stars_flip();
        check(memcmp(sx0, sx, sizeof sx0) == 0 && memcmp(sy0, sy, sizeof sy0) == 0, "FLIP dos veces devuelve las particulas a su sitio");
    }

    /* hyperspace tunnel: drawing twice with the same step erases everything (XOR) and does not leave the view */
    {
        int lit = 0;
        gfx_clear(C_BLACK); sun_col = C_WHITE;
        tunnel_draw(4);
        for (j = 0; j < 136L * 128L; j++) if (screen[j]) lit++;
        check(lit > 100, "tunel: se dibujan anillos");
        for (j = 136L * 128L; j < 32768L; j++) if (screen[j]) { lit = -1; break; }
        check(lit > 0, "tunel: no se sale del area de vista");
        tunnel_draw(4);
        lit = 0;
        for (j = 0; j < 32768L; j++) if (screen[j]) lit++;
        check(lit == 0, "tunel: dibujarlo otra vez lo borra (XOR)");
        gfx_clear(C_BLACK);
        tunnel_draw(8);
        lit = 0;
        for (j = 0; j < 32768L; j++) if (screen[j]) lit++;
        check(lit > 100, "tunel de lanzamiento (paso 8)");
    }

    /* laser sight: only if the view has a laser; colour according to the type */
    game_new();
    gfx_clear(C_BLACK);
    chart_sight(1);
    {
        int lit = 0;
        for (j = 0; j < 32768L; j++) if (screen[j]) lit++;
        check(lit == 0, "mira: sin laser en la vista trasera no se dibuja nada");
    }
    chart_sight(0);
    {
        int lit = 0, blue = 0;
        for (j = 0; j < 32768L; j++) if (screen[j]) lit++;
        check(lit > 0, "mira: el laser de pulsos de la vista frontal dibuja la mira");
        blue = (gfx_get(112, 63) == C_BLUE) && (gfx_get(128, 63) == C_BLACK);
        check(blue, "mira: dos cruces concentricas en (128, 63): el brazo largo azul y el centro vacio (XOR)");
        gfx_clear(C_BLACK); chart_sight(0); chart_sight(0);
        lit = 0;
        for (j = 0; j < 32768L; j++) if (screen[j]) lit++;
        check(lit == 0, "mira: dibujarla dos veces la borra (XOR)");
    }

    /* missile indicators and bulbs */
    game_new(); gfx_clear(C_BLACK); dash_show();
    nomsl = 3; dash_msblob();
    check(gfx_get(40, 185) == C_GREEN && gfx_get(45, 188) == C_GREEN && gfx_get(24, 186) == C_GREEN && gfx_get(16, 185) == C_BLACK, "misiles: 3 indicadores verdes (x 40, 32, 24) y el cuarto apagado");
    nomsl = 2; dash_msblob();
    check(gfx_get(32, 185) == C_GREEN && gfx_get(24, 185) == C_BLACK, "misiles: con 2, el tercero se apaga");
    {
        u8 b2[32768];
        memcpy(b2, screen, 32768);
        dash_spblb(); dash_ecblb(); dash_spblb(); dash_ecblb();
        check(memcmp(b2, screen, 32768) == 0, "bombillas S y E: encender y apagar (XOR) deja el panel igual");
    }

    /* fast fill and copy: equal to setting pixel by pixel, with clipping and for the 8 colours */
    {
        int col, y0, nn, x, y, bad = 0;
        static u8 ref[32768];
        for (col = 0; col < 8; col++) {
            gfx_clear(C_BLACK);
            for (y = 0; y < 256; y++) for (x = 0; x < 256; x++) gfx_pixel(x, y, (u8)col);
            memcpy(ref, screen, 32768);
            gfx_clear((u8)(col ^ 7)); gfx_clear((u8)col);
            if (memcmp(ref, screen, 32768) != 0) bad = 1;
        }
        check(!bad, "gfx_clear: igual que poner los 256 x 256 pixeles uno a uno (8 colores)");
        bad = 0;
        for (i = 0; i < 40; i++) {
            col = (int)(rnd() & 7); y0 = (int)(rnd() % 300) - 20; nn = (int)(rnd() % 120);
            gfx_clear(C_BLACK); gfx_clear(C_YELLOW); memcpy(ref, screen, 32768);
            for (y = y0; y < y0 + nn; y++) if (y >= 0 && y < 256) for (x = 0; x < 256; x++) gfx_pixel(x, y, (u8)col);
            memcpy(snap, screen, 32768);
            memcpy(screen, ref, 32768);
            gfx_fill_lines(y0, nn, (u8)col);
            if (memcmp(snap, screen, 32768) != 0) bad = 1;
        }
        check(!bad, "gfx_fill_lines: igual que pixel a pixel, con recorte por arriba y por abajo");
        gfx_clear(C_BLACK);
        gfx_rect(0, 10, 256, 7, C_RED);
        for (y = 10; y < 17; y++) for (x = 0; x < 256; x++) if (gfx_get(x, y) != C_RED) bad = 1;
        check(!bad && gfx_get(0, 9) == C_BLACK && gfx_get(255, 17) == C_BLACK, "gfx_rect de ancho completo usa el relleno rapido y no se sale");
        {
            static u8 src[7200], dst[7200];
            int off;
            for (i = 0; i < 7200; i++) src[i] = (u8)(i * 7 + 3);
            bad = 0;
            for (off = 0; off < 4; off++) {
                memset(dst, 0, sizeof dst);
                gfx_copy(dst + off, src + (3 - off), 7168L);
                if (memcmp(dst + off, src + (3 - off), 7168) != 0 || dst[off - 1 + (off == 0)] == 99) bad = 1;
            }
            check(!bad, "gfx_copy: copia exacta con direcciones pares e impares");
        }
    }

    /* ---- ships: MVEIT, PLUT, explosion (DOEXP), planet and sun (PLANET), add and remove (NWSHP, KILLSHP) ---- */
    game_new(); gfx_clear(C_BLACK); ships_clear(); ships_hooks();
    /* MVEIT: advance along the nose (speed 10, nose towards us): z drops speed * 4 * 96 / 256 = 15 */
    ship_zinf();
    set_pos(inwk, 0, 0, 0, 0x2000L);
    inwk[27] = 10; inwk[32] = 0;
    ztype = SHIPT_VIPER; zxx0 = SHIP_BP(SHIPT_VIPER); move_xsav = 0;
    alpha = 0; beta = 0; zDELTA = 0; alp1 = 0; bet1 = 0; alp2[0] = alp2[1] = 0; bet2[0] = bet2[1] = 0;
    mveit();
    zQ = 40;
    {
        u8 dec = em_fmltu(96);                             /* FMLTU: 96 * 40 / 256 (approximate with logarithms: 14) */
        check(inwk[6] == (u8)(0x2000 - dec) && inwk[7] == 0x1F && inwk[27] == 10 && dec >= 14 && dec <= 15, "MVEIT: z baja FMLTU(96) con velocidad 10 y el morro hacia nosotros");
    }
    /* speed is limited to the blueprint's maximum and acceleration is consumed */
    ship_zinf(); inwk[27] = 200; inwk[28] = 100; set_pos(inwk, 0, 0, 0, 0x2000L);
    mveit();
    check(inwk[27] == SHIP_BP(SHIPT_VIPER)[15] && inwk[28] == 0, "MVEIT: velocidad limitada por el plano y aceleracion a 0");
    /* roll counter: drops one by one and rotates the vectors */
    ship_zinf(); set_pos(inwk, 0, 0, 0, 0x2000L); inwk[29] = 40; inwk[27] = 0;
    mveit();
    check((inwk[29] & 0x7F) == 39 && (inwk[15] | inwk[16]) != 0, "MV45: el contador de alabeo baja a 39 y el techo gira");
    ship_zinf(); set_pos(inwk, 0, 0, 0, 0x2000L); inwk[29] = 0x7F; inwk[27] = 0;
    mveit();
    check((inwk[29] & 0x7F) == 0x7F, "MV45: un contador a 127 no baja (giro continuo)");
    /* PLUT: rear, left and right views */
    ship_zinf(); set_pos(inwk, 0, 100L, 200L, 300L);
    plut(0);
    check(inwk[0] == 100 && inwk[3] == 200 && inwk[6] == 44 && inwk[7] == 1, "PLUT 0: sin cambios");
    plut(1);
    check((inwk[2] & 0x80) && (inwk[8] & 0x80) && !(inwk[5] & 0x80) && inwk[0] == 100, "PLUT 1: atras invierte x y z");
    ship_zinf(); set_pos(inwk, 0, 100L, 200L, 300L);
    plut(2);
    check(inwk[0] == 44 && inwk[1] == 1 && inwk[6] == 100 && inwk[7] == 0 && inwk[3] == 200, "PLUT 2: izquierda intercambia x y z");

    /* additions and removals */
    ships_clear();
    ship_zinf(); set_pos(inwk, 0, 0, 0, 0x3000L);
    check(ship_new(SHIPT_VIPER) == 0 && ship_new(SHIPT_PYTHON) == 1 && ship_new(SHIPT_COBRA_MK_3) == 2, "NWSHP: ranuras 0, 1 y 2");
    check(many[SHIPT_VIPER] == 1 && frin[1] == SHIPT_PYTHON && state_slot(0)[35] == SHIP_BP(SHIPT_VIPER)[14], "NWSHP: contadores y energia del plano");
    state_slot(2)[3] = 77;
    ship_kill(1);
    check(frin[0] == SHIPT_VIPER && frin[1] == SHIPT_COBRA_MK_3 && frin[2] == 0 && state_slot(1)[3] == 77 && many[SHIPT_PYTHON] == 0, "KILLSHP: las demas bajan una ranura");
    for (i = 0; i < 20; i++) { ship_zinf(); if (ship_new(SHIPT_ESCAPE_POD) < 0) break; }
    check(i == 6 && frin[NOSH - 1] == SHIPT_ESCAPE_POD, "NWSHP: caben NOSH ranuras y no mas");
    ships_clear();
    for (i = 0; i < 20; i++) { ship_zinf(); if (ship_new(SHIPT_COBRA_MK_3) < 0) break; }
    check(i == 4, "NWSHP: el monton de lineas del original (SLSP) solo deja 4 Cobras");
    ships_clear();

    /* planet and sun: drawing twice is the same as once, and removing them leaves the view empty */
    {
        static u8 one[32768];
        int lit = 0;
        game_new(); gfx_clear(C_BLACK); sun_forget();
        ship_zinf(); set_pos(inwk, 0, 0, 0, 0x20000L);       /* z = 2 * 65536 */
        ztype = 128; planet();
        memcpy(one, screen, 32768);
        for (j = 0; j < 136L * 128L; j++) if (one[j]) lit++;
        check(lit > 20, "PLANET: el planeta de frente se dibuja (circulo)");
        planet();
        check(memcmp(one, screen, 32768) == 0, "PLANET: volver a dibujar el planeta no cambia nada (borra y dibuja)");
        set_pos(inwk, 0, 0, 0, 0x300000L);                    /* very far: removed */
        planet();
        lit = 0;
        for (j = 0; j < 32768L; j++) if (screen[j]) lit++;
        check(lit == 0, "PLANET: demasiado lejos se borra");
        gfx_clear(C_BLACK); sun_forget();
        set_pos(inwk, 0, 0, 0, 0x20000L);
        ztype = 129; planet();
        lit = 0;
        for (j = 0; j < 136L * 128L; j++) if (screen[j]) lit++;
        check(lit > 50, "PLANET: el sol se dibuja (SUN)");
        set_pos(inwk, 0, 0, 0, 0x300000L);
        planet();
        lit = 0;
        for (j = 0; j < 32768L; j++) if (screen[j]) lit++;
        check(lit == 0, "PLANET: el sol demasiado lejos se borra (WPLS)");
    }

    /* explosion: the ship explodes, the cloud grows and at the end it is removed; the view ends up the same as in a run without the ship */
    {
        static u8 withship[32768];
        int frames = 0, k3, n, bad;
        for (k3 = 0; k3 < 4; k3++) {
            game_new(); em_rand[0] = 5; em_rand[1] = 6; em_rand[2] = 7; em_rand[3] = 8;
            gfx_clear(C_BLACK); sun_forget(); space_view = (u8)k3; space_launch();
            zDELTA = 0; alpha = 0; beta = 0;
            for (i = 0; i < 6; i++) space_step();
            n = ships_spawn(SHIPT_COBRA_MK_3);
            check(n >= 0, "explosion: nave de prueba");
            state_slot(n)[7] = 14; state_slot(n)[1] = 0; state_slot(n)[4] = 0; state_slot(n)[2] = 0; state_slot(n)[5] = 0;
            state_slot(n)[8] = 0; state_slot(n)[27] = 0; state_slot(n)[29] = 0; state_slot(n)[30] = 0;
            zDELTA = 0; alpha = 0; beta = 0;
            for (i = 0; i < 4; i++) space_step();
            check(ships_explode_nearest() == n, "explosion: se elige la nave de prueba");
            for (frames = 0; frames < 300 && frin[n] == SHIPT_COBRA_MK_3; frames++) space_step();
            check(frin[n] != SHIPT_COBRA_MK_3 && frames > 10 && frames < 300, "explosion: la nube dura un rato y la nave se quita");
            check(many[SHIPT_COBRA_MK_3] == 0, "explosion: el contador de naves baja");
            memcpy(withship, screen, 32768);
            game_new(); em_rand[0] = 5; em_rand[1] = 6; em_rand[2] = 7; em_rand[3] = 8;
            gfx_clear(C_BLACK); sun_forget(); space_view = (u8)k3; space_launch();
            zDELTA = 0; alpha = 0; beta = 0;
            for (i = 0; i < 6; i++) space_step();
            for (i = 0; i < 4 + frames; i++) space_step();
            bad = 0;
            for (j = 0; j < 136L * 128L; j++) if (withship[j] != screen[j]) { bad = 1; break; }
            check(!bad, "explosion: sin restos de la nave ni de la nube en la vista");
        }
    }

    /* the station is lost from sight as it moves away and the sun returns; near the planet, part 14 puts it back */
    game_new(); gfx_clear(C_BLACK); space_view = 0; space_launch();
    zDELTA = 40; alpha = 0; beta = 0; state_slot(0)[8] = 0x81;
    for (i = 0; i < 2000; i++) { u8 ev = space_step(); mcnt--; if (ev & SPACE_EV_STATION) break; }
    {
        int sun = 0, stn = 0;
        for (i = 0; i < NOSH; i++) { if (frin[i] == 129) sun++; if (frin[i] == SHIPT_CORIOLIS) stn++; }
        check(sspr == 0 && sun == 1 && stn == 0, "estacion: al alejarse se quita y reaparece el sol");
    }
    /* part 14: with the planet ahead and near, every 32 iterations the station appears at planet + 2 * nose and the sun is erased */
    {
        int k2;
        memset(state_slot(0), 0, NI); set_pos(state_slot(0), 0, 0, 0, 0x8000L);
        state_slot(0)[14] = 0x60 | 0x80; state_slot(0)[18] = 0x60; state_slot(0)[22] = 0x60;   /* nose towards us */
        zDELTA = 0; mcnt = 1;
        for (k2 = 0; k2 < 3 && !sspr; k2++) { space_step(); mcnt--; }
        if (!(sspr == 1 && frin[1] == SHIPT_CORIOLIS && state_slot(1)[7] == 0x40 && state_slot(1)[8] == 0x80))
            printf("  parte 14: sspr %d frin1 %d z %02x %02x %02x\n", sspr, frin[1], state_slot(1)[6], state_slot(1)[7], state_slot(1)[8]);
        check(sspr == 1 && frin[1] == SHIPT_CORIOLIS && state_slot(1)[7] == 0x40 && state_slot(1)[8] == 0x80,
              "parte 14: aparece la estacion en planeta + 2 * morro (z = 0x8000 - 0xC000 = -0x4000)");
    }
    printf("%d comprobaciones, %d fallos\n", total, fails);
    return fails ? 1 : 0;
}
