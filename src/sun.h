/* SUN, CIRCLE y sus rutinas de apoyo (Elite Apple II, port sin verificar: ver PENDIENTES.md).
   SUN dibuja un sol/planeta relleno linea a linea con borde irregular (K3, K4 = centro de 16 bits, K = radio);
   el montón LSO guarda el semiancho de cada linea para borrar el dibujo anterior. CIRCLE/CIRCLE2 dibuja una circunferencia
   de K pixeles de radio con segmentos (BLINE + recorte LL145); el montón LSX2/LSY2 guarda los puntos para borrarla. */
#ifndef SUN_H
#define SUN_H
#include "platform.h"

#define SUN_YX2M1 135                     /* Yx2M1: ultima linea del area de vista (136 lineas) */

extern u8 sun_col;                        /* COL: color de dibujo (indice de paleta del QL) */
extern u8 sun_lso[256];                   /* LSO: semiancho de cada linea del sol */
extern u8 sun_lsx;                        /* LSX: estado del montón de lineas ($FF = vacio) */
extern u8 circ_lsx2[256], circ_lsy2[256]; /* LSX2, LSY2: puntos del ultimo circulo */
extern u8 circ_lsp, circ_flag, circ_stp, circ_cnt;   /* LSP, FLAG, STP, CNT */

void sun_flflls(void);                    /* FLFLLS: vacia el montón de lineas del sol */
u8   sun_chkon(void);                     /* CHKON: 1 si el circulo (K3, K4, K) queda fuera de la vista */
void sun_draw(void);                      /* SUN: dibuja (XOR) el sol de K3, K4, K y borra el anterior */
void sun_wpls(void);                      /* WPLS: borra el sol anterior */
u8   circle_draw(void);                   /* CIRCLE: devuelve el acarreo (1 = fuera de la vista) */
u8   circle2_draw(void);                  /* CIRCLE2: como CIRCLE sin CHKON y con STP ya puesto; devuelve 0 */
void circle_wpls2(void);                  /* WPLS2: borra el circulo anterior */
extern void (*sun_hiss_hook)(int y);      /* sonido de cada anillo del tunel (SOHISS) */
void tunnel_draw(u8 stp);                 /* HFS2/HFS1: tunel de hiperespacio (STP = 4) o de lanzamiento (STP = 8) con el color sun_col */
#endif
