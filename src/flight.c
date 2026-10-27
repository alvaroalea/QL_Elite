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

int flight_testkeys = 0;                   /* tests (make test_game): N, K and C without docking computer */

#define VIEW_H 160                         /* lines of the view on screen */
#define ROW_MSG   27                       /* text under the panel (lines 216..255): status (LY, view or message, credits), HERE, TARGET; 30 free and FPS on 31 */
#define ROW_HERE  28                       /* HERE on the left and TARGET on the right */
#define ROW_HELP  31                       /* rotating help; the FPS overwrite its last 8 columns */
#define FRAME_TICKS 5                      /* minimum 50 Hz ticks per flight frame (5 = 10 FPS at most) */
#define HELP_FRAMES 100                    /* frames each help line lasts (~2 s at 50 fps; provisional) */
#define TGT_COL 16
#define MSG_FRAMES 100                     /* frames a message lasts before the view is shown again (~2 s at 50 fps; provisional) */
#define HYPER_FRAMES 8                     /* frames per countdown step (provisional: ~0.16 s at 50 fps) */
#define TUNNEL_TICKS 1                     /* 50 Hz ticks per unit of TUNNEL_FRAMES (30 = ~0.6 s) */
#define RING_TICKS 2                       /* minimum 50 Hz ticks per tunnel ring (~35 rings: ~1.4 s) */
#define TUNNEL_FRAMES 30                   /* how long the launch or hyperspace tunnel is shown */

static int view = 0;
static int hyper = 0;                      /* countdown (15..1), 0 = none */
static int hyper_gal = 0;                  /* 1 = galactic jump */
static int hyper_tick = 0;
static const char *vnames[4] = { "FRONT", "REAR ", "LEFT ", "RIGHT" };

static void hiss(int y) { snd_sohiss(y); }

static void wait_frames(int n)             /* on the QL lets real time pass reading the keyboard; with a script it consumes no keys */
{
    int i;
    unsigned long t0;
    if (in_script) return;
    t0 = plat_ticks();
    if (t0 == 0UL) { for (i = 0; i < n; i++) in_poll(); return; }
    while (plat_ticks() - t0 < (unsigned long)n * TUNNEL_TICKS) in_poll();     /* own: by time, not by iterations */
}

/* own: each tunnel ring takes at least RING_TICKS 50 Hz ticks (on the real QL slowness drew them one by one) */
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
static char fps_text[9] = "";                /* the FPS that overwrite the end of the help ("" = off) */

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
    panel_invalidate();                                     /* the rectangle below is erased entirely: nothing of the previous content stays painted */
    gfx_rect(0, DASH_Y0 + DASH_LINES, 256, 256 - (DASH_Y0 + DASH_LINES), C_BLACK);
    txt_color(C_CYAN, C_BLACK);
    txt_print_at(0, ROW_HERE, "HERE");
    txt_print_at(TGT_COL, ROW_HERE, "TARGET");
    help_draw();
}

/* value in tenths with a suffix (for example 12345 and " Cr" -> "1234.5 Cr"); returns the length */
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

/* The last thing painted (columns) in each text that changes length: if the new one is shorter, the rectangle of cells left
   over is calculated and emptied at once (gfx_clear_cells: assembly); the rest is overwritten cell by cell, without flicker */
static int len_here = 0, len_tgt = 0;

#ifdef DEBUG
static void panel_invalidate(void) { len_here = len_tgt = 0; }
#else
static void panel_invalidate(void) { len_here = len_tgt = 0; }
#endif

static int cstr_len(const char *s) { int n = 0; while (s[n]) n++; return n; }

static void panel_name(int col, const char *name, int *oldlen)       /* name after its label, on the HERE row */
{
    int n = cstr_len(name);
    if (*oldlen > n) gfx_clear_cells(col + n, ROW_HERE, *oldlen - n, 1);
    txt_print_at(col, ROW_HERE, name);
    *oldlen = n;
}

