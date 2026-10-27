/* Elite arithmetic: same semantics as the original (8-bit sign-magnitude values,
   16-bit results in (A P), etc.). The zero-page variables are exposed as globals. */
#ifndef EMATH_H
#define EMATH_H
#include "platform.h"

extern u8 zP, zP1, zQ, zR, zS, zT, zU;      /* P, P+1, Q, R, S, T, U */
extern u8 em_x;                             /* X register returned by MAD/ADD */
extern u8 em_carry;                         /* C flag returned by FMLTU */
extern u8 em_rand[4];                       /* RAND(0..3), generator seed */
extern u8 em_rand_x, em_rand_c, em_rand_v;  /* X, C and V after DORND */

u8 em_dornd(void);                          /* random A; uses em_rand_c as the incoming carry */
u8 em_dornd2(void);                         /* like DORND but with C=0 on input */

u8 em_multu(void);                          /* (A P) = P * Q  unsigned */
u8 em_mu11(u8 x);                           /* (A P) = P * X  (X nonzero) */
u8 em_squa2(u8 a);                          /* (A P) = A * A */
u8 em_squa(u8 a);                           /* idem, clearing bit 7 of A first */
u8 em_mult1(u8 a);                          /* (A P) = Q * A, sign-magnitude */
void em_mult12(u8 a);                       /* (S R) = Q * A */
u8 em_mad(u8 a);                            /* (A X) = Q * A + (S R); X en em_x */
u8 em_add(u8 a);                            /* (A X) = (A P) + (S R); X en em_x */
u8 em_mults(u8 a);                          /* (A P) = P * A with the sign of A (only the low 5 bits of P) */
u8 em_fmltu(u8 a);                          /* A = A * Q / 256 with logarithms (approximate, like the original) */
u8 em_mltu2(u8 a);                          /* (A P+1 P) = (A ~P) * Q */
u8 em_tis2(u8 a);                           /* A = A / Q, sign-magnitude, 96 = 1.0 */
u8 em_dvid96(u8 a);                         /* A = |A| * 256 / 96 (8 bits) with the sign of A */
u8 em_tis1(u8 x, u8 a);                     /* A = (-X * A + (S R)) / 96 */

/* ---- Second batch: divisions, square root, MULT3, sines ---- */
extern u8 zK[4];                            /* K(0..3) */
extern u8 zU, zDELTA;                       /* U (high byte of LL61) and DELTA (speed) */

u8 em_ll5(void);                            /* Q = SQRT(R Q), (R Q) 16 bits; returns Q */
u8 em_ll28(u8 a);                           /* R = 256 * A / Q (A < Q) with logarithms; C in em_carry (1 = overflow, R = 255) */
u8 em_logdiv(u8 a, u8 b);                    /* LOIN division by logarithms: 255 if log(a) >= log(b), otherwise alogh[log(a) - log(b)] (a, b > 0) */
u8 em_ll28_nocheck(u8 a);                   /* LL28+4: without the A >= Q check */
u8 em_ll61(u8 a);                           /* (U R) = 256 * A / Q for A >= Q; the caller sets U = 0; error: U = R = 50 */
u8 em_ll38(u8 a);                           /* (S A) = (S R) + (A Q), signs only in bit 7; C in em_carry */
u8 em_dvid4(u8 a);                          /* (P R) = 256 * A / Q; returns P; approximate remainder in R */
u8 em_dv41(u8 a);                           /* (P R) = 256 * DELTA / A */
u8 em_dv42(u8 z_hi);                        /* (P R) = 256 * DELTA / z_hi (dust particle) */
u8 em_dvidt(u8 a);                          /* (P+1 A) = (A P) / Q, sign-magnitude; returns A */
void em_mult3(u8 a);                        /* K(3 2 1 0) = (A P+1 P) * Q */
void em_dvid3b(u8 a, u8 z_lo, u8 z_hi, u8 z_sign);   /* K(3 2 1 0) = (A P+1 P) / (z_sign z_hi z_lo) (DVID3B2 with INWK already read) */
u8 em_fmltu2(u8 a);                         /* A = K * sin(A) (A + 16 = cosine), uses K(0) */
u8 em_mlu2(u8 a);                           /* (A P) = |A| * Q */
u8 em_mlu1(u8 sy, u8 *y1);                  /* Y1 = sy; (A P) = |sy| * Q */
#endif
