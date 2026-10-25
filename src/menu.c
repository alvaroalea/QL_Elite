#ifndef ELITE_VERSION
#define ELITE_VERSION "pre0.2"    /* normalmente lo pone el Makefile (VERSION) */
#endif
#include "menu.h"
#include "text.h"
#include "input.h"
#include "game.h"
#include "state.h"
#include "cmdr.h"
#include "save.h"
#include "sound.h"
#include "shipview.h"
#include "chart.h"
#include "sun.h"
#include "emath.h"
#include "missions.h"
#include "detok.h"
#include "config.h"
#include "dash.h"

#define ROW_MSG 28

static int edit_text(int row, const char *prompt, char *buf, int maxlen);

/* ---------------------------------------------------------------- utilidades */

void ui_clear(void)
{
    gfx_clear(C_BLACK);
    txt_color(C_WHITE, C_BLACK);
    txt_at(0, 0);
}

static int slen(const char *s) { int n = 0; while (s[n]) n++; return n; }

static void ui_title_bg(const char *t, u8 bg)
{
    int n = slen(t), col;
    txt_color(C_YELLOW, bg);
    txt_clear_rows(0, 1, bg);
    col = (TXT_COLS - n) / 2;
    if (col < 0) col = 0;
    txt_print_at(col, 0, t);
    txt_color(C_WHITE, C_BLACK);
}

void ui_title(const char *t) { ui_title_bg(t, C_BLUE); }

/* titulo de las cartas: como el de STATUS y DATA (fila 1, amarillo sobre negro, centrado); la linea azul de debajo la dibuja la carta */
static void ui_title_row1(const char *t)
{
    int col = (TXT_COLS - slen(t)) / 2;
    txt_color(C_YELLOW, C_BLACK);
    txt_print_at(col < 0 ? 0 : col, 1, t);
    txt_color(C_WHITE, C_BLACK);
}

void ui_footer(void)
{
    txt_color(C_CYAN, C_BLACK);
    if (qq12) {
        txt_print_at(0, 30, "1BUY 2SELL 3EQUIP 5GAL 6LOCAL");
        txt_print_at(0, 31, "7DATA 8MKT 9STAT 0INV IDISK UGO");
    } else {                                           /* en vuelo: 1-4 vistas, 5-9 pantallas (como en el original) */
        txt_print_at(0, 30, "1-4 VIEW 5GAL 6LOCAL 7DATA");
        txt_print_at(0, 31, "8MKT 9STAT 0INV H/G JUMP ESC");
    }
    txt_color(C_WHITE, C_BLACK);
}

/* Cifras decimales de v en b (sin ceros a la izquierda, al menos una); devuelve cuantas. Restas de potencias de 10 en vez de dividir:
   en el 68008 la division de 32 bits es una rutina de libreria y costaba mas que dibujar el digito */
int dec_digits(unsigned long v, char *b)
{
    static const unsigned long p10[10] = { 1000000000UL, 100000000UL, 10000000UL, 1000000UL, 100000UL, 10000UL, 1000UL, 100UL, 10UL, 1UL };
    int k, n = 0;
    char d;
    for (k = 0; k < 10; k++) {
        d = 0;
        while (v >= p10[k]) { v -= p10[k]; d++; }
        if (d || n || k == 9) b[n++] = (char)('0' + d);
    }
    return n;
}

void ui_num(unsigned long v, int width)
{
    char b[24];
    int n = dec_digits(v, b + 12), k = 0, pad;
    for (pad = n; pad < width && k < 12; pad++) b[k++] = ' ';
    for (pad = 0; pad < n; pad++) b[k++] = b[12 + pad];
    b[k] = 0;
    txt_puts(b);                                                  /* de una vez: txt_putc por caracter es mucho mas lento */
}

void ui_tenths(unsigned long v)
{
    char b[24];
    int n = dec_digits(v, b + 12), k = 0;
    if (n < 2) { b[13] = b[12]; b[12] = '0'; n = 2; }              /* 5 -> "0.5" */
    while (k < n - 1) { b[k] = b[12 + k]; k++; }
    b[k++] = '.';
    b[k++] = b[12 + n - 1];
    b[k] = 0;
    txt_puts(b);
}

void ui_msg(const char *s, int error)
{
    txt_clear_rows(ROW_MSG, 1, C_BLACK);
    txt_color(error ? C_RED : C_GREEN, C_BLACK);
    txt_print_at(0, ROW_MSG, s);
    txt_color(C_WHITE, C_BLACK);
    if (error) snd_boop(); else snd_beep();
}

static void lbl(int row, const char *k)
{
    txt_color(C_WHITE, C_BLACK);
    txt_print_at(0, row, k);
}

static void val(int col, int row)
{
    txt_color(C_GREEN, C_BLACK);
    txt_at(col, row);
}

