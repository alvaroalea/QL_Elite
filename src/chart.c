#include "chart.h"
#include "dash.h"
#include "lines.h"
#include "sun.h"
#include "state.h"
#include "emath.h"
#include "ship_math.h"
#include "game.h"
#include "text.h"

#include "cmdr.h"
#define YHALF_VIEW 68                  /* original Y: half the height of the view */
#define GCYT CHART_GCYT
#define GCYB CHART_GCYB

static u8 q19[6];                                 /* QQ19..QQ19+5 */

void (*chart_line_hook)(u8 kind, u8 a, u8 b, u8 c) = 0;
void (*chart_event_hook)(char kind, u8 a, u8 b, u8 c) = 0;   /* tests: PIXEL (p), TT128 (c), SUN (s), cpl (n) */
#define EV(k, a, b, c) do { if (chart_event_hook) chart_event_hook(k, (u8)(a), (u8)(b), (u8)(c)); } while (0)
static void hloin(u8 x1, u8 x2, u8 y) { if (chart_line_hook) chart_line_hook(0, x1, x2, y); sun_xf_hloin(x1, x2, y, sun_col); }   /* HLOIN of the original (gfx.c) */
static void vloin(u8 x, u8 y1, u8 y2) { if (chart_line_hook) chart_line_hook(1, x, y1, y2); sun_xf_vloin(x, y1, y2, sun_col); }   /* VLOIN (CPIX: 2 wide) */
/* DVLOIN: VLOIN from (A, GCYT) to (A, GCYB) with the COL colour (vertical of the galactic frame) */
static void dvloin(u8 x) { vloin(x, GCYT, GCYB); }

/* TT15: cross of size QQ19+2 centred at (QQ19, QQ19+1); local = QQ11 negative */
static void tt15(int local)
{
    u8 a, q5, x1, x2, y1, y2;
    unsigned s, c;
    q5 = local ? 0 : GCYT;
    a = (u8)(q19[0] - q19[2]);
    if (!local && a < 34) a = 34;                  /* BIT QQ11; BMI; CMP #34; BCS; LDA #34 */
    x1 = a;
    s = (unsigned)q19[0] + 2; c = s >> 8;          /* CLC; ADC #2; ADC QQ19+2 */
    s = (unsigned)(u8)s + q19[2] + c;
    a = (u8)s;
    if (!local && a >= 224) a = 224;               /* CMP #224; BCC; LDA #224 */
    x2 = a;
    y1 = (u8)(q19[1] + q5);
    hloin(x1, x2, y1);
    a = (q19[1] >= q19[2]) ? (u8)(q19[1] - q19[2]) : 0;      /* SEC; SBC; BCS TT86; LDA #0 */
    y1 = (u8)(a + q5);
    s = (unsigned)q19[1] + q19[2]; c = s >> 8;     /* CLC; ADC QQ19+2; ADC QQ19+5 */
    s = (unsigned)(u8)s + q5 + c;
    a = (u8)s;
    if (a >= GCYB && !local) a = GCYB;             /* CMP #GCYB; BCC; LDX QQ11; BMI; LDA #GCYB */
    y2 = a;
    vloin(q19[0], y1, y2);
}

void chart_tt15(int local, u8 x, u8 y, u8 size) { q19[0] = x; q19[1] = y; q19[2] = size; tt15(local); }

/* SIGHT: laser sight of the view: two concentric crosses (sizes 20 and 10) centred at (128, Y-24) with colour according to the laser
   (pulse blue, beam red, military white, others white). If there is no laser in that view it draws nothing. It is TT15 of the space view
   (QQ11 = 0): the centre goes down GCYT dots and the arms are clipped as in the galactic one. The two crosses in XOR leave four sticks
   (10 to 20 dots from the centre). */
static u8 sight_shape = 0, sight_col;           /* what is on screen: 0 nothing, 1 cross, 2 X shape (station near) */

static void sight_cross(void)
{
    q19[0] = 128; q19[1] = (u8)(YHALF_VIEW - 24); q19[2] = 20;
    if (view_native) q19[1] = (u8)(YHALF_VIEW - GCYT);  /* own: TT15 adds GCYT; the original (Y - 24 + 19 = 63) leaves it 5 dots
                                                           above the centre of the view; in the native view it is centred on view_cy = 80,
                                                           where the laser and the projection converge */
    tt15(0);
    q19[2] = 10;
    tt15(0);
}

