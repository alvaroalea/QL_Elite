#include <stdio.h>
#include "dash.h"
#include "space.h"
#include "state.h"
#include "emath.h"
#include "ship_math.h"
#include "save.h"
#include "text.h"

/* original colour constants (MASKT offsets); 0xFF = "use the warning colour" */
#define D_VIOLET 4
#define D_GREEN  8
#define D_WHITE  12
#define D_BLUE   16
#define D_RED    20
#define D_WARN   0xFF

static u8 dash_bg[DASH_BYTES];
static u8 dash_bg4[DASH_BYTES];        /* the same background in mode 4 (black, red, green and white), converted on load */
static int bg_ok = 0;
static int bg_loads = 0;               /* times the file has been read from disk (must be 1 per session) */

/* DIALS tables: bar y, threshold between "low" and "high", colour below and above the threshold */
static const u8 dialY[13]  = { 137, 144, 152, 185, 177, 169, 161, 137, 145, 153, 177, 161, 169 };
static const u8 dialle[13] = {  28,   0,   0,  16,   0,   0,   0,   8,   8,   0,   8,  24,  24 };
static const u8 dialc1[13] = { D_WHITE, D_WHITE, D_WHITE, D_WARN, D_VIOLET, D_VIOLET, D_VIOLET, D_WARN, D_WARN, D_GREEN, D_WARN, D_BLUE, D_BLUE };
static const u8 dialc2[13] = { D_WARN, D_WHITE, D_WHITE, D_VIOLET, D_VIOLET, D_VIOLET, D_VIOLET, D_VIOLET, D_VIOLET, D_GREEN, D_GREEN, D_WARN, D_WARN };

u8 dash_dials[13];                      /* last value drawn for each indicator */
u8 dash_dialc[13];                      /* last colour drawn */
static u8 dy;                             /* Y register: current indicator */
static u8 dial_k, dial_k1, dial_k2;       /* K, K+1, K+2 */
static u8 dial_col;                       /* COL */
u8 dash_comx, dash_comy, dash_comc;               /* compass: position and size of the dot */
u8 dash_mscol[4];                       /* colour of each missile indicator (0 = off) */
static const u8 msloc[4] = { 40, 32, 24, 16 };     /* x of the four missile indicators */

int dash_loaded(void) { return bg_ok; }
int dash_loads(void) { return bg_loads; }

void dash_reset(void)
{
    int i;
    for (i = 0; i < 13; i++) { dash_dials[i] = 0; dash_dialc[i] = 0; }
    for (i = 0; i < 4; i++) dash_mscol[i] = 0;
    dash_comc = 0;
}

/* Background in mode 8 -> mode 4: each dot (bit pair) becomes the 2 physical dots of its place. Only pure red and pure green
   keep their colour; black stays black and any other colour is white (like gfx_ccode). */
static void dash_to_mode4(void)
{
    long i;
    int k;
    for (i = 0; i < (long)DASH_BYTES; i += 2) {
        u8 g = dash_bg[i], rb = dash_bg[i + 1], og = 0, orr = 0;
        for (k = 0; k < 4; k++) {
            int s = 2 * (3 - k);
            int gg = (g >> (s + 1)) & 1, rr = (rb >> (s + 1)) & 1, bb = (rb >> s) & 1;
            u8 cc, m = (u8)(3 << s);
            if (!(gg | rr | bb)) continue;
            cc = (!bb && gg && !rr) ? 12 : ((!bb && rr && !gg) ? 3 : 15);
            og |= (u8)(GFX_GM(cc) & m);
            orr |= (u8)(GFX_RM(cc) & m);
        }
        dash_bg4[i] = og; dash_bg4[i + 1] = orr;
    }
}

int dash_load(void)
{
    char fn[48];
    u8 hdr[8];
    FILE *f;
    long n;
    bg_ok = 0;
    save_filename("DASH", fn);                 /* <drive>elite_dash */
    f = fopen(fn, "rb");
    if (!f) return DASH_E_OPEN;
    n = (long)fread(hdr, 1, 8, f);
    if (n != 8 || hdr[0] != 'E' || hdr[1] != 'L' || hdr[2] != 'Q' || hdr[3] != 'D' || hdr[4] != 1 || hdr[5] != DASH_LINES) {
        fclose(f);
        return DASH_E_FORMAT;
    }
    n = (long)fread(dash_bg, 1, DASH_BYTES, f);
    fclose(f);
    if (n != (long)DASH_BYTES) return DASH_E_FORMAT;
    dash_to_mode4();
    bg_ok = 1;
    bg_loads++;
    return DASH_OK;
}