int ui_nav(int ch)
{
    ch = in_up(ch);
    if (!qq12) {                                       /* en vuelo */
        switch (ch) {
        case '1': return SC_VIEW0;
        case '2': return SC_VIEW1;
        case '3': return SC_VIEW2;
        case '4': return SC_VIEW3;
        case '5': return SC_GALCHART;                  /* 5-0 iguales en vuelo y atracado */
        case '6': return SC_LOCCHART;
        case '7': return SC_DATA;
        case '8': return SC_MARKET;
        case '9': return SC_STATUS;
        case '0': return SC_INVENTORY;
        case 'h': return SC_HYPER;
        case 'g': return SC_GHYPER;
#ifdef DEBUG
        case '!': return SC_DBG_BUY;                   /* SHIFT-1/2/3 (el UK de los menus) */
        case '@': return SC_DBG_SELL;
        case '#': return SC_DBG_EQUIP;
#endif
        }
        return -1;
    }
    switch (ch) {
#ifdef DEBUG
    case '!': return SC_DBG_BUY;
    case '@': return SC_DBG_SELL;
    case '#': return SC_DBG_EQUIP;
#endif
    case 'u': return SC_LAUNCH;                        /* U = undock */
    case '1': return SC_BUY;
    case '2': return SC_SELL;
    case '3': return SC_EQUIP;                         /* 4 no hace nada atracado (son las vistas del espacio) */
    case '5': return SC_GALCHART;                      /* 5-0 iguales en vuelo y atracado */
    case '6': return SC_LOCCHART;
    case '7': return SC_DATA;
    case '8': return SC_MARKET;
    case '9': return SC_STATUS;
    case '0': return SC_INVENTORY;
    case 'i': return SC_DISK;
    case 'h': case 'g': ui_msg("DOCKED", 1); return -1;    /* dockEd: no se puede saltar atracado */
    }
    return -1;
}

/* ESC: atracado vuelve al estado; en vuelo, a la vista espacial */
static int back_sc(void) { return qq12 ? SC_STATUS : SC_FLIGHT; }

static char unit_char(int good)
{
    static const char u[3] = { 't', 'k', 'g' };
    return u[uni_goods[good].units];
}

/* Espera una tecla de navegacion (u otra tecla que la pantalla no use). Devuelve SC_* o SC_QUIT; ESC vuelve al estado. */
static int wait_nav(void)
{
    int ch, sc;
    for (;;) {
        ch = in_wait();
        if (ch == KC_QUIT) return SC_QUIT;
        if (ch == KC_ESC) return back_sc();
        sc = ui_nav(ch);
        if (sc >= 0) return sc;
    }
}

/* ---------------------------------------------------------------- estado */

int scr_status(void)
{
    missions_set_ink(C_WHITE);
    missions_set_title_ink(C_YELLOW);
    status_mode();
    missions_set_title_ink(0);                                     /* STATUS del original con el motor de fichas */
    txt_color(C_WHITE, C_BLACK);
    ui_footer();
    return wait_nav();
}

/* ---------------------------------------------------------------- inventario y mercado */

int scr_inventory(void)
{
    int i, row = 7;
    ui_clear();
    ui_title("INVENTORY");
    lbl(2, "Fuel:");  val(7, 2); ui_tenths(qq14); txt_puts(" Light Years");
    lbl(3, "Cash:");  val(7, 3); ui_tenths(cash_get()); txt_puts(" Cr");
    lbl(4, "Hold:");  val(7, 4); ui_num((unsigned long)trade_hold_t(), 0); txt_putc('/'); ui_num((unsigned long)(crgo - 2), 0); txt_puts(" t");
    txt_color(C_CYAN, C_BLACK);
    txt_print_at(0, 6, "CARGO:");
    txt_color(C_WHITE, C_BLACK);
    for (i = 0; i < NGOODS; i++) {
        if (!qq20[i]) continue;
        txt_at(1, row); txt_puts(uni_goods[i].name);
        txt_at(16, row); ui_num(qq20[i], 3); txt_putc(' '); txt_putc(unit_char(i));
        row++;
    }
    if (row == 7) txt_print_at(1, 7, "(empty)");
    ui_footer();
    return wait_nav();
}

static void market_header(void)
{
    txt_color(C_YELLOW, C_BLUE);
    txt_print_at(0, 2, "PRODUCT      U  PRICE  QTY HOLD");
    txt_color(C_WHITE, C_BLACK);
}

