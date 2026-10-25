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
#include "ship3d.h"
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

#define VIEW_H 160                         /* lineas de la vista en pantalla */
#define ROW_MSG   27                       /* texto bajo el panel (lineas 216..255): estado (LY, vista o mensaje, creditos), HERE, TARGET; la 30 libre y FPS en la 31 */
#define ROW_HERE  28                       /* HERE a la izquierda y TARGET a la derecha */
#define ROW_HELP  31                       /* ayuda rotativa; los FPS pisan sus 8 ultimas columnas */
#define FRAME_TICKS 5                      /* cuadros de 50 Hz minimos por fotograma de vuelo (5 = 10 FPS como maximo) */
#define HELP_FRAMES 100                    /* fotogramas que dura cada linea de ayuda (~2 s a 50 fps; provisional) */
#define TGT_COL 16
#define MSG_FRAMES 100                     /* fotogramas que dura un mensaje antes de volver a mostrarse la vista (~2 s a 50 fps; provisional) */
#define HYPER_FRAMES 8                     /* fotogramas por cuenta atras (provisional: ~0,16 s a 50 fps) */
#define TUNNEL_TICKS 1                     /* cuadros de 50 Hz por cada unidad de TUNNEL_FRAMES (30 = ~0,6 s) */
#define RING_TICKS 2                       /* cuadros de 50 Hz minimos por anillo del tunel (~35 anillos: ~1,4 s) */
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
    unsigned long t0;
    if (in_script) return;
    t0 = plat_ticks();
    if (t0 == 0UL) { for (i = 0; i < n; i++) in_poll(); return; }
    while (plat_ticks() - t0 < (unsigned long)n * TUNNEL_TICKS) in_poll();     /* propio: por tiempo, no por vueltas */
}

/* propio: cada anillo del tunel tarda como minimo RING_TICKS cuadros de 50 Hz (en el QL real los iba dibujando la lentitud) */
static void ring_pace(void)
{
    static unsigned long last = 0;
    unsigned long now;
    if (in_script || plat_ticks() == 0UL) return;
    do { now = plat_ticks(); } while (now - last < RING_TICKS && now >= last);
    last = now;
}

static const char *const help_lines[3] = { "CURSORS TURN F FAST D SLOW", "1-4 VIEW 5-0 INFO H/G/J JUMPS", "B BOMB C/P DOCK = PAUSE" };
static int help_i = 0, help_t = 0;
static char fps_text[9] = "";                /* los FPS que pisan el final de la ayuda ("" = apagado) */

static void help_draw(void)
{
    txt_clear_rows(ROW_HELP, 1, C_BLACK);
    txt_color(C_CYAN, C_BLACK);
    txt_nowrap(1);
    txt_print_at(0, ROW_HELP, help_lines[help_i]);
    if (fps_text[0]) { txt_color(C_YELLOW, C_BLACK); txt_print_at(24, ROW_HELP, fps_text); }
    txt_nowrap(0);
    txt_color(C_WHITE, C_BLACK);
}

static void help_tick(void)
{
    if (++help_t < HELP_FRAMES) return;
    help_t = 0;
    help_i = (help_i + 1) % 3;
    help_draw();
}

static void panel_invalidate(void);

static void panel_static(void)
{
    panel_invalidate();                                     /* el rectangulo de abajo se borra entero: nada de lo anterior sigue pintado */
    gfx_rect(0, DASH_Y0 + DASH_LINES, 256, 256 - (DASH_Y0 + DASH_LINES), C_BLACK);
    txt_color(C_CYAN, C_BLACK);
    txt_print_at(0, ROW_HERE, "HERE");
    txt_print_at(TGT_COL, ROW_HERE, "TARGET");
    help_draw();
}

