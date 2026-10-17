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
#include "stars.h"
#include "sun.h"
#include "chart.h"
#include "shipdata.h"
#include "ships.h"

#define VIEW_H 136
#define HYPER_FRAMES 8                     /* fotogramas por cuenta atras (provisional: ~0,16 s a 50 fps) */
#define TUNNEL_FRAMES 30                   /* cuanto se deja ver el tunel de lanzamiento o de hiperespacio */

static int view = 0;
static int hyper = 0;                      /* cuenta atras (15..1), 0 = ninguna */
static int hyper_gal = 0;                  /* 1 = salto galactico */
static int hyper_tick = 0;
static const char *vnames[4] = { "FRONT", "REAR ", "LEFT ", "RIGHT" };

static void hiss(int y) { snd_sohiss(y); }

static void wait_frames(int n)             /* en el QL deja pasar tiempo real leyendo el teclado; con guion no consume teclas */
{
    int i;
    if (in_script) return;
    for (i = 0; i < n; i++) in_poll();
}

/* Texto bajo el panel (lineas 192..255 = filas 24..31) */
static void panel_static(void)
{
    gfx_rect(0, DASH_Y0 + DASH_LINES, 256, 256 - (DASH_Y0 + DASH_LINES), C_BLACK);
    txt_color(C_CYAN, C_BLACK);
    txt_print_at(0, 26, "VIEW");
    txt_print_at(0, 27, "HERE");
    txt_print_at(0, 28, "TARGET");
    txt_print_at(0, 29, "CURSORS TURN F FAST D SLOW");
    txt_print_at(0, 30, "1-4 VIEW 5-9 INFO H/G JUMP");
    txt_print_at(0, 31, "C DOCK N SHIP K BLOW UP (TEST)");
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

/* TT66 de la vista espacial: borra el area de vista (el panel no se toca) */
static void clear_view(void)
{
    gfx_fill_lines(0, VIEW_H, C_BLACK);     /* rapido: palabras largas, no pixel a pixel */
    ships_forget_lines();                   /* las lineas de las naves, el sol y el planeta ya no estan en pantalla */
    sun_forget();
}

/* LOOK1 al cambiar de vista estando en el espacio: el polvo se refleja en la diagonal (FLIP) y se vuelve a poner la mira */
static void change_view(int v)
{
    if (v == view) return;
    view = v;
    space_view = (u8)view;                  /* las naves se redibujan en los ejes de la vista nueva (PLUT) */
    clear_view();
    stars_flip();
    chart_sight((u8)view);
    panel_update();
}

/* Pantalla de vuelo completa (al despegar y al volver de una pantalla de texto): panel, texto, radar, mira y polvo nuevo (NWSTARS) */
static void redraw_all(void)
{
    gfx_clear(C_BLACK);
    ships_forget_lines();                  /* la pantalla se ha borrado: ninguna nave figura como dibujada */
    sun_forget();
    dash_show();                           /* el fondo viene de memoria: el fichero se lee una sola vez */
    space_scanner_reset();
    dash_msblob();
    if (sspr) dash_spblb();
    panel_static();
    panel_update();
    chart_sight((u8)view);
    stars_new();
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

/* tunel (HFS2): se borra la vista, se dibujan los anillos con su sonido, se deja ver un momento y se borra */
static void do_tunnel(u8 stp)
{
    sun_hiss_hook = hiss;
    clear_view();
    sun_col = C_WHITE;
    tunnel_draw(stp);
    wait_frames(TUNNEL_FRAMES);
    clear_view();
}

static void arrive(void)
{
    char b[32];
    int i = 0, ok;
    const char *p = "ARRIVED AT ";
    u8 x;
    /* LL164: sonido de hiperespacio y tunel */
    x = 255;
    do { snd_soblip(x, 90); x = (u8)(x - 10); } while (x >= 170);
    do_tunnel(4);
    space_scanner_clear();                 /* quita del radar lo que hubiera */
    if (sspr) dash_spblb();                /* se apaga la S */
    ok = hyper_gal ? jump_galactic() : jump_do(tgt_sys);
    hyper = 0;
    if (!ok) { say("JUMP FAILED", 1); return; }
    space_arrive();                        /* sistema nuevo: planeta lejos y sin estacion */
    stars_new();
    chart_sight((u8)view);
    while (*p) b[i++] = *p++;
    for (p = uni_galaxy[cur_sys].name; *p && i < 31; p++) b[i++] = *p;
    b[i] = 0;
    say(b, 0);
    snd_beep();
    panel_update();
}

/* Pantallas de texto desde el vuelo (teclas 5..9 y 0): como en el original; el tiempo no avanza mientras se ven. Devuelve 0 o FL_QUIT */
static int flight_screens(int sc)
{
    int pending = 0;
    key_ctx_menu();
    for (;;) {
        sc = scr_dispatch(sc);
        if (sc == SC_QUIT || in_quit) { return FL_QUIT; }
        if (sc >= SC_VIEW0 && sc <= SC_VIEW3) { view = sc - SC_VIEW0; break; }
        if (sc == SC_FLIGHT) break;
        if (sc == SC_HYPER) { pending = 1; break; }
        if (sc == SC_GHYPER) { pending = 2; break; }
    }
    key_ctx_flight();
    redraw_all();
    if (pending == 1) start_hyper(0);
    else if (pending == 2) start_hyper(1);
    return 0;
}

int flight_run(void)
{
    int ch, sc;
    u8 ev;
    struct controls c;
    static const u8 contact_types[6] = { SHIPT_VIPER, SHIPT_COBRA_MK_3, SHIPT_PYTHON, SHIPT_ASTEROID, SHIPT_SIDEWINDER, SHIPT_KRAIT };

    ui_clear();
    gfx_clear(C_BLACK);
    key_ctx_flight();                                       /* menos filas de teclado por fotograma: ver keys.c */
    if (!dash_loaded()) dash_load();                        /* solo la primera vez; luego se queda en memoria */
    view = 0; hyper = 0;
    space_launch();
    redraw_all();
    snd_sohiss(0); snd_sohiss(0);                           /* LAUN */
    do_tunnel(8);                                           /* tunel de lanzamiento (STP = 8) */
    stars_new();                                            /* el tunel borro la vista: polvo y mira nuevos */
    chart_sight((u8)view);
    say(dash_loaded() ? "LAUNCHED" : "LAUNCHED (NO ELITE_DASH FILE)", 0);
    for (;;) {
        ch = in_poll();
        if (ch == KC_QUIT) { key_ctx_menu(); return FL_QUIT; }
        if (ch >= 0) {
            ch = in_up(ch);
            if (ch == 'c') { key_ctx_menu(); return FL_DOCK; }
            if (ch == 'n') { space_spawn(contact_types[em_dornd() % 6]); }
            else if (ch == 'k') { if (ships_explode_nearest() < 0) say("NOTHING TO BLOW UP", 1); else snd_exno2(); }
            else {
                sc = ui_nav(ch);
                if (sc >= SC_VIEW0 && sc <= SC_VIEW3) change_view(sc - SC_VIEW0);
                else if (sc == SC_HYPER) start_hyper(0);
                else if (sc == SC_GHYPER) start_hyper(1);
                else if (sc >= 0) { if (flight_screens(sc) == FL_QUIT) { key_ctx_menu(); return FL_QUIT; } }
            }
        }
        space_view = (u8)view;
        ctl_read(&c);
        space_controls(&c);                                 /* ALPHA, BETA y velocidad */
        ev = space_step();                                  /* planeta, estacion y contactos; radar */
        if (ev & SPACE_EV_STATION) dash_spblb();            /* se apaga la S */
        stars_set_speed(zDELTA);                            /* DELT4 */
        stars_update((u8)view);                             /* STARS */
        dash_update();
        if (hyper && ++hyper_tick >= HYPER_FRAMES) {
            hyper_tick = 0;
            hyper--;
            if (hyper == 0) arrive(); else say_hyper();
        }
        if ((mcnt & 31) == 0) panel_update();               /* el dinero y la distancia cambian poco */
    }
}