static void market_row(int i, int row, int hl)
{
    txt_color(hl ? C_BLACK : C_WHITE, hl ? C_YELLOW : C_BLACK);
    txt_clear_rows(row, 1, hl ? C_YELLOW : C_BLACK);
    txt_at(0, row); txt_puts(uni_goods[i].name);
    txt_at(13, row); txt_putc(unit_char(i));
    txt_at(15, row); ui_num(mkt.price[i] / 10, 3); txt_putc('.'); txt_putc((char)('0' + (int)(mkt.price[i] % 10)));
    txt_at(23, row);
    if (mkt.quantity[i]) ui_num(mkt.quantity[i], 3); else txt_puts("  -");
    txt_at(28, row);
    if (qq20[i]) ui_num(qq20[i], 3); else txt_puts("  -");
    txt_color(C_WHITE, C_BLACK);
}

int scr_market(void)
{
    int i;
    char t[28];
    int n = 0;
    const char *p = "MARKET PRICES: ";
    while (*p) t[n++] = *p++;
    for (p = uni_galaxy[cur_sys].name; *p && n < 27; p++) t[n++] = *p;
    t[n] = 0;
    ui_clear();
    ui_title(t);
    market_header();
    for (i = 0; i < NGOODS; i++) market_row(i, 3 + i, 0);
    lbl(22, "Cash:"); val(7, 22); ui_tenths(cash_get()); txt_puts(" Cr");
    ui_footer();
    return wait_nav();
}

/* ---------------------------------------------------------------- compra y venta (lista con cursor) */

static void trade_info(int buying, int sel, const char *entry)
{
    txt_clear_rows(21, 6, C_BLACK);
    lbl(21, "Cash:"); val(7, 21); ui_tenths(cash_get()); txt_puts(" Cr");
    lbl(22, "Hold:"); val(7, 22); ui_num((unsigned long)trade_hold_t(), 0); txt_putc('/'); ui_num((unsigned long)(crgo - 2), 0); txt_puts(" t");
    txt_color(C_CYAN, C_BLACK);
    txt_print_at(0, 24, buying ? "BUY " : "SELL ");
    txt_puts(uni_goods[sel].name);
    txt_puts(": ");
    txt_color(C_WHITE, C_BLACK);
    txt_puts(entry);
    txt_puts("_");
    txt_color(C_CYAN, C_BLACK);
    txt_print_at(0, 26, buying ? "UP/DOWN ENTER=BUY M=MAX ESC=EXIT" : "UP/DOWN ENTER=SELL M=ALL ESC=EXIT");
    txt_color(C_WHITE, C_BLACK);
}

static int trade_screen(int buying, int dbg)
{
    int sel = 0, ch, i, nlen = 0, r;
    unsigned qty;
    char entry[4];
    entry[0] = 0;
    ui_clear();
    if (dbg) ui_title_bg(buying ? "BUY CARGO (DEBUG: FREE)" : "SELL CARGO (DEBUG: KEEP)", C_RED);
    else ui_title(buying ? "BUY CARGO" : "SELL CARGO");
    market_header();
    for (i = 0; i < NGOODS; i++) market_row(i, 3 + i, i == sel);
    trade_info(buying, sel, entry);
    for (;;) {
        ch = in_wait();
        if (ch == KC_QUIT) return SC_QUIT;
        if (ch == KC_ESC) return SC_STATUS;
        if (ch == KC_UP || ch == KC_DOWN) {
            int old = sel;
            sel += (ch == KC_DOWN) ? 1 : -1;
            if (sel < 0) sel = NGOODS - 1;
            if (sel >= NGOODS) sel = 0;
            market_row(old, 3 + old, 0);
            market_row(sel, 3 + sel, 1);
            nlen = 0; entry[0] = 0;
            trade_info(buying, sel, entry);
        } else if (ch >= '0' && ch <= '9') {
            if (nlen < 3) { entry[nlen++] = (char)ch; entry[nlen] = 0; }
            trade_info(buying, sel, entry);
        } else if (ch == KC_BS) {
            if (nlen > 0) entry[--nlen] = 0;
            trade_info(buying, sel, entry);
        } else if (ch == KC_ENTER || in_up(ch) == 'm') {
            if (in_up(ch) == 'm') qty = buying ? trade_max_buy(sel) : qq20[sel];
            else if (nlen == 0) qty = 1;
            else { qty = 0; for (i = 0; i < nlen; i++) qty = qty * 10 + (unsigned)(entry[i] - '0'); }
            if (qty == 0) { ui_msg(buying ? "NOTHING TO BUY" : "NOTHING TO SELL", 1); continue; }
            r = buying ? trade_buy(sel, qty) : trade_sell(sel, qty);
            nlen = 0; entry[0] = 0;
            for (i = 0; i < NGOODS; i++) market_row(i, 3 + i, i == sel);
            trade_info(buying, sel, entry);
            switch (r) {
            case TR_OK:      ui_msg(buying ? "BOUGHT" : "SOLD", 0); break;
            case TR_NOCASH:  ui_msg("NOT ENOUGH CASH", 1); break;
            case TR_NOROOM:  ui_msg("CARGO HOLD FULL", 1); break;
            case TR_NOSTOCK: ui_msg(buying ? "NOT ENOUGH IN STOCK" : "NOT ENOUGH IN HOLD", 1); break;
            default:         ui_msg("NO", 1); break;
            }
        }
    }
}

