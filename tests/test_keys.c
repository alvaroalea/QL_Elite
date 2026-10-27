#include <stdio.h>
#include <string.h>
#include "keys.h"
u8 *screen;
void plat_init(void){} void plat_present(void){} void plat_shutdown(void){}
extern u8 host_matrix[8];
extern int host_reads;
static int fails, total;
static void check(int ok, const char *what) { total++; if (!ok) { fails++; printf("FALLO: %s\n", what); } }
static void press(u8 k) { host_matrix[KEY_ROW(k)] |= (u8)(1 << KEY_BIT(k)); }
static void release(u8 k) { host_matrix[KEY_ROW(k)] &= (u8)~(1 << KEY_BIT(k)); }

int main(void)
{
    struct controls c;
    int i;
    key_init();
    memset(host_matrix, 0, 8);

    /* encoding according to the KEYROW table: Left+Up in row 1 = 6, SHIFT in row 7 = 1 */
    check(KEY_ROW(K_LEFT) == 1 && (1 << KEY_BIT(K_LEFT)) == 2, "izquierda = fila 1 valor 2");
    check((1 << KEY_BIT(K_UP)) == 4, "arriba = fila 1 valor 4");
    check((1 << KEY_BIT(K_RIGHT)) == 16 && (1 << KEY_BIT(K_ESC)) == 8 && (1 << KEY_BIT(K_DOWN)) == 128, "derecha 16, ESC 8, abajo 128");
    check(KEY_ROW(K_SHIFT) == 7 && (1 << KEY_BIT(K_SHIFT)) == 1 && (1 << KEY_BIT(K_CTRL)) == 2 && (1 << KEY_BIT(K_ALT)) == 4, "SHIFT/CTRL/ALT en fila 7");

    /* simultaneous keys: Left + Up + Space + A (fire) + X */
    press(K_LEFT); press(K_UP); press(K_SPACE); press(K_A); press(K_X);
    host_reads = 0; key_scan();
    check(host_reads == 7, "se leen 7 filas por defecto (1..7)");
    check(key_down(K_LEFT) && key_down(K_UP) && key_down(K_SPACE) && key_down(K_A) && key_down(K_X), "cinco teclas a la vez");
    check(!key_down(K_RIGHT) && !key_down(K_S), "las demas sueltas");
    check(key_pressed(K_A) && !key_released(K_A), "flanco de pulsacion en el primer scan");
    key_scan();
    check(key_down(K_A) && !key_pressed(K_A), "sin flanco si se mantiene");
    release(K_A); key_scan();
    check(!key_down(K_A) && key_released(K_A), "flanco de soltado");

    /* flight controls */
    memset(host_matrix, 0, 8);
    press(K_LEFT); press(K_DOWN); press(K_A); press(K_SPACE); key_scan();
    ctl_read(&c);
    check(c.roll_left && c.pitch_up && c.fire && !c.speed_up && !c.roll_right && !c.slow_down && !c.pitch_down, "cursores: izquierda = alabeo izq., abajo = cabeceo arriba (el espacio dispara)");
    memset(host_matrix, 0, 8);
    press(K_D); press(K_RIGHT); press(K_UP); press(K_ESC); press(K_T); press(K_M); key_scan();
    ctl_read(&c);
    check(c.slow_down && c.roll_right && c.pitch_down && c.escape_pod && c.target && c.missile && !c.fire && !c.speed_up, "D frena, alabeo der., picar (arriba), pod, objetivo y misil");
    memset(host_matrix, 0, 8);
    press(K_F); key_scan(); ctl_read(&c);
    check(c.speed_up && !c.slow_down, "F acelera");
    memset(host_matrix, 0, 8);
    press(K_SPACE); press(K_SLASH); key_scan(); ctl_read(&c);
    check(c.fire && !c.speed_up && !c.slow_down, "el espacio dispara; / ya no frena ni el espacio acelera");
    memset(host_matrix, 0, 8);
    press(K_COMMA); press(K_PERIOD); press(K_S); press(K_X); key_scan();
    ctl_read(&c);
    check(!c.roll_left && !c.roll_right && !c.pitch_up && !c.pitch_down, ", . S X ya no giran la nave");

    /* joystick 1 (cursors+space): left + fire */
    memset(host_matrix, 0, 8);
    press(K_LEFT); press(K_SPACE); key_scan();
    check(joy_read(1) == (JOY_LEFT | JOY_FIRE), "joystick 1: izquierda + disparo");
    ctl_joystick = 1; ctl_read(&c);
    check(c.roll_left && c.fire && !c.speed_up, "con el CTL1 el espacio dispara y no acelera");
    press(K_F); key_scan(); ctl_read(&c);
    check(c.speed_up && c.fire, "con el CTL1 F acelera y el espacio sigue disparando");

    /* joystick 2 (F1..F5) in row 0 */
    memset(host_matrix, 0, 8);
    key_set_rows(0xFF);
    press(K_F4); press(K_F3); press(K_F5); key_scan();
    check(joy_read(2) == (JOY_UP | JOY_RIGHT | JOY_FIRE), "joystick 2: arriba + derecha + disparo");
    ctl_joystick = 2; ctl_read(&c);
    check(c.pitch_down && c.roll_right && c.fire, "controles con el joystick 2 (arriba = picar)");
    ctl_joystick = 0;

    /* characters */
    memset(host_matrix, 0, 8); key_set_rows(0xFF); key_scan();
    while (key_getchar() >= 0) {}
    press(K_J); key_scan();
    check(key_getchar() == 'j' && key_getchar() == -1, "j");
    release(K_J); key_scan();
    press(K_SHIFT); key_scan(); press(K_J); key_scan();
    check(key_getchar() == 'J', "SHIFT+j = J");
    release(K_J); release(K_SHIFT); key_scan();
    press(K_ENTER); key_scan(); check(key_getchar() == 13, "ENTER = 13");
    release(K_ENTER); key_scan();
    press(K_SHIFT); press(K_2); key_scan(); release(K_SHIFT); release(K_2);
    check(key_getchar() == '@', "SHIFT+2 = @ (provisional)");
    key_scan();
    press(K_CTRL); key_scan(); press(K_LEFT); key_scan();
    check(key_getchar() == 8, "CTRL+Izq = borrar (8)");
    release(K_LEFT); release(K_CTRL); key_scan();
    press(K_CTRL); key_scan(); check(key_getchar() == -1, "CTRL solo no genera caracter");
    release(K_CTRL); key_scan();
    press(K_A); press(K_B); press(K_S); key_scan();     /* three at once: all three come out, in row order */
    check(key_getchar() != -1 && key_getchar() != -1 && key_getchar() != -1 && key_getchar() == -1, "tres pulsaciones simultaneas -> tres caracteres");
    release(K_A); release(K_B); release(K_S); key_scan();

    /* any key (ignores modifiers) */
    memset(host_matrix, 0, 8); key_scan();
    press(K_SHIFT); press(K_CTRL); key_scan();
    check(!key_any(), "solo modificadores no cuenta");
    press(K_Q); key_scan();
    check(key_any(), "una tecla normal si");

    /* rows not read keep their last value */
    memset(host_matrix, 0, 8); key_set_rows(0xFF); press(K_E); key_scan();
    key_set_rows(0x02); release(K_E); key_scan();
    check(key_down(K_E), "fila 6 no leida conserva su estado");
    (void)i;
    /* read contexts: fewer rows per scan (each row is an IPC call on the real QL) */
    key_init(); memset(host_matrix, 0, 8);
    key_ctx_menu(); host_reads = 0; key_scan();
    check(host_reads == 3, "menus: 3 lecturas por scan (filas 1 y 7 + una rotando)");
    press(K_5); { int n = 0; while (!key_down(K_5) && n < 10) { key_scan(); n++; } check(key_down(K_5) && n <= 6, "menus: una tecla de la fila 0 se detecta en <= 6 scans"); }
    check(key_getchar() == '5', "menus: la pulsacion de la fila rotante genera su caracter");
    memset(host_matrix, 0, 8);
    for (i = 0; i < 8; i++) key_scan();
    press(K_LEFT); key_scan();
    check(key_down(K_LEFT) && key_pressed(K_LEFT), "menus: la fila 1 se lee en cada scan");
    memset(host_matrix, 0, 8); key_ctx_flight(); for (i = 0; i < 8; i++) key_scan();
    press(K_CTRL); key_scan(); press(K_LEFT); key_scan();
    check(key_getchar() == 8, "menus: CTRL+Izq = borrar (fila 7 siempre leida)");
    memset(host_matrix, 0, 8);
    key_ctx_flight(); host_reads = 0; key_scan();
    check(host_reads == 4, "vuelo: 4 lecturas por scan (filas 1, 3 y 4 + una rotando)");
    press(K_9); { int n = 0; while (!key_down(K_9) && n < 10) { key_scan(); n++; } check(key_down(K_9) && n <= 5, "vuelo: la fila 5 (9) se lee rotando, en <= 5 scans"); }
    memset(host_matrix, 0, 8);
    for (i = 0; i < 8; i++) key_scan();
    press(K_4); press(K_B); press(K_E); press(K_SHIFT);
    for (i = 0; i < 5; i++) key_scan();
    check(key_down(K_4) && key_down(K_B) && key_down(K_E) && key_down(K_SHIFT), "vuelo: las filas 0, 2, 6 y 7 se leen en 5 scans");
    memset(host_matrix, 0, 8);
    press(K_A); press(K_D); press(K_F); press(K_G); press(K_LEFT); key_scan();
    check(key_down(K_A) && key_down(K_D) && key_down(K_F) && key_down(K_G) && key_down(K_LEFT), "vuelo: las filas 1, 3 y 4 (cursores, F, G, A, D) se leen en cada scan");
    memset(host_matrix, 0, 8);
    ctl_joystick = 2; key_ctx_flight(); host_reads = 0; key_scan();
    check(host_reads == 5, "vuelo con joystick 2: 5 lecturas (la fila 0 pasa a rapida)");
    press(K_F1); key_scan();
    check(key_down(K_F1), "vuelo con joystick 2: la fila 0 (F1..F5) es rapida");
    ctl_joystick = 0;

    printf("%d comprobaciones, %d fallos\n", total, fails);
    return fails != 0;
}