/* valor en decimas con un sufijo (por ejemplo 12345 y " Cr" -> "1234.5 Cr"); devuelve la longitud */
static int tenths_text(char *b, unsigned long tenths, const char *suffix)
{
    char t[12];
    int n = dec_digits(tenths, t), i = 0, k;
    if (n < 2) { t[1] = t[0]; t[0] = '0'; n = 2; }              /* 5 -> "0.5" */
    for (k = 0; k < n - 1; k++) b[i++] = t[k];
    b[i++] = '.';
    b[i++] = t[n - 1];
    while (*suffix) b[i++] = *suffix++;
    b[i] = 0;
    return i;
}

/* Lo ultimo que se pinto (columnas) en cada texto que cambia de longitud: si el nuevo es mas corto, se calcula el rectangulo de celdas que
   sobra y se vacia de una vez (gfx_clear_cells: ensamblador); lo demas se sobrescribe celda a celda, sin parpadeo */
static int len_here = 0, len_tgt = 0;

#ifdef DEBUG
static void panel_invalidate(void) { len_here = len_tgt = 0; }
#else
static void panel_invalidate(void) { len_here = len_tgt = 0; }
#endif

static int cstr_len(const char *s) { int n = 0; while (s[n]) n++; return n; }

static void panel_name(int col, const char *name, int *oldlen)       /* nombre tras su etiqueta, en la fila de HERE */
{
    int n = cstr_len(name);
    if (*oldlen > n) gfx_clear_cells(col + n, ROW_HERE, *oldlen - n, 1);
    txt_print_at(col, ROW_HERE, name);
    *oldlen = n;
}

static void panel_right(int row, const char *s, int *oldlen)         /* valor alineado a la derecha (hasta la columna 31) */
{
    int n = cstr_len(s);
    if (*oldlen > n) gfx_clear_cells(32 - *oldlen, row, *oldlen - n, 1);
    txt_print_at(32 - n, row, s);
    *oldlen = n;
}

/* Fila de estado: anos luz hasta el destino a la izquierda, creditos a la derecha y en medio la vista ("FRONT VIEW"); un mensaje
   la ocupa entera, centrado, hasta que caduca (MSG_FRAMES) */
static char msg_text[34];
static u8 msg_col = 0;
static int msg_t = 0;

static void status_draw(void)
{
    char ly[24], cr[24];
    int n = 0, c0 = 0, nl, nc;
    txt_clear_rows(ROW_MSG, 1, C_BLACK);
    txt_nowrap(1);
    nl = tenths_text(ly, (unsigned long)jump_distance(tgt_sys), " LY");
    nc = tenths_text(cr, cash_get(), " Cr");
    if (msg_t) {                                            /* el mensaje centrado; LY y creditos solo si caben a sus lados */
        n = cstr_len(msg_text);
        c0 = n < 32 ? (32 - n) / 2 : 0;
        txt_color(msg_col, C_BLACK);
        txt_print_at(c0, ROW_MSG, msg_text);
    } else {
        txt_color(C_CYAN, C_BLACK);
        txt_print_at(11, ROW_MSG, vnames[view]);
        txt_print_at(17, ROW_MSG, "VIEW");
        c0 = 11; n = 10;
    }
    txt_color(C_WHITE, C_BLACK);
    if (c0 > nl) txt_print_at(0, ROW_MSG, ly);
    if (c0 + n < 32 - nc) txt_print_at(32 - nc, ROW_MSG, cr);
    txt_nowrap(0);
}

static void msg_show(const char *s, u8 color)
{
    int i;
    for (i = 0; s[i] && i < 33; i++) msg_text[i] = s[i];
    msg_text[i] = 0;
    msg_col = color;
    msg_t = i ? MSG_FRAMES : 0;
    status_draw();
}

static void msg_tick(void)
{
    if (msg_t && --msg_t == 0) status_draw();
}

static void panel_cash(void) { status_draw(); }                     /* los creditos van en la fila de estado */

static void panel_update(void)
{
    txt_color(C_WHITE, C_BLACK);
    txt_nowrap(1);                                          /* un nombre largo no debe saltar a la fila siguiente */
    panel_name(5, uni_galaxy[cur_sys].name, &len_here);
    panel_name(TGT_COL + 7, uni_galaxy[tgt_sys].name, &len_tgt);
    txt_nowrap(0);
    status_draw();
}