int scr_buy(void)  { return trade_screen(1, 0); }
int scr_sell(void) { return trade_screen(0, 0); }

#ifdef DEBUG
/* DEBUG: compra gratis, venta sin quitar carga y equipo completo gratis (titulo en rojo) */
static int scr_dbg_trade(int buying)
{
    int r;
    dbg_free = 1;
    r = trade_screen(buying, 1);
    dbg_free = 0;
    return r;
}

static int equip_screen(int dbg);

static int scr_dbg_equip(void)
{
    int r;
    dbg_free = 1;
    r = equip_screen(1);
    dbg_free = 0;
    return r;
}
#endif

/* ---------------------------------------------------------------- equipo */

static void equip_row(int i, int row, int hl)
{
    txt_color(hl ? C_BLACK : C_WHITE, hl ? C_YELLOW : C_BLACK);
    txt_clear_rows(row, 1, hl ? C_YELLOW : C_BLACK);
    txt_at(0, row); ui_num((unsigned long)(i + 1), 2);
    txt_at(3, row); txt_puts(eq_names[i]);
    txt_at(23, row); ui_tenths(equip_price(i)); txt_puts(" Cr");
    txt_color(C_WHITE, C_BLACK);
}

static int equip_screen(int dbg)
{
    int n = equip_count(), sel = 0, ch, i, r, view;
    static const char *vn[4] = { "FRONT", "REAR", "LEFT", "RIGHT" };
    ui_clear();
    if (dbg) ui_title_bg("EQUIP SHIP (DEBUG: FREE)", C_RED);
    else ui_title("EQUIP SHIP");
    for (;;) {
        for (i = 0; i < n; i++) equip_row(i, 2 + i, i == sel);
        txt_clear_rows(21, 8, C_BLACK);
        lbl(21, "Cash:"); val(7, 21); ui_tenths(cash_get()); txt_puts(" Cr");
        txt_color(C_CYAN, C_BLACK);
        txt_print_at(0, 24, "UP/DOWN ENTER=BUY 5-0=SCREENS");
        txt_color(C_WHITE, C_BLACK);
        for (;;) {
            ch = in_wait();
            if (ch == KC_QUIT) return SC_QUIT;
            if (ch == KC_ESC) return SC_STATUS;
            if (ch == KC_UP || ch == KC_DOWN) {
                int old = sel;
                sel += (ch == KC_DOWN) ? 1 : -1;
                if (sel < 0) sel = n - 1;
                if (sel >= n) sel = 0;
                equip_row(old, 2 + old, 0);
                equip_row(sel, 2 + sel, 1);
            } else if (ch == KC_ENTER) break;
            else {                                     /* las teclas de pantalla (1-9, 0, U, I...) navegan sin pasar por ESC */
                int sc = ui_nav(ch);
                if (sc >= 0 && sc != SC_EQUIP) return sc;
            }
        }
        view = 0;
        if (equip_is_laser(sel)) {
            txt_clear_rows(26, 3, C_BLACK);
            for (;;) {                                 /* vista con los cursores: izquierda/derecha (o arriba/abajo) y ENTER */
                txt_color(C_CYAN, C_BLACK);
                txt_print_at(0, 26, "VIEW? <> ENTER ESC:");
                txt_color(C_YELLOW, C_BLACK);
                txt_print_at(20, 26, "      ");
                txt_print_at(20, 26, vn[view]);
                txt_color(C_WHITE, C_BLACK);
                ch = in_wait();
                if (ch == KC_QUIT) return SC_QUIT;
                if (ch == KC_ESC) { view = -1; break; }
                if (ch == KC_ENTER) break;
                if (ch == KC_RIGHT || ch == KC_DOWN) view = (view + 1) & 3;
                else if (ch == KC_LEFT || ch == KC_UP) view = (view + 3) & 3;
                else if (ch >= '0' && ch <= '3') { view = ch - '0'; break; }
            }
            if (view < 0) continue;
        }
        r = equip_buy(sel, view);
        txt_clear_rows(26, 3, C_BLACK);
        for (i = 0; i < n; i++) equip_row(i, 2 + i, i == sel);
        lbl(21, "Cash:"); val(7, 21); txt_puts("               ");
        val(7, 21); ui_tenths(cash_get()); txt_puts(" Cr");
        if (r == EQ_OK) {
            txt_clear_rows(ROW_MSG, 1, C_BLACK);
            txt_color(C_GREEN, C_BLACK);
            txt_print_at(0, ROW_MSG, eq_names[sel]);
            if (equip_is_laser(sel)) { txt_puts(" ("); txt_puts(vn[view]); txt_putc(')'); }
            txt_puts(" FITTED");
            txt_color(C_WHITE, C_BLACK);
            snd_beep();
        } else if (r == EQ_NOCASH) ui_msg("NOT ENOUGH CASH", 1);
        else if (r == EQ_PRESENT) ui_msg("ALREADY FITTED", 1);
        else ui_msg("NO", 1);
        if (sel >= 0 && n != equip_count()) n = equip_count();
    }
}

