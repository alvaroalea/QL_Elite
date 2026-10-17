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
#include "emath.h"

#define ROW_MSG 28

/* ---------------------------------------------------------------- utilidades */

void ui_clear(void)
{
    gfx_clear(C_BLACK);
    txt_color(C_WHITE, C_BLACK);
    txt_at(0, 0);
}

static int slen(const char *s) { int n = 0; while (s[n]) n++; return n; }

void ui_title(const char *t)
{
    int n = slen(t), col;
    txt_color(C_YELLOW, C_BLUE);
    txt_clear_rows(0, 1, C_BLUE);
    col = (TXT_COLS - n) / 2;
    if (col < 0) col = 0;
    txt_print_at(col, 0, t);
    txt_color(C_WHITE, C_BLACK);
}

void ui_footer(void)
{
    txt_color(C_CYAN, C_BLACK);
    txt_print_at(0, 30, "1BUY 2SELL 3EQUIP 4GAL 5LOCAL");
    txt_print_at(0, 31, "6DATA 7MKT 8STAT 9INV IDISK 0GO");
    txt_color(C_WHITE, C_BLACK);
}

void ui_num(unsigned long v, int width)
{
    char b[12];
    int i = 0, k;
    do { b[i++] = (char)('0' + (int)(v % 10UL)); v /= 10UL; } while (v);
    for (k = i; k < width; k++) txt_putc(' ');
    while (i) txt_putc(b[--i]);
}

void ui_tenths(unsigned long v)
{
    ui_num(v / 10UL, 0);
    txt_putc('.');
    txt_putc((char)('0' + (int)(v % 10UL)));
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
    switch (ch) {
    case '0': return SC_LAUNCH;
    case '1': return SC_BUY;
    case '2': return SC_SELL;
    case '3': return SC_EQUIP;
    case '4': return SC_GALCHART;
    case '5': return SC_LOCCHART;
    case '6': return SC_DATA;
    case '7': return SC_MARKET;
    case '8': return SC_STATUS;
    case '9': return SC_INVENTORY;
    case 'i': return SC_DISK;
    }
    return -1;
}

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
        if (ch == KC_ESC) return SC_STATUS;
        sc = ui_nav(ch);
        if (sc >= 0) return sc;
    }
}

/* ---------------------------------------------------------------- estado */

