/* Texto 8x8 sobre la pantalla del QL (modo 8): 32 columnas x 32 filas. Alineado a celdas de 8 pixeles. */
#ifndef TEXT_H
#define TEXT_H
#include "platform.h"

#define TXT_COLS 32
#define TXT_ROWS 32
#define TXT_TRANSPARENT (-1)       /* papel transparente: respeta lo que haya dibujado debajo */

void txt_init(void);                       /* tablas internas; llamar tras plat_init() */
void txt_color(u8 ink, int paper);         /* tinta 0..7; papel 0..7 o TXT_TRANSPARENT */
void txt_at(int col, int row);             /* posiciona el cursor (se recorta a la pantalla) */
int  txt_col(void);
int  txt_row(void);
void txt_putc(int ch);                     /* \n = CR+LF, \r = columna 0, \b = atras, resto ASCII 32..127 */
void txt_puts(const char *s);
void txt_print_at(int col, int row, const char *s);
void txt_clear_rows(int row0, int nrows, u8 color);   /* borra filas de texto completas */
void txt_nowrap(int on);                   /* 1: lo que no cabe en la fila se recorta (no salta a la fila de abajo); 0: normal */
void txt_cursor(int on);                   /* muestra/oculta el cursor de bloque (XOR) */
void txt_cursor_blink(void);               /* alterna el cursor; llamar desde el temporizador */
void txt_cursor_color(u8 c);               /* color del bloque (por defecto blanco) */
#endif