int scr_equip(void) { return equip_screen(0); }

/* ---------------------------------------------------------------- datos del sistema */

int scr_data(void)
{
    unsigned d;
    game_tt111();                                      /* TT102 (f6): TT111 y TT25 */
    d = jump_distance(tgt_sys);
    qq8[0] = (u8)d; qq8[1] = (u8)(d >> 8);
    missions_set_ink(C_WHITE);
    missions_set_title_ink(C_YELLOW);
    tt25(tgt_sys);
    missions_set_title_ink(0);                                     /* TT25 del original con el motor de fichas (y PDESC) */
    txt_color(C_WHITE, C_BLACK);
    ui_footer();
    return wait_nav();
}

/* ---------------------------------------------------------------- cartas */


static void chart_info(int local)
{
    unsigned d = jump_distance(tgt_sys);
    int r0 = local ? 21 : 19;                          /* la carta local es mas grande: el texto baja 3 filas; la galactica 1 (el marco acaba en y = 143) */
    txt_clear_rows(r0, local ? 5 : 6, C_BLACK);
    txt_color(C_CYAN, C_BLACK);
    txt_print_at(0, r0, "Target:"); txt_color(C_WHITE, C_BLACK); txt_puts(" "); txt_puts(uni_galaxy[tgt_sys].name);
    txt_color(C_CYAN, C_BLACK);
    txt_print_at(0, r0 + 1, "Distance:"); txt_color(C_WHITE, C_BLACK); txt_puts(" "); ui_tenths(d); txt_puts(" LY");
    if (tgt_sys == cur_sys) { txt_color(C_YELLOW, C_BLACK); txt_puts("  (HERE)"); }
    else if (d > qq14) { txt_color(C_RED, C_BLACK); txt_puts("  OUT OF RANGE"); }
    else { txt_color(C_GREEN, C_BLACK); txt_puts("  IN RANGE"); }
    txt_color(C_CYAN, C_BLACK);
    txt_print_at(0, r0 + 2, "Fuel:"); txt_color(C_WHITE, C_BLACK); txt_puts(" "); ui_tenths(qq14); txt_puts(" LY");
    txt_color(C_CYAN, C_BLACK);
    txt_print_at(0, r0 + 4, qq12 ? "CURSORS O=HOME D=DIST F=FIND" : "CURSORS=MOVE O=HOME D=DIST");
    txt_color(C_WHITE, C_BLACK);
}

/* El dibujo de las cartas esta en chart.c (TT22/TT23/TT14/TT103/TT105 con SUN y CIRCLE). Aqui, el teclado: la mira se mueve con
   TT16/TT123 (QQ9, QQ10 libres, como el original) y el objetivo que se muestra abajo es el sistema mas cercano (TT111; propio). */
static int scr_chart_run(int local)
{
    int ch, dx, dy, sc, step = local ? 1 : 2, idx;
    ui_clear();
    ui_title_row1(local ? "SHORT RANGE CHART" : "GALACTIC CHART");
    chart_draw(local);
    ui_footer();
    chart_info(local);
    for (;;) {
        dx = dy = 0;
        ch = in_poll();
        if (ch == KC_QUIT) return SC_QUIT;
        if (ch == KC_ESC) return back_sc();
        if (ch == KC_LEFT) dx = -step; else if (ch == KC_RIGHT) dx = step;
        else if (ch == KC_UP) dy = -step; else if (ch == KC_DOWN) dy = step;
        else if (ch < 0) {
            if (in_held(KC_LEFT)) dx = -step; else if (in_held(KC_RIGHT)) dx = step;
            if (in_held(KC_UP)) dy = -step; else if (in_held(KC_DOWN)) dy = step;
        } else if (in_up(ch) == 'o') {                          /* O: ping, el cursor vuelve al sistema actual */
            chart_ping(local);
            chart_info(local);
            continue;
        } else if (in_up(ch) == 'd') {                          /* D: hm, sistema mas cercano a la cruz, con nombre y distancia */
            chart_hm(local);
            chart_info(local);
            continue;
        } else if (in_up(ch) == 'f' && qq12) {                  /* F: HME2, buscar un sistema por nombre (solo atracado) */
            char name[9];
            int k, found;
            for (k = 0; k < 9; k++) name[k] = 0;
            k = edit_text(26, "PLANET NAME?", name, 8);
            txt_clear_rows(26, 2, C_BLACK);
            if (k > 0) {
                found = chart_find_system(name);
                if (found >= 0) {
                    chart_cursor(local);
                    game_set_target(found);              /* HME5: QQ9/QQ10 del encontrado */
                    game_tt111();                        /* y TT111 */
                    chart_cursor(local);
                    chart_info(local);
                    ui_msg("FOUND", 0);
                } else {
                    chart_info(local);
                    ui_msg("UNKNOWN PLANET", 1);
                }
            } else if (in_quit) return SC_QUIT;
            else chart_info(local);
            continue;
        } else {
            sc = ui_nav(ch);
            if (sc >= 0) return sc;
        }
        if (dx || dy) {
            chart_tt16(local, (u8)dx, (u8)(-dy));               /* TT16: la mira se mueve sola (QQ9, QQ10), sin pegarse a un sistema */
            idx = uni_nearest(qq9, qq10);                       /* propio: el objetivo que daria TT111, para la informacion de abajo */
            if (idx != tgt_sys) {
                game_target_only(idx);
                chart_info(local);
            }
        }
    }
}

