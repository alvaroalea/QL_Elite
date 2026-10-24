/* Backend QL (XTC68 / qcc). SIN PROBAR en hardware: validar en QL real o emulador.

   Consola: el runtime de c68 abre una ventana (centrada, con marco rojo y cursor). El juego no la usa
   (dibuja directo en $20000 y lee el teclado con KEYROW), pero el IPC sigue metiendo cada pulsacion en la
   cola de teclado de esa ventana, que es la que tiene el foco. Si nadie la lee, al salir SuperBASIC recibe
   todas las teclas. Por eso: (1) la ventana se redefine a pantalla completa, negra y sin cursor,
   (2) la cola se vacia al arrancar, en cada plat_present() y al salir (atexit). */
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <qdos.h>

u8 *screen = (u8 *)0x20000L;               /* pantalla 1 del QL estandar */

/* ql_ipc.asm: trap #3 generico; devuelve D0 (0 = ok, negativo = error QDOS) */
extern long ql_trap3(long d0, long d1, long d2, long d3, long a0, long a1);
extern long ql_trap1(long d0, long d1, long d2, long d3);

/* Operaciones de trap #3 usadas */
#define IO_FBYTE 0x01L                      /* D3 = timeout; D1.B = byte; ERR.NC (-1) si no hay nada */
#define SD_WDEF  0x0DL                      /* D1.B = color borde, D2.W = ancho borde, A1 -> 4 words w,h,x,y */
#define SD_CURE  0x0EL                      /* activa el cursor */
#define SD_CURS  0x0FL                      /* suprime el cursor */
#define SD_CLEAR 0x20L                      /* borra toda la ventana */
#define FLUSH_MAX 512                       /* tope de bytes por vaciado, por si el canal devolviera datos sin fin */

static long con_in = -1L;                   /* canal QDOS de stdin (lectura) */
static long con_out = -1L;                  /* canal QDOS de stdout (ventana) */
extern void ql_tick_on(void);
extern void ql_tick_off(void);
extern long ql_ticks(void);
static int tick_up = 0;
static long last_tick = 0;
static int  con_up = 0;                     /* plat_init hecho y plat_shutdown pendiente */
static short win_def[4];                    /* ancho, alto, x, y */

/* Vacia la cola de teclado de la consola sin esperar (timeout 0). */
void plat_flush_input(void)
{
    int n;
    if (con_in < 0L) return;
    for (n = 0; n < FLUSH_MAX; n++)
        if (ql_trap3(IO_FBYTE, 0L, 0L, 0L, con_in, 0L) != 0L) break;
}

void plat_init(void)
{
    short mode = 8, tv = -1;               /* modo 8, sin cambiar TV */
    mt_dmode(&mode, &tv);
    /* NOTA: escribir directo en $20000 puede requerir modo supervisor (trap #0) o SD.EXTOP */

    con_in  = (long)fgetchid(stdin);
    con_out = (long)fgetchid(stdout);

    if (con_out >= 0L) {
        win_def[0] = 512; win_def[1] = 256; win_def[2] = 0; win_def[3] = 0;   /* pantalla completa */
        ql_trap3(SD_WDEF, 0L, 0L, -1L, con_out, (long)win_def);              /* borde negro, ancho 0 */
        ql_trap3(SD_CLEAR, 0L, 0L, -1L, con_out, 0L);
        ql_trap3(SD_CURS, 0L, 0L, -1L, con_out, 0L);                         /* sin cursor */
    }
    plat_flush_input();
    if (!con_up) { con_up = 1; atexit(plat_shutdown); }
#ifndef NOTICK
    if (!tick_up) { ql_tick_on(); tick_up = 1; last_tick = ql_ticks(); }
#endif
}

/* Espera de tecla para los demos: sin ella el runtime de c68 borra la pantalla al volver a QDOS.
   Lee la matriz con el mismo comando IPC que keys_ql.c (KEYROW), pero sin depender de keys.c. */
extern long ql_ipc(char *blk);
static long wk_blk[2];                      /* long: direccion par */

static int wk_row(int row)
{
    char *b = (char *)wk_blk;
    long r;
    b[0] = 9; b[1] = 1; b[2] = 0; b[3] = 0; b[4] = 0; b[5] = 0; b[6] = (char)row; b[7] = 2;
    r = ql_ipc(b);
    return (r < 0L) ? 0 : (int)(r & 0xFF);
}

static int wk_any(void)                      /* alguna tecla (salvo SHIFT/CTRL/ALT) */
{
    int r, v;
    for (r = 0; r < 8; r++) {
        v = wk_row(r);
        if (r == 7) v &= 0xF8;
        if (v) return 1;
    }
    return 0;
}

/* directorios por defecto que el runtime de c68 toma de Toolkit II (DATA_USE / PROG_USE) al arrancar;
   sin Toolkit II quedan vacios */
int plat_default_dir(int prog, char *buf, int max)
{
    buf[0] = 0;
    if (!(prog ? getcpd(buf, max) : getcdd(buf, max))) return 0;
    return buf[0] != 0;
}

void plat_wait_key(void)
{
    while (wk_any()) plat_flush_input();    /* primero que suelte la tecla con la que lanzo el programa */
    while (!wk_any()) plat_flush_input();
    while (wk_any()) plat_flush_input();    /* y que la suelte: asi SuperBASIC no la recibe */
    plat_flush_input();
}

/* Se llama una vez por fotograma desde el bucle: aprovecha para vaciar la cola de teclado. */

unsigned long plat_ticks(void) { return tick_up ? (unsigned long)ql_ticks() : 0UL; }

/* Limite de 50 fotogramas por segundo: si el fotograma ha durado menos de un cuadro (20 ms), se espera al siguiente. Un fotograma que
   ya ha durado un cuadro o mas (un QL normal: 5-10 fps) no espera nada; solo frena a un QL rapido (Super Gold Card o similar) */
void plat_present(void)
{
    plat_flush_input();
    if (tick_up) {
        while (ql_ticks() == last_tick) { }
        last_tick = ql_ticks();
    }
}

void plat_shutdown(void)
{
    if (tick_up) { ql_tick_off(); tick_up = 0; }      /* antes de terminar: la tarea del reloj esta en nuestro codigo */
    if (!con_up) return;                    /* idempotente: tambien lo llama atexit */
    con_up = 0;
    plat_flush_input();
    if (con_out >= 0L) {
        ql_trap3(SD_CLEAR, 0L, 0L, -1L, con_out, 0L);
        ql_trap3(SD_CURE, 0L, 0L, -1L, con_out, 0L);                         /* devuelve el cursor */
    }
}

/* Salida sin el \"Press a key to exit\" del runtime de c68: se limpia la consola y el trabajo se elimina a si mismo
   con MT.FRJOB (trap #1, D0=5, D1=-1 = este trabajo, D3 = codigo de error 0). No vuelve. Si por lo que sea falla,
   plat_exit regresa y el programa termina por el camino normal. */
void plat_exit(void)
{
    plat_shutdown();
    ql_trap1(5L, -1L, 0L, 0L);
}