void (*dash_trace)(char kind, u8 a, u8 b, u8 c, u8 d) = 0;

static u8 qlcol(u8 c)
{
    switch (c) {
    case D_VIOLET: return C_MAGENTA;
    case D_GREEN:  return C_GREEN;
    case D_WHITE:  return C_WHITE;
    case D_BLUE:   return C_BLUE;
    case D_RED:    return C_RED;               /* Elite's "RED" is the Apple II's orange */
    }
    return C_BLACK;
}

/* the original's panel is on lines 136..191; with the native view it moves down to DASH_Y0 */
static void panel_y(int on) { gfx_yoff_set(on && view_native ? DASH_Y0 - 136 : 0); }

/* panel primitives with the original's colour (D_*): they log the call if there is a dash_trace (tests) */
static void d_hloin(u8 x1, u8 x2, u8 y, u8 dcol)
{
    if (dash_trace) dash_trace('h', x1, x2, y, dcol);
    panel_y(1);
    gfx_hloin(x1, x2, y, qlcol(dcol));
    panel_y(0);
}

static void d_vloin(u8 x, u8 y1, u8 y2, u8 dcol)
{
    if (dash_trace) dash_trace('v', x, y1, y2, dcol);
    panel_y(1);
    gfx_vloin(x, y1, y2, qlcol(dcol));
    panel_y(0);
}

/* minimal panel if the file is missing: frame, dotted tracks and labels */
static void fallback_panel(void)
{
#define DR ((DASH_Y0 - 136) / 8)             /* text rows the panel moves down relative to the original */
    static const char *left[6] = { "FS", "AS", "FU", "CT", "LT", "AL" };
    int i, x;
    gfx_rect(0, DASH_Y0, 256, DASH_LINES, C_BLACK);
    for (x = 0; x < 256; x += 2) { gfx_pixel(x, DASH_Y0, C_MAGENTA); gfx_pixel(x, DASH_Y0 + DASH_LINES - 1, C_MAGENTA); }
    gfx_rect(0, DASH_Y0, 1, DASH_LINES, C_MAGENTA);
    gfx_rect(255, DASH_Y0, 1, DASH_LINES, C_MAGENTA);
    for (i = 0; i < 13; i++) {
        int base = (i == 1 || i == 2) ? 210 : (i >= 7 ? 16 : 210);
        for (x = base; x < base + 32; x += 2) gfx_pixel(x, dialY[i] + 6 + (DASH_Y0 - 136), C_RED);
    }
    txt_color(C_WHITE, TXT_TRANSPARENT);
    for (i = 0; i < 6; i++) txt_print_at(0, (DASH_Y0 + 1 + 8 * i) / 8, left[i]);
    txt_print_at(30, 17 + DR, "SP"); txt_print_at(30, 18 + DR, "RL"); txt_print_at(30, 19 + DR, "DC");
    txt_print_at(30, 20 + DR, "1"); txt_print_at(30, 21 + DR, "2"); txt_print_at(30, 22 + DR, "3"); txt_print_at(30, 23 + DR, "4");
    txt_print_at(6, 21 + DR, "(NO ELITE_DASH FILE)");
    txt_color(C_WHITE, C_BLACK);
}

void dash_show(void)
{
    u8 *dst = screen + (long)DASH_Y0 * 128L;
    if (bg_ok) gfx_copy(dst, gfx_m4 ? dash_bg4 : dash_bg, (long)DASH_BYTES);
    else fallback_panel();
    dash_reset();
}

/* ---------------------------------------------------------------- indicators (DIALS) */

static void msbars(u8 x1, u8 x2, u8 y1)       /* MSBARS: four lines below Y1 */
{
    u8 k;
    for (k = 1; k <= 4; k++) d_hloin(x1, x2, (u8)(y1 + k), dial_col);
}

static void dis7(u8 a, u8 xcol)
{
    u8 x2;
    dial_col = xcol;                           /* STX COL */
    x2 = (u8)((u8)(a + dial_k) & 0xFE);        /* CLC; ADC K; AND #$FE */
    msbars(dial_k, x2, dialY[dy - 1]);
}