/* la carta se dibuja con la transformacion de pantalla (sun.c: local mas ancha que alta y centrada, galactica 8 puntos abajo) */
int scr_chart(int local)
{
    int r;
    chart_xf_set(local ? 1 : 2);
    r = scr_chart_run(local);
    chart_xf_set(0);
    return r;
}

/* ---------------------------------------------------------------- nombre y disco */

/* Editor de linea: devuelve la longitud (0 = cancelado con ESC). buf >= maxlen + 1; admite letras, cifras, _ y - (en mayusculas) */
static int edit_text(int row, const char *prompt, char *buf, int maxlen)
{
    int n = 0, ch;
    txt_clear_rows(row, 2, C_BLACK);
    txt_color(C_CYAN, C_BLACK);
    txt_print_at(0, row, prompt);
    txt_color(C_WHITE, C_BLACK);
    while (buf[n]) n++;
    txt_at(0, row + 1); txt_puts("> "); txt_puts(buf); txt_puts("_\b");          /* raya como cursor */
    for (;;) {
        ch = in_wait();
        if (ch == KC_QUIT) { txt_puts(" \b"); in_quit = 1; return 0; }
        if (ch == KC_ESC) { txt_puts(" \b"); return 0; }
        if (ch == KC_ENTER) { txt_puts(" \b"); return n; }
        if (ch == KC_BS) {
            if (n > 0) { n--; buf[n] = 0; txt_puts(" \b\b_\b"); }
        } else if (n < maxlen &&
                   ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_' || ch == '-')) {
            if (ch >= 'a' && ch <= 'z') ch = ch - 'a' + 'A';
            buf[n++] = (char)ch; buf[n] = 0;
            txt_putc(ch); txt_puts("_\b");
        }
    }
}

static int edit_name(int row, const char *prompt, char *buf) { return edit_text(row, prompt, buf, CMDR_NAMELEN); }

static void disk_draw(void)
{
    ui_clear();
    ui_title("DISK ACCESS MENU");
    lbl(2, "Commander:"); val(11, 2); txt_puts(cmdr_name);
    lbl(3, "Drive:");     val(11, 3); txt_puts(save_dev[0] ? save_dev : "(here)");
    txt_color(C_WHITE, C_BLACK);
    txt_print_at(2, 6, "1  LOAD COMMANDER");
    txt_print_at(2, 7, "2  SAVE COMMANDER");
    txt_print_at(2, 8, "3  CHANGE DRIVE/PATH");
    txt_print_at(2, 9, "4  RESET TO DEFAULT JAMESON");
    txt_print_at(2, 10, "5  RETURN TO GAME");
    txt_print_at(2, 11, "Q  QUIT GAME");
    txt_color(C_CYAN, C_BLACK);
    txt_print_at(0, 13, "ESC = BACK");
}

