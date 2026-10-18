/* Sonido del QL por IPC (comandos 10 = BEEP y 11 = BEEP OFF). C89.
   En el Apple II los sonidos son bucles que hacen "clic" en el altavoz y bloquean la CPU; en el QL el IPC genera
   el tono por su cuenta, asi que cada efecto se traduce a parametros BEEP calculados con los mismos tiempos del original.
   Avisos: (1) un BEEP nuevo cancela el anterior; por eso hay prioridades (ver snd_tick);
   (2) el manual avisa de no usar BEEP mientras se carga/descarga un job (puede colgar el QL): silenciar durante carga de ficheros;
   (3) tonos muy agudos dificultan la lectura del teclado por el IPC: el pitch minimo es SND_MIN_PITCH. */
#ifndef SOUND_H
#define SOUND_H
#include "platform.h"

#define SND_MIN_PITCH 2

#define SND_P_UI    1      /* beep, boop, campana */
#define SND_P_LASER 2
#define SND_P_BLIP  3      /* ECM, hiperespacio, siseos */
#define SND_P_BOMB  4
#define SND_P_EXPL  5
#define SND_P_DEATH 6

struct snd_params {
    u8 pitch1, pitch2;
    unsigned grad_x;       /* intervalo entre notas (unidades de ~72 us) */
    unsigned duration;     /* 1..32767 (0 seria "infinito") */
    u8 grad_y, wrap, fuzz, rnd;   /* 4 bits cada uno */
};

/* Backend: envia un bloque MT.IPCOM de 'len' bytes. Devuelve 0 si va bien. */
int snd_ipc(const u8 *blk, int len);

void snd_init(void);
extern void (*snd_dornd_hook)(void);     /* un DORND del juego (lo pone ships_hooks): SOEXPL, SOHISS y SOBOMB del original llaman a
                                            DORND una vez por clic para el retardo, y eso cambia la secuencia de numeros al azar */
void snd_enable(int on);           /* equivale a DNOIZ del original; al desactivar silencia */
int  snd_enabled(void);
void snd_off(void);                /* BEEP sin parametros */
void snd_tick(long ms);            /* el bucle principal avisa del tiempo transcurrido */
void snd_send(const struct snd_params *p, int prio);

/* Efectos del original (mismos nombres y argumentos que las rutinas del Apple II) */
void snd_sobeep(int x, int y);     /* tono: periodo x, longitud y (clics) */
void snd_soblip(int x, int y);     /* ECM (y=20) e hiperespacio (y=90): el periodo baja = tono sube */
void snd_soexpl(int y);            /* explosion: y = 15, 40, 55 o 210 */
void snd_sohiss(int y);            /* siseo: y = 0 (256), 10, 50, 120 */
void snd_sobomb(void);
void snd_lasnoise(void);
void snd_beep(void);
void snd_boop(void);
void snd_bell(void);
void snd_exno(void);               /* golpe de laser a otra nave */
void snd_exno3(void);              /* colision, contenedor o misil explotando */
void snd_exno2(void);              /* nave destruida */
void snd_death(void);

/* Conversiones (expuestas para pruebas) */
long snd_pitch_from_cycles(long cycles);          /* ciclos del 6502 entre clics -> pitch QL */
long snd_units_from(long clicks, long cycles);    /* duracion en unidades QL */
#endif