/* DIS2: bar indicator with A = 0..31 (saturated); only redraws if the value or colour changes */
static void dis2(u8 a)
{
    u8 x, newa;
    if (a >= 32) a = 31;
    x = dialc1[dy];
    if (!(a < dialle[dy])) x = dialc2[dy];
    if (x == D_WARN) x = dial_k2;
    dy++;
    newa = a;
    if (newa == dash_dials[dy - 1] && x == dash_dialc[dy - 1]) return;     /* DI8 */
    {
        u8 oldc = dash_dialc[dy - 1];
        dash_dialc[dy - 1] = x;                     /* DI6 */
        dis7(dash_dials[dy - 1], oldc);             /* erases the previous drawing (XOR with the previous colour) */
        dash_dials[dy - 1] = newa;
        dis7(newa, x);                         /* draws the new one */
    }
}

static void dis1(u8 a) { dis2((u8)(a >> 3)); }

/* DIS5: roll or pitch marker (7-dot vertical line) at x = 224 +/- value */
static void dis6(u8 x1)
{
    u8 y1 = dialY[dy - 1];
    d_vloin(x1, y1, (u8)(y1 + 6), dial_col);      /* VLOIN: 7 dots high and 2 wide */
}

static void dis5(u8 a, u8 neg)
{
    if (neg) a = (u8)((a ^ 0xFF) + 1);         /* BPL DI9; EOR #$FF; CLC; ADC #1 */
    dy++;
    a = (u8)(a + 224);
    if (a == dash_dials[dy - 1]) return;            /* BEQ DIR1 */
    if (dash_dials[dy - 1] != 0) dis6(dash_dials[dy - 1]);   /* erases the previous one */
    dash_dials[dy - 1] = a;
    dis6(a);
}

/* ---------------------------------------------------------------- compass (COMPAS, SP1, SP2, DOT) */

/* DOT: the compass dot in XOR (drawing it again erases it). The port's own: 3 x 4 dots with the top left corner at
   (COMX, COMY - 2), yellow (mode 8) or red (mode 4) if it points to the station and white if it points to the planet; ahead (COMC = $60) a filled disc and
   behind (COMC = $30) a ring. It is painted by columns with gfx_vline_xor and gfx_xor_pixel (assembly on the QL), not dot by dot.
   The original PIXEL (3 x 1 ahead, 3 x 2 behind, white) was gfx_dust_6502 */
static u8 com_col = C_WHITE;                        /* colour the dot currently on screen was painted with */

static void dot(void)
{
    int x, y;
    if (dash_comc == 0) return;                     /* BEQ COR1 */
    if (dash_trace) dash_trace('p', dash_comx, dash_comy, dash_comc, 0);
    x = dash_comx; y = dash_comy - 2;
    panel_y(1);
    if (dash_comc & 0x40) {                         /* ahead: disc  .X. / XXX / XXX / .X. */
        gfx_vline_xor(x, y + 1, y + 2, com_col);
        gfx_vline_xor(x + 1, y, y + 3, com_col);
        gfx_vline_xor(x + 2, y + 1, y + 2, com_col);
    } else {                                        /* behind: ring  .X. / X.X / X.X / .X. */
        gfx_vline_xor(x, y + 1, y + 2, com_col);
        gfx_vline_xor(x + 2, y + 1, y + 2, com_col);
        gfx_xor_pixel(x + 1, y, com_col);
        gfx_xor_pixel(x + 1, y + 3, com_col);
    }
    panel_y(0);
}

static void compas(void)
{
    u8 x, t, ox = dash_comx, oy = dash_comy, oc = dash_comc, col;
    unsigned s;
    int d;
    if (sspr) sm_sps4(); else sm_sps1();       /* XX15 = unit vector towards the station or the planet */
    dash_comz = zx15[2];
    sm_sps2(zx15[0]);                          /* SP2 */
    x = sm_x;
    s = (unsigned)x + 195 + em_carry;          /* TXA; ADC #195 with the carry SPS2 returns with */
    sm_sps2(zx15[1]);
    t = sm_x;
    d = 148 - (int)t - (1 - (int)em_carry);    /* LDA #148; SBC T */
    col = sspr ? (gfx_m4 ? C_RED : C_YELLOW) : C_WHITE;   /* own: station = yellow (mode 8) or red (mode 4); planet = white */
    if (!dash_trace && oc != 0 && (u8)s == ox && (u8)d == oy && oc == ((zx15[2] & 0x80) ? 0x30 : 0x60) && col == com_col)
        return;                                /* own: if nothing changes it is neither erased nor repainted (no flicker) */
    dot();                                     /* erases the previous dot (with its colour) */
    dash_comx = (u8)s;
    dash_comy = (u8)d;
    dash_comc = 0x60;
    if (zx15[2] & 0x80) dash_comc = (u8)(dash_comc >> 1);    /* behind us: ring */
    com_col = col;
    dot();
}

