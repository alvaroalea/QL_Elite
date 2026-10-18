#include "flight.h"
#include "missions.h"
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
#include "combat.h"
#include "damage.h"
#include "equip.h"
#include "dockit.h"
#include "encounters.h"
#include "tactics.h"
#include "cmdr.h"
#include "config.h"
#include "detok.h"

int flight_testkeys = 0;                   /* pruebas (make test_game): N, K y C sin ordenador de atraque */

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
    txt_print_at(0, 31, "A FIRE B BOMB J JUMP C/P DOCK N K");
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
    laser_forget();                         /* TTX66: LAS2 = 0 */
}

/* Lo que LOOK1 y la llegada vuelven a poner tras borrar la vista: la mira y, si la bomba esta explotando, su rayo */
static void view_extras(void)
{
    chart_sight((u8)view);
    if (bomb & 0x80) bomb_off();
}

/* TT66 con la vista del espacio (QQ11 = 0): borra la vista, BOX y el nombre de la vista ("FRONT VIEW") arriba */
static void space_tt66(void)
{
    scr_viewdir = (u8)view;
    tt66(0);
}

/* LOOK1 al cambiar de vista estando en el espacio: el polvo se refleja en la diagonal (FLIP) y se vuelve a poner la mira */
static void change_view(int v)
{
    if (v == view) return;
    view = v;
    space_view = (u8)view;                  /* las naves se redibujan en los ejes de la vista nueva (PLUT) */
    space_tt66();
    stars_flip();
    view_extras();
    panel_update();
}

/* Pantalla de vuelo completa (al despegar y al volver de una pantalla de texto): panel, texto, radar, mira y polvo nuevo (NWSTARS) */
static void redraw_panel(void)
{
    gfx_clear(C_BLACK);
    ships_forget_lines();                  /* la pantalla se ha borrado: ninguna nave figura como dibujada */
    sun_forget();
    laser_forget();
    dash_show();                           /* el fondo viene de memoria: el fichero se lee una sola vez */
    space_scanner_reset();
    dash_msblob();
    if (sspr) dash_spblb();
    panel_static();
    panel_update();
    if (ecma) dash_ecblb();                /* la bombilla E sigue encendida */
}

static void redraw_all(void)
{
    redraw_panel();
    view_extras();
    stars_new();
}

/* MESS: mensajes en vuelo (fila 24, como los demas avisos del port; el original los pone en la vista). Con mess_de se anade
   " DESTROYED" (OUCH). Los textos son los de los tokens del original */
static void flight_mess(u8 token)
{
    static const char *eq[8] = { "E.C.M.SYSTEM", 0, 0, "FUEL SCOOPS", 0, "ENERGY BOMB", "ENERGY UNIT", "DOCKING COMPUTERS" };
    const char *t = 0;
    char b[16];
    int i;
    txt_clear_rows(24, 1, C_BLACK);
    txt_color(C_GREEN, C_BLACK);
    txt_at(0, 24);
    if (token == MESS_RIGHT_ON) t = "RIGHT ON COMMANDER!";
    else if (token == MESS_ENERGY_LOW) { t = "ENERGY LOW"; snd_beep(); }
    else if (token == MESS_DOCK_ON) t = "DOCKING COMPUTERS ON";
    else if (token == MESS_SCOOP_ON) { t = "FUEL SCOOPS ON"; snd_beep(); }
    else if (token == MESS_INCOMING) t = "INCOMING MISSILE";
    else if (token == MESS_TARGET_LOST) t = "TARGET LOST";
    else if (token == MESS_JAMMED) t = "MISSILE JAMMED";
    else if (token >= 108 && token <= 115) t = eq[token - 108];
    else if (token >= MESS_GOOD && token < MESS_GOOD + UNI_NGOODS) {
        const char *n = uni_goods[token - MESS_GOOD].name;
        for (i = 0; n[i] && i < 15; i++) b[i] = (char)((n[i] >= 'a' && n[i] <= 'z') ? n[i] - 32 : n[i]);
        b[i] = 0; t = b;
    }
    if (t) txt_puts(t);
    else { ui_tenths(cash_get()); txt_puts(" CR"); }
    if (mess_de & 2) txt_puts(" DESTROYED");
    txt_color(C_WHITE, C_BLACK);
}

static void flight_mess_off(void) { txt_clear_rows(24, 1, C_BLACK); }

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
        game_tt111();                      /* hyp: TTX110 (o hm en las cartas) llama a TT111 antes de TTX111 */
        if (tgt_sys == cur_sys) { say("NO TARGET SELECTED", 1); snd_boop(); return; }
        if (!jump_possible(tgt_sys)) { say("OUT OF RANGE", 1); snd_boop(); return; }
    }
    hyper = 15; hyper_gal = galactic; hyper_tick = 0;
    say_hyper();
}

static void blip(int x, int y) { snd_soblip(x, y); }

static void tunnel_hooks(void)
{
    sun_hiss_hook = hiss; sun_blip_hook = blip; sun_tt66_hook = space_tt66;
    sun_col = C_WHITE;
}

