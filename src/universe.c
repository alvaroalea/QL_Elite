/* Universo de Elite. Port a C89 de TXTELITE.C 1.5 (C) Ian Bell, con la distancia y el salto galactico del 6502. */
#include "universe.h"
#include "emath.h"
#include "universe_text.h"

uni_system uni_galaxy[UNI_NSYS];
void (*uni_putc)(int);

/* Semillas base de la galaxia 1 */
#define BASE0 0x5A4A
#define BASE1 0x0248
#define BASE2 0xB753

/* Pares de letras: "pairs" para nombres de sistema (el primer par es nulo) y "pairs0" para nombres aleatorios de las descripciones */
static const char pairs[] = "..LEXEGEZACEBISOUSESARMAINDIREA.ERATENBERALAVETIEDORQUANTEISRION";
static const char pairs0[] = "ABOUSEITILETSTONLONUTHNOALLEXEGEZACEBISOUSESARMAINDIREA.ERATENBERALAVETIEDORQUANTEISRION";

const char *uni_govnames[8] = { "Anarchy", "Feudal", "Multi-gov", "Dictatorship", "Communist", "Confederacy", "Democracy", "Corporate State" };
const char *uni_econnames[8] = { "Rich Ind", "Average Ind", "Poor Ind", "Mainly Ind", "Mainly Agri", "Rich Agri", "Average Agri", "Poor Agri" };

/* precio base, gradiente, cantidad base, mascara, unidades, nombre */
const uni_good uni_goods[UNI_NGOODS] = {
    { 0x13, -0x02, 0x06, 0x01, 0, "Food" },
    { 0x14, -0x01, 0x0A, 0x03, 0, "Textiles" },
    { 0x41, -0x03, 0x02, 0x07, 0, "Radioactives" },
    { 0x28, -0x05, 0xE2, 0x1F, 0, "Slaves" },
    { 0x53, -0x05, 0xFB, 0x0F, 0, "Liquor/Wines" },
    { 0xC4, +0x08, 0x36, 0x03, 0, "Luxuries" },
    { 0xEB, +0x1D, 0x08, 0x78, 0, "Narcotics" },
    { 0x9A, +0x0E, 0x38, 0x03, 0, "Computers" },
    { 0x75, +0x06, 0x28, 0x07, 0, "Machinery" },
    { 0x4E, +0x01, 0x11, 0x1F, 0, "Alloys" },
    { 0x7C, +0x0D, 0x1D, 0x07, 0, "Firearms" },
    { 0xB0, -0x09, 0xDC, 0x3F, 0, "Furs" },
    { 0x20, -0x01, 0x35, 0x03, 0, "Minerals" },
    { 0x61, -0x01, 0x42, 0x07, 1, "Gold" },
    { 0xAB, -0x02, 0x37, 0x1F, 1, "Platinum" },
    { 0x2D, -0x01, 0xFA, 0x0F, 2, "Gem-Strones" },
    { 0x35, +0x0F, 0xC0, 0x07, 0, "Alien Items" }
};

/* ---- semillas ---- */

static void tweakseed(uni_seed *s)
{
    u16 temp = (u16)(s->w0 + s->w1 + s->w2);
    s->w0 = s->w1;
    s->w1 = s->w2;
    s->w2 = temp;
}

static u16 rotatel(u16 x)                       /* rotar a la izquierda un byte */
{
    u16 temp = (u16)(x & 128);
    return (u16)(2 * (x & 127) + (temp >> 7));
}

static u16 twist(u16 x) { return (u16)(256 * rotatel((u16)(x >> 8)) + rotatel((u16)(x & 255))); }

static void nextgalaxy(uni_seed *s)
{
    s->w0 = twist(s->w0);
    s->w1 = twist(s->w1);
    s->w2 = twist(s->w2);
}