int scr_disk(void)
{
    char name[CMDR_NAMELEN + 1], path[SAVE_DEV_MAX + 1];
    int ch, r, i;
    disk_draw();
    for (;;) {
        ch = in_wait();
        if (ch == KC_QUIT) return SC_QUIT;
        if (ch == KC_ESC) return back_sc();
        ch = in_up(ch);
        if (ch == '5') return back_sc();                     /* volver al juego sin cargar ni guardar */
        if (ch == '1') {
            for (i = 0; i < CMDR_NAMELEN + 1; i++) name[i] = 0;
            if (edit_name(15, "LOAD WHICH COMMANDER?", name) > 0) {
                snd_off();                                       /* BEEP durante E/S de disco puede colgar el QL */
                ui_msg("LOADING...", 0);
                r = load_commander(name);
                if (r == SAVE_OK) { game_after_load(); return SC_STATUS; }
                disk_draw();
                ui_msg(save_errstr(r), 1);
            } else if (in_quit) return SC_QUIT; else disk_draw();
        } else if (ch == '2') {
            for (i = 0; i < CMDR_NAMELEN + 1; i++) name[i] = cmdr_name[i];
            if (edit_name(15, "SAVE COMMANDER AS:", name) > 0) {
                cmdr_set_name(name);
                snd_off();
                ui_msg("SAVING...", 0);
                save_seed = (u8)(em_dornd() | 1);
                r = save_commander();
                disk_draw();
                if (r == SAVE_OK) ui_msg("COMMANDER SAVED", 0); else ui_msg(save_errstr(r), 1);
            } else if (in_quit) return SC_QUIT; else disk_draw();
        } else if (ch == '3') {
            for (i = 0; i < SAVE_DEV_MAX + 1; i++) path[i] = save_dev[i];
            if (edit_text(15, "DEVICE (E.G. WIN1_ OR FLP1_DIR_):", path, SAVE_DEV_MAX) > 0) {
                disk_draw();
                if (save_valid_device(path)) save_set_device(path);
                else ui_msg("BAD PATH: AAA1_[TEXT_]", 1);
            } else if (in_quit) return SC_QUIT;
            disk_draw();
        } else if (ch == '4') {
            txt_clear_rows(15, 3, C_BLACK);
            txt_color(C_RED, C_BLACK);
            txt_print_at(0, 15, "RESET TO JAMESON? (Y/N)");
            txt_color(C_WHITE, C_BLACK);
            ch = in_up(in_wait());
            if (ch == 'y') { game_new(); return SC_STATUS; }
            disk_draw();
        } else if (ch == 'q') {
            txt_clear_rows(15, 3, C_BLACK);
            txt_color(C_RED, C_BLACK);
            txt_print_at(0, 15, "QUIT TO QDOS? (Y/N)");
            txt_color(C_WHITE, C_BLACK);
            ch = in_up(in_wait());
            if (ch == 'y') return SC_QUIT;
            disk_draw();
        }
    }
}

/* ---------------------------------------------------------------- titulo */

static u8 title_heap[SHIPVIEW_HEAP];

#define TITLE_Z 506L                             /* distancia final del Cobra Mk III (como el original: 1 * 256 + 250) */
#define TITLE_FAR 4096L                          /* distancia de la que parte cada nave */
#define TITLE_HOLD 500                           /* fotogramas que dura cada modelo (~10 s a 50 fps; provisional) */
#define TITLE_CY 92                              /* centro del hueco entre el titulo (fila 1) y la opcion 1 (fila 21) */
#define TITLE_CLIP 164
#define TITLE_NSHIPS 10

static const u8 title_ships[TITLE_NSHIPS] = {
    SHIPT_COBRA_MK_3, SHIPT_VIPER, SHIPT_MAMBA, SHIPT_PYTHON, SHIPT_KRAIT,
    SHIPT_FER_DE_LANCE, SHIPT_ASP_MK_2, SHIPT_ADDER, SHIPT_ANACONDA, SHIPT_THARGOID
};

/* Cabeceo de la nave del titulo (otro eje, ademas del giro): una onda triangular lenta de -6 a +6 pasos de 64 (unos +-34 grados), de modo que
   se ve la parte de abajo y la de arriba (con 10 fijos solo se veia la de abajo). t = unidades de 40 ms (periodo 8 s) o fotogramas del titulo */
#define TITLE_PITCH_PERIOD 200
static int title_pitch(int t)
{
    int p = t % TITLE_PITCH_PERIOD, h = TITLE_PITCH_PERIOD / 2, v;
    v = (p < h) ? p - h / 2 : h + h / 2 - p;                      /* -h/2 .. h/2 */
    return (v * 6) / (h / 2);
}