static void say(const char *s, int error)
{
    msg_show(s, error ? C_RED : C_GREEN);
}

#ifdef DEBUG
/* SHIFT-F: fotogramas por segundo en la ultima fila, alineados a la derecha; se calculan cada segundo
   (50 cuadros de plat_ticks) con una decima */
static int dbg_fps = 0;
static unsigned long fps_t0 = 0, fps_n = 0;

static void dbg_fps_key(void)
{
    dbg_fps = !dbg_fps;
    fps_t0 = plat_ticks(); fps_n = 0;
    if (!dbg_fps) { fps_text[0] = 0; help_draw(); }
}

static void dbg_fps_frame(void)
{
    char b[16];
    unsigned long now, dt;
    if (!dbg_fps) return;
    fps_n++;
    now = plat_ticks(); dt = now - fps_t0;
    if (dt < 50UL) return;
    tenths_text(b, fps_n * 500UL / dt, " FPS");
    {
        int n = cstr_len(b), i = 0, k;
        for (; i < 8 - n; i++) fps_text[i] = ' ';          /* siempre 8 caracteres */
        for (k = 0; k < n && i < 8; k++, i++) fps_text[i] = b[k];
        fps_text[i] = 0;
    }
    help_draw();
    fps_t0 = now; fps_n = 0;
}

/* SHIFT-D: cicla lo que se salta (ship_dbg_skip, 3 bits, 8 estados): bit 0 = matematicas de LL9, bit 1 = dibujo de las naves, bit 2 = pintado de
   las estrellas (se calculan pero no se pintan). Sin matematicas, cada nave repinta dos veces las lineas que ya tenia (mismo coste de dibujo, no se
   mueve en pantalla); sin dibujo, LL9 calcula pero no pinta. Al cambiar el dibujo de las naves o el de las estrellas se borra la vista (el monton
   de lineas o las estrellas ya no coinciden con la pantalla) */
static void clear_view(void);
static void view_extras(void);
static void dbg_skip_key(void)
{
    static const char *const names[8] = { "SKIP: NONE", "SKIP: MATHS", "SKIP: DRAWING", "SKIP: MATHS+DRAWING",
                                          "SKIP: STARS", "SKIP: STARS+MATHS", "SKIP: STARS+DRAWING", "SKIP: ALL" };
    u8 old = ship_dbg_skip;
    ship_dbg_skip = (u8)((ship_dbg_skip + 1) & 7);
    stars_dbg_nodraw = (u8)((ship_dbg_skip >> 2) & 1);
    if ((old ^ ship_dbg_skip) & 6) { clear_view(); stars_new(); view_extras(); }
    say(names[ship_dbg_skip], 0);
}
#endif

/* Pausa: "PAUSED" a mitad del hueco entre la mira (hasta y = 75) y el panel (y = 160): fila de texto 14 (y = 112..119), centrado.
   Se guarda lo que habia debajo (naves, polvo) y se restaura al reanudar */
#define PAUSE_ROW 14
#define PAUSE_COL 10
#define PAUSE_CELLS 11
static u8 pause_save[16][PAUSE_CELLS * 4];

static void pause_banner(int on)
{
    int r, i;
    u8 *p;
    for (r = 0; r < 16; r++) {
        p = screen + (long)(PAUSE_ROW * 8 + r) * 128L + PAUSE_COL * 4;     /* 128 bytes por linea; 4 bytes por celda de 8 puntos */
        for (i = 0; i < PAUSE_CELLS * 4; i++) { if (on) pause_save[r][i] = p[i]; else p[i] = pause_save[r][i]; }
    }
    if (on) {
        txt_color(C_WHITE, C_BLACK);
        txt_print_at(PAUSE_COL + 3, PAUSE_ROW, "PAUSED");
        txt_print_at(PAUSE_COL, PAUSE_ROW + 1, "O = OPTIONS");
    }
}