/* LL164: sonido de hiperespacio y tunel (HFS2 con STP = 4); se deja ver un momento y se borra la vista */
static void hyper_tunnel(void)
{
    tunnel_hooks();
    tunnel_ll164();
    wait_frames(TUNNEL_FRAMES);
    clear_view();
}

/* MJP: salto fallido al espacio entre sistemas (witchspace): otro tunel, sin planeta ni sol ni estacion, tres Thargoides con
   sus Thargons, MJ = $FF, polvo con 2 particulas y la coordenada y de la galaxia con los 5 bits bajos invertidos */
static void misjump(void)
{
    hyper_tunnel();                                         /* TT66 + LL164 */
    space_reset();                                          /* RES2 */
    if (ecma) ecm_off();
    mstg = 0xFF;
    ships_clear();
    sspr = 0;
    mj = 0xFF;                                              /* STY MJ */
    do gthg(); while (many[SHIPT_THARGOID] <= 2);           /* MJP1: hasta tener tres */
    stars_nostm = 2;                                        /* STA NOSTM con A = 2 */
    view = 0; space_view = 0;                               /* LDX #0; JSR LOOK1 */
    space_tt66();
    stars_new();
    view_extras();
    qq1 = (u8)(qq1 ^ 0x1F);                                 /* QQ1 EOR #%00011111 */
    say("WITCHSPACE", 1);
    panel_update();
}

