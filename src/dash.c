#include <stdio.h>
#include "dash.h"
#include "space.h"
#include "state.h"
#include "emath.h"
#include "ship_math.h"
#include "save.h"
#include "text.h"

/* constantes de color del original (desplazamientos de MASKT); 0xFF = "usar el color de aviso" */
#define D_VIOLET 4
#define D_GREEN  8
#define D_WHITE  12
#define D_BLUE   16
#define D_RED    20
#define D_WARN   0xFF

static u8 dash_bg[DASH_BYTES];
static int bg_ok = 0;
static int bg_loads = 0;               /* veces que se ha leido el fichero del disco (debe ser 1 por sesion) */

/* tablas de DIALS: y de la barra, umbral entre "bajo" y "alto", color por debajo y por encima del umbral */
static const u8 dialY[13]  = { 137, 144, 152, 185, 177, 169, 161, 137, 145, 153, 177, 161, 169 };
static const u8 dialle[13] = {  28,   0,   0,  16,   0,   0,   0,   8,   8,   0,   8,  24,  24 };
static const u8 dialc1[13] = { D_WHITE, D_WHITE, D_WHITE, D_WARN, D_VIOLET, D_VIOLET, D_VIOLET, D_WARN, D_WARN, D_GREEN, D_WARN, D_BLUE, D_BLUE };
static const u8 dialc2[13] = { D_WARN, D_WHITE, D_WHITE, D_VIOLET, D_VIOLET, D_VIOLET, D_VIOLET, D_VIOLET, D_VIOLET, D_GREEN, D_GREEN, D_WARN, D_WARN };

u8 dash_dials[13];                      /* ultimo valor dibujado de cada indicador */
u8 dash_dialc[13];                      /* ultimo color dibujado */
static u8 dy;                             /* registro Y: indicador en curso */
static u8 dial_k, dial_k1, dial_k2;       /* K, K+1, K+2 */
static u8 dial_col;                       /* COL */
u8 dash_comx, dash_comy, dash_comc;               /* brujula: posicion y tamano del punto */
u8 dash_mscol[4];                       /* color de cada indicador de misil (0 = apagado) */
static const u8 msloc[4] = { 40, 32, 24, 16 };     /* x de los cuatro indicadores de misil */

int dash_loaded(void) { return bg_ok; }
int dash_loads(void) { return bg_loads; }

void dash_reset(void)
{
    int i;
    for (i = 0; i < 13; i++) { dash_dials[i] = 0; dash_dialc[i] = 0; }
    for (i = 0; i < 4; i++) dash_mscol[i] = 0;
    dash_comc = 0;
}

int dash_load(void)
{
    char fn[48];
    u8 hdr[8];
    FILE *f;
    long n;
    bg_ok = 0;
    save_filename("DASH", fn);                 /* <unidad>elite_dash */
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
    case D_RED:    return C_RED;               /* el "RED" de Elite es el naranja del Apple II */
    }
    return C_BLACK;
}

/* primitivas del panel con el color del original (D_*): registran la llamada si hay dash_trace (pruebas) */
static void d_hloin(u8 x1, u8 x2, u8 y, u8 dcol)
{
    if (dash_trace) dash_trace('h', x1, x2, y, dcol);
    gfx_hloin(x1, x2, y, qlcol(dcol));
}

static void d_vloin(u8 x, u8 y1, u8 y2, u8 dcol)
{
    if (dash_trace) dash_trace('v', x, y1, y2, dcol);
    gfx_vloin(x, y1, y2, qlcol(dcol));
}

/* panel minimo si falta el fichero: marco, pistas punteadas y etiquetas */
static void fallback_panel(void)
{
    static const char *left[6] = { "FS", "AS", "FU", "CT", "LT", "AL" };
    int i, x;
    gfx_rect(0, DASH_Y0, 256, DASH_LINES, C_BLACK);
    for (x = 0; x < 256; x += 2) { gfx_pixel(x, DASH_Y0, C_MAGENTA); gfx_pixel(x, DASH_Y0 + DASH_LINES - 1, C_MAGENTA); }
    gfx_rect(0, DASH_Y0, 1, DASH_LINES, C_MAGENTA);
    gfx_rect(255, DASH_Y0, 1, DASH_LINES, C_MAGENTA);
    for (i = 0; i < 13; i++) {
        int base = (i == 1 || i == 2) ? 210 : (i >= 7 ? 16 : 210);
        for (x = base; x < base + 32; x += 2) gfx_pixel(x, dialY[i] + 6, C_RED);
    }
    txt_color(C_WHITE, TXT_TRANSPARENT);
    for (i = 0; i < 6; i++) txt_print_at(0, (136 + 1 + 8 * i) / 8, left[i]);
    txt_print_at(30, 17, "SP"); txt_print_at(30, 18, "RL"); txt_print_at(30, 19, "DC");
    txt_print_at(30, 20, "1"); txt_print_at(30, 21, "2"); txt_print_at(30, 22, "3"); txt_print_at(30, 23, "4");
    txt_print_at(8, 21, "(NO ELITE_DASH FILE)");
    txt_color(C_WHITE, C_BLACK);
}

void dash_show(void)
{
    u8 *dst = screen + (long)DASH_Y0 * 128L;
    if (bg_ok) gfx_copy(dst, dash_bg, (long)DASH_BYTES);
    else fallback_panel();
    dash_reset();
}

/* ---------------------------------------------------------------- indicadores (DIALS) */

static void msbars(u8 x1, u8 x2, u8 y1)       /* MSBARS: cuatro lineas debajo de Y1 */
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