/* Own (native view, station near: SSPR != 0): the four sticks in an X shape, centred at (128, view_cy), 10 to 20 dots from the centre
   in x like those of the cross and with the slope of the view's diagonals (256 x 160: dy = dx * 5 / 8), with XOR lines
   (loin_draw: assembly on the QL) */
static void sight_x(u8 col)
{
    int cx = 128, cy = view_cy, sx, sy;
    for (sx = -1; sx <= 1; sx += 2)
        for (sy = -1; sy <= 1; sy += 2)
            loin_draw((u8)(cx + 10 * sx), (u8)(cy + 6 * sy), (u8)(cx + 20 * sx), (u8)(cy + 12 * sy), col);
}

static void sight_draw(u8 shape, u8 col)
{
    int m;
    if (shape == 2) { sight_x(col); return; }
    m = chart_xf_mode();
    sun_col = col;
    if (view_native) chart_xf_set(3);              /* native view: the crosses are stretched 6/5 vertically and centred at 80 */
    sight_cross();
    chart_xf_set(m);
}

/* Own: aligned for docking = the ISDK conditions (damage.c) with the station in slot 1: not hostile, its nose towards
   us (nosev_z high >= 214, < ~26 degrees), ahead of us in the ~22 degree cone (z of the unit vector towards it
   >= 89; calculated by COMPAS in dash_comz) and the slot almost horizontal (|roofv_x high| >= 80) */
static int station_aligned(void)
{
    const u8 *st = state_slot(1);
    if (st[I_NEWB] & 0x04) return 0;
    if (st[14] < 214) return 0;
    if ((dash_comz & 0x80) || dash_comz < 89) return 0;
    if ((st[16] & 0x7F) < 80) return 0;
    return 1;
}

static u8 sight_want(u8 view, u8 *col)
{
    u8 l = laser[view & 3];
    if (l == 0) return 0;
    if (l == LASER_POW) *col = C_BLUE;
    else if (l == LASER_BEAM) *col = C_RED;
    else *col = C_WHITE;
    if (!(view_native && sspr)) return 1;
    if (station_aligned()) *col = C_GREEN;         /* only calculated with the X shape */
    return 2;
}

void chart_sight(u8 view)                          /* after erasing the view */
{
    u8 col = C_WHITE, sh = sight_want(view, &col);
    sight_shape = sh; sight_col = col;
    if (sh) sight_draw(sh, col);
}

void chart_sight_forget(void) { sight_shape = 0; }   /* the view has been erased: the sight is no longer there */

void chart_sight_update(u8 view)                   /* every frame: if the S changes, the cross becomes an X shape or vice versa */
{
    u8 col = C_WHITE, sh = sight_want(view, &col);
    if (sh == sight_shape && (sh == 0 || col == sight_col)) return;
    if (sight_shape) sight_draw(sight_shape, sight_col);         /* XOR: erases the one that was there */
    sight_shape = sh; sight_col = col;
    if (sh) sight_draw(sh, col);
}

/* TT14: cross of the current system and fuel range circle */
static void tt14(int local)
{
    if (local) {                                   /* TT126 */
        q19[0] = 105; q19[1] = 75; q19[2] = 16;
        sun_col = C_GREEN;
        tt15(1);
        zK[0] = sm_scaley2(qq14);
    } else {
        zK[0] = sm_scaley((u8)(qq14 >> 1));
        q19[0] = sm_scalex(qq0);
        q19[1] = sm_scaley(qq1);
        q19[2] = 7;
        sun_col = C_GREEN;
        tt15(0);
        q19[1] = (u8)(q19[1] + GCYT);
    }
    zk3[0] = q19[0]; zk4[0] = q19[1];              /* TT128 */
    zk4[1] = 0; zk3[1] = 0;
    circ_lsp = 1; circ_stp = 2;
    EV('c', zk3[0], zk4[0], zK[0]);
    if (chart_xf_mode() == 1 || chart_xf_mode() == 2) sun_xf_circle(zk3[0], zk4[0], zK[0], sun_col);   /* screen: ellipse with the aspect */
    else circle2_draw();
}

