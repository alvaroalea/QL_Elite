/* Timing of text printing: characters per second and microseconds per character.
   Each test prints N characters and measures 50 Hz ticks (plat_ticks, 20 ms resolution). Results on screen. */
#include "text.h"
#include <stdlib.h>

#define N 2000L

static const char line32[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345";
static int use_sub = 0;                    /* 0: digits with % and / (as ui_num was); 1: subtractions of powers of 10 (dec_digits) */

static void put_num(unsigned long v)
{
    static const unsigned long p10[10] = { 1000000000UL, 100000000UL, 10000000UL, 1000000UL, 100000UL, 10000UL, 1000UL, 100UL, 10UL, 1UL };
    char b[12], o[12];
    int i = 0, k, n = 0;
    char d;
    if (use_sub) {
        for (k = 0; k < 10; k++) {
            d = 0;
            while (v >= p10[k]) { v -= p10[k]; d++; }
            if (d || i || k == 9) { o[n++] = (char)('0' + d); i++; }
        }
    } else {
        do { b[i++] = (char)('0' + (int)(v % 10UL)); v /= 10UL; } while (v);
        while (i) o[n++] = b[--i];
    }
    o[n] = 0;
    txt_puts(o);                                    /* like ui_num: the string at once */
}

/* prints on row 'row' "name ticks us/char"; nchars = characters printed in the test */
static void report(int row, const char *name, unsigned long t, unsigned long nchars)
{
    txt_color(C_WHITE, C_BLACK);
    txt_print_at(0, row, name);
    txt_at(13, row); put_num(t); txt_puts(" t ");
    put_num(t * 20000UL / nchars); txt_puts(" us/c");
}

static unsigned long run_putc(int transp)
{
    long i;
    unsigned long t0;
    txt_color(C_YELLOW, transp ? TXT_TRANSPARENT : C_BLUE);
    txt_at(0, 0);
    t0 = plat_ticks();
    for (i = 0; i < N; i++) {
        txt_putc('A' + (int)(i & 15));
        if ((i & 31) == 31) txt_at(0, ((int)(i >> 5) & 7));          /* back to rows 0..7 */
    }
    return plat_ticks() - t0;
}

static unsigned long run_puts(void)
{
    long i;
    unsigned long t0;
    txt_color(C_YELLOW, C_BLUE);
    t0 = plat_ticks();
    for (i = 0; i < N / 32; i++) { txt_at(0, (int)(i & 7)); txt_puts(line32); }
    return plat_ticks() - t0;
}

/* 15 characters per iteration: 7 digits and 8 spaces */
static unsigned long run_num(void)
{
    long i;
    unsigned long t0;
    txt_color(C_YELLOW, C_BLUE);
    t0 = plat_ticks();
    for (i = 0; i < N / 8; i++) { txt_at(0, (int)(i & 7)); put_num(1234567UL + (unsigned long)i); txt_puts("        "); }
    return plat_ticks() - t0;
}

/* Calibration: time of a trivial C loop (machine speed without text or screen) */
static unsigned long run_loop(long n)
{
    long i;
    unsigned long t0, s = 0;
    t0 = plat_ticks();
    for (i = 0; i < n; i++) s += (unsigned long)i;
    if (s == 1UL) txt_putc(' ');                                  /* so it is not optimised away */
    return plat_ticks() - t0;
}

int main(void)
{
    unsigned long t_loop = 0;
    unsigned long t_op, t_tr, t_pu, t_n1, t_n2, t_ram;
    u8 *video, *ram;
    plat_init(); txt_init(); gfx_clear(C_BLACK);
    t_op = run_putc(0);
    t_tr = run_putc(1);
    t_pu = run_puts();
    t_n1 = run_num();
    use_sub = 1;
    t_n2 = run_num();
    use_sub = 0;
    t_ram = 0;
    ram = (u8 *)malloc(32768L);                 /* the same opaque putc, but writing to normal RAM and not video RAM */
    if (ram) {
        video = screen; screen = ram;
        t_ram = run_putc(0);
        screen = video;
    }
    t_loop = run_loop(100000L);
    gfx_clear(C_BLACK);
    report(12, "putc opaco", t_op, N);
    report(13, "putc transp.", t_tr, N);
    report(14, "puts 32 car.", t_pu, N);
    report(15, "num % /", t_n1, (N / 8) * 15);
    report(16, "num restas", t_n2, (N / 8) * 15);
    if (ram) report(17, "putc en RAM", t_ram, N);
    report(18, "bucle 100000", t_loop, 100000UL);
    txt_color(C_CYAN, C_BLACK);
    txt_print_at(0, 22, "t = cuadros de 20 ms");
#ifdef ASM_TEXT
    txt_print_at(0, 23, "ql_text.asm: SI");
#else
    txt_print_at(0, 23, "ql_text.asm: NO (C)");
#endif
    plat_present();
    plat_wait_key();
    plat_shutdown();
    plat_exit();
    return 0;
}