void uni_make_system(uni_seed *s, uni_system *t)
{
    unsigned pair1, pair2, pair3, pair4;
    u16 longnameflag = (u16)(s->w0 & 64);
    unsigned long prod;
    int i, j;

    t->x = (u8)(s->w1 >> 8);
    t->y = (u8)(s->w0 >> 8);
    t->govtype = (u8)((s->w1 >> 3) & 7);
    t->economy = (u8)((s->w0 >> 8) & 7);
    if (t->govtype <= 1) t->economy = (u8)(t->economy | 2);

    t->techlev = (u8)(((s->w1 >> 8) & 3) + (t->economy ^ 7));
    t->techlev = (u8)(t->techlev + (t->govtype >> 1));
    if (t->govtype & 1) t->techlev++;

    t->population = (u8)(4 * t->techlev + t->economy + t->govtype + 1);

    prod = (unsigned long)((t->economy ^ 7) + 3) * (t->govtype + 4);      /* MULTU: <= 110 */
    prod = (prod * t->population) & 0xFFFFUL;                            /* MULTU: 16 bits */
    t->productivity = (u16)((prod << 3) & 0xFFFFUL);                      /* x8 con 16 bits */

    t->radius = (u16)(256 * (((s->w2 >> 8) & 15) + 11) + t->x);

    t->seed0 = s->w0;
    t->goatsoupseed.a = (u8)(s->w1 & 0xFF);
    t->goatsoupseed.b = (u8)(s->w1 >> 8);
    t->goatsoupseed.c = (u8)(s->w2 & 0xFF);
    t->goatsoupseed.d = (u8)(s->w2 >> 8);

    pair1 = 2 * ((s->w2 >> 8) & 31); tweakseed(s);
    pair2 = 2 * ((s->w2 >> 8) & 31); tweakseed(s);
    pair3 = 2 * ((s->w2 >> 8) & 31); tweakseed(s);
    pair4 = 2 * ((s->w2 >> 8) & 31); tweakseed(s);        /* siempre cuatro giros */

    t->name[0] = pairs[pair1]; t->name[1] = pairs[pair1 + 1];
    t->name[2] = pairs[pair2]; t->name[3] = pairs[pair2 + 1];
    t->name[4] = pairs[pair3]; t->name[5] = pairs[pair3 + 1];
    if (longnameflag) {
        t->name[6] = pairs[pair4]; t->name[7] = pairs[pair4 + 1];
        t->name[8] = 0;
    } else t->name[6] = 0;
    for (i = j = 0; t->name[i]; i++)                      /* quitar los puntos */
        if (t->name[i] != '.') t->name[j++] = t->name[i];
    t->name[j] = 0;
}

void uni_build_galaxy(int galaxynum)
{
    uni_seed seed;
    int g, i;
    seed.w0 = BASE0; seed.w1 = BASE1; seed.w2 = BASE2;
    for (g = 1; g < galaxynum; g++) nextgalaxy(&seed);
    for (i = 0; i < UNI_NSYS; i++) uni_make_system(&seed, &uni_galaxy[i]);
}

void uni_galaxy_seed(int galaxynum, u8 *out)
{
    uni_seed seed;
    int g;
    seed.w0 = BASE0; seed.w1 = BASE1; seed.w2 = BASE2;
    for (g = 1; g < galaxynum; g++) nextgalaxy(&seed);
    out[0] = (u8)(seed.w0 & 255); out[1] = (u8)(seed.w0 >> 8);        /* EQUW del original: byte bajo primero */
    out[2] = (u8)(seed.w1 & 255); out[3] = (u8)(seed.w1 >> 8);
    out[4] = (u8)(seed.w2 & 255); out[5] = (u8)(seed.w2 >> 8);
}

/* ---- mercado ---- */

void uni_gen_market(unsigned fluct, const uni_system *p, uni_market *m)
{
    int i, q, product, changing;
    for (i = 0; i < UNI_NGOODS; i++) {
        product = p->economy * uni_goods[i].gradient;
        changing = (int)(fluct & uni_goods[i].maskbyte);
        q = uni_goods[i].basequant + changing - product;
        q &= 0xFF;
        if (q & 0x80) q = 0;                              /* recorte a 8 bits positivos */
        m->quantity[i] = (unsigned)(q & 0x3F);            /* 6 bits */
        q = uni_goods[i].baseprice + changing + product;
        q &= 0xFF;
        m->price[i] = (unsigned)(q * 4);
    }
    m->quantity[UNI_NGOODS - 1] = 0;                      /* Alien Items: no disponible */
}