/* TT103 / TT105 */
void chart_cursor(int local)
{
    u8 a;
    sun_col = C_GREEN;
    if (!local) {
        q19[0] = sm_scalex(qq9);
        q19[1] = sm_scaley(qq10);
        q19[2] = 4;
        tt15(0);
        return;
    }
    a = (u8)(qq9 - qq0);                           /* TT105 */
    if (a >= 29 && a < 227) return;
    q19[0] = sm_scaley2((u8)((u8)(a << 2) + 140));
    a = (u8)(qq10 - qq1);
    if (a >= 35 && a < 230) return;
    q19[1] = sm_scaley2((u8)((u8)(a << 1) + 99));
    q19[2] = 8;
    tt15(1);
}

/* carry with which cpl returns: that of the last TT54 (seed twist: w2 = w0 + w1 + w2 in 16 bits) */
static u8 cpl_carry(const uni_system *p)
{
    unsigned long w0 = p->seed0, w1 = ((unsigned)p->goatsoupseed.b << 8) | p->goatsoupseed.a;
    unsigned long w2 = ((unsigned)p->goatsoupseed.d << 8) | p->goatsoupseed.c, t;
    int n = (p->seed0 & 64) ? 4 : 3, i;
    u8 c = 0;
    for (i = 0; i < n; i++) {
        t = (w0 + w1) & 0xFFFFUL;
        t = t + w2;
        c = (u8)((t >> 16) & 1);
        w0 = w1; w1 = w2; w2 = t & 0xFFFFUL;
    }
    return c;
}

static void put_name(int col, int row, const char *s)
{
    txt_color(C_WHITE, TXT_TRANSPARENT);
    txt_at(col, row);
    while (*s && col < 32) { txt_putc(*s); s++; col++; }
}

/* TT23: local chart */
static void tt23(void)
{
    u8 rows[256];                                  /* INWK+Y; with Y = 255 the original reads zero page at INWK-1 = SC+1 (never 0) */
    int i;
    u8 a, d, c, xx12, k4, y, row, col;
    unsigned s, dist;
    for (i = 0; i < 256; i++) rows[i] = 0;
    rows[255] = 0x20;
    for (i = 0; i < UNI_NSYS; i++) {
        const uni_system *p = &uni_galaxy[i];
        a = (u8)(p->x - qq0);
        if (p->x < qq0) a = (u8)((u8)(a ^ 0xFF) + 1);
        if (a >= 29) continue;                     /* TT182 ... TT187s */
        a = (u8)(p->y - qq1);
        if (p->y < qq1) a = (u8)((u8)(a ^ 0xFF) + 1);
        if (a >= 40) continue;
        d = (u8)(p->x - qq0);                      /* LDA QQ15+3; SEC; SBC QQ0; ASL; ASL; ADC #140 (the carry of ASL is added) */
        c = (u8)((d >> 6) & 1);
        s = (unsigned)(u8)(d << 2) + 140 + c;
        xx12 = sm_scaley2((u8)s);
        col = (u8)((xx12 >> 3) + 1);               /* XC */
        d = (u8)(p->y - qq1);                      /* ASL A; ADC #99 */
        c = (u8)(d >> 7);
        s = (unsigned)(u8)(d << 1) + 99 + c;
        k4 = sm_scaley2((u8)s);
        c = (u8)((k4 >> 2) & 1);                   /* carry after LSR A x3 */
        y = (u8)(k4 >> 3);
        row = y;
        if (rows[y] == 0) goto ee4;
        y++;
        if (rows[y] == 0) goto ee4;
        y = (u8)(y - 2);
        if (rows[y] != 0) goto ee1;
ee4:
        if (y < 3) continue;                       /* CPY #3; BCC TT187 */
        if (y >= 17) continue;                     /* CPY #17; BCS TT187 */
        dist = uni_distance(&uni_galaxy[cur_sys], p);   /* readdistnce -> QQ8 */
        if (dist >= 70) continue;                  /* QQ8+1 != 0 o QQ8 >= 70 */
        rows[y] = 255;
        EV('n', col, y, i);
        if (chart_xf_mode()) put_name(sun_xf_x((int)col * 8) >> 3, sun_xf_y((int)y * 8) >> 3, p->name);   /* with the transformation: the name cell (original column and row) on screen */
        else put_name(col, y, p->name);
        c = cpl_carry(p);
ee1:
        zk3[1] = 0; zk4[1] = 0; zK[1] = 0;
        zk3[0] = xx12; zk4[0] = k4;
        zK[0] = (u8)((p->goatsoupseed.d & 1) + 2 + c);       /* LDA QQ15+5; AND #1; ADC #2 */
        (void)row;
        EV('s', zk3[0], zk4[0], zK[0]);
        if (chart_xf_mode() == 1) sun_xf_disc(zk3[0], zk4[0], zK[0], C_RED);   /* screen: filled 1-dot ellipse */
        else { sun_flflls(); sun_draw(); sun_flflls(); }
    }
}

