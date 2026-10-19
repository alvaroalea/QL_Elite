/* Estado del juego Elite (version Apple II): bloque de nave INWK, tabla de naves K%, ranuras FRIN/UNIV,
   espacio de trabajo de la pagina cero y variables globales QQ*. C89.
   Los nombres siguen los del original en minusculas; los tamanos y el orden de los bytes son los del 6502,
   porque las rutinas aritmeticas indexan estas estructuras con desplazamientos (INWK+10,X etc.). */
#ifndef STATE_H
#define STATE_H
#include "platform.h"

#define NI    37          /* bytes por bloque de nave (NI%) */
#define NOSH  8           /* maximo de naves en la burbuja local */
#define NTY   32          /* tipos de nave */
#define NOST  12          /* particulas de polvo en espacio normal */

/* Tipos de nave usados por las rutinas */
#define SHIP_MSL 1        /* misil */
#define SHIP_SST 2        /* estacion Coriolis */
#define SHIP_ESC 3        /* capsula de escape */
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

/* Centro de la vista espacial (X y Y). Original: 256x136, centro (128, 68). Vista nativa del QL en vuelo (view_set_native): 256x160,
   centro (128, 80) y los desplazamientos verticales proyectados se multiplican por 6/5 (aspecto de los puntos del QL). */
#define VIEW_CX 128
#define VIEW_CY view_cy
extern int view_native;      /* 1 = vista de 160 lineas */
extern int view_yscale;      /* 1 = la proyeccion multiplica por 6/5 los desplazamientos verticales */
extern int view_cy;          /* centro vertical de la proyeccion (68 o 80) */
extern int clip_h;           /* altura de recorte (136 o 160) */
void view_set_native(int on);
int  view_ty(int vy);        /* coordenada y del original (136 lineas, centro 68) a la de pantalla de la vista nativa */

/* ---- Bloque de nave (INWK y cada ranura de K%): desplazamientos ---- */
#define I_XLO 0           /* posicion x: lo, hi, signo (signo-magnitud, bit 7 = signo) */
#define I_XHI 1
#define I_XSG 2
#define I_YLO 3
#define I_YHI 4
#define I_YSG 5
#define I_ZLO 6
#define I_ZHI 7
#define I_ZSG 8
#define I_NOSEV 9         /* vector de morro: x lo,hi, y lo,hi, z lo,hi (9..14) */
#define I_ROOFV 15        /* vector de techo (15..20) */
#define I_SIDEV 21        /* vector lateral (21..26) */
#define I_SPEED 27
#define I_ACC   28
#define I_ROLL  29        /* contador de alabeo */
#define I_PITCH 30        /* contador de cabeceo */
#define I_STATE 31        /* banderas de estado/visualizacion */
#define I_AI    32        /* banderas de IA */
#define I_HEAP  33        /* puntero del monton de lineas (2 bytes: XX19) */
#define I_ENERGY 35
#define I_NEWB  36        /* "new byte flags" */

/* ---- Tabla de naves y ranuras ---- */
extern u8   inwk[NI];                 /* nave actual (INWK + XX19 + NEWB) */
extern u8   ks[(NOSH + 1) * NI];      /* K%: NOSH ranuras de NI bytes (+1 de resguardo: UNIV tiene NOSH+1 entradas) */
extern u8  *univ[NOSH + 1];           /* UNIV: direccion de cada ranura */
extern u8   frin[NOSH + 1];           /* FRIN: tipo de nave de cada ranura (0 = libre); la ultima hace de terminador */
extern u8   many[NTY + 1];            /* MANY: naves de cada tipo; many[SHIP_SST] es SSPR (estacion presente) */
#define sspr many[SHIP_SST]
extern u8   junk_count;               /* JUNK */
extern u8   dock_auto;                /* auto: ordenador de atraque */
extern u8   ecmp;                     /* nuestro ECM activo */
extern u8   mj;                       /* MJ: en el hiperespacio por mision/jump */
extern u8   mstg;                     /* MSTG: ranura del blanco del misil ($FF = ninguno; RES2) */

/* ---- Espacio de trabajo de la pagina cero ---- */
extern u8   zP2;                      /* P+2 (P y P+1 estan en emath: zP, zP1) */
extern u8   zk3[16];                  /* K3(0..9..13) y K4: K4 = zk3[14], zk3[15] (XX2 + K4 contiguos) */
#define zk4 (zk3 + 14)
extern u8   zx15[6];                  /* XX15: X1,Y1,X2,Y2 y 2 bytes mas */
extern u8   zx12[6];                  /* XX12 */
extern u8   zxx16[18];                /* XX16 */
extern u8   zv[2];                    /* V: sin uso con punteros reales (las rutinas reciben el puntero) */

/* ---- Variables globales QQ* y bloque del comandante (tamanos del original) ---- */
extern u8   qq0, qq1;                 /* coordenadas del sistema actual en la galaxia */
extern u8   qq2[6];                   /* semilla del sistema actual */
extern u8   qq3, qq4, qq5;            /* economia, gobierno, nivel tecnologico del sistema objetivo */
extern u8   qq6[2], qq7[2], qq8[2];   /* productividad, poblacion, distancia (16 bits) */
extern u8   qq9, qq10;                /* coordenadas del sistema objetivo */
extern u8   qq11;                     /* tipo de pantalla/vista actual */
extern u8   qq12;                     /* bandera de atracado */
extern u8   qq14;                     /* combustible */
extern u8   qq15[6];                  /* semilla del sistema objetivo */
extern u8   qq17;                     /* banderas de formato de texto */
extern u8   qq19[6];                  /* datos del cursor/cruz de la carta */
extern u8   qq20[17];                 /* bodega de carga */
extern u8   qq21[6];                  /* semilla de la galaxia */
extern u8   qq22[2];                  /* contadores del hiperespacio */
extern u8   qq24, qq25, qq28, qq29;   /* precio, cantidad disponible, unidad, cantidad a comprar */
extern u8   qq26;                     /* byte aleatorio del mercado */
extern u8   cash[4];                  /* dinero, 32 bits en big-endian: cash[0] = byte alto; decimas de credito */
extern u8   gcnt;                     /* numero de galaxia - 1 */
extern u8   laser[4];                 /* laser de cada vista */
extern u8   crgo;                     /* capacidad de bodega */
extern u8   ecm_fitted, bst, bomb, engy, dkcmp, ghyp, escp;   /* equipo */
extern u8   tribble[2], tallyl, nomsl, fist;
extern u8   avl[17];                  /* cantidades disponibles en el mercado */
extern u8   tally[2];                 /* muertes: tally[0] = byte bajo (TALLY), tally[1] = alto */
extern u8   tp;                       /* TP: estado de las misiones */
extern u8   cok;                      /* COK: banderas de competicion (bit 7 = datos alterados) */
extern u8   svc;                      /* SVC: contador de guardados */

void state_clear(void);               /* pone a cero todo el estado e inicializa UNIV */
u8  *state_slot(int n);               /* direccion de la ranura n de K% (0..NOSH) */
#endif
