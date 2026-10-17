#include "flight.h"
#include "input.h"
#include "keys.h"
#include "text.h"
#include "menu.h"
#include "game.h"
#include "state.h"
#include "emath.h"
#include "sound.h"
#include "dash.h"
#include "space.h"

#define NSTARS 24
#define VIEW_H 136
#define HYPER_FRAMES 8                     /* fotogramas por cuenta atras (provisional: ~0,16 s a 50 fps) */

/* Polvo estelar PROVISIONAL (sustituye a STARS, aun sin portar): cada particula tiene coordenadas respecto a la nave en
   punto fijo 8.8 (x derecha, y arriba, z hacia delante); la nave avanza (z baja) y gira con las MISMAS ecuaciones que MV40
   usa para el planeta (alabeo ALPHA y cabeceo BETA). La vista elige hacia donde se mira:
   delante (x, y, z), atras (-x, y, -z), izquierda (z, y, -x) y derecha (-z, y, x). */
struct star { long x, y, z; int sx, sy, on; };
static struct star stars[NSTARS];
static int view = 0;
static int hyper = 0;                      /* cuenta atras (15..1), 0 = ninguna */
static int hyper_gal = 0;                  /* 1 = salto galactico */
static int hyper_tick = 0;
static const char *vnames[4] = { "FRONT", "REAR ", "LEFT ", "RIGHT" };

static void respawn(struct star *s)
{
    s->x = ((long)em_dornd() * 2L - 255L) * 256L;
    s->y = ((long)em_dornd() * 2L - 255L) * 256L;
    s->z = ((long)em_dornd() * 2L - 255L) * 256L;
    s->on = 0;
}

static void star_erase(struct star *s)
{
    if (s->on) { gfx_xor_pixel(s->sx, s->sy, C_WHITE); s->on = 0; }
}

static void star_draw(struct star *s)
{
    int vx, vz, sx, sy, x = (int)(s->x >> 8), y = (int)(s->y >> 8), z = (int)(s->z >> 8);
    switch (view) {
    case 1:  vx = -x; vz = -z; break;
    case 2:  vx = z;  vz = -x; break;
    case 3:  vx = -z; vz = x;  break;
    default: vx = x;  vz = z;  break;
    }
    if (vz < 8) return;                                   /* detras del observador */
    sx = 128 + (int)(((long)vx * 64L) / vz);
    sy = 68 + (int)(((long)y * 64L) / vz);
    if (sx < 0 || sx > 255 || sy < 0 || sy >= VIEW_H) return;
    s->sx = sx; s->sy = sy; s->on = 1;
    gfx_xor_pixel(sx, sy, C_WHITE);
}

static void stars_reset(void)
{
    int i;
    for (i = 0; i < NSTARS; i++) { stars[i].on = 0; respawn(&stars[i]); }
}

/* Texto bajo el panel (lineas 192..255 = filas 24..31) */
static void panel_static(void)
{
    gfx_rect(0, DASH_Y0 + DASH_LINES, 256, 256 - (DASH_Y0 + DASH_LINES), C_BLACK);
    txt_color(C_CYAN, C_BLACK);
    txt_print_at(0, 26, "VIEW");
    txt_print_at(0, 27, "HERE");
    txt_print_at(0, 28, "TARGET");
    txt_color(C_CYAN, C_BLACK);
    txt_print_at(0, 29, "CURSORS TURN SPACE FAST / SLOW");
    txt_print_at(0, 30, "0-3 VIEW H JUMP G GALAXY C DOCK");
    txt_print_at(0, 31, "SHIFT+ESC QUIT");
    txt_color(C_WHITE, C_BLACK);
}

static void panel_update(void)
{
    txt_color(C_WHITE, C_BLACK);
    txt_print_at(7, 26, vnames[view]);
    txt_print_at(7, 27, uni_galaxy[cur_sys].name); txt_puts("        ");
    txt_at(7, 28); txt_puts(uni_galaxy[tgt_sys].name); txt_puts("  ");
    ui_tenths(jump_distance(tgt_sys)); txt_puts(" LY  ");
    ui_tenths(cash_get()); txt_puts(" Cr    ");
}

static void say(const char *s, int error)
{
    txt_clear_rows(24, 1, C_BLACK);
    txt_color(error ? C_RED : C_GREEN, C_BLACK);
    txt_print_at(0, 24, s);
    txt_color(C_WHITE, C_BLACK);
}