static int title_loop(void)
{
    int ay = 0, ax = 0, ft = 0, ch, r, i, t = 0, idx = 0, phase = 0, rc;
    long z = TITLE_FAR, zend, rcz;
    u8 type = title_ships[0];
    char name[CMDR_NAMELEN + 1];
    ui_clear();
    state_clear();                                   /* LL9 trabaja sobre INWK/K%: el titulo usa su propio estado */
    txt_color(C_YELLOW, C_BLACK);
    txt_print_at(6, 1, "---- E L I T E ----");         /* TITLE: la ficha 30 del original (plf) en la columna 6 de la fila 1 */
    txt_color(C_WHITE, C_BLACK);
    txt_print_at(4, 21, "1  NEW COMMANDER (JAMESON)");
    txt_print_at(4, 22, "2  LOAD COMMANDER");
    txt_print_at(4, 23, "Q  QUIT");
    txt_color(C_CYAN, C_BLACK);
    if (patg) txt_print_at(0, 27, "BY D.BRABEN & I.BELL");      /* PATG: autores (ficha 13 del original) */
    txt_print_at(0, 28, "QL Port from Original 6502 Code");
    txt_print_at(0, 29, "by A.Alea & Claude 2026 R " ELITE_VERSION);
    if (!dash_loaded()) {                            /* la carga correcta es invisible; solo se avisa si falta el fichero */
        txt_color(C_RED, C_BLACK);
        txt_print_at(0, 30, "NO ELITE_DASH FILE (BASIC PANEL)");
    }
    rc = shipview_radius(SHIPT_COBRA_MK_3);
    zend = TITLE_Z;
    shipview_draw16(type, 0, 0, z, ay, ax, title_heap, state_slot(0), 1);
    for (;;) {
        ch = in_poll();
        if (ch == KC_QUIT) return TITLE_QUIT;
        if (ch < 0) {                                /* la nave gira mientras se espera */
            ay = (ay + 1) & (SHIPVIEW_ANG - 1);
            ++ft;
            ax = title_pitch((!in_script && plat_ticks() != 0UL) ? (int)(plat_ticks() / 2UL % 1000UL) : ft);       /* por tiempo (periodo 8 s) si hay reloj */
            if (in_script == 0 || (in_frames() & 7) == 0) {
                if (phase == 0) {                    /* se acerca frenando */
                    z -= (z - zend) / 10 + 4;
                    if (z <= zend) { z = zend; phase = 1; t = 0; }
                } else if (phase == 1) {             /* quieta girando */
                    if (++t >= TITLE_HOLD) phase = 2;
                } else {                             /* se aleja; al perderse entra otro modelo */
                    z += (z - zend) / 8 + 12;
                    if (z >= TITLE_FAR) {
                        zxx19 = title_heap; zlsnum = 1; zlsnum2 = title_heap[0]; ship_lsclr();   /* borra las lineas de la ultima */
                        state_clear();
                        idx = (idx + 1) % TITLE_NSHIPS;
                        type = title_ships[idx];
                        rcz = (long)shipview_radius(type) * TITLE_Z / rc;
                        zend = rcz < 400L ? 400L : (rcz > 1400L ? 1400L : rcz);
                        z = TITLE_FAR; phase = 0;
                        shipview_draw16(type, 0, 0, z, ay, ax, title_heap, state_slot(0), 1);
                        continue;
                    }
                }
                shipview_draw16(type, 0, 0, z, ay, ax, title_heap, state_slot(0), 0);
            }
            continue;
        }
        ch = in_up(ch);
        if (ch == '1') return TITLE_NEW;
        if (ch == 'q') return TITLE_QUIT;
        if (ch == '2') {
            for (i = 0; i < CMDR_NAMELEN + 1; i++) name[i] = 0;
            if (edit_name(25, "LOAD WHICH COMMANDER?", name) > 0) {
                snd_off();
                r = load_commander(name);
                if (r == SAVE_OK) { game_after_load(); return TITLE_LOAD; }
                txt_clear_rows(25, 3, C_BLACK);
                ui_msg(save_errstr(r), 1);
            } else if (in_quit) return TITLE_QUIT;
            else txt_clear_rows(25, 3, C_BLACK);
        }
    }
}

int scr_title(void)
{
    int svc = view_cy, svh = clip_h, svy = view_yscale, r;
    view_cy = TITLE_CY; clip_h = TITLE_CLIP; view_yscale = 1;     /* centro del hueco libre y aspecto 1,2 del monitor */
    r = title_loop();
    view_cy = svc; clip_h = svh; view_yscale = svy;
    return r;
}

/* ---------------------------------------------------------------- despacho */

int scr_dispatch(int sc)
{
    switch (sc) {
    case SC_STATUS:    return scr_status();
    case SC_BUY:       return scr_buy();
    case SC_SELL:      return scr_sell();
    case SC_EQUIP:     return scr_equip();
    case SC_GALCHART:  return scr_chart(0);
    case SC_LOCCHART:  return scr_chart(1);
    case SC_DATA:      return scr_data();
    case SC_MARKET:    return scr_market();
    case SC_INVENTORY: return scr_inventory();
    case SC_DISK:      return scr_disk();
#ifdef DEBUG
    case SC_DBG_BUY:   return scr_dbg_trade(1);
    case SC_DBG_SELL:  return scr_dbg_trade(0);
    case SC_DBG_EQUIP:  return scr_dbg_equip();
#endif
    }
    return sc;
}
