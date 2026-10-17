/* Aritmetica de Elite: misma semantica que el original (valores de 8 bits con signo-magnitud,
   resultados de 16 bits en (A P), etc.). Las variables de la pagina cero se exponen como globales. */
#ifndef EMATH_H
#define EMATH_H
#include "platform.h"

extern u8 zP, zP1, zQ, zR, zS, zT, zU;      /* P, P+1, Q, R, S, T, U */
extern u8 em_x;                             /* registro X devuelto por MAD/ADD */
extern u8 em_carry;                         /* flag C devuelto por FMLTU */
extern u8 em_rand[4];                       /* RAND(0..3), semilla del generador */
extern u8 em_rand_x, em_rand_c, em_rand_v;  /* X, C y V tras DORND */

u8 em_dornd(void);                          /* A aleatorio; usa em_rand_c como acarreo de entrada */
u8 em_dornd2(void);                         /* como DORND pero con C=0 de entrada */

u8 em_multu(void);                          /* (A P) = P * Q  sin signo */
u8 em_mu11(u8 x);                           /* (A P) = P * X  (X distinto de 0) */
u8 em_squa2(u8 a);                          /* (A P) = A * A */
u8 em_squa(u8 a);                           /* idem, borrando antes el bit 7 de A */
u8 em_mult1(u8 a);                          /* (A P) = Q * A, signo-magnitud */
void em_mult12(u8 a);                       /* (S R) = Q * A */
u8 em_mad(u8 a);                            /* (A X) = Q * A + (S R); X en em_x */
u8 em_add(u8 a);                            /* (A X) = (A P) + (S R); X en em_x */
u8 em_mults(u8 a);                          /* (A P) = P * A con signo de A (solo 5 bits bajos de P) */
u8 em_fmltu(u8 a);                          /* A = A * Q / 256 con logaritmos (aproximado, como el original) */
u8 em_mltu2(u8 a);                          /* (A P+1 P) = (A ~P) * Q */
u8 em_tis2(u8 a);                           /* A = A / Q, signo-magnitud, 96 = 1.0 */
u8 em_dvid96(u8 a);                         /* A = |A| * 256 / 96 (8 bits) con el signo de A */
u8 em_tis1(u8 x, u8 a);                     /* A = (-X * A + (S R)) / 96 */

/* ---- Segunda tanda: divisiones, raiz cuadrada, MULT3, senos ---- */
extern u8 zK[4];                            /* K(0..3) */
extern u8 zU, zDELTA;                       /* U (alto de LL61) y DELTA (velocidad) */

u8 em_ll5(void);                            /* Q = SQRT(R Q), (R Q) de 16 bits; devuelve Q */
u8 em_ll28(u8 a);                           /* R = 256 * A / Q (A < Q) con logaritmos; C en em_carry (1 = desbordado, R = 255) */
u8 em_ll28_nocheck(u8 a);                   /* LL28+4: sin la comprobacion A >= Q */
u8 em_ll61(u8 a);                           /* (U R) = 256 * A / Q para A >= Q; el llamante pone U = 0; error: U = R = 50 */
u8 em_ll38(u8 a);                           /* (S A) = (S R) + (A Q), signos solo en bit 7; C en em_carry */
u8 em_dvid4(u8 a);                          /* (P R) = 256 * A / Q; devuelve P; resto aproximado en R */
u8 em_dv41(u8 a);                           /* (P R) = 256 * DELTA / A */
u8 em_dv42(u8 z_hi);                        /* (P R) = 256 * DELTA / z_hi (particula de polvo) */
u8 em_dvidt(u8 a);                          /* (P+1 A) = (A P) / Q, signo-magnitud; devuelve A */
void em_mult3(u8 a);                        /* K(3 2 1 0) = (A P+1 P) * Q */
void em_dvid3b(u8 a, u8 z_lo, u8 z_hi, u8 z_sign);   /* K(3 2 1 0) = (A P+1 P) / (z_sign z_hi z_lo) (DVID3B2 con el INWK ya leido) */
u8 em_fmltu2(u8 a);                         /* A = K * sin(A) (A + 16 = coseno), usa K(0) */
u8 em_mlu2(u8 a);                           /* (A P) = |A| * Q */
u8 em_mlu1(u8 sy, u8 *y1);                  /* Y1 = sy; (A P) = |sy| * Q */
#endif
