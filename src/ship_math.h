/* Elite arithmetic routines that depend on the game state (INWK, K%, K, K3, XX15, CASH).
   Each function follows the 6502 flow, including the "fall-throughs" into the next routine:
     TAS2 ends in NORM; SPS1 and SPS4 end in TAS2 (and so in NORM); TIS3 ends in DVIDT;
     TAS3 and TAS4 end in MAD; SCALEY falls into SCALEY2; LCASH falls into MCASH if there is no balance.
   The 6502 output registers are returned as a value (A) or in sm_x, sm_y, sm_c (X, Y and C).
   Positions/vectors are in sign-magnitude (bit 7 = sign), as in the original.
   There is no TAS5 in this version (Apple II). */
#ifndef SHIP_MATH_H
#define SHIP_MATH_H
#include "state.h"
#include "emath.h"

extern u8 sm_x, sm_y, sm_c;                /* X, Y and carry of the last routine */
extern u8 sm_a;                            /* A on exit from PROJ (SHPPT uses it even if it overflowed) */

/* K(3 2 1) += vector (x_sign x_hi x_lo) of INWK + x; K+3 holds the sign. Returns A (= K+3). */
u8   sm_mvt3(u8 x);
/* INWK(x) = INWK(x) + 2 * (vector of INWK + y); returns A = resulting sign without bit 7 */
u8   sm_mas1(u8 y, u8 x);
u8   sm_mas2(u8 a, u8 y);                  /* A = (a | signs x,y,z of the K% slot + y) and 0x7F */
u8   sm_mas2m(u8 y);                       /* .m entry: the same with A = 0 */
u8   sm_mas3(u8 y);                        /* A = x_hi^2 + y_hi^2 + z_hi^2 (K% slot + y), 255 on overflow */
u8   sm_farof2(u8 a);                      /* FAROF2: C = 1 if x_hi, y_hi and z_hi of INWK are <= a (FAROF: a = 224) */
u8   sm_mas4(u8 a);                        /* A = a | x_hi | y_hi | z_hi of INWK */

/* K3(0..8) = vector INWK(0..8) - vector at v (x,y,z position of another ship, 3 bytes per axis) */
void sm_vcsub(const u8 *v);
void sm_vcsu1(void);                       /* idem with v = K% slot 1 */
void sm_tas1(const u8 *v, u8 y);           /* one axis: K3[y-2 ..] = INWK[y-2 ..] - v[y-2 ..]; y = 2, 5 or 8 */
u8   sm_tas2(void);                        /* normalises K3 into XX15 (and falls into NORM) */
u8   sm_tas3(u8 y);                        /* (A X) = XX15 . vector of INWK + y (uses 3 components, step 2) */
u8   sm_tas4(u8 y);                        /* idem with the vector of K% slot 1 */
void sm_tas6(void);                        /* XX15 = -XX15 */

void sm_norm(void);                        /* XX15 = XX15 / |XX15| (1.0 = 96) */
u8   sm_tis3(u8 a, u8 x, u8 y);            /* orthogonal component for TIDY; falls into DVIDT */
void sm_tidy(void);                        /* re-orthogonalises and normalises the orientation matrix of INWK */

void sm_sps1(void);                        /* XX15 = unit vector towards the planet (K% slot 0) */
u8   sm_sps2(u8 a);                        /* (Y X) = a * 2 / 20 with sign; X returned, Y in sm_y */
void sm_sps3(u8 x);                        /* K3(x..x+2) = coordinate of K% slot 0 shifted 8 bits */
void sm_sps4(void);                        /* XX15 = unit vector towards K% slot 1 */

u8   sm_pls6(u8 a);                        /* (X K) = (a P+1 P) / z of INWK; returns the carry (1 = overflow); X in sm_x */
u8   sm_proj(void);                        /* projects INWK to the screen: K3(1 0) = x, K4(1 0) = y; returns the carry */

u8   sm_lcash(u8 x, u8 y);                 /* CASH -= (y x); carry 1 = there was a balance; otherwise leaves it as it was */
u8   sm_mcash(u8 x, u8 y);                 /* CASH += (y x); returns 0 */
u8   sm_gcash(void);                       /* (Y X) = P * Q * 4; X returned, Y in sm_y */

u8   sm_scaley2(u8 a);                     /* output carry in sm_c */
u8   sm_scaley(u8 a);
u8   sm_scalex(u8 a);
#endif