/* Ventana "Options" de la pausa (tecla O; propio): una linea por opcion de TGINT con la tecla y ON/OFF; las teclas las cambian con dks3 */
#define OPT_COL 1
#define OPT_ROW 2
#define OPT_W 30

static void clear_view(void);
static void redraw_panel(void);

static void opt_line(int row, const char *name, char key, int on)
{
    char buf[48];
    int n = 0;
    while (name[n]) { buf[n] = name[n]; n++; }
    buf[n++] = ' '; buf[n++] = '('; buf[n++] = key; buf[n++] = ')';
    while (n < OPT_W - 3) buf[n++] = '.';
    buf[n] = 0;
    txt_color(C_WHITE, C_BLACK);
    txt_print_at(OPT_COL, row, buf);
    txt_color(on ? C_GREEN : C_RED, C_BLACK);
    txt_print_at(OPT_COL + OPT_W - 3, row, on ? " ON" : "OFF");
}

static void opt_draw(void)
{
    int r = OPT_ROW;
    txt_color(C_WHITE, C_BLACK);
    txt_color(C_YELLOW, C_BLACK);
    txt_print_at(OPT_COL + 11, r, "OPTIONS");
    r += 2;
    opt_line(r++, "Damping", 'D', damp == 0);
    opt_line(r++, "Key auto-centre", 'A', djd == 0);
    opt_line(r++, "Flashing bars", 'F', (flh & 0x80) != 0);
    opt_line(r++, "Joystick Y flip", 'Y', !(jstgy & 0x80));
    opt_line(r++, "Joystick axes flip", 'J', (jste & 0x80) != 0);
    opt_line(r++, "Caps in missions", 'U', (uptog & 0x80) != 0);
    opt_line(r++, "Author credits", 'X', (patg & 0x80) != 0);
    opt_line(r++, "Sound", 'S', snd_enabled());
    opt_line(r++, "512 mode", 'M', gfx_hires_opt);
    r++;
    txt_color(C_CYAN, C_BLACK);
    txt_print_at(OPT_COL, r++, "ESC  Back to game");
    txt_print_at(OPT_COL, r, "Q    Back to menu");
}

static int mode_changed = 0;                        /* se cambio de modo de pantalla en la pausa: al reanudar hay que repintarlo todo */

static int pause_options(void)
{
    int ch;
    u8 x, y;
    clear_view();                                   /* la ventana ocupa toda la vista 3D */
    opt_draw();
    for (;;) {
        ch = in_poll();
        if (in_quit) return 0;
        if (ch < 0) continue;
        x = (u8)((ch >= 'a' && ch <= 'z') ? ch - 32 : ch);
        if (x == 'Q') return 1;                             /* al menu principal (DEATH2) */
        if (x == 27 || x == '-') return 0;                  /* de vuelta al juego */
        for (y = 0; y < 9; y++) if (y != 6) dks3(x, y);     /* K (JSTK) fuera: el joystick siempre esta activo */
        if (x == 'S') snd_enable(!snd_enabled());
        if (x == 'M') {                                     /* propio: de modo 8 a modo 4 (512) y al reves: la pantalla se borra, se repinta el panel */
            gfx_hires_opt = (u8)!gfx_hires_opt;
            plat_mode(gfx_hires_opt);
            redraw_panel();
            clear_view();
            mode_changed = 1;
        }
        opt_draw();
    }
}

