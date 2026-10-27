/* Elite QL - platform layer (C89, suitable for XTC68/qcc, where int may be 16 bits).
   Screen = native QL mode 8 format (32 KB, 256x256, 128 bytes/line). */
#ifndef PLATFORM_H
#define PLATFORM_H

typedef unsigned char u8;

#define SCR_W 256
#define SCR_H 256
#define SCR_STRIDE 128
#define SCR_BYTES 32768L          /* does not fit in a 16-bit int: always long */

/* Two screen modes (same 32 KB, 128 bytes per line, same byte layout per 8 x 8 text cell = 4 bytes):
   mode 8 (256 x 256, 8 colours; the views inside the station) and mode 4 (512 x 256, black, red, green and white; title and flight).
   Game coordinates are always the 256-column ones: in mode 4 each game dot is 2 physical dots wide, except the
   lines and the ellipses of the planet and rings, which are drawn at the 512 dots (physical x = 2 x). Mode 4: word = 8 dots, even byte =
   green (bit 7 = leftmost dot), odd byte = red; white = both. */
extern u8 gfx_m4;                 /* 1 = mode 4 (change only with plat_mode) */
extern void (*gfx_mode_hook)(void);   /* called when the mode changes (the text recalculates its masks) */
extern u8 gfx_hires_opt;          /* user option (flight options menu, M key): 1 = title and flight in mode 4 (512 x 256); 0 (default) = everything in mode 8 */
void plat_mode(int m4);           /* changes mode (QL: MT.DMODE and console again) and clears the screen to black; PC: only the flag */

/* QL colours: bit2=Green, bit1=Red, bit0=Blue */
#define C_BLACK   0
#define C_BLUE    1
#define C_RED     2
#define C_MAGENTA 3
#define C_GREEN   4
#define C_CYAN    5
#define C_YELLOW  6
#define C_WHITE   7

extern u8 *screen;                /* screen buffer in QL format */

void plat_init(void);
unsigned long plat_ticks(void);    /* 50 Hz ticks since start-up (QL: frame interrupt task; PC: clock) */
void plat_present(void);          /* once per frame; on the QL it also empties the console keyboard queue */
void plat_shutdown(void);         /* idempotent; on the QL it is registered with atexit */
void plat_flush_input(void);
void plat_exit(void);             /* QL: ends the program without the c68 runtime's "Press a key to exit" message (MT.FRJOB); PC: does nothing, main ends with return */
int  plat_default_dir(int prog, char *buf, int max); /* QL: Toolkit II default directory (prog = 0: DATA_USE, 1: PROG_USE) into buf; 1 if there is one, 0 if not. PC: 0 */
void plat_wait_key(void);         /* QL: waits for a key press (release, press, release) so the screen can be seen before returning to QDOS; PC: does nothing */      /* QL: discards the keys accumulated in the console (KEYROW does not consume them) */

/* gfx.c */
u8   gfx_ccode(u8 c);                      /* C_* colour -> internal screen code: the same in mode 8; in mode 4 there are only black, C_RED, C_GREEN and white (everything else is white),
                                               with bit 3 = second physical dot of green and bit 0 = that of red (3 = red, 12 = green, 15 = white) */
#define GFX_GM(cc) ((u8)(((cc) & 4 ? 0xAA : 0) | ((cc) & 8 ? 0x55 : 0)))   /* even (green) byte of 4 filled dots of the internal code */
#define GFX_RM(cc) ((u8)(((cc) & 2 ? 0xAA : 0) | ((cc) & 1 ? 0x55 : 0)))   /* odd byte (red and blue) */
void gfx_set_mode(int m4);                 /* only the flag and the notice to the text (plat_mode calls this) */
void gfx_xor_pixel_hr(int x, int y, u8 c);   /* mode 4: one physical dot (x from 0 to 511) in XOR; in mode 8 the game dot x / 2 */
void gfx_clear(u8 c);
void gfx_fill_lines(int y, int n, u8 c);   /* sets n whole lines to a colour from y: fast (long words on the QL) */
void gfx_copy(u8 *dst, const u8 *src, long n);   /* fast copy of n bytes (multiple of 32 to go by long words) */
void gfx_pixel(int x, int y, u8 c);
void gfx_xor_pixel(int x, int y, u8 c);
void gfx_yoff_set(int off);                         /* vertical offset of what is drawn in XOR (flight panel: +24) */
u8   gfx_get(int x, int y);
void gfx_line(int x0, int y0, int x1, int y1, u8 c, int xor_mode);
void gfx_hline(int x0, int x1, int y, u8 c);
void gfx_hline_xor(int x0, int x1, int y, u8 c);
void gfx_vline_xor(int x, int y0, int y1, u8 c);   /* column of dots (x, y0..y1) in XOR */
void gfx_hloin(int x1, int x2, int y, u8 c);        /* HLOIN of the original: XOR, endpoints to even, covers X1..X2-1 */
void gfx_vloin(int x, int y1, int y2, u8 c);         /* VLOIN of the original: vertical 2 dots wide (x, x+1) in XOR, from y1 to y2 inclusive */
void gfx_cpix(int x, int y, u8 c);                   /* CPIX of the original: a colour dot 2 wide (x, x+1) in XOR */
void gfx_dust_6502(int x, int y, u8 zz);            /* PIXEL of the original (the compass; verified against the 6502 in make test_lines) */
void gfx_dust(int x, int y, u8 zz);                  /* game stardust: zz >= 128 = 1 dot, 80..127 = 1 x 2, < 80 = 2 x 3 (width x height) */
void gfx_rect(int x, int y, int w, int h, u8 c);
void gfx_clear_cells(int col, int row, int ncols, int nrows);   /* empties (black) a rectangle of 8 x 8 text cells (assembly on the QL) */
void gfx_ell_update(int ocx, int ocy, int ory, const u8 *otab, int ncx, int ncy, int nry, const u8 *ntab, u8 c, int cly1);   /* replaces one ellipse with another row by row (gfx_ell_spans tables; ory/nry = -1: none) */
void gfx_ell_spans(int rx, int ry, u8 *tab);                  /* table of outline spans per row (2 bytes per row: lo, hi); 2 * (ry + 1) bytes */
void gfx_ellipse_rows(int rx, int ry, u8 *half);          /* half-width at each vertical distance dy = 0..ry from the ellipse centre (the rightmost point of each row of its outline) */
void gfx_frame_xor(int cx, int cy, int hw, int hh, u8 c, int clx0, int cly0, int clx1, int cly1);   /* rectangle outline in XOR (each point once), clipped */
void gfx_ellipse_xor(int cx, int cy, int rx, int ry, u8 c, int clx0, int cly0, int clx1, int cly1);   /* ellipse outline in XOR (midpoint, integers only), clipped to the rectangle; each point just once */
#endif
