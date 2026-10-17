/* Demo de teclado/joystick: muestra las 8 filas de la matriz en binario, los joysticks y los caracteres tecleados.
   Sirve para validar en un QL real o emulador. Salir: SHIFT+ESC.  En el PC (backend host) dibuja un cuadro y termina. */
#include "text.h"
#include "keys.h"

static void bits8(int v, char *out)
{
    int i;
    for (i = 0; i < 8; i++) out[i] = (v & (0x80 >> i)) ? '1' : '0';
    out[8] = 0;
}

static void joystr(int j, char *s)
{
    s[0] = (j & JOY_UP) ? 'U' : '-';    s[1] = (j & JOY_DOWN) ? 'D' : '-';
    s[2] = (j & JOY_LEFT) ? 'L' : '-';  s[3] = (j & JOY_RIGHT) ? 'R' : '-';
    s[4] = (j & JOY_FIRE) ? 'F' : '-';  s[5] = 0;
}

static char typed[33];
static int ntyped;

static void draw(void)
{
    int r, ch;
    char buf[16], js[8];
    txt_color(C_YELLOW, C_BLUE);
    txt_print_at(0, 0, " PRUEBA TECLADO  (SHIFT+ESC) ");
    txt_color(C_WHITE, C_BLACK);
    for (r = 0; r < 8; r++) {
        bits8(key_raw(r), buf);
        txt_print_at(0, 2 + r, "fila ");
        txt_putc('0' + r); txt_puts(" "); txt_puts(buf);
    }
    txt_print_at(0, 11, "CTL1 "); joystr(joy_read(1), js); txt_puts(js);
    txt_print_at(0, 12, "CTL2 "); joystr(joy_read(2), js); txt_puts(js);
    while ((ch = key_getchar()) >= 0) {
        if (ch == 8) { if (ntyped > 0) ntyped--; }
        else if (ch >= 32 && ntyped < 31) typed[ntyped++] = (char)ch;
        typed[ntyped] = 0;
    }
    txt_color(C_GREEN, C_BLACK);
    txt_print_at(0, 14, "texto: "); txt_puts(typed); txt_puts("                ");
}

int main(void)
{
    plat_init(); txt_init(); key_init();
    gfx_clear(C_BLACK);
    key_set_rows(0xFF);
    for (;;) {
        key_scan();
        draw();
        plat_present();
        if (key_down(K_SHIFT) && key_down(K_ESC)) break;
#ifdef HOST_DEMO
        break;
#endif
    }
    plat_shutdown();
    return 0;
}
