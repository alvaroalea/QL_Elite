/* Elite universe: galaxies, systems, market and descriptions. C89, no floating point.
   Based on Ian Bell's TXTELITE.C 1.5 ("defines the classic Elite universe"), with these differences:
   - the distance follows Elite's 6502 code (|dx|^2 + (|dy|/2)^2, integer root, x4), not Text Elite's floating-point sqrt;
   - the galactic hyperspace jumps to the system nearest (0x60,0x60), like the original Elite;
   - text output goes through a character function (to use txt_putc), not printf. */
#ifndef UNIVERSE_H
#define UNIVERSE_H
#include "platform.h"

typedef unsigned short u16;

typedef struct { u8 a, b, c, d; } uni_fastseed;     /* 4 bytes: description seed */
typedef struct { u16 w0, w1, w2; } uni_seed;         /* 6 bytes: galaxy seed */

typedef struct {
    u8 x, y;                  /* coordinates in the galaxy (0..255) */
    u8 economy, govtype, techlev, population;
    u16 productivity;         /* 16 bits, with the original's arithmetic (modulo 65536) */
    u16 radius;
    uni_fastseed goatsoupseed;
    u16 seed0;                /* w0 of the seed when generating the system (with goatsoupseed = w1, w2 completes the 6 bytes of QQ15) */
    char name[12];
} uni_system;

#define UNI_NSYS 256
#define UNI_NGOODS 17         /* 16 commodities + "Alien Items" */

typedef struct {
    unsigned quantity[UNI_NGOODS];
    unsigned price[UNI_NGOODS];   /* in tenths of a credit */
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
extern void (*uni_putc)(int);                 /* character output of uni_describe */

void uni_build_galaxy(int galaxynum);         /* 1..8: fills uni_galaxy */
void uni_galaxy_seed(int galaxynum, u8 *out); /* base seed of the galaxy (6 bytes, like QQ21): galaxy 1 = 4A 5A 48 02 53 B7 */
void uni_make_system(uni_seed *s, uni_system *out);   /* modifies the seed (4 rotations) */
void uni_gen_market(unsigned fluct, const uni_system *p, uni_market *m);
unsigned uni_distance(const uni_system *a, const uni_system *b);   /* in tenths of a light year (multiple of 4) */
int  uni_nearest(u8 x, u8 y);                 /* TT111: system nearest a point (in the built galaxy) */
int  uni_galactic_jump(int *galaxynum);       /* galactic hyperspace: returns the arrival system */
void uni_describe(const uni_system *p);       /* description ("goat soup") through uni_putc */
#endif