/* ---- distancia y navegacion (algoritmo del 6502: TT111 / readdistnce) ---- */

unsigned uni_distance(const uni_system *a, const uni_system *b)
{
    u8 dx = (u8)(a->x > b->x ? a->x - b->x : b->x - a->x);
    u8 dy = (u8)((a->y > b->y ? a->y - b->y : b->y - a->y) >> 1);
    u8 hi1, lo1, hi2, lo2, lo, hi;
    unsigned s;

    hi1 = em_squa2(dx); lo1 = zP;                          /* K(1 0) = dx^2 */
    hi2 = em_squa2(dy); lo2 = zP;                          /* (A P) = dy^2 */
    s = (unsigned)lo1 + lo2;
    lo = (u8)s;
    s = (unsigned)hi1 + hi2 + (s >> 8);
    hi = (s > 255) ? 255 : (u8)s;                          /* saturacion de la suma alta */
    zR = hi; zQ = lo;
    em_ll5();
    return (unsigned)zQ * 4;
}

int uni_nearest(u8 x, u8 y)
{
    int best = 0, i;
    u8 t = 127, dx, dy, d;
    for (i = 0; i < UNI_NSYS; i++) {
        dx = (u8)(uni_galaxy[i].x > x ? uni_galaxy[i].x - x : x - uni_galaxy[i].x);
        dy = (u8)(uni_galaxy[i].y > y ? uni_galaxy[i].y - y : y - uni_galaxy[i].y);
        d = (u8)((dx >> 1) + (dy >> 1));
        if (d < t) { t = d; best = i; }
    }
    return best;
}

int uni_galactic_jump(int *galaxynum)
{
    *galaxynum = (*galaxynum >= 8) ? 1 : *galaxynum + 1;
    uni_build_galaxy(*galaxynum);
    return uni_nearest(0x60, 0x60);
}

/* ---- descripciones ("goat soup") ---- */

static uni_fastseed rnd_seed;

static int gen_rnd_number(void)
{
    int a, x;
    x = (rnd_seed.a * 2) & 0xFF;
    a = x + rnd_seed.c;
    if (rnd_seed.a > 127) a++;
    rnd_seed.a = (u8)(a & 0xFF);
    rnd_seed.c = (u8)x;
    a = a / 256;                                           /* acarreo restante */
    x = rnd_seed.b;
    a = (a + x + rnd_seed.d) & 0xFF;
    rnd_seed.b = (u8)a;
    rnd_seed.d = (u8)x;
    return a;
}

static int lower(int c) { return (c >= 'A' && c <= 'Z') ? c + 'a' - 'A' : c; }

static void out(int c) { if (uni_putc) uni_putc(c); }

static void goat_soup(const char *source, const uni_system *psy)
{
    int c, rnd, i, len, x;
    for (;;) {
        c = (u8)*source++;
        if (c == 0) break;
        if (c < 0x80) { out(c); continue; }
        if (c <= 0xA4) {
            rnd = gen_rnd_number();
            goat_soup(uni_desc_list[c - 0x81][(rnd >= 0x33) + (rnd >= 0x66) + (rnd >= 0x99) + (rnd >= 0xCC)], psy);
        } else switch (c) {
        case 0xB0:                                         /* nombre del planeta */
            out(psy->name[0]);
            for (i = 1; psy->name[i]; i++) out(lower(psy->name[i]));
            break;
        case 0xB1:                                         /* <nombre>ian */
            out(psy->name[0]);
            for (i = 1; psy->name[i]; i++)
                if (psy->name[i + 1] != '\0' || (psy->name[i] != 'E' && psy->name[i] != 'I'))
                    out(lower(psy->name[i]));
            out('i'); out('a'); out('n');
            break;
        case 0xB2:                                         /* nombre aleatorio */
            len = gen_rnd_number() & 3;
            for (i = 0; i <= len; i++) {
                x = gen_rnd_number() & 0x3e;
                out(i == 0 ? pairs0[x] : lower(pairs0[x]));
                out(lower(pairs0[x + 1]));
            }
            break;
        default:
            return;
        }
    }
}

void uni_describe(const uni_system *p)
{
    rnd_seed = p->goatsoupseed;
    goat_soup("\x8F is \x97.", p);
}
