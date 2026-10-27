/* 8x8 text on the QL screen (mode 8): 32 columns x 32 rows. Aligned to 8-pixel cells. */
#ifndef TEXT_H
#define TEXT_H
#include "platform.h"

#define TXT_COLS 32
#define TXT_ROWS 32
#define TXT_TRANSPARENT (-1)       /* transparent paper: respects whatever is drawn underneath */

void txt_init(void);                       /* internal tables; call after plat_init() */
void txt_color(u8 ink, int paper);         /* ink 0..7; paper 0..7 or TXT_TRANSPARENT */
void txt_at(int col, int row);             /* positions the cursor (clipped to the screen) */
int  txt_col(void);
int  txt_row(void);
void txt_putc(int ch);                     /* 
 = CR+LF, \r = column 0, \b = back, the rest ASCII 32..127 */
void txt_puts(const char *s);
void txt_print_at(int col, int row, const char *s);
void txt_clear_rows(int row0, int nrows, u8 color);   /* clears whole text rows */
void txt_nowrap(int on);                   /* 1: what does not fit in the row is clipped (does not wrap to the next row); 0: normal */
#endif