static void panel_right(int row, const char *s, int *oldlen)         /* value right-aligned (up to column 31) */
{
    int n = cstr_len(s);
    if (*oldlen > n) gfx_clear_cells(32 - *oldlen, row, *oldlen - n, 1);
    txt_print_at(32 - n, row, s);
    *oldlen = n;
}

/* Status row: light years to the destination on the left, credits on the right and the view in the middle ("FRONT VIEW"); a message
   takes it over entirely, centred, until it expires (MSG_FRAMES) */
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
    if (msg_t) {                                            /* the centred message; LY and credits only if they fit at its sides */
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

static void panel_cash(void) { status_draw(); }                     /* the credits go in the status row */

static void panel_update(void)
{
    txt_color(C_WHITE, C_BLACK);
    txt_nowrap(1);                                          /* a long name must not wrap onto the next row */
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
/* SHIFT-F: frames per second on the last row, right-aligned; calculated every second
   (50 ticks of plat_ticks) with one decimal */
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
        for (; i < 8 - n; i++) fps_text[i] = ' ';          /* always 8 characters */
        for (k = 0; k < n && i < 8; k++, i++) fps_text[i] = b[k];
        fps_text[i] = 0;
    }
    help_draw();
    fps_t0 = now; fps_n = 0;
}

/* SHIFT-D: cycles what is skipped (ship_dbg_skip, 3 bits, 8 states): bit 0 = LL9 maths, bit 1 = ship drawing, bit 2 = star
   painting (calculated but not painted). Without maths, each ship repaints the lines it already had twice (same drawing cost, it does not
   move on screen); without drawing, LL9 calculates but does not paint. When the ship or star drawing is changed the view is erased (the line heap
   or the stars no longer match the screen) */
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

/* Pause: "PAUSED" in the middle of the gap between the sight (up to y = 75) and the panel (y = 160): text row 14 (y = 112..119), centred.
   What was underneath (ships, dust) is saved and restored on resume */
#define PAUSE_ROW 14
#define PAUSE_COL 10
#define PAUSE_CELLS 11
static u8 pause_save[16][PAUSE_CELLS * 4];

static void pause_banner(int on)
{
    int r, i;
    u8 *p;
    for (r = 0; r < 16; r++) {
        p = screen + (long)(PAUSE_ROW * 8 + r) * 128L + PAUSE_COL * 4;     /* 128 bytes per line; 4 bytes per 8-dot cell */
        for (i = 0; i < PAUSE_CELLS * 4; i++) { if (on) pause_save[r][i] = p[i]; else p[i] = pause_save[r][i]; }
    }
    if (on) {
        txt_color(C_WHITE, C_BLACK);
        txt_print_at(PAUSE_COL + 3, PAUSE_ROW, "PAUSED");
        txt_print_at(PAUSE_COL, PAUSE_ROW + 1, "O = OPTIONS");
    }
}

/* Pause "Options" window (O key; own): one line per TGINT option with the key and ON/OFF; the keys change them with dks3 */
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

static int mode_changed = 0;                        /* the screen mode was changed during the pause: on resume everything must be repainted */

static int pause_options(void)
{
    int ch;
    u8 x, y;
    clear_view();                                   /* the window covers the whole 3D view */
    opt_draw();
    for (;;) {
        ch = in_poll();
        if (in_quit) return 0;
        if (ch < 0) continue;
        x = (u8)((ch >= 'a' && ch <= 'z') ? ch - 32 : ch);
        if (x == 'Q') return 1;                             /* to the main menu (DEATH2) */
        if (x == 27 || x == '-') return 0;                  /* back to the game */
        for (y = 0; y < 9; y++) if (y != 6) dks3(x, y);     /* K (JSTK) removed: the joystick is always active */
        if (x == 'S') snd_enable(!snd_enabled());
        if (x == 'M') {                                     /* own: from mode 8 to mode 4 (512) and back: the screen is cleared, the panel repainted */
            gfx_hires_opt = (u8)!gfx_hires_opt;
            plat_mode(gfx_hires_opt);
            redraw_panel();
            clear_view();
            mode_changed = 1;
        }
        opt_draw();
    }
}