int scr_status(void)
{
    int row, i;
    static const char *vname[4] = { "Front", "Rear", "Left", "Right" };
    char title[24];
    int n = 0;
    const char *p = "COMMANDER ";
    while (*p) title[n++] = *p++;
    for (p = cmdr_name; *p && n < 22; p++) title[n++] = *p;
    title[n] = 0;

    ui_clear();
    ui_title(title);
    lbl(2, "Present System:");   val(19, 2); txt_puts(uni_galaxy[cur_sys].name);
    lbl(3, "Hyperspace System:"); val(19, 3); txt_puts(uni_galaxy[tgt_sys].name);
    lbl(4, "Condition:");        val(19, 4); txt_puts(qq12 ? "Docked" : "Green");
    lbl(5, "Fuel:");             val(19, 5); ui_tenths(qq14); txt_puts(" LY");
    lbl(6, "Cash:");             val(19, 6); ui_tenths(cash_get()); txt_puts(" Cr");
    lbl(7, "Legal Status:");     val(19, 7); txt_puts(legal_name());
    lbl(8, "Rating:");           val(19, 8); txt_puts(rating_name());
    txt_color(C_CYAN, C_BLACK);
    txt_print_at(0, 10, "EQUIPMENT:");
    txt_color(C_WHITE, C_BLACK);
    row = 11;
    if (nomsl) { txt_at(1, row); txt_puts("Missiles: "); ui_num(nomsl, 0); row++; }
    if (crgo > 22) { txt_print_at(1, row++, "Large Cargo Bay"); }
    if (escp) { txt_print_at(1, row++, "Escape Pod"); }
    if (bst) { txt_print_at(1, row++, "Fuel Scoops"); }
    if (ecm_fitted) { txt_print_at(1, row++, "E.C.M.System"); }
    if (bomb) { txt_print_at(1, row++, "Energy Bomb"); }
    if (engy) { txt_print_at(1, row++, "Energy Unit"); }
    if (dkcmp) { txt_print_at(1, row++, "Docking Computer"); }
    if (ghyp) { txt_print_at(1, row++, "Galactic Hyperdrive"); }
    for (i = 0; i < 4; i++) {
        if (!laser[i]) continue;
        txt_at(1, row++);
        txt_puts(vname[i]); txt_puts(": "); txt_puts(laser_name(laser[i]));
    }
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

static int trade_screen(int buying)
{
    int sel = 0, ch, i, nlen = 0, r;
    unsigned qty;
    char entry[4];
    entry[0] = 0;
    ui_clear();
    ui_title(buying ? "BUY CARGO" : "SELL CARGO");
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

int scr_buy(void)  { return trade_screen(1); }
int scr_sell(void) { return trade_screen(0); }

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

int scr_equip(void)
{
    int n = equip_count(), sel = 0, ch, i, r, view;
    static const char *vn[4] = { "FRONT", "REAR", "LEFT", "RIGHT" };
    ui_clear();
    ui_title("EQUIP SHIP");
    for (;;) {
        for (i = 0; i < n; i++) equip_row(i, 2 + i, i == sel);
        txt_clear_rows(21, 8, C_BLACK);
        lbl(21, "Cash:"); val(7, 21); ui_tenths(cash_get()); txt_puts(" Cr");
        txt_color(C_CYAN, C_BLACK);
        txt_print_at(0, 24, "UP/DOWN ENTER=BUY ESC=EXIT");
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
        }
        view = 0;
        if (equip_is_laser(sel)) {
            txt_clear_rows(26, 3, C_BLACK);
            txt_color(C_CYAN, C_BLACK);
            txt_print_at(0, 26, "VIEW? 0 FRONT 1 REAR 2 LEFT 3 RIGHT");
            txt_color(C_WHITE, C_BLACK);
            for (;;) {
                ch = in_wait();
                if (ch == KC_QUIT) return SC_QUIT;
                if (ch == KC_ESC) { view = -1; break; }
                if (ch >= '0' && ch <= '3') { view = ch - '0'; break; }
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

/* ---------------------------------------------------------------- datos del sistema */

static char desc[512];
static int dlen;
static void cap(int c) { if (dlen < 511) desc[dlen++] = (char)c; desc[dlen] = 0; }

static void put_wrapped(const char *s)
{
    char word[40];
    int n;
    while (*s) {
        while (*s == ' ') s++;
        n = 0;
        while (*s && *s != ' ' && n < 39) word[n++] = *s++;
        word[n] = 0;
        if (n == 0) break;
        if (txt_col() + n > TXT_COLS) txt_putc('\n');
        txt_puts(word);
        if (txt_col() != 0) txt_putc(' ');
    }
}

int scr_data(void)
{
    const uni_system *p = &uni_galaxy[tgt_sys];
    char t[28];
    int n = 0;
    const char *q = "DATA ON ";
    while (*q) t[n++] = *q++;
    for (q = p->name; *q && n < 27; q++) t[n++] = *q;
    t[n] = 0;
    ui_clear();
    ui_title(t);
    lbl(2, "Distance:");   val(17, 2); ui_tenths(jump_distance(tgt_sys)); txt_puts(" LY");
    lbl(3, "Economy:");    val(17, 3); txt_puts(uni_econnames[p->economy]);
    lbl(4, "Government:"); val(17, 4); txt_puts(uni_govnames[p->govtype]);
    lbl(5, "Tech.Level:"); val(17, 5); ui_num((unsigned long)p->techlev + 1, 0);
    lbl(6, "Population:"); val(17, 6); ui_num((unsigned long)(p->population >> 3), 0); txt_putc('.'); ui_num((unsigned long)(p->population & 7), 0); txt_puts(" Billion");
    lbl(7, "Productivity:"); val(17, 7); ui_num(p->productivity, 0); txt_puts(" M CR");
    lbl(8, "Avg. Radius:"); val(17, 8); ui_num(p->radius, 0); txt_puts(" km");
    txt_color(C_CYAN, C_BLACK);
    txt_at(0, 10);
    dlen = 0; desc[0] = 0;
    uni_putc = cap;
    uni_describe(p);
    put_wrapped(desc);
    txt_color(C_WHITE, C_BLACK);
    ui_footer();
    return wait_nav();
}

/* ---------------------------------------------------------------- cartas */

static int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

static void chart_info(int local)
{
    unsigned d = jump_distance(tgt_sys);
    txt_clear_rows(18, 6, C_BLACK);
    txt_color(C_CYAN, C_BLACK);
    txt_print_at(0, 18, "Target:"); txt_color(C_WHITE, C_BLACK); txt_puts(" "); txt_puts(uni_galaxy[tgt_sys].name);
    txt_color(C_CYAN, C_BLACK);
    txt_print_at(0, 19, "Distance:"); txt_color(C_WHITE, C_BLACK); txt_puts(" "); ui_tenths(d); txt_puts(" LY");
    if (tgt_sys == cur_sys) { txt_color(C_YELLOW, C_BLACK); txt_puts("  (HERE)"); }
    else if (d > qq14) { txt_color(C_RED, C_BLACK); txt_puts("  OUT OF RANGE"); }
    else { txt_color(C_GREEN, C_BLACK); txt_puts("  IN RANGE"); }
    txt_color(C_CYAN, C_BLACK);
    txt_print_at(0, 20, "Fuel:"); txt_color(C_WHITE, C_BLACK); txt_puts(" "); ui_tenths(qq14); txt_puts(" LY");
    txt_color(C_CYAN, C_BLACK);
    txt_print_at(0, 22, local ? "CURSORS=MOVE O=HOME" : "CURSORS=MOVE O=HOME");
    txt_color(C_WHITE, C_BLACK);
}

/* El dibujo de las cartas esta en chart.c (TT22/TT23/TT14/TT103/TT105 con SUN y CIRCLE). Aqui, el teclado:
   la mira se mueve libre (cx, cy) y el objetivo es el sistema mas cercano (TT111); la cruz del objetivo se dibuja en XOR. */
int scr_chart(int local)
{
    int cx = qq9, cy = qq10, ch, dx, dy, sc, step = local ? 1 : 2, idx;
    ui_clear();
    ui_title(local ? "SHORT RANGE CHART" : "GALACTIC CHART");
    chart_draw(local);
    ui_footer();
    chart_info(local);
    for (;;) {
        dx = dy = 0;
        ch = in_poll();
        if (ch == KC_QUIT) return SC_QUIT;
        if (ch == KC_ESC) return SC_STATUS;
        if (ch == KC_LEFT) dx = -step; else if (ch == KC_RIGHT) dx = step;
        else if (ch == KC_UP) dy = -step; else if (ch == KC_DOWN) dy = step;
        else if (ch < 0) {
            if (in_held(KC_LEFT)) dx = -step; else if (in_held(KC_RIGHT)) dx = step;
            if (in_held(KC_UP)) dy = -step; else if (in_held(KC_DOWN)) dy = step;
        } else if (in_up(ch) == 'o') {
            chart_cursor(local);                                /* borra la cruz */
            cx = qq0; cy = qq1;
            game_set_target(uni_nearest((u8)cx, (u8)cy));
            chart_cursor(local);
            chart_info(local);
            continue;
        } else {
            sc = ui_nav(ch);
            if (sc >= 0) return sc;
        }
        if (dx || dy) {
            cx = clampi(cx + dx, 0, 255); cy = clampi(cy + dy, 0, 255);
            if (local) {                                        /* la mira no sale de la parte visible de la carta local */
                cx = clampi(cx, clampi((int)qq0 - 27, 0, 255), clampi((int)qq0 + 27, 0, 255));
                cy = clampi(cy, clampi((int)qq1 - 38, 0, 255), clampi((int)qq1 + 38, 0, 255));
            }
            idx = uni_nearest((u8)cx, (u8)cy);
            if (idx != tgt_sys) {
                chart_cursor(local);                            /* borra la cruz anterior */
                game_set_target(idx);
                chart_cursor(local);
                chart_info(local);
            }
        }
    }
}

/* ---------------------------------------------------------------- nombre y disco */

/* Editor de linea: devuelve la longitud (0 = cancelado con ESC). buf >= CMDR_NAMELEN + 1 */
static int edit_name(int row, const char *prompt, char *buf)
{
    int n = 0, ch;
    txt_clear_rows(row, 2, C_BLACK);
    txt_color(C_CYAN, C_BLACK);
    txt_print_at(0, row, prompt);
    txt_color(C_WHITE, C_BLACK);
    while (buf[n]) n++;
    txt_at(0, row + 1); txt_puts("> "); txt_puts(buf); txt_cursor(1);
    for (;;) {
        ch = in_wait();
        if (ch == KC_QUIT) { txt_cursor(0); in_quit = 1; return 0; }
        if (ch == KC_ESC) { txt_cursor(0); return 0; }
        if (ch == KC_ENTER) { txt_cursor(0); return n; }
        if (ch == KC_BS) {
            if (n > 0) { n--; buf[n] = 0; txt_putc('\b'); txt_putc(' '); txt_putc('\b'); }
        } else if (n < CMDR_NAMELEN &&
                   ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_' || ch == '-')) {
            if (ch >= 'a' && ch <= 'z') ch = ch - 'a' + 'A';
            buf[n++] = (char)ch; buf[n] = 0;
            txt_putc(ch);
        }
    }
}

static void disk_draw(void)
{
    ui_clear();
    ui_title("DISK ACCESS MENU");
    lbl(2, "Commander:"); val(11, 2); txt_puts(cmdr_name);
    lbl(3, "Drive:");     val(11, 3); txt_puts(save_dev[0] ? save_dev : "(here)");
    txt_color(C_WHITE, C_BLACK);
    txt_print_at(2, 6, "1  LOAD COMMANDER");
    txt_print_at(2, 7, "2  SAVE COMMANDER");
    txt_print_at(2, 8, "3  CHANGE DRIVE");
    txt_print_at(2, 9, "4  RESET TO DEFAULT JAMESON");
    txt_print_at(2, 10, "Q  QUIT GAME");
    txt_color(C_CYAN, C_BLACK);
    txt_print_at(0, 13, "ESC = BACK");
}

int scr_disk(void)
{
    char name[CMDR_NAMELEN + 1];
    int ch, r, i;
    disk_draw();
    for (;;) {
        ch = in_wait();
        if (ch == KC_QUIT) return SC_QUIT;
        if (ch == KC_ESC) return SC_STATUS;
        ch = in_up(ch);
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
            save_next_device();
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

int scr_title(void)
{
    int ay = 0, ch, r, i;
    char name[CMDR_NAMELEN + 1];
    ui_clear();
    state_clear();                                   /* LL9 trabaja sobre INWK/K%: el titulo usa su propio estado */
    txt_color(C_YELLOW, C_BLACK);
    txt_print_at(7, 18, "E L I T E  -  QL");
    txt_color(C_WHITE, C_BLACK);
    txt_print_at(4, 21, "1  NEW COMMANDER (JAMESON)");
    txt_print_at(4, 22, "2  LOAD COMMANDER");
    txt_print_at(4, 23, "Q  QUIT");
    txt_color(C_CYAN, C_BLACK);
    txt_print_at(0, 27, "BY BELL AND BRABEN, 1984-85");
    txt_print_at(0, 28, "QL PORT FROM THE APPLE II CODE");
    shipview_draw(SHIPT_COBRA_MK_3, 0, 0, 5, ay, 10, title_heap, state_slot(0), 1);
    for (;;) {
        ch = in_poll();
        if (ch == KC_QUIT) return TITLE_QUIT;
        if (ch < 0) {                                /* la nave gira mientras se espera */
            ay = (ay + 1) & (SHIPVIEW_ANG - 1);
            if (in_script == 0 || (in_frames() & 7) == 0)
                shipview_draw(SHIPT_COBRA_MK_3, 0, 0, 5, ay, 10, title_heap, state_slot(0), 0);
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
    }
    return sc;
}
