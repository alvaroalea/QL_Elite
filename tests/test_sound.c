#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "sound.h"
u8 *screen;
extern u8 host_ipc_log[16][24];
extern int host_ipc_len[16];
extern int host_ipc_n;
static int fails, total;
#define CHECK(c, msg) do { total++; if (!(c)) { fails++; printf("FALLO: %s\n", msg); } } while (0)
static void reset(void) { host_ipc_n = 0; snd_init(); }

int main(void)
{
    long c;
    int bad;
    struct snd_params p;

    /* conversiones contra la formula en coma flotante (tolerancia 1) */
    bad = 0;
    for (c = 100; c <= 4000; c += 7) {
        double pf = 22917.0 * c / 1020500.0 - 9.6;
        long pi = snd_pitch_from_cycles(c);
        long want = (long)floor(pf + 0.5);
        if (want < SND_MIN_PITCH) want = SND_MIN_PITCH;
        if (labs(pi - want) > 1) bad++;
    }
    CHECK(bad == 0, "pitch entero ~ formula de coma flotante (+-1)");
    CHECK(snd_pitch_from_cycles(1) == SND_MIN_PITCH, "pitch minimo respetado");
    CHECK(snd_pitch_from_cycles(100000L) == 255, "pitch maximo 255");
    CHECK(snd_units_from(30, 561) >= 377 && snd_units_from(30, 561) <= 379, "duracion BEEP ~378 pasos de 43,64 us (16,5 ms)");
    CHECK(snd_units_from(256, 100000L) == 32767, "duracion limitada a 32767");

    /* bloque IPC de BEEP: formato verificado con el codigo de ejemplo de la comunidad QL */
    reset();
    p.pitch1 = 5; p.pitch2 = 9; p.grad_x = 0x1234; p.duration = 0x0ABC; p.grad_y = 0x1F; p.wrap = 3; p.fuzz = 7; p.rnd = 0x1A;
    snd_send(&p, SND_P_UI);
    CHECK(host_ipc_n == 1 && host_ipc_len[0] == 15, "un bloque de 15 bytes");
    {   /* 8 bytes de parametros, 16 bits con el byte bajo primero, nibbles paso/vuelta y aleatorio/difuso, respuesta 1 = ninguna */
        const u8 e[15] = { 10, 8, 0, 0, 0xAA, 0xAA, 5, 9, 0x34, 0x12, 0xBC, 0x0A, 0xF3, 0xA7, 1 };
        CHECK(memcmp(host_ipc_log[0], e, 15) == 0, "bytes del bloque BEEP (formato del 8049)");
    }
    reset();
    snd_off();
    {
        const u8 e[7] = { 11, 0, 0, 0, 0, 0, 1 };
        CHECK(host_ipc_len[0] == 7 && memcmp(host_ipc_log[0], e, 7) == 0, "bloque BEEP OFF");
    }

    /* efectos: valores esperados a partir de los tiempos del original */
    reset(); snd_beep();
    CHECK(host_ipc_log[0][6] == 3 && host_ipc_log[0][7] == 0, "BEEP: pitch 3 (22917 / (2 * 12,6) = 909 Hz)");
    {
        unsigned d = (unsigned)(host_ipc_log[0][11] << 8 | host_ipc_log[0][10]);
        CHECK(d >= 377 && d <= 379, "BEEP: ~16,5 ms");
    }
    reset(); snd_boop();
    CHECK(host_ipc_log[0][6] >= 17 && host_ipc_log[0][6] <= 19, "BOOP: pitch ~18 (~397 Hz)");
    {
        unsigned d = (unsigned)(host_ipc_log[0][11] << 8 | host_ipc_log[0][10]);
        CHECK(d >= 2805 && d <= 2905, "BOOP: ~125 ms");
    }
    reset(); snd_lasnoise();
    CHECK(host_ipc_log[0][6] < host_ipc_log[0][7] && (host_ipc_log[0][12] >> 4) >= 1 && (host_ipc_log[0][12] >> 4) <= 7, "laser: tono que baja (pitch1 < pitch2, paso positivo)");
    reset(); snd_soblip(224, 20);                                    /* ECM: el tono sube */
    CHECK((host_ipc_log[0][12] >> 4) >= 9, "ECM: paso negativo (tono que sube)");
    reset(); snd_soblip(255, 90);                                    /* hiperespacio */
    CHECK(host_ipc_log[0][6] < host_ipc_log[0][7] && (host_ipc_log[0][12] >> 4) >= 9, "hiperespacio: sube; pitch1 < pitch2");
    reset(); snd_soexpl(55);
    CHECK((host_ipc_log[0][13] & 15) != 0 && (host_ipc_log[0][13] >> 4) != 0, "explosion: con fuzz y aleatoriedad");
    reset(); snd_sohiss(0);
    {
        unsigned d = (unsigned)(host_ipc_log[0][11] << 8 | host_ipc_log[0][10]);
        CHECK(d > 4950 && d < 6270, "siseo largo (256 clics) ~245 ms");
    }
    reset(); snd_sobomb();
    CHECK(host_ipc_n == 1 && (host_ipc_log[0][13] & 15) == 15, "bomba: ruido");

    /* prioridades: un sonido menos importante no pisa a uno en curso */
    reset(); snd_exno2();
    snd_lasnoise();
    CHECK(host_ipc_n == 1, "el laser no interrumpe una explosion");
    snd_tick(50);
    snd_lasnoise();
    CHECK(host_ipc_n == 1, "50 ms despues la explosion sigue sonando");
    snd_tick(500);
    snd_lasnoise();
    CHECK(host_ipc_n == 2, "pasada la explosion, el laser suena");
    snd_death();
    CHECK(host_ipc_n == 3, "la muerte (prioridad mayor) interrumpe");
    snd_exno();
    CHECK(host_ipc_n == 3, "una explosion menor no interrumpe la muerte");

    /* activar/desactivar (DNOIZ) */
    reset(); snd_enable(0);
    CHECK(host_ipc_n == 1 && host_ipc_log[0][0] == 11, "desactivar envia BEEP OFF");
    snd_beep(); snd_soexpl(15);
    CHECK(host_ipc_n == 1, "con el sonido desactivado no se envia nada");
    snd_enable(1); snd_beep();
    CHECK(host_ipc_n == 2, "reactivado vuelve a sonar");

    printf("%d comprobaciones, %d fallos\n", total, fails);
    return fails != 0;
}
