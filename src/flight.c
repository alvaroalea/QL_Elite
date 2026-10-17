#include "flight.h"
#include "input.h"
#include "keys.h"
#include "text.h"
#include "menu.h"
#include "game.h"
#include "state.h"
#include "emath.h"
#include "sound.h"

#define NSTARS 24
#define VIEW_H 136
#define SPEED_MAX 40
#define HYPER_FRAMES 8                     /* fotogramas por cuenta atras (provisional: ~0,16 s a 50 fps) */

/* Polvo estelar PROVISIONAL (sustituye a STARS, aun sin portar): cada particula tiene coordenadas respecto a la nave
   (x derecha, y arriba, z hacia delante); la nave avanza (z baja). La vista elige hacia donde se mira:
   delante (x, y, z), atras (-x, y, -z), izquierda (z, y, -x) y derecha (-z, y, x). */
struct star { int x, y, z; int sx, sy, on; };
static struct star stars[NSTARS];
static int view = 0;
static int hyper = 0;                      /* cuenta atras (15..1), 0 = ninguna */
static int hyper_gal = 0;                  /* 1 = salto galactico */
static int hyper_tick = 0;
static u8 last_speed = 255;
static u8 last_fuel = 255;
static const char *vnames[4] = { "FRONT", "REAR ", "LEFT ", "RIGHT" };

static void respawn(struct star *s)
{
    s->x = (int)em_dornd() * 2 - 255;
    s->y = (int)em_dornd() * 2 - 255;
    s->z = (int)em_dornd() * 2 - 255;
    s->on = 0;
}

static void star_erase(struct star *s)
{
    if (s->on) { gfx_xor_pixel(s->sx, s->sy, C_WHITE); s->on = 0; }
}

static void star_draw(struct star *s)
{
    int vx, vz, sx, sy;
    switch (view) {
    case 1:  vx = -s->x; vz = -s->z; break;
    case 2:  vx = s->z;  vz = -s->x; break;
    case 3:  vx = -s->z; vz = s->x;  break;
    default: vx = s->x;  vz = s->z;  break;
    }
    if (vz < 8) return;                                   /* detras del observador */
    sx = 128 + (int)(((long)vx * 64L) / vz);
    sy = 68 + (int)(((long)s->y * 64L) / vz);
    if (sx < 0 || sx > 255 || sy < 0 || sy >= VIEW_H) return;
    s->sx = sx; s->sy = sy; s->on = 1;
    gfx_xor_pixel(sx, sy, C_WHITE);
}

static void stars_reset(void)
{
    int i;
    for (i = 0; i < NSTARS; i++) { stars[i].on = 0; respawn(&stars[i]); }
}

static void panel_static(void)
{
    gfx_rect(0, VIEW_H, 256, 120, C_BLACK);
    gfx_rect(0, VIEW_H, 256, 1, C_RED);
    txt_color(C_CYAN, C_BLACK);
    txt_print_at(0, 18, "SPEED");
    txt_print_at(0, 19, "FUEL");
    txt_print_at(0, 20, "CASH");
    txt_print_at(0, 21, "VIEW");
    txt_print_at(0, 22, "HERE");
    txt_print_at(0, 23, "TARGET");
    txt_color(C_YELLOW, C_BLACK);
    txt_print_at(0, 27, "FLIGHT MODEL NOT PORTED YET");
    txt_color(C_CYAN, C_BLACK);
    txt_print_at(0, 29, "0-3 VIEW H JUMP G GALAXY");
    txt_print_at(0, 30, "SPACE FASTER / SLOWER C DOCK");
    txt_print_at(0, 31, "SHIFT+ESC QUIT");
    txt_color(C_WHITE, C_BLACK);
    last_speed = 255; last_fuel = 255;
}

static void bar(int row, int value, int max, u8 color)
{
    int w = (int)((long)value * 128L / max);
    gfx_rect(56, row * 8 + 1, 128, 6, C_BLACK);
    if (w > 0) gfx_rect(56, row * 8 + 1, w, 6, color);
}

static void panel_update(int force)
{
    if (force || last_speed != zDELTA) {
        bar(18, zDELTA, SPEED_MAX, C_GREEN);
        last_speed = zDELTA;
    }
    if (force || last_fuel != qq14) {
        bar(19, qq14, 70, C_YELLOW);
        last_fuel = qq14;
    }
    txt_color(C_WHITE, C_BLACK);
    txt_clear_rows(20, 1, C_BLACK);
    txt_color(C_CYAN, C_BLACK); txt_print_at(0, 20, "CASH"); txt_color(C_WHITE, C_BLACK);
    txt_at(7, 20); ui_tenths(cash_get()); txt_puts(" Cr");
    txt_print_at(7, 21, vnames[view]);
    txt_print_at(7, 22, uni_galaxy[cur_sys].name); txt_puts("        ");
    txt_at(7, 23); txt_puts(uni_galaxy[tgt_sys].name); txt_puts("  ");
    ui_tenths(jump_distance(tgt_sys)); txt_puts(" LY ");
}

static void say(const char *s, int error)
{
    txt_clear_rows(25, 1, C_BLACK);
    txt_color(error ? C_RED : C_GREEN, C_BLACK);
    txt_print_at(0, 25, s);
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
    panel_update(1);
}

int flight_run(void)
{
    int ch, i, step;
    struct controls c;

    ui_clear();
    gfx_clear(C_BLACK);
    panel_static();
    stars_reset();
    view = 0; hyper = 0;
    if (zDELTA == 0) zDELTA = 3;
    say("LAUNCHED", 0);
    panel_update(1);
    for (;;) {
        ch = in_poll();
        if (ch == KC_QUIT) return FL_QUIT;
        if (ch >= 0) {
            ch = in_up(ch);
            if (ch >= '0' && ch <= '3') { view = ch - '0'; panel_update(0); }
            else if (ch == 'h') start_hyper(0);
            else if (ch == 'g') start_hyper(1);
            else if (ch == 'c') { for (i = 0; i < NSTARS; i++) star_erase(&stars[i]); return FL_DOCK; }
        }
        ctl_read(&c);
        if (c.speed_up && zDELTA < SPEED_MAX) zDELTA++;
        if (c.slow_down && zDELTA > 1) zDELTA--;
        step = 1 + zDELTA / 4;
        for (i = 0; i < NSTARS; i++) {
            struct star *s = &stars[i];
            star_erase(s);
            s->z -= step;                                  /* la nave avanza */
            if (c.roll_left)  { s->x -= s->y >> 5; s->y += s->x >> 5; }
            if (c.roll_right) { s->x += s->y >> 5; s->y -= s->x >> 5; }
            if (c.pitch_up)   { s->y += s->z >> 5; s->z -= s->y >> 5; }
            if (c.pitch_down) { s->y -= s->z >> 5; s->z += s->y >> 5; }
            if (s->z < -255 || s->z > 255 || s->x < -255 || s->x > 255 || s->y < -255 || s->y > 255) {
                respawn(s);
                s->z = 255;                                /* reaparece lejos, por delante */
            }
            star_draw(s);
        }
        if (zDELTA != last_speed) { bar(18, zDELTA, SPEED_MAX, C_GREEN); last_speed = zDELTA; }
        if (hyper && ++hyper_tick >= HYPER_FRAMES) {
            hyper_tick = 0;
            hyper--;
            if (hyper == 0) arrive(); else say_hyper();
        }
        if (qq14 != last_fuel) panel_update(0);
    }
}
