/* Universo de Elite: galaxias, sistemas, mercado y descripciones. C89, sin coma flotante.
   Basado en TXTELITE.C 1.5 de Ian Bell ("define el universo clasico de Elite"), con estas diferencias:
   - la distancia sigue el codigo 6502 de Elite (|dx|^2 + (|dy|/2)^2, raiz entera, x4), no el sqrt en coma flotante de Text Elite;
   - el hiperespacio galactico salta al sistema mas cercano a (0x60,0x60), como el Elite original;
   - la salida de texto va por una funcion de caracter (para usar txt_putc), no por printf. */
#ifndef UNIVERSE_H
#define UNIVERSE_H
#include "platform.h"

typedef unsigned short u16;

typedef struct { u8 a, b, c, d; } uni_fastseed;     /* 4 bytes: semilla de la descripcion */
typedef struct { u16 w0, w1, w2; } uni_seed;         /* 6 bytes: semilla de la galaxia */

typedef struct {
    u8 x, y;                  /* coordenadas en la galaxia (0..255) */
    u8 economy, govtype, techlev, population;
    u16 productivity;         /* 16 bits, con la aritmetica del original (modulo 65536) */
    u16 radius;
    uni_fastseed goatsoupseed;
    char name[12];
} uni_system;

#define UNI_NSYS 256
#define UNI_NGOODS 17         /* 16 mercancias + "Alien Items" */

typedef struct {
    unsigned quantity[UNI_NGOODS];
    unsigned price[UNI_NGOODS];   /* en decimas de credito */
} uni_market;

typedef struct {
    u8 baseprice;
    int gradient;
    u8 basequant, maskbyte, units;   /* units: 0 = t, 1 = kg, 2 = g */
    char name[13];
} uni_good;

extern uni_system uni_galaxy[UNI_NSYS];
extern const uni_good uni_goods[UNI_NGOODS];
extern const char *uni_govnames[8];
extern const char *uni_econnames[8];
extern void (*uni_putc)(int);                 /* salida de caracteres de uni_describe */

void uni_build_galaxy(int galaxynum);         /* 1..8: rellena uni_galaxy */
void uni_make_system(uni_seed *s, uni_system *out);   /* modifica la semilla (4 giros) */
void uni_gen_market(unsigned fluct, const uni_system *p, uni_market *m);
unsigned uni_distance(const uni_system *a, const uni_system *b);   /* en decimas de anio luz (multiplo de 4) */
int  uni_nearest(u8 x, u8 y);                 /* TT111: sistema mas cercano a un punto (en la galaxia construida) */
int  uni_galactic_jump(int *galaxynum);       /* hiperespacio galactico: devuelve el sistema de llegada */
void uni_describe(const uni_system *p);       /* descripcion ("goat soup") por uni_putc */
#endif
