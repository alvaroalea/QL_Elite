/* Demo: ficha de un sistema (datos, descripcion y mercado) con el modulo de texto. En el PC dibuja un cuadro y termina. */
#include "text.h"
#include "universe.h"

static char desc[512];
static int dlen;
static void cap(int c) { if (dlen < 511) desc[dlen++] = (char)c; desc[dlen] = 0; }

/* escribe una cadena con ajuste por palabras a 32 columnas */
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

static void num(unsigned v, int width)
{
    char b[8]; int i = 0, k;
    do { b[i++] = (char)('0' + v % 10); v /= 10; } while (v);
    for (k = i; k < width; k++) txt_putc(' ');
    while (i) txt_putc(b[--i]);
}

int main(void)
{
    const uni_system *p;
    uni_market m;
    int i, galaxy = 1, sys = 7;
    static const char unitc[3] = { 't', 'k', 'g' };

    plat_init(); txt_init(); gfx_clear(C_BLACK);
    uni_putc = cap;
    uni_build_galaxy(galaxy);
    p = &uni_galaxy[sys];

    txt_color(C_YELLOW, C_BLUE);
    txt_print_at(0, 0, "                                ");
    txt_print_at(10, 0, "DATA ON "); txt_puts(p->name);
    txt_color(C_WHITE, C_BLACK);
    txt_print_at(0, 2, "Economy: ");    txt_color(C_GREEN, C_BLACK);  txt_puts(uni_econnames[p->economy]);
    txt_color(C_WHITE, C_BLACK); txt_print_at(0, 3, "Government: "); txt_color(C_GREEN, C_BLACK); txt_puts(uni_govnames[p->govtype]);
    txt_color(C_WHITE, C_BLACK); txt_print_at(0, 4, "Tech.Level: "); txt_color(C_GREEN, C_BLACK); num(p->techlev + 1, 0);
    txt_color(C_WHITE, C_BLACK); txt_print_at(0, 5, "Population: "); txt_color(C_GREEN, C_BLACK); num(p->population, 0);
    txt_color(C_WHITE, C_BLACK); txt_print_at(0, 6, "Gross Productivity: "); txt_color(C_GREEN, C_BLACK); num(p->productivity, 0); txt_puts(" M CR");
    txt_color(C_WHITE, C_BLACK); txt_print_at(0, 7, "Average Radius: "); txt_color(C_GREEN, C_BLACK); num(p->radius, 0); txt_puts(" km");
    txt_color(C_CYAN, C_BLACK); txt_at(0, 9);
    uni_describe(p);
    put_wrapped(desc);

    txt_color(C_YELLOW, C_BLUE); txt_print_at(0, 14, "PRODUCT      UNIT  PRICE QTY ");
    uni_gen_market(0xAB, p, &m);
    txt_color(C_WHITE, C_BLACK);
    for (i = 0; i < UNI_NGOODS - 1; i++) {
        txt_at(0, 15 + i); txt_puts(uni_goods[i].name);
        txt_at(13, 15 + i); txt_putc(unitc[uni_goods[i].units]);
        txt_at(17, 15 + i); num(m.price[i] / 10, 3); txt_putc('.'); txt_putc((char)('0' + m.price[i] % 10));
        txt_at(25, 15 + i); num(m.quantity[i], 3);
    }
    plat_present();
    return 0;
}