static void say_hyper(void)
{
    char b[24];
    int i = 0;
    const char *p = hyper_gal ? "GALACTIC JUMP " : "HYPERSPACE ";
    while (*p) b[i++] = *p++;
    if (hyper >= 10) b[i++] = (char)('0' + hyper / 10);
    b[i++] = (char)('0' + hyper % 10);
    b[i] = 0;
    say(b, 0);
}

static void start_hyper(int galactic)
{
    if (hyper) return;
    if (galactic) {
        if (!ghyp) { say("NO GALACTIC HYPERDRIVE", 1); snd_boop(); return; }
    } else {
        if (tgt_sys == cur_sys) { say("NO TARGET SELECTED", 1); snd_boop(); return; }
        if (!jump_possible(tgt_sys)) { say("OUT OF RANGE", 1); snd_boop(); return; }
    }
    hyper = 15; hyper_gal = galactic; hyper_tick = 0;
    say_hyper();
}

static void arrive(void)
{
    char b[32];
    int i = 0;
    const char *p = "ARRIVED AT ";
    int ok = hyper_gal ? jump_galactic() : jump_do(tgt_sys);
    hyper = 0;
    stars_reset();
    if (!ok) { say("JUMP FAILED", 1); return; }
    while (*p) b[i++] = *p++;
    for (p = uni_galaxy[cur_sys].name; *p && i < 31; p++) b[i++] = *p;
    b[i] = 0;
    say(b, 0);
    snd_beep();
    space_launch();                                         /* nuevo sistema: planeta y estacion en su sitio */
    panel_update();
}

/* una particula de polvo: avance y giro (las ecuaciones de MV40 sobre la posicion: roll con ALPHA, pitch con BETA) */
static void star_move(struct star *s, int step, long al, long be)
{
    long x = s->x, y = s->y, z = s->z;
    z -= (long)step * 256L;                                 /* la nave avanza */
    y = y - ((x * al) >> 8);
    z = z + ((y * be) >> 8);
    y = y - ((z * be) >> 8);
    x = x + ((y * al) >> 8);
    s->x = x; s->y = y; s->z = z;
}

int flight_run(void)
{
    int ch, i, step;
    long al, be;
    struct controls c;

    ui_clear();
    gfx_clear(C_BLACK);
    key_ctx_flight();                                       /* menos filas de teclado por fotograma: ver keys.c */
    if (!dash_loaded()) dash_load();
    dash_show();
    panel_static();
    stars_reset();
    view = 0; hyper = 0;
    space_launch();
    say(dash_loaded() ? "LAUNCHED" : "LAUNCHED (NO ELITE_DASH FILE)", 0);
    panel_update();
    for (;;) {
        ch = in_poll();
        if (ch == KC_QUIT) { key_ctx_menu(); return FL_QUIT; }
        if (ch >= 0) {
            ch = in_up(ch);
            if (ch >= '0' && ch <= '3') { view = ch - '0'; panel_update(); }
            else if (ch == 'h') start_hyper(0);
            else if (ch == 'g') start_hyper(1);
            else if (ch == 'c') { for (i = 0; i < NSTARS; i++) star_erase(&stars[i]); key_ctx_menu(); return FL_DOCK; }
        }
        ctl_read(&c);
        space_controls(&c);                                 /* ALPHA, BETA y velocidad */
        space_step();                                       /* planeta y estacion */
        step = 1 + zDELTA / 4;
        al = (long)space_signed(alpha);
        be = (long)space_signed(beta);
        for (i = 0; i < NSTARS; i++) {
            struct star *s = &stars[i];
            star_erase(s);
            star_move(s, step, al, be);
            if (s->z < -255L * 256L || s->z > 255L * 256L || s->x < -255L * 256L || s->x > 255L * 256L ||
                s->y < -255L * 256L || s->y > 255L * 256L) {
                respawn(s);
                s->z = 255L * 256L;                         /* reaparece lejos, por delante */
            }
            star_draw(s);
        }
        dash_update();
        if (hyper && ++hyper_tick >= HYPER_FRAMES) {
            hyper_tick = 0;
            hyper--;
            if (hyper == 0) arrive(); else say_hyper();
        }
        if ((mcnt & 31) == 0) panel_update();               /* el dinero y la distancia cambian poco */
    }
}