/* TT66 de la vista espacial: borra el area de vista (el panel no se toca) */
static void clear_view(void)
{
    gfx_fill_lines(0, VIEW_H, C_BLACK);     /* rapido: palabras largas, no pixel a pixel */
    ships_forget_lines();                   /* las lineas de las naves, el sol y el planeta ya no estan en pantalla */
    sun_forget();
    laser_forget();                         /* TTX66: LAS2 = 0 */
    chart_sight_forget();
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
    chart_sight_forget();
    dash_mblb_forget();
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

/* MESS: mensajes en vuelo (fila 27, como los demas avisos del port; el original los pone en la vista). Con mess_de se anade
   " DESTROYED" (OUCH). Los textos son los de los tokens del original */
static void flight_mess(u8 token)
{
    static const char *eq[8] = { "E.C.M.SYSTEM", 0, 0, "FUEL SCOOPS", 0, "ENERGY BOMB", "ENERGY UNIT", "DOCKING COMPUTERS" };
    const char *t = 0;
    char b[16], c[24], b2[40];
    int i, n;
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
    if (!t) { tenths_text(c, cash_get(), " CR"); t = c; }
    if (mess_de & 2) { for (i = 0; t[i] && i < 20; i++) b2[i] = t[i]; for (n = 0; " DESTROYED"[n]; n++) b2[i++] = " DESTROYED"[n]; b2[i] = 0; t = b2; }
    msg_show(t, C_GREEN);
}

static void flight_mess_off(void) { msg_t = 0; status_draw(); }

static void say_hyper(void)
{
    char b[24];
    int i = 0;
    static const char *const start[5] = { "Computer Initialization", "Loading Vectors", "Tracing Route", "Verifying Route", "Aligning Engines" };
    const char *p = hyper_gal ? "GALACTIC JUMP " : "HYPERSPACE ";
    if (hyper >= 11) { say(start[15 - hyper], 0); return; }     /* los 5 primeros pasos de la cuenta atras (15..11) */
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
        if (tgt_sys == cur_sys) {
#ifdef DEBUG
            dbg_range = 0;
#endif
            say("NO TARGET SELECTED", 1); snd_boop(); return;
        }
        if (!jump_possible(tgt_sys)) { say("OUT OF RANGE", 1); snd_boop(); return; }
    }
    hyper = 15; hyper_gal = galactic; hyper_tick = 0;
    say_hyper();
}

static void blip(int x, int y) { snd_soblip(x, y); }

