/* SUN, CIRCLE y sus rutinas de apoyo (Elite Apple II, port sin verificar: ver docs/PENDIENTES.md).
   SUN dibuja un sol/planeta relleno linea a linea con borde irregular (K3, K4 = centro de 16 bits, K = radio);
   el montón LSO guarda el semiancho de cada linea para borrar el dibujo anterior. CIRCLE/CIRCLE2 dibuja una circunferencia
   de K pixeles de radio con segmentos (BLINE + recorte LL145); el montón LSX2/LSY2 guarda los puntos para borrarla. */
#ifndef SUN_H
#define SUN_H
#include "platform.h"

extern int sun_yx2m1;                     /* Yx2M1: ultima linea del area de vista (135; 133 en el espacio virtual de la vista nativa) */
#define SUN_YX2M1 sun_yx2m1
void view_virtual_begin(void);            /* vista nativa: planetas, soles y circulos se calculan en el espacio del original (136 lineas) y se dibujan con 6/5 en vertical */
void view_virtual_end(void);

extern u8 sun_col;                        /* COL: color de dibujo (indice de paleta del QL) */
extern u8 sun_lso[256];                   /* LSO: semiancho de cada linea del sol */
extern u8 sun_lsx;                        /* LSX: estado del montón de lineas ($FF = vacio) */
extern u8 circ_lsx2[256], circ_lsy2[256]; /* LSX2, LSY2: puntos del ultimo circulo */
extern u8 circ_lsp, circ_flag, circ_stp, circ_cnt;   /* LSP, FLAG, STP, CNT */

void chart_xf_set(int mode);              /* transformacion de dibujo de las cartas: 0 ninguna, 1 local (4:3, centrada y mas grande), 2 galactica (8 puntos abajo y estirada 6/5) */
int  chart_xf_mode(void);
int  sun_xf_x(int x);                     /* coordenadas de pantalla a partir de las del original */
int  sun_xf_y(int y);
void sun_xf_hloin(int x1, int x2, int y, u8 col);   /* gfx_hloin con la transformacion */
void sun_xf_vloin(int x, int y1, int y2, u8 col);
void sun_xf_circle(int cx, int cy, int k, u8 col);   /* circulo de las cartas: elipse de punto medio con el aspecto de la carta */
void sun_xf_disc(int cx, int cy, int k, u8 col);   /* planeta de la carta corta (elipse rellena a 1 punto) */
void sun_xf_dust(int x, int y, u8 zz);
void sun_flflls(void);                    /* FLFLLS: vacia el montón de lineas del sol */
u8   sun_chkon(void);                     /* CHKON: 1 si el circulo (K3, K4, K) queda fuera de la vista */
void sun_draw(void);                      /* SUN: dibuja (XOR) el sol de K3, K4, K y borra el anterior */
void sun_wpls(void);                      /* WPLS: borra el sol anterior */
u8   circle_draw(void);                   /* CIRCLE: devuelve el acarreo (1 = fuera de la vista) */
u8   circle2_draw(void);                  /* CIRCLE2: como CIRCLE sin CHKON y con STP ya puesto; devuelve 0 */
void circle_wpls2(void);
void circle_update(void);                 /* vista nativa: PL9 = borra el circulo anterior y dibuja el nuevo (nada si no cambia); solo con view_native */
int  sun_rows_get(int y, int *lo, int *hi);   /* pruebas: tramo [lo, hi] que el sol de la vista nativa tiene encendido en la fila y (0 si vacia) */                  /* WPLS2: borra el circulo anterior */
extern void (*sun_hiss_hook)(int y);      /* sonido de cada anillo del tunel (SOHISS) */
void sun_get_sunx(u8 *v);                 /* pruebas: SUNX */
void sun_set_sunx(u8 lo, u8 hi);
extern void (*sun_loin_hook)(void);       /* pruebas: cada LOIN del circulo (X1..Y2 en zx15 ya ordenados, zswap) */
extern void (*sun_hloin_hook)(u8 l, u8 r, u8 y);   /* pruebas: cada HLOIN del sol que dibuja */
void sun_forget(void);                    /* tras borrar la vista: ni el sol ni el planeta figuran como dibujados */
extern void (*sun_blip_hook)(int x, int y);  /* SOBLIP de LL164 */
extern void (*sun_tt66_hook)(void);          /* TT66 de HFS2 */
extern void (*sun_circle_hook)(u8 k3, u8 k4, u8 k, u8 stp);
void tunnel_laun(void);                      /* LAUN */
void tunnel_ll164(void);                     /* LL164 */
void tunnel_draw(u8 stp);                 /* HFS2/HFS1: tunel de hiperespacio (STP = 4) o de lanzamiento (STP = 8) con el color sun_col */
#endif
