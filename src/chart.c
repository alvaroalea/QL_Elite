#include "chart.h"
#include "lines.h"
#include "sun.h"
#include "state.h"
#include "emath.h"
#include "ship_math.h"
#include "game.h"
#include "text.h"

#include "cmdr.h"
#define YHALF_VIEW 68                  /* Y del original: mitad de la altura de la vista */
#define GCYT CHART_GCYT
#define GCYB CHART_GCYB

static u8 q19[6];                                 /* QQ19..QQ19+5 */

void (*chart_line_hook)(u8 kind, u8 a, u8 b, u8 c) = 0;
void (*chart_event_hook)(char kind, u8 a, u8 b, u8 c) = 0;   /* pruebas: PIXEL (p), TT128 (c), SUN (s), cpl (n) */
#define EV(k, a, b, c) do { if (chart_event_hook) chart_event_hook(k, (u8)(a), (u8)(b), (u8)(c)); } while (0)
static void hloin(u8 x1, u8 x2, u8 y) { if (chart_line_hook) chart_line_hook(0, x1, x2, y); sun_xf_hloin(x1, x2, y, sun_col); }   /* HLOIN del original (gfx.c) */
static void vloin(u8 x, u8 y1, u8 y2) { if (chart_line_hook) chart_line_hook(1, x, y1, y2); sun_xf_vloin(x, y1, y2, sun_col); }   /* VLOIN (CPIX: 2 de ancho) */
/* DVLOIN: VLOIN de (A, GCYT) a (A, GCYB) con el color de COL (vertical del marco de la galactica) */
static void dvloin(u8 x) { vloin(x, GCYT, GCYB); }

/* TT15: cruz de tamano QQ19+2 centrada en (QQ19, QQ19+1); local = QQ11 negativo */
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

/* SIGHT: mira del laser de la vista: dos cruces concentricas (tamanos 20 y 10) centradas en (128, Y-24) con el color segun el laser
   (pulsos azul, haz rojo, militar blanco, otros blanco). Si no hay laser en esa vista no dibuja nada. Es TT15 de la vista espacial
   (QQ11 = 0): el centro baja GCYT puntos y los brazos se recortan como en la galactica. */
void chart_sight(u8 view)
{
    u8 l = laser[view & 3], col;
    int m;
    if (l == 0) return;
    if (l == LASER_POW) col = C_BLUE;
    else if (l == LASER_BEAM) col = C_RED;
    else col = C_WHITE;
    m = chart_xf_mode();
    sun_col = col;
    if (view_native) chart_xf_set(3);              /* vista nativa: las cruces se estiran 6/5 en vertical y se centran en 80 */
    q19[0] = 128; q19[1] = (u8)(YHALF_VIEW - 24); q19[2] = 20;
    tt15(0);
    q19[2] = 10;
    tt15(0);
    chart_xf_set(m);
}

/* TT14: cruz del sistema actual y circulo del alcance de combustible */
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
    circle2_draw();
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

/* acarreo con que cpl vuelve: el del ultimo TT54 (giro de semilla: w2 = w0 + w1 + w2 en 16 bits) */
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

/* TT23: carta local */
static void tt23(void)
{
    u8 rows[256];                                  /* INWK+Y; con Y = 255 el original lee la pagina cero en INWK-1 = SC+1 (nunca 0) */
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
        d = (u8)(p->x - qq0);                      /* LDA QQ15+3; SEC; SBC QQ0; ASL; ASL; ADC #140 (el acarreo de ASL suma) */
        c = (u8)((d >> 6) & 1);
        s = (unsigned)(u8)(d << 2) + 140 + c;
        xx12 = sm_scaley2((u8)s);
        col = (u8)((xx12 >> 3) + 1);               /* XC */
        d = (u8)(p->y - qq1);                      /* ASL A; ADC #99 */
        c = (u8)(d >> 7);
        s = (unsigned)(u8)(d << 1) + 99 + c;
        k4 = sm_scaley2((u8)s);
        c = (u8)((k4 >> 2) & 1);                   /* carry tras LSR A x3 */
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
        if (chart_xf_mode()) put_name(sun_xf_x((int)col * 8) >> 3, sun_xf_y((int)y * 8) >> 3, p->name);   /* con la transformacion: la celda del nombre (columna y fila del original) en pantalla */
        else put_name(col, y, p->name);
        c = cpl_carry(p);
ee1:
        zk3[1] = 0; zk4[1] = 0; zK[1] = 0;
        zk3[0] = xx12; zk4[0] = k4;
        zK[0] = (u8)((p->goatsoupseed.d & 1) + 2 + c);       /* LDA QQ15+5; AND #1; ADC #2 */
        (void)row;
        EV('s', zk3[0], zk4[0], zK[0]);
        if (chart_xf_mode() == 1) sun_xf_disc(zk3[0], zk4[0], zK[0], C_RED);   /* pantalla: elipse rellena de 1 punto */
        else { sun_flflls(); sun_draw(); sun_flflls(); }
    }
}

