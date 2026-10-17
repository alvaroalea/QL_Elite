/* Pruebas propias de space.c y dash.c (no comparan con el original: ver PENDIENTES.md). */
#include <stdio.h>
#include <string.h>
#include "space.h"
#include "dash.h"
#include "state.h"
#include "emath.h"
#include "game.h"
#include "keys.h"

static int fails, total;
static void check(int ok, const char *what) { total++; if (!ok) { fails++; printf("FALLO: %s\n", what); } }

static struct controls none(void) { struct controls c; memset(&c, 0, sizeof c); return c; }

static unsigned rnd_s = 777;
static unsigned rnd(void) { rnd_s = rnd_s * 25173u + 13849u; return (rnd_s >> 4) & 0xFFFF; }

static void set_pos(u8 *s, int ax, long x, long y, long z)    /* posicion en unidades: ax = 0 */
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

    /* cntr: un paso hacia 128, salvo con la amortiguacion desactivada */
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

    /* controles: valores de alabeo y cabeceo tras un fotograma (transcripcion del bucle principal) */
    space_reset();
    c = none(); c.roll_left = 1; space_controls(&c);
    check(jstx == 146 && alp2[0] == 0 && alp1 == 2 && alpha == 2, "alabeo a la izquierda: JSTX 146, ALPHA 2");
    space_reset();
    c = none(); c.roll_right = 1; space_controls(&c);
    check(jstx == 110 && alp2[0] == 0x80 && alp1 == 2 && alpha == 0x82, "alabeo a la derecha: JSTX 110, ALPHA con signo");
    space_reset();
    for (i = 0; i < 40; i++) { c = none(); c.roll_left = 1; space_controls(&c); }
    check(jstx > 128 && alp1 > 2, "alabeo sostenido: crece");
    c = none();
    for (i = 0; i < 400; i++) space_controls(&c);
    check(jstx == 128 && alp1 == 0 && (alpha & 0x7F) == 0, "al soltar la tecla el giro vuelve al centro");
    space_reset();
    c = none(); c.speed_up = 1;
    for (i = 0; i < 100; i++) space_controls(&c);
    check(zDELTA == 40, "la velocidad llega a 40 y no pasa");
    c = none(); c.slow_down = 1;
    for (i = 0; i < 100; i++) space_controls(&c);
    check(zDELTA == 1, "la velocidad baja hasta 1 y no llega a 0");

    /* movimiento del planeta: z baja DELTA por fotograma (MV45/MVT1) */
    space_reset();
    memset(state_slot(0), 0, NI);
    set_pos(state_slot(0), 0, 0, 0, 0x1000L);
    alpha = 0; beta = 0; zDELTA = 10;
    for (i = 0; i < 5; i++) space_step();
    check(state_slot(0)[6] == 0xCE && state_slot(0)[7] == 0x0F && state_slot(0)[8] == 0, "planeta: z = 0x1000 - 5 * 10");
    memset(state_slot(0), 0, NI);
    set_pos(state_slot(0), 0, 0, 0, 5L);
    space_step();
    check(state_slot(0)[6] == 5 && state_slot(0)[7] == 0 && (state_slot(0)[8] & 0x80), "planeta: cruzar z = 0 deja z = -5");

    /* rotacion por el alabeo: y' = y - x * ALPHA / 256 (ALPHA positivo = bit 7 limpio) */
    memset(state_slot(0), 0, NI);
    set_pos(state_slot(0), 0, 0x2000L, 0, 0x4000L);
    alpha = 20; beta = 0; zDELTA = 0;
    space_step();
    {
        long y = (long)state_slot(0)[3] | ((long)state_slot(0)[4] << 8);
        check((state_slot(0)[5] & 0x80) && y >= 638 && y <= 642, "alabeo: y baja x * 20 / 256 = 640");
    }

    /* altitud: cada 32 fotogramas, solo si el planeta esta cerca */
    memset(state_slot(0), 0, NI);
    set_pos(state_slot(0), 0, 0, 0, 0x7800L);
    alpha = 0; beta = 0; zDELTA = 0; mcnt = 9; altit = 255;
    space_step();
    check(altit >= 68 && altit <= 70, "altitud: z_hi = 120 -> (hi(120^2) - 37) = 19 -> sqrt(19 * 256) = 69");
    memset(state_slot(0), 0, NI);
    set_pos(state_slot(0), 0, 0, 0, 0x5000L);
    mcnt = 9; space_step();
    check(altit == 0, "altitud: dentro del planeta = 0 (el original muere)");
    memset(state_slot(0), 0, NI);
    set_pos(state_slot(0), 0, 0, 0, 0x30000L);
    mcnt = 9; space_step();
    check(altit == 255, "altitud: lejos = 255");

    /* despegue: planeta delante y estacion detras; la estacion se pierde al alejarse */
    space_launch();
    check(sspr == 1 && (state_slot(1)[8] & 0x80) && state_slot(0)[8] >= 3 && state_slot(0)[8] <= 6, "despegue: estacion detras, planeta delante");
    zDELTA = 40; alpha = 0; beta = 0;
    for (i = 0; i < 400; i++) space_step();
    check(sspr == 0, "al alejarse la estacion deja de ser el objetivo de la brujula");

    /* panel: dibujar en XOR deja la pantalla igual que dibujar desde cero con el ultimo estado */
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
        dash_show();                                   /* repinta el fondo y reinicia las memorias */
        dash_update();                                 /* con el ultimo estado */
        {
            int bad = 0;
            for (j = 0; j < 32768L; j++) if (snap[j] != screen[j]) { bad = 1; break; }
            check(!bad, k == 0 ? "panel: animar = dibujar desde cero (sin fichero de fondo: panel minimo)" : "panel: animar = dibujar desde cero (2)");
        }
    }
    printf("%d comprobaciones, %d fallos\n", total, fails);
    return fails ? 1 : 0;
}