/* TT22: galactic chart */
static void tt22(void)
{
    int i;
    u8 x, y;
    sun_col = C_BLUE;                              /* NLIN2 sets COL = BLUE and the frame inherits it */
#if CHART_TOP_FULLWIDTH
    { int m = chart_xf_mode(); chart_xf_set(0); hloin(0, 255, GCYT - 1); chart_xf_set(m); }   /* NLIN5 -> NLIN2: X1 = 0, X2 = 255 (line under the title, not moving down with the map) */
    if (chart_xf_mode()) gfx_hline_xor(30, 226, sun_xf_y(GCYT - 1), sun_col);   /* top edge of the frame: from stick to stick (30..226), moves down with the map */
#else
    hloin(30, 228, GCYT - 1);                      /* variant: the top line only closes the frame */
#endif
    if (chart_xf_mode()) gfx_hline_xor(30, 226, sun_xf_y(GCYB + 1), sun_col);   /* bottom edge: from stick to stick */
    else hloin(31, 228, GCYB + 1);
    dvloin(30);                                    /* DVLOIN */
    dvloin(226);
    tt14(0);
    for (i = 0; i < UNI_NSYS; i++) {
        x = sm_scalex(uni_galaxy[i].x);
        y = (u8)(sm_scaley(uni_galaxy[i].y) + GCYT);
        EV('p', x, y, 255);
        sun_xf_dust(x, y, 255);                    /* PIXEL with ZZ = 255: two dots */
    }
    chart_cursor(0);
}

/* TT123: QQ19+4 = coordinate + delta (signed), without moving it if it wraps over the top or bottom */
static u8 tt123(u8 a, u8 d)
{
    unsigned s = (unsigned)a + d;
    if (!(d & 0x80)) return (u8)(s > 255 ? a : s);            /* BCC TT125; RTS (C = 1: it would exceed 255) */
    return (u8)(s > 255 ? s : a);                             /* TT124: negative delta, C = 0 -> would go below 0 */
}

/* TT16: moves the chart crosshair (X = delta x, Y = delta y upwards): erases the cross (TT103), moves QQ10 with -Y and QQ9 with
   X (TT123) and draws it again (falls into TT103) */
void chart_tt16(int local, u8 x, u8 y)
{
    u8 dy = (u8)((u8)(y - 1) ^ 0xFF);              /* DEY; TYA; EOR #$FF */
    chart_cursor(local);
    qq10 = tt123(qq10, dy);
    qq9 = tt123(qq9, x);
    chart_cursor(local);
}

/* ping: the cursor returns to the current system (QQ9, QQ10 = QQ0, QQ1) */
void chart_ping_set(void) { qq9 = qq0; qq10 = qq1; }   /* ping: QQ9/QQ10 = QQ0/QQ1 */

void chart_ping(int local)
{
    chart_cursor(local);                           /* TT103: erases */
    chart_ping_set();
    game_set_target(cur_sys);
    chart_cursor(local);                           /* TT103: draws */
}

/* hm: selects the system nearest the cursor and redraws the cross (TT103, TT111, TT103) */
void chart_hm(int local)
{
    chart_cursor(local);
    game_tt111();                                  /* TT111: also places the cross over the system */
    chart_cursor(local);
}

/* HME2 (data part): searches for a system by name, case-insensitive; returns its index or -1.
   Compares the full name, like the original (which compares with the text cpl would print). */
int chart_find_system(const char *name)
{
    int i, k;
    char a, b;
    for (i = 0; i < UNI_NSYS; i++) {
        const char *p = uni_galaxy[i].name;
        for (k = 0; ; k++) {
            a = name[k]; b = p[k];
            if (a >= 'A' && a <= 'Z') a = (char)(a + 32);
            if (b >= 'A' && b <= 'Z') b = (char)(b + 32);
            if (a != b) break;
            if (a == 0) return i;
        }
    }
    return -1;
}

void chart_draw(int local)
{
    if (!local) { tt22(); return; }
    nlin4();                                       /* NLIN3: title (set by menu.c) and the NLIN4 line at y = 19 */
    sun_flflls();
    tt14(1);                                       /* TT14 -> TT126 */
    chart_cursor(1);                               /* TT103 -> TT105 */
    tt23();
}