/* TT66 of the space view: erases the view area (the panel is not touched) */
static void clear_view(void)
{
    gfx_fill_lines(0, VIEW_H, C_BLACK);     /* fast: long words, not pixel by pixel */
    ships_forget_lines();                   /* the lines of the ships, sun and planet are no longer on screen */
    sun_forget();
    laser_forget();                         /* TTX66: LAS2 = 0 */
    chart_sight_forget();
}

/* What LOOK1 and arrival put back after erasing the view: the sight and, if the bomb is exploding, its bolt */
static void view_extras(void)
{
    chart_sight((u8)view);
    if (bomb & 0x80) bomb_off();
}

/* TT66 with the space view (QQ11 = 0): erases the view, BOX and the view name ("FRONT VIEW") at the top */
static void space_tt66(void)
{
    scr_viewdir = (u8)view;
    tt66(0);
}

/* LOOK1 when changing view while in space: the dust is mirrored on the diagonal (FLIP) and the sight is put back */
static void change_view(int v)
{
    if (v == view) return;
    view = v;
    space_view = (u8)view;                  /* the ships are redrawn in the axes of the new view (PLUT) */
    space_tt66();
    stars_flip();
    view_extras();
    panel_update();
}

/* Full flight screen (on launch and on returning from a text screen): panel, text, radar, sight and new dust (NWSTARS) */
static void redraw_panel(void)
{
    gfx_clear(C_BLACK);
    ships_forget_lines();                  /* the screen has been erased: no ship counts as drawn */
    sun_forget();
    laser_forget();
    chart_sight_forget();
    dash_mblb_forget();
    dash_show();                           /* the background comes from memory: the file is read only once */
    space_scanner_reset();
    dash_msblob();
    if (sspr) dash_spblb();
    panel_static();
    panel_update();
    if (ecma) dash_ecblb();                /* the E bulb stays on */
}

static void redraw_all(void)
{
    redraw_panel();
    view_extras();
    stars_new();
}

/* MESS: in-flight messages (row 27, like the port's other notices; the original puts them in the view). With mess_de
   " DESTROYED" is appended (OUCH). The texts are those of the original's tokens */
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
    if (hyper >= 11) { say(start[15 - hyper], 0); return; }     /* the first 5 steps of the countdown (15..11) */
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
        game_tt111();                      /* hyp: TTX110 (or hm on the charts) calls TT111 before TTX111 */
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

/* LL164: hyperspace sound and tunnel (HFS2 with STP = 4); it is shown for a moment and the view is erased */
static void hyper_tunnel(void)
{
    tunnel_hooks();
    tunnel_ll164();
    wait_frames(TUNNEL_FRAMES);
    clear_view();
}

/* MJP: failed jump into the space between systems (witchspace): another tunnel, with no planet, sun or station, three Thargoids with
   their Thargons, MJ = $FF, dust with 2 particles and the galaxy y coordinate with the low 5 bits inverted */