static void arrive(void)
{
    char b[32];
    int i = 0, ok;
    const char *p = "ARRIVED AT ";
    hyper_tunnel();                        /* LL164: sonido de hiperespacio y tunel */
    space_scanner_clear();                 /* quita del radar lo que hubiera */
    if (sspr) dash_spblb();                /* se apaga la S */
    if (!hyper_gal && jump_possible(tgt_sys)) {             /* TT18: el combustible se gasta antes de saber si llegamos */
        if (((patg & 0x80) && key_down(K_TAB)) || em_dornd() >= 253) {   /* ee5: TAB con PATG (ptg) o 0,78 %: MJP */
            unsigned d = jump_distance(tgt_sys);
            if ((patg & 0x80) && key_down(K_TAB)) cok = (u8)(cok | 1);         /* ptg: COK bit 0 */
            qq14 = (u8)(qq14 >= d ? qq14 - d : 0);
            hyper = 0;
            misjump();
            return;
        }
    }
    ok = hyper_gal ? jump_galactic() : jump_do(tgt_sys);
    hyper = 0;
    if (ecma) ecm_off();                                    /* RES2 */
    if (!(mstg & 0x80)) abort_missile(MS_GREEN);
    stars_nostm = NOST;                                     /* RES2 */
    if (!ok) { say("JUMP FAILED", 1); return; }
    space_arrive();                        /* sistema nuevo: planeta lejos y sin estacion */
    enc_ev = 0;                            /* EV = 0 */
    stars_new();
    view_extras();
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

/* ESCAPE: la capsula; vemos alejarse nuestra Cobra, perdemos la carga, la capsula y la condicion de fugitivo, el deposito
   se llena y aparecemos en la estacion (GOIN) */
static void flight_escape(void)
{
    int i;
    space_reset();                                          /* RES2 */
    ships_erase_all();
    ships_clear();
    clear_view();
    stars_new();
    ships_escape();
    for (i = 0; i <= 16; i++) qq20[i] = 0;
    fist = 0;
    escp = 0;
    qq14 = 70;
}

/* DEATH: explosion, GAME OVER y los restos de nuestra nave alejandose (D1/D2); luego DEATH2 vuelve al titulo */
static void flight_death(void)
{
    int f;
    struct controls c;
    snd_death();                                            /* SOEXPL con Y = 210 */
    space_reset();                                          /* RES2 */
    ships_clear();
    zDELTA = (u8)(zDELTA << 2);                             /* ASL DELTA dos veces */
    space_tt66();                                           /* TT66 con la vista 0 (VIEW no cambia) */
    qq17 = 0x80; dt_mt2(); dtw2 = 0x80;
    stars_new();                                            /* nWq */
    dt_xc = 12; dt_yc = 12;
    dt_ex(146);                                             /* ex: ficha 146, "GAME OVER" */
    ships_death_debris();                                   /* D1 */
    zDELTA = 0;
    for (f = 0; f <= 64; f++) {                             /* JSR M% y D2: 64 vueltas mas (LASCT) */
        if (!in_script) in_poll();
        ctl_read(&c);                                       /* M% lee el teclado (DOKEY): se puede girar */
        space_controls(&c);
        ships_step((u8)view);                               /* el M% del original: MCNT queda en 255, sin comprobaciones */
        stars_set_speed(zDELTA);
        stars_update((u8)view);
        dash_update();
    }
    say("", 0);
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
    player_dead = 0; mess_de = 0; dock_auto = 0; mj = 0; mstg = 0xFF; msar = 0;
    ecma = 0; ecmp = 0;
    stars_nostm = NOST;
    combat_reset();                                         /* DOENTRY: energia, escudos y laser como al atracar */
    combat_mess_hook = flight_mess;
    combat_mess_off_hook = flight_mess_off;
    redraw_panel();                                         /* el panel (el original lo tiene siempre en pantalla) */
    tunnel_hooks();
    tunnel_laun();                                          /* TT110: LAUN (dos SOHISS y el tunel, STP = 8) */
    wait_frames(TUNNEL_FRAMES);
    space_launch();                                         /* RES2, TT111, SOS1, NWSPS, DELTA = 12, FIST */
    enc_ev = 0;
    tunnel_draw(8);                                         /* HFS1 con QQ11 = 255: los mismos anillos (y sonidos) los borran */
    space_tt66();                                           /* LOOK1 -> LQ: TT66, SIGHT, BOMBOFF, NWSTARS */
    view_extras();
    stars_new();
    say(dash_loaded() ? "LAUNCHED" : "LAUNCHED (NO ELITE_DASH FILE)", 0);
    for (;;) {
        ch = in_poll();
        if (ch == KC_QUIT) { key_ctx_menu(); return FL_QUIT; }
        if (ch >= 0) {
            ch = in_up(ch);
            if (ch == 'c') {                                /* KY19: ordenador de atraque si lo llevamos (MA64: STA auto) */
                if (dkcmp) dock_auto = dkcmp;
                else if (flight_testkeys && sspr) { key_ctx_menu(); return FL_DOCK; }    /* pruebas: atraque inmediato */
                ch = -1;
            }
            if (ch == '=') {                                /* DK4: pausa (FREEZE) y teclas de configuracion */
                key_ctx_menu();
                if (freeze()) { snd_off(); return FL_DEATH; }    /* ESC: DEATH2 */
                if (in_quit) return FL_QUIT;
                key_ctx_flight();
                ch = -1;
            }
            if (ch == 'u') { missile_unarm(); ch = -1; }   /* KY15: MA4..MA20 */
            if (ch == 't') { missile_target(); ch = -1; }  /* KY14: MA20..MA25 */
            if (ch == 'm') { missile_fire(); ch = -1; }    /* KY16: MA25..MA24 */
            if (ch == 'e') { ecm_key(); ch = -1; }         /* KY17 */
            if (ch == 'p') { dock_auto = 0; ch = -1; }     /* KY20: MA76..MA78 */
            if (ch == 'b') { bomb_key(); ch = -1; }        /* KY12: MA24..MA76 */
            if (ch == 'j') {                                /* KY18: WARP */
                if (warp()) { mcnt = 1; enc_ev = 0; clear_view(); stars_new(); view_extras(); }
                ch = -1;
            }
            if (ch == KC_ESC) {                             /* KY13: capsula de escape (no en el hiperespacio) */
                if (escp && !mj) { flight_escape(); snd_off(); key_ctx_menu(); return FL_DOCK; }
                ch = -1;
            }
            if (ch < 0) { }
            else if (ch == 'n' && flight_testkeys) { space_spawn(contact_types[em_dornd() % 6]); }
            else if (ch == 'k' && flight_testkeys) { if (ships_explode_nearest() < 0) say("NOTHING TO BLOW UP", 1); else snd_exno2(); }
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
        dokey_auto(&c);                                     /* DOKEY: el ordenador de atraque pulsa por nosotros */
        space_controls(&c);                                 /* ALPHA, BETA y velocidad */
        laser_fire((u8)view, c.fire);                       /* parte 3: disparo (LASLI) */
        ev = space_step();                                  /* naves (HITCH, danos, choques), radar, energia, parte 15 */
        if (ev & SPACE_EV_DOCK) { snd_off(); key_ctx_menu(); return FL_DOCK; }       /* por la ranura: DOENTRY */
        if (ev & SPACE_EV_DEATH) { flight_death(); key_ctx_menu(); return FL_DEATH; }
        bomb_frame();                                       /* parte 16 (MA18): la bomba sigue explotando */
        ecm_frame();                                        /* parte 16 (MA16..MA66): ECM */
        laser_frame_end();                                  /* parte 16: fin del pulso (LASLI2) */
        if (ev & SPACE_EV_STATION) dash_spblb();            /* se apaga la S */
        stars_set_speed(zDELTA);                            /* DELT4 */
        stars_update((u8)view);                             /* STARS */
        dly_tick();                                         /* TT100: el mensaje en vuelo caduca */
        mcnt--;                                             /* me3: DEC MCNT; a 0, partes 2-4 (aparicion de naves) */
        if (mcnt == 0) encounters_tick(em_rand_c);          /* acarreo: el del ultimo DORND (el del 6502 sale de M%) */
        laser_cool();                                       /* MLOOP: enfriamiento y cuenta del pulso */
        dash_update();
        if (hyper && ++hyper_tick >= HYPER_FRAMES) {
            hyper_tick = 0;
            hyper--;
            if (hyper == 0) arrive(); else say_hyper();
        }
        if ((mcnt & 31) == 0) panel_update();               /* el dinero y la distancia cambian poco */
    }
}
