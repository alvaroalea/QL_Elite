/* Elite game state (Apple II version): ship block INWK, ship table K%, FRIN/UNIV slots,
   zero-page workspace and global QQ* variables. C89.
   The names follow the original's in lower case; the sizes and byte order are the 6502's,
   because the arithmetic routines index these structures with offsets (INWK+10,X etc.). */
#ifndef STATE_H
#define STATE_H
#include "platform.h"

#define NI    37          /* bytes per ship block (NI%) */
#define NOSH  8           /* maximum ships in the local bubble */
#define NTY   32          /* ship types */
#define NOST  12          /* dust particles in normal space */

/* Ship types used by the routines */
#define SHIP_MSL 1        /* missile */
#define SHIP_SST 2        /* Coriolis station */
#define SHIP_ESC 3        /* escape pod */
#define SHIP_PLT 4
#define SHIP_OIL 5
#define SHIP_AST 7
#define SHIP_SPL 8
#define SHIP_SHU 9
#define SHIP_CYL 11
#define SHIP_ANA 14
#define SHIP_HER 15
#define SHIP_COPS 16
#define SHIP_SH3 17
#define SHIP_KRA 19
#define SHIP_ADA 20
#define SHIP_WRM 23
#define SHIP_CYL2 24
#define SHIP_ASP 25
#define SHIP_THG 29
#define SHIP_TGL 30
#define SHIP_CON 31
#define SHIP_DOD 32

/* Centre of the space view (X and Y). Original: 256x136, centre (128, 68). Native QL flight view (view_set_native): 256x160,
   centre (128, 80) and the projected vertical offsets are multiplied by 6/5 (aspect of the QL dots). */
#define VIEW_CX 128
#define VIEW_CY view_cy
extern int view_native;      /* 1 = 160-line view */
extern int view_yscale;      /* 1 = the projection multiplies the vertical offsets by 6/5 */
extern int view_cy;          /* vertical centre of the projection (68 or 80) */
extern int clip_h;           /* clipping height (136 or 160) */
void view_set_native(int on);
int  view_ty(int vy);        /* original y coordinate (136 lines, centre 68) to the native view's screen one */

/* ---- Ship block (INWK and each K% slot): offsets ---- */
#define I_XLO 0           /* x position: lo, hi, sign (sign-magnitude, bit 7 = sign) */
#define I_XHI 1
#define I_XSG 2
#define I_YLO 3
#define I_YHI 4
#define I_YSG 5
#define I_ZLO 6
#define I_ZHI 7
#define I_ZSG 8
#define I_NOSEV 9         /* nose vector: x lo,hi, y lo,hi, z lo,hi (9..14) */
#define I_ROOFV 15        /* roof vector (15..20) */
#define I_SIDEV 21        /* side vector (21..26) */
#define I_SPEED 27
#define I_ACC   28
#define I_ROLL  29        /* roll counter */
#define I_PITCH 30        /* pitch counter */
#define I_STATE 31        /* state/display flags */
#define I_AI    32        /* AI flags */
#define I_HEAP  33        /* line heap pointer (2 bytes: XX19) */
#define I_ENERGY 35
#define I_NEWB  36        /* "new byte flags" */

/* ---- Ship table and slots ---- */
extern u8   inwk[NI];                 /* current ship (INWK + XX19 + NEWB) */
extern u8   ks[(NOSH + 1) * NI];      /* K%: NOSH slots of NI bytes (+1 spare: UNIV has NOSH+1 entries) */
extern u8  *univ[NOSH + 1];           /* UNIV: address of each slot */
extern u8   frin[NOSH + 1];           /* FRIN: ship type of each slot (0 = free); the last one acts as terminator */
extern u8   many[NTY + 1];            /* MANY: ships of each type; many[SHIP_SST] is SSPR (station present) */
#define sspr many[SHIP_SST]
extern u8   junk_count;               /* JUNK */
extern u8   dock_auto;                /* auto: docking computer */
extern u8   ecmp;                     /* our ECM active */
extern u8   mj;                       /* MJ: in hyperspace by mission/jump */
extern u8   mstg;                     /* MSTG: slot of the missile target ($FF = none; RES2) */

/* ---- Zero-page workspace ---- */
extern u8   zP2;                      /* P+2 (P and P+1 are in emath: zP, zP1) */
extern u8   zk3[16];                  /* K3(0..9..13) and K4: K4 = zk3[14], zk3[15] (XX2 + K4 contiguous) */
#define zk4 (zk3 + 14)
extern u8   zx15[6];                  /* XX15: X1,Y1,X2,Y2 and 2 more bytes */
extern u8   zx12[6];                  /* XX12 */
extern u8   zxx16[18];                /* XX16 */
extern u8   zv[2];                    /* V: unused with real pointers (the routines receive the pointer) */

/* ---- Global QQ* variables and commander block (original sizes) ---- */
extern u8   qq0, qq1;                 /* coordinates of the current system in the galaxy */
extern u8   qq2[6];                   /* seed of the current system */
extern u8   qq3, qq4, qq5;            /* economy, government, tech level of the target system */
extern u8   qq6[2], qq7[2], qq8[2];   /* productivity, population, distance (16 bits) */
extern u8   qq9, qq10;                /* coordinates of the target system */
extern u8   qq11;                     /* current screen/view type */
extern u8   qq12;                     /* docked flag */
extern u8   qq14;                     /* fuel */
extern u8   qq15[6];                  /* seed of the target system */
extern u8   qq17;                     /* text format flags */
extern u8   qq19[6];                  /* chart cursor/crosshair data */
extern u8   qq20[17];                 /* cargo hold */
extern u8   qq21[6];                  /* galaxy seed */
extern u8   qq22[2];                  /* hyperspace counters */
extern u8   qq24, qq25, qq28, qq29;   /* price, quantity available, unit, quantity to buy */
extern u8   qq26;                     /* random market byte */
extern u8   cash[4];                  /* cash, 32 bits big-endian: cash[0] = high byte; tenths of a credit */
extern u8   gcnt;                     /* galaxy number - 1 */
extern u8   laser[4];                 /* laser of each view */
extern u8   crgo;                     /* hold capacity */
extern u8   ecm_fitted, bst, bomb, engy, dkcmp, ghyp, escp;   /* equipment */
extern u8   tribble[2], tallyl, nomsl, fist;
extern u8   avl[17];                  /* quantities available on the market */
extern u8   tally[2];                 /* kills: tally[0] = low byte (TALLY), tally[1] = high */
extern u8   tp;                       /* TP: mission status */
extern u8   cok;                      /* COK: competition flags (bit 7 = tampered data) */
extern u8   svc;                      /* SVC: save counter */

void state_clear(void);               /* zeroes the whole state and initialises UNIV */
u8  *state_slot(int n);               /* address of K% slot n (0..NOSH) */
#endif