void misjump(void)
{
    hyper_tunnel();                                         /* TT66 + LL164 */
    space_reset();                                          /* RES2 */
    if (ecma) ecm_off();
    mstg = 0xFF;
    ships_clear();
    sspr = 0;
    mj = 0xFF;                                              /* STY MJ */
    do gthg(); while (many[SHIPT_THARGOID] <= 2);           /* MJP1: until there are three */
    stars_nostm = 2;                                        /* STA NOSTM with A = 2 */
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
    hyper_tunnel();                        /* LL164: hyperspace sound and tunnel */
    space_scanner_clear();                 /* removes whatever was on the radar */
    if (!hyper_gal && jump_possible(tgt_sys)) {             /* TT18: fuel is spent before knowing whether we arrive */
        if (((patg & 0x80) && key_down(K_TAB)) || (em_rand_c = 0, em_dornd() >= 253)) {   /* ee5: TAB with PATG (ptg) or 0.78 %: MJP (DORND enters with the C of CMP #9: 0 with no key) */
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
    space_arrive();                        /* new system: planet far away and no station */
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

/* Text screens from flight (keys 5..9 and 0): as in the original; time does not advance while they are shown. Returns 0 or FL_QUIT */
static int flight_screens(int sc)
{
    int pending = 0;
    key_ctx_menu();
    view_set_native(0);                                     /* the text screens and the charts use the original's coordinates */
    plat_mode(0);                                           /* and mode 8, as inside the station */
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
    plat_mode(gfx_hires_opt);                               /* back to the flight view (redraw_all repaints everything) */
    redraw_all();
    if (pending == 1) start_hyper(0);
    else if (pending == 2) start_hyper(1);
    return 0;
}

/* ESCAPE: the pod; we watch our Cobra move away, lose the cargo, the pod and the fugitive status, the tank
   is filled and we appear at the station (GOIN) */
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

/* DEATH: explosion, GAME OVER and the remains of our ship moving away (D1/D2); then DEATH2 returns to the title */
static void flight_death(void)
{
    int f;
    struct controls c;
    snd_death();                                            /* SOEXPL with Y = 210 */
    space_reset();                                          /* RES2 */
    ships_clear();
    zDELTA = (u8)(zDELTA << 2);                             /* ASL DELTA twice */
    space_tt66();                                           /* TT66 with view 0 (VIEW does not change) */
    qq17 = 0x80; dt_mt2(); dtw2 = 0x80;
    stars_new();                                            /* nWq */
    dt_xc = 12; dt_yc = 12;
    dt_ex(146);                                             /* ex: token 146, "GAME OVER" */
    ships_death_debris();                                   /* D1 */
    zDELTA = 0;
    for (f = 0; f <= 64; f++) {                             /* JSR M% and D2: 64 more iterations (LASCT) */
        if (!in_script) in_poll();
        ctl_read(&c);                                       /* M% reads the keyboard (DOKEY): we can turn */
        space_controls(&c);
        ships_step((u8)view);                               /* the original's M%: MCNT stays at 255, no checks */
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
    key_ctx_flight();                                       /* fewer keyboard rows per frame: see keys.c */
    view = 0; hyper = 0;
    player_dead = 0; mess_de = 0; dock_auto = 0; mj = 0; mstg = 0xFF; msar = 0;
    ecma = 0; ecmp = 0;
    stars_nostm = NOST;
    combat_reset();                                         /* DOENTRY: energy, shields and laser as on docking */
    combat_mess_hook = flight_mess;
    combat_mess_off_hook = flight_mess_off;
    redraw_panel();                                         /* the panel (the original always has it on screen) */
    tunnel_hooks();
    tunnel_laun();                                          /* TT110: LAUN (two SOHISS and the tunnel, STP = 8) */
    wait_frames(TUNNEL_FRAMES);
    space_launch();                                         /* RES2, TT111, SOS1, NWSPS, DELTA = 12, FIST */
    enc_ev = 0;
    tunnel_draw(8);                                         /* HFS1 with QQ11 = 255: the same rings (and sounds) erase them */
    space_tt66();                                           /* LOOK1 -> LQ: TT66, SIGHT, BOMBOFF, NWSTARS */
    view_extras();
    stars_new();
    say(dash_loaded() ? "LAUNCHED" : "LAUNCHED (NO ELITE_DASH FILE)", 0);
    last_t = plat_ticks();
    for (;;) {
        /* own: fixed step. The whole simulation counts per frame and the original ran at ~10 FPS or less: if faster,
           it waits until FRAME_TICKS 50 Hz ticks since the previous frame (with no clock or in scripts, it does not) */
        if (!in_script && last_t != 0UL) {
            unsigned long now_t;
            do { now_t = plat_ticks(); } while (now_t - last_t < FRAME_TICKS);
            last_t = now_t;
        }
        ch = in_poll();
        if (ch == KC_QUIT) { key_ctx_menu(); return FL_QUIT; }
        if (ch >= 0) {
#ifdef DEBUG
            if (ch == 'C') { dock_auto = 0xFF; ch = -1; }                          /* SHIFT-C: automatic docking, as with the computer */
            else if (ch == 'P') { snd_off(); key_ctx_menu(); return FL_DOCK; }     /* SHIFT-P: instant docking */
            else if (ch == 'J') {                                                  /* SHIFT-J: the station appears close, ahead */
                if (ships_dbg_near_station()) say("STATION NEARBY", 0); else say("NO STATION HERE", 1);
                ch = -1;
            }
            else if (ch == 'H') { dbg_range = 1; ch = 'h'; }                       /* SHIFT-H: jumps even if the fuel is not enough */
            else if (ch == 'F') { dbg_fps_key(); ch = -1; }                        /* SHIFT-F: frames-per-second counter */
            else if (ch == 'D') { dbg_skip_key(); ch = -1; }                       /* SHIFT-D: skip maths / ship drawing */
#endif
            ch = in_up(ch);
            if (ch == 'c') {                                /* KY19: docking computer if we carry it (MA64: STA auto) */
                if (dkcmp) dock_auto = dkcmp;
                else if (flight_testkeys && sspr) { key_ctx_menu(); return FL_DOCK; }    /* tests: immediate docking */
                ch = -1;
            }
            if (ch == '=') {                                /* DK4: pause (FREEZE) and configuration keys */
                key_ctx_menu();
                pause_banner(1);
                config_options_hook = pause_options;
                config_options_used = 0;
                r = freeze();
                pause_banner(0);
                if (mode_changed) { mode_changed = 0; redraw_all(); }                      /* another screen mode: the panel and the view are painted again (what was saved from PAUSED is no longer valid) */
                else if (config_options_used) { clear_view(); stars_new(); view_extras(); }    /* the window covered the view */
                if (r) { snd_off(); return FL_DEATH; }    /* ESC: DEATH2 */
                if (in_quit) return FL_QUIT;
                key_ctx_flight();
                ch = -1;
            }
            if (ch == 'u') { missile_unarm(); ch = -1; }   /*  */
            if (ch == 't') { missile_target(); ch = -1; }  /* KY14: MA20..MA25 */
            if (ch == 'm') { missile_fire(); ch = -1; }    /* KY16: MA25..MA24 */
            if (ch == 'e') { ecm_key(); ch = -1; }         /* KY17 */
            if (ch == 'p') { dock_auto = 0; ch = -1; }     /* KY20: MA76..MA78 */
            if (ch == 'b') { bomb_key(); ch = -1; }        /* KY12: MA24..MA76 */
            if (ch == 'j') {                                /* KY18: WARP */
                if (warp()) { mcnt = 1; enc_ev = 0; clear_view(); stars_new(); view_extras(); }
                ch = -1;
            }
            if (ch == KC_ESC) {                             /* KY13: escape pod (not in hyperspace) */
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
        dokey_auto(&c);                                     /* DOKEY: the docking computer presses for us */
        space_controls(&c);                                 /* ALPHA, BETA and speed */
        laser_fire((u8)view, c.fire);                       /* part 3: firing (LASLI) */
        ev = space_step();                                  /* ships (HITCH, damage, collisions), radar, energy, part 15 */
        if (ev & SPACE_EV_DOCK) { snd_off(); key_ctx_menu(); return FL_DOCK; }       /* through the slot: DOENTRY */
        if (ev & SPACE_EV_DEATH) { flight_death(); key_ctx_menu(); return FL_DEATH; }
        bomb_frame();                                       /* part 16 (MA18): the bomb keeps exploding */
        ecm_frame();                                        /* parte 16 (MA16..MA66): ECM */
        laser_frame_end();                                  /* part 16: end of the pulse (LASLI2) */
        stars_set_speed(zDELTA);                            /* DELT4 */
        stars_update((u8)view);                             /* STARS */
        dly_tick();                                         /* TT100: the in-flight message expires */
        help_tick();                                        /* the help line changes every ~2 s */
        msg_tick();                                         /* the message expires and the view returns */
        mcnt--;                                             /* me3: DEC MCNT; at 0, parts 2-4 (ships appear) */
        if (mcnt == 0) encounters_tick(em_rand_c);          /* carry: that of the last DORND (the 6502's comes from M%) */
        laser_cool();                                       /* MLOOP: cooling and pulse count */
        dash_update();
        dash_mblb(warp_blocked());                          /* own: red M with the J jump blocked */
        chart_sight_update((u8)view);                       /* own: cross in the sight with the station near (S) */
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

/* Entering the station (own): before the information screens, ~2 s of a tunnel of at most 3 equally spaced rectangles
   (those of the exit, with the view's proportion: half-height = 5/8 of the half-width) that approach and slow down until they stop. Perspective:
   the rectangle at distance z (1 = the edge of the view) has half-width 128 / z; the three distances are 1 + u, 2 + u and 3 + u with u in
   [0, 1) decreasing; when u passes 0 the nearest leaves the view and another far one enters (always 3). The travel follows a braking
   curve (1 - (1 - t)^3) up to 2.5 spacings. Time goes by the 50 Hz clock (on a slow QL there would be few frames): with no clock
   (NOTICK) DOCK_FRAMES frames are counted. It is not done in scripted tests. */
#define DOCK_TICKS 100                     /* 2 s a 50 Hz */
#define DOCK_FRAMES 100                    /* frames of the animation if there is no clock (NOTICK) */
#define DOCK_TRAVEL 640L                   /* total travel in 1/256 of a spacing (2.5) */

/* Half-widths of the three rectangles at instant t (0..256 = from 0 to 2 s) */
static void dock_rects(long t, int *hw)
{
    long r = 256L - t, d, u;
    int i;
    d = DOCK_TRAVEL * (256L - (r * r * r) / 65536L) / 256L;        /* travel in 1/256 of a spacing (braking curve) */
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
    clk = (t0 != 0UL);                                              /* with no clock (NOTICK) time is counted in frames */
    clear_view();
    for (;;) {
        t = clk ? (long)(plat_ticks() - t0) * 256L / DOCK_TICKS : (long)frames * 256L / DOCK_FRAMES;
        if (t > 256L) t = 256L;
        dock_rects(t, hw);
        for (i = 0; i < 3; i++) {                                   /* each rectangle: the old is erased and the new drawn consecutively */
            if (old_hw[i]) dock_frame(128, old_hw[i]);
            if (t < 256L) dock_frame(128, hw[i]);
            old_hw[i] = (t < 256L) ? hw[i] : 0;
        }
        if (t >= 256L) break;
        in_poll();                                                  /* the frame passes (clock, keyboard, 50 fps) */
        frames++;
    }
    clear_view();
}

/* Flight works with the native 160-line view (view_set_native): it is switched on on entry and off on exit (title, docked and charts do not use it) */
int flight_run(void)
{
    int r;
    msg_t = 0; help_i = 0; help_t = 0;
    plat_mode(gfx_hires_opt);                               /* the flight view uses mode 4 if the 512 option is on */
    view_set_native(1);
    ships_spblb_hook = dash_spblb;                          /* NWSPS and KS4 switch the S on and off */
    r = flight_loop();
    if (r == FL_DOCK) dock_tunnel();                       /* entering the station */
    ships_spblb_hook = 0;
    view_set_native(0);
    if (r != FL_DEATH && r != FL_QUIT) plat_mode(0);        /* docked: mode 8 (after death comes the title, in mode 4) */
    return r;
}