/* ---------------------------------------------------------------- missiles and bulbs */

/* MSBAR/MSBAR2: missile indicator X (1..4): erases the previous drawing (XOR with its colour) and draws the new one; COLOR 0 = off */
static void msbar(u8 x, u8 color)
{
    u8 x1;
    if (x < 1 || x > 4) return;
    x1 = msloc[x - 1];
    if (dash_mscol[x - 1] != 0) { dial_col = dash_mscol[x - 1]; msbars(x1, (u8)(x1 + 6), 184); }       /* 4 lines: 185..188 */
    dash_mscol[x - 1] = color;
    if (color != 0) { dial_col = color; msbars(x1, (u8)(x1 + 6), 184); }
}

/* MSBAR for whoever calls it (tactics.c) */
void dash_msbar(u8 x, u8 color) { msbar(x, color); }

/* msblob: the first NOMSL indicators in green and the rest off */
void dash_msblob(void)
{
    u8 x;
    for (x = 4; x != 0; x--) msbar(x, (u8)(x <= nomsl ? D_GREEN : 0));
}

/* BULB: 7 x 8 dot bulb in XOR (E = ECM, S = station near) on text row 22 */
static const u8 bulb_e[8] = { 0x7F, 0x7F, 0x07, 0x7F, 0x7F, 0x07, 0x7F, 0x7F };
static const u8 bulb_s[8] = { 0x7F, 0x7F, 0x07, 0x7F, 0x7F, 0x70, 0x7F, 0x7F };

static void bulb(const u8 *rows, int x)
{
    int r, b;
    panel_y(1);
    for (r = 0; r < 8; r++)
        for (b = 0; b < 7; b++)
            if (rows[r] & (1 << b)) gfx_xor_pixel(x + b, 176 + r, C_WHITE);
    panel_y(0);
}

void dash_spblb(void) { bulb(bulb_s, 24 * 8); }       /* SPBLB: switches the "S" on or off */
void dash_ecblb(void) { bulb(bulb_e, 7 * 8); }        /* ECBLB: switches the "E" on or off */

/* M bulb (own): J jump blocked. Top left corner of the radar (the lower ones are the E and the S; the top right
   has the compass), red, with the shape of the others (7 x 8, bit 0 on the left) */
static const u8 bulb_m[8] = { 0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63, 0x63 };
static u8 mblb_on = 0;

void dash_mblb(u8 on)
{
    int r, b;
    if (on == mblb_on) return;
    mblb_on = on;
    panel_y(1);
    for (r = 0; r < 8; r++)
        for (b = 0; b < 7; b++)
            if (bulb_m[r] & (1 << b)) gfx_xor_pixel(7 * 8 + b, 140 + r, C_RED);
    panel_y(0);
}
void dash_mblb_forget(void) { mblb_on = 0; }

void dash_update(void)
{
    u8 a, x;
    dy = 0;
    dial_k = 210;
    x = D_RED;
    if (((mcnt & 8) & flh) != 0) x = D_WHITE;  /* flashing warning bars (FLH) */
    dial_k2 = x;
    dis2(zDELTA);                              /* speed */
    dial_col = D_WHITE;
    dis5((u8)(alp1 >> 1), (u8)(alp2[1] & 0x80));          /* alabeo: BIT ALP2+1 */
    dis5((u8)(bet1 << 1), (u8)(bet2[0] & 0x80));          /* cabeceo: BIT BET2 */
    a = (u8)(energy >> 1);
    do {                                       /* four energy banks of 32 */
        dial_k1 = a;
        dis2(a);
        a = dial_k1;
        a = (a >= 32) ? (u8)(a - 32) : 0;
    } while (dy != 7);
    dial_k = 16;
    dis1(fsh);                                 /* front shield */
    dis1(ash);                                 /* rear shield */
    dis2((u8)(qq14 >> 1));                     /* fuel (DIS1+2: just one LSR) */
    dis1(altit);
    dis1(cabtmp);
    dis1(gntmp);
    compas();
}