/* DIS2: indicador de barra con A = 0..31 (se satura); solo redibuja si cambia el valor o el color */
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
        dis7(dash_dials[dy - 1], oldc);             /* borra el dibujo anterior (XOR con el color anterior) */
        dash_dials[dy - 1] = newa;
        dis7(newa, x);                         /* dibuja el nuevo */
    }
}

static void dis1(u8 a) { dis2((u8)(a >> 3)); }

/* DIS5: marcador de alabeo o cabeceo (linea vertical de 7 puntos) en x = 224 +/- valor */
static void dis6(u8 x1)
{
    u8 y1 = dialY[dy - 1];
    d_vloin(x1, y1, (u8)(y1 + 6), dial_col);      /* VLOIN: 7 puntos de alto y 2 de ancho */
}

static void dis5(u8 a, u8 neg)
{
    if (neg) a = (u8)((a ^ 0xFF) + 1);         /* BPL DI9; EOR #$FF; CLC; ADC #1 */
    dy++;
    a = (u8)(a + 224);
    if (a == dash_dials[dy - 1]) return;            /* BEQ DIR1 */
    if (dash_dials[dy - 1] != 0) dis6(dash_dials[dy - 1]);   /* borra el anterior */
    dash_dials[dy - 1] = a;
    dis6(a);
}

/* ---------------------------------------------------------------- brujula (COMPAS, SP1, SP2, DOT) */

static void dot(void)
{
    if (dash_comc == 0) return;                     /* BEQ COR1 */
    if (dash_trace) dash_trace('p', dash_comx, dash_comy, dash_comc, 0);
    gfx_dust(dash_comx, dash_comy, dash_comc);                /* JMP PIXEL con ZZ = COMC */
}

static void compas(void)
{
    u8 x, t;
    unsigned s;
    int d;
    dot();                                     /* borra el punto anterior */
    if (sspr) sm_sps4(); else sm_sps1();       /* XX15 = vector unitario hacia la estacion o el planeta */
    sm_sps2(zx15[0]);                          /* SP2 */
    x = sm_x;
    s = (unsigned)x + 195 + em_carry;          /* TXA; ADC #195 con el acarreo con que vuelve SPS2 */
    dash_comx = (u8)s;
    sm_sps2(zx15[1]);
    t = sm_x;
    d = 148 - (int)t - (1 - (int)em_carry);    /* LDA #148; SBC T */
    dash_comy = (u8)d;
    dash_comc = 0x60;
    if (zx15[2] & 0x80) dash_comc = (u8)(dash_comc >> 1);    /* detras de nosotros: punto mas tenue */
    dot();
}

/* ---------------------------------------------------------------- misiles y bombillas */

/* MSBAR/MSBAR2: indicador de misil X (1..4): borra el dibujo anterior (XOR con su color) y dibuja el nuevo; COLOR 0 = apagado */
static void msbar(u8 x, u8 color)
{
    u8 x1;
    if (x < 1 || x > 4) return;
    x1 = msloc[x - 1];
    if (dash_mscol[x - 1] != 0) { dial_col = dash_mscol[x - 1]; msbars(x1, (u8)(x1 + 6), 184); }       /* 4 lineas: 185..188 */
    dash_mscol[x - 1] = color;
    if (color != 0) { dial_col = color; msbars(x1, (u8)(x1 + 6), 184); }
}

/* MSBAR para quien lo llame (tactics.c) */
void dash_msbar(u8 x, u8 color) { msbar(x, color); }

/* msblob: los NOMSL primeros indicadores en verde y el resto apagados */
void dash_msblob(void)
{
    u8 x;
    for (x = 4; x != 0; x--) msbar(x, (u8)(x <= nomsl ? D_GREEN : 0));
}

/* BULB: bombilla de 7 x 8 puntos en XOR (E = ECM, S = estacion cerca) en la fila de texto 22 */
static const u8 bulb_e[8] = { 0x7F, 0x7F, 0x07, 0x7F, 0x7F, 0x07, 0x7F, 0x7F };
static const u8 bulb_s[8] = { 0x7F, 0x7F, 0x07, 0x7F, 0x7F, 0x70, 0x7F, 0x7F };

static void bulb(const u8 *rows, int x)
{
    int r, b;
    for (r = 0; r < 8; r++)
        for (b = 0; b < 7; b++)
            if (rows[r] & (1 << b)) gfx_xor_pixel(x + b, 176 + r, C_WHITE);
}

void dash_spblb(void) { bulb(bulb_s, 24 * 8); }       /* SPBLB: enciende o apaga la "S" */
void dash_ecblb(void) { bulb(bulb_e, 7 * 8); }        /* ECBLB: enciende o apaga la "E" */

void dash_update(void)
{
    u8 a, x;
    dy = 0;
    dial_k = 210;
    x = D_RED;
    if (((mcnt & 8) & flh) != 0) x = D_WHITE;  /* barras de aviso parpadeantes (FLH) */
    dial_k2 = x;
    dis2(zDELTA);                              /* velocidad */
    dial_col = D_WHITE;
    dis5((u8)(alp1 >> 1), (u8)(alp2[1] & 0x80));          /* alabeo: BIT ALP2+1 */
    dis5((u8)(bet1 << 1), (u8)(bet2[0] & 0x80));          /* cabeceo: BIT BET2 */
    a = (u8)(energy >> 1);
    do {                                       /* cuatro bancos de energia de 32 */
        dial_k1 = a;
        dis2(a);
        a = dial_k1;
        a = (a >= 32) ? (u8)(a - 32) : 0;
    } while (dy != 7);
    dial_k = 16;
    dis1(fsh);                                 /* escudo delantero */
    dis1(ash);                                 /* escudo trasero */
    dis2((u8)(qq14 >> 1));                     /* combustible (DIS1+2: solo un LSR) */
    dis1(altit);
    dis1(cabtmp);
    dis1(gntmp);
    compas();
}