static void tunnel_hooks(void)
{
    sun_hiss_hook = hiss; sun_blip_hook = blip; sun_tt66_hook = space_tt66; sun_pace_hook = ring_pace;
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
void misjump(void)
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
    if (!hyper_gal && jump_possible(tgt_sys)) {             /* TT18: el combustible se gasta antes de saber si llegamos */
        if (((patg & 0x80) && key_down(K_TAB)) || (em_rand_c = 0, em_dornd() >= 253)) {   /* ee5: TAB con PATG (ptg) o 0,78 %: MJP (DORND entra con el C de CMP #9: 0 sin tecla) */
            unsigned d = jump_distance(tgt_sys);
            if ((patg & 0x80) && key_down(K_TAB)) cok = (u8)(cok | 1);         /* ptg: COK bit 0 */
            qq14 = (u8)(qq14 >= d ? qq14 - d : 0);
            hyper = 0;
            misjump();
            return;
        }
    }
    ok = hyper_gal ? jump_galactic() : jump_do(tgt_sys);
#ifdef DEBUG
    dbg_range = 0;
#endif
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
    view_set_native(0);                                     /* las pantallas de texto y las cartas usan las coordenadas del original */
    plat_mode(0);                                           /* y el modo 8, como dentro de la estacion */
    for (;;) {
        sc = scr_dispatch(sc);
        if (sc == SC_QUIT || in_quit) { return FL_QUIT; }
        if (sc >= SC_VIEW0 && sc <= SC_VIEW3) { view = sc - SC_VIEW0; break; }
        if (sc == SC_FLIGHT) break;
        if (sc == SC_HYPER) { pending = 1; break; }
        if (sc == SC_GHYPER) { pending = 2; break; }
    }
    key_ctx_flight();
    view_set_native(1);
    plat_mode(gfx_hires_opt);                               /* de vuelta a la vista de vuelo (redraw_all repinta todo) */
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

static int flight_loop(void)
{
    int ch, sc, r;
    u8 ev;
    struct controls c;
    unsigned long last_t;
    static const u8 contact_types[6] = { SHIPT_VIPER, SHIPT_COBRA_MK_3, SHIPT_PYTHON, SHIPT_ASTEROID, SHIPT_SIDEWINDER, SHIPT_KRAIT };

    ui_clear();
    gfx_clear(C_BLACK);
    key_ctx_flight();                                       /* menos filas de teclado por fotograma: ver keys.c */
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
    last_t = plat_ticks();
    for (;;) {
        /* propio: paso fijo. Toda la simulacion cuenta por fotograma y el original iba a ~10 FPS o menos: con mas
           rapidez se espera hasta FRAME_TICKS cuadros de 50 Hz desde el fotograma anterior (sin reloj o en scripts, no) */
        if (!in_script && last_t != 0UL) {
            unsigned long now_t;
            do { now_t = plat_ticks(); } while (now_t - last_t < FRAME_TICKS);
            last_t = now_t;
        }
        ch = in_poll();
        if (ch == KC_QUIT) { key_ctx_menu(); return FL_QUIT; }
        if (ch >= 0) {
#ifdef DEBUG
            if (ch == 'C') { dock_auto = 0xFF; ch = -1; }                          /* SHIFT-C: atraque automatico, como con ordenador */
            else if (ch == 'P') { snd_off(); key_ctx_menu(); return FL_DOCK; }     /* SHIFT-P: atraque instantaneo */
            else if (ch == 'J') {                                                  /* SHIFT-J: la estacion aparece cerca, delante */
                if (ships_dbg_near_station()) say("STATION NEARBY", 0); else say("NO STATION HERE", 1);
                ch = -1;
            }
            else if (ch == 'H') { dbg_range = 1; ch = 'h'; }                       /* SHIFT-H: salta aunque no llegue el combustible */
            else if (ch == 'F') { dbg_fps_key(); ch = -1; }                        /* SHIFT-F: contador de fotogramas por segundo */
            else if (ch == 'D') { dbg_skip_key(); ch = -1; }                       /* SHIFT-D: saltar matematicas / dibujo de las naves */
#endif
            ch = in_up(ch);
            if (ch == 'c') {                                /* KY19: ordenador de atraque si lo llevamos (MA64: STA auto) */
                if (dkcmp) dock_auto = dkcmp;
                else if (flight_testkeys && sspr) { key_ctx_menu(); return FL_DOCK; }    /* pruebas: atraque inmediato */
                ch = -1;
            }
            if (ch == '=') {                                /* DK4: pausa (FREEZE) y teclas de configuracion */
                key_ctx_menu();
                pause_banner(1);
                config_options_hook = pause_options;
                config_options_used = 0;
                r = freeze();
                pause_banner(0);
                if (mode_changed) { mode_changed = 0; redraw_all(); }                      /* otro modo de pantalla: el panel y la vista se pintan de nuevo (lo guardado de PAUSED ya no vale) */
                else if (config_options_used) { clear_view(); stars_new(); view_extras(); }    /* la ventana tapaba la vista */
                if (r) { snd_off(); return FL_DEATH; }    /* ESC: DEATH2 */
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
        stars_set_speed(zDELTA);                            /* DELT4 */
        stars_update((u8)view);                             /* STARS */
        dly_tick();                                         /* TT100: el mensaje en vuelo caduca */
        help_tick();                                        /* la linea de ayuda cambia cada ~2 s */
        msg_tick();                                         /* el mensaje caduca y vuelve la vista */
        mcnt--;                                             /* me3: DEC MCNT; a 0, partes 2-4 (aparicion de naves) */
        if (mcnt == 0) encounters_tick(em_rand_c);          /* acarreo: el del ultimo DORND (el del 6502 sale de M%) */
        laser_cool();                                       /* MLOOP: enfriamiento y cuenta del pulso */
        dash_update();
        dash_mblb(warp_blocked());                          /* propio: M roja con el salto J bloqueado */
        chart_sight_update((u8)view);                       /* propio: aspa en la mira con la estacion cerca (S) */
#ifdef DEBUG
        dbg_fps_frame();
#endif
        if (hyper && ++hyper_tick >= HYPER_FRAMES) {
            hyper_tick = 0;
            hyper--;
            if (hyper == 0) arrive(); else say_hyper();
        }
    }
}

/* Entrada en la estacion (propio): antes de las pantallas de informacion, ~2 s de un tunel de como mucho 3 rectangulos igualmente espaciados
   (los de la salida, con la proporcion de la vista: semialto = 5/8 del semiancho) que se acercan y van frenando hasta pararse. Perspectiva:
   el rectangulo a distancia z (1 = el borde de la vista) tiene semiancho 128 / z; las tres distancias son 1 + u, 2 + u y 3 + u con u en
   [0, 1) bajando; al pasar u por 0 el mas cercano sale de la vista y entra otro lejano (siempre 3). El recorrido sigue una curva de frenado
   (1 - (1 - t)^3) hasta 2,5 separaciones. El tiempo va por el reloj de 50 Hz (en el QL lento serian pocos fotogramas): sin reloj
   (NOTICK) se cuentan DOCK_FRAMES fotogramas. En las pruebas con guion no se hace. */
#define DOCK_TICKS 100                     /* 2 s a 50 Hz */
#define DOCK_FRAMES 100                    /* fotogramas de la animacion si no hay reloj (NOTICK) */
#define DOCK_TRAVEL 640L                   /* recorrido total en 1/256 de separacion (2,5) */

/* Semianchos de los tres rectangulos en el instante t (0..256 = de 0 a 2 s) */
static void dock_rects(long t, int *hw)
{
    long r = 256L - t, d, u;
    int i;
    d = DOCK_TRAVEL * (256L - (r * r * r) / 65536L) / 256L;        /* recorrido en 1/256 de separacion (curva de frenado) */
    u = (1024L - d) & 255L;
    for (i = 0; i < 3; i++) hw[i] = (int)(32768L / (256L + u + 256L * i));
}

static void dock_frame(int cx, int hw)
{
    int hh = (hw * 5) / 8;
    gfx_frame_xor(cx, view_cy, hw, hh < 1 ? 1 : hh, C_WHITE, 0, 0, 255, clip_h - 1);
}

static void dock_tunnel(void)
{
    int old_hw[3] = { 0, 0, 0 }, hw[3], i, frames = 0, clk;
    unsigned long t0 = plat_ticks();
    long t;
    if (in_script) return;
    clk = (t0 != 0UL);                                              /* sin reloj (NOTICK) el tiempo se cuenta en fotogramas */
    clear_view();
    for (;;) {
        t = clk ? (long)(plat_ticks() - t0) * 256L / DOCK_TICKS : (long)frames * 256L / DOCK_FRAMES;
        if (t > 256L) t = 256L;
        dock_rects(t, hw);
        for (i = 0; i < 3; i++) {                                   /* cada rectangulo: se borra el viejo y se dibuja el nuevo seguidos */
            if (old_hw[i]) dock_frame(128, old_hw[i]);
            if (t < 256L) dock_frame(128, hw[i]);
            old_hw[i] = (t < 256L) ? hw[i] : 0;
        }
        if (t >= 256L) break;
        in_poll();                                                  /* pasa el fotograma (reloj, teclado, 50 fps) */
        frames++;
    }
    clear_view();
}

/* El vuelo trabaja con la vista nativa de 160 lineas (view_set_native): se activa al entrar y se quita al salir (titulo, atracado y cartas no la usan) */
int flight_run(void)
{
    int r;
    msg_t = 0; help_i = 0; help_t = 0;
    plat_mode(gfx_hires_opt);                               /* la vista de vuelo va en modo 4 si esta la opcion de 512 */
    view_set_native(1);
    ships_spblb_hook = dash_spblb;                          /* NWSPS y KS4 encienden y apagan la S */
    r = flight_loop();
    if (r == FL_DOCK) dock_tunnel();                       /* entrada en la estacion */
    ships_spblb_hook = 0;
    view_set_native(0);
    if (r != FL_DEATH && r != FL_QUIT) plat_mode(0);        /* atracado: modo 8 (tras la muerte viene el titulo, en modo 4) */
    return r;
}