/* TT22: carta galactica */
static void tt22(void)
{
    int i;
    u8 x, y;
    sun_col = C_BLUE;                              /* NLIN2 pone COL = BLUE y el marco la hereda */
#if CHART_TOP_FULLWIDTH
    { int m = chart_xf_mode(); chart_xf_set(0); hloin(0, 255, GCYT - 1); chart_xf_set(m); }   /* NLIN5 -> NLIN2: X1 = 0, X2 = 255 (linea bajo el titulo, sin bajar con el mapa) */
    if (chart_xf_mode()) gfx_hline_xor(30, 226, sun_xf_y(GCYT - 1), sun_col);   /* arista superior del marco: de palo a palo (30..226), baja con el mapa */
#else
    hloin(30, 228, GCYT - 1);                      /* variante: la linea superior solo cierra el marco */
#endif
    if (chart_xf_mode()) gfx_hline_xor(30, 226, sun_xf_y(GCYB + 1), sun_col);   /* arista inferior: de palo a palo */
    else hloin(31, 228, GCYB + 1);
    dvloin(30);                                    /* DVLOIN */
    dvloin(226);
    tt14(0);
    for (i = 0; i < UNI_NSYS; i++) {
        x = sm_scalex(uni_galaxy[i].x);
        y = (u8)(sm_scaley(uni_galaxy[i].y) + GCYT);
        EV('p', x, y, 255);
        sun_xf_dust(x, y, 255);                    /* PIXEL con ZZ = 255: dos puntos */
    }
    chart_cursor(0);
}

/* TT123: QQ19+4 = coordenada + delta (con signo), sin moverla si da la vuelta por arriba o por abajo */
static u8 tt123(u8 a, u8 d)
{
    unsigned s = (unsigned)a + d;
    if (!(d & 0x80)) return (u8)(s > 255 ? a : s);            /* BCC TT125; RTS (C = 1: se pasaria de 255) */
    return (u8)(s > 255 ? s : a);                             /* TT124: delta negativo, C = 0 -> bajaria de 0 */
}

/* TT16: mueve la mira de las cartas (X = delta x, Y = delta y hacia arriba): borra la cruz (TT103), mueve QQ10 con -Y y QQ9 con
   X (TT123) y la vuelve a dibujar (cae en TT103) */
void chart_tt16(int local, u8 x, u8 y)
{
    u8 dy = (u8)((u8)(y - 1) ^ 0xFF);              /* DEY; TYA; EOR #$FF */
    chart_cursor(local);
    qq10 = tt123(qq10, dy);
    qq9 = tt123(qq9, x);
    chart_cursor(local);
}

/* ping: el cursor vuelve al sistema actual (QQ9, QQ10 = QQ0, QQ1) */
void chart_ping_set(void) { qq9 = qq0; qq10 = qq1; }   /* ping: QQ9/QQ10 = QQ0/QQ1 */

void chart_ping(int local)
{
    chart_cursor(local);                           /* TT103: borra */
    chart_ping_set();
    game_set_target(cur_sys);
    chart_cursor(local);                           /* TT103: dibuja */
}

/* hm: selecciona el sistema mas cercano al cursor y redibuja la cruz (TT103, TT111, TT103) */
void chart_hm(int local)
{
    chart_cursor(local);
    game_tt111();                                  /* TT111: tambien coloca la cruz sobre el sistema */
    chart_cursor(local);
}

/* HME2 (parte de datos): busca un sistema por nombre, sin distinguir mayusculas; devuelve su indice o -1.
   Compara el nombre completo, como el original (que compara con el texto que imprimiria cpl). */
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
    nlin4();                                       /* NLIN3: titulo (lo pone menu.c) y la linea de NLIN4 en y = 19 */
    sun_flflls();
    tt14(1);                                       /* TT14 -> TT126 */
    chart_cursor(1);                               /* TT103 -> TT105 */
    tt23();
}
