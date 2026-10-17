#include "text.h"
#include <math.h>

int main(void)
{
    int a, c;
    char s[2];
    double r;
    plat_init(); txt_init(); gfx_clear(C_BLACK);

    /* fondo de lineas para probar el papel transparente */
    for (a = 0; a < 360; a += 10) {
        r = a * 3.14159265 / 180.0;
        gfx_line(128, 150, 128 + (int)(100 * cos(r)), 150 + (int)(70 * sin(r)), (u8)((a / 10) % 7 + 1), 0);
    }
    txt_color(C_YELLOW, C_BLUE);
    txt_print_at(0, 0, "  ELITE QL - PRUEBA DE TEXTO    ");
    txt_color(C_WHITE, C_BLACK);
    txt_print_at(0, 2, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    txt_print_at(0, 3, "abcdefghijklmnopqrstuvwxyz");
    txt_print_at(0, 4, "0123456789 !\"#$%&'()*+,-./:;<=>?@");
    for (c = 1; c < 8; c++) {                         /* una linea por color de tinta */
        txt_color((u8)c, C_BLACK);
        txt_at(c * 4, 6); s[0] = 'A' + c; s[1] = 0; txt_puts("TXT"); txt_puts(s);
    }
    txt_color(C_GREEN, TXT_TRANSPARENT);
    txt_print_at(8, 14, "PAPEL TRANSPARENTE");
    txt_color(C_WHITE, C_RED);
    txt_print_at(0, 28, "Ajuste de linea: esta frase es mas larga que treinta y dos columnas.");
    txt_color(C_CYAN, C_BLACK);
    txt_print_at(0, 22, "Comandante: JAMESON");
    txt_cursor(1);                                    /* cursor visible tras el texto */
    plat_present();
    plat_wait_key();
    plat_shutdown();
    plat_exit();                                  /* QL: sin "Press a key to exit" */
    return 0;
}
