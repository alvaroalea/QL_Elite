/* LOIN of the original (Apple II), transcribed: the slope comes from the logarithm tables (log, logL, alogh), the slope error
   starts at S = 128 and one point is drawn for each x (or y) of the game, in XOR. The more horizontal lines (|dy| < |dx|)
   go left to right and the others bottom to top; depending on where it goes and whether the endpoints had to be swapped (SWAP),
   the original does not draw one of them:
     - more horizontal and rising (Y2 < Y1 after sorting): (X2, Y2) of the call is missing;
     - more horizontal and falling or flat: (X1, Y1) of the call is missing;
     - more vertical: (X1, Y1) of the call is missing.
   The carries entering S + Q (or S + P) are always 0 in the original (checked in tools/gen_lines.py). */
#include "lines.h"
#include "emath.h"

void (*loin_hook)(u8 x1, u8 y1, u8 x2, u8 y2) = 0;

/* The transcription of the original LOIN (verified point by point against the 6502: make test_lines). The game no longer uses it. */
void loin_draw_6502(u8 x1, u8 y1, u8 x2, u8 y2, u8 c)
{
    u8 p, q, s = 128, swap = 0, t, n, x, y;
    unsigned w;
    if (loin_hook) loin_hook(x1, y1, x2, y2);
    p = (u8)(x2 >= x1 ? x2 - x1 : x1 - x2);
    q = (u8)(y2 >= y1 ? y2 - y1 : y1 - y2);
    if (q < p) {                                        /* STPX: one point for each x */
        if (x1 >= x2) { swap = 0xFF; t = x1; x1 = x2; x2 = t; t = y1; y1 = y2; y2 = t; }
        q = q ? em_logdiv(q, p) : 0;                    /* Q = |dy| / |dx| * 256 */
        n = (u8)(p + 1);
        x = x1; y = y1;
        if (y2 >= y1) {                                 /* DOWN: y grows */
            if (!swap) goto li9;
            n--;
            for (;;) {
                gfx_xor_pixel(x, y, c);
li9:            x++;
                w = (unsigned)s + q; s = (u8)w;
                if (w >> 8) y++;
                if (--n == 0) return;
            }
        } else {                                        /* y decreases */
            if (swap) goto li6;
            n--;
            for (;;) {
                gfx_xor_pixel(x, y, c);
li6:            x++;
                w = (unsigned)s + q; s = (u8)w;
                if (w >> 8) y--;
                if (--n == 0) return;
            }
        }
    } else {                                            /* STPY: one point for each y, bottom to top */
        if (y1 < y2) { swap = 0xFF; t = x1; x1 = x2; x2 = t; t = y1; y1 = y2; y2 = t; }
        if (p) p = em_logdiv(p, q);                     /* P = |dx| / |dy| * 256 (LIfudge if |dx| = 0) */
        n = (u8)(q + 1);
        x = x1; y = y1;
        if (x2 >= x1) {                                 /* to the right */
            if (!swap) goto li17;
            n--;
            for (;;) {
                gfx_xor_pixel(x, y, c);
li17:           y--;
                w = (unsigned)s + p; s = (u8)w;
                if (w >> 8) x++;
                if (--n == 0) return;
            }
        } else {                                        /* LFT: to the left */
            if (!swap) goto li18;
            n--;
            for (;;) {
                gfx_xor_pixel(x, y, c);
li18:           y--;
                w = (unsigned)s + p; s = (u8)w;
                if (w >> 8) x--;
                if (--n == 0) return;
            }
        }
    }
}

/* Game line: pure Bresenham in XOR, with both endpoints. The endpoints are always sorted the same way (smaller x first; with equal x,
   smaller y first), so drawing (A, B) or (B, A) lights the same points and erasing = drawing again. One point for each x (the
   line is more horizontal than vertical, |dy| <= |dx|) or for each y; the error starts at half. On the QL (ASM_LINE, ql_line.asm) an
   assembly routine that writes directly to the screen does it; on the PC (and to check it) this C version. */
void loin_bres_c(u8 x1, u8 y1, u8 x2, u8 y2, u8 c)
{
    int x = x1, y = y1, dx, dy, sy, e, n, hr = gfx_m4;
    if (x1 > x2 || (x1 == x2 && y1 > y2)) { x = x2; y = y2; x2 = x1; y2 = y1; }
    dx = (int)x2 - x;
    dy = (int)y2 - y;
    if (hr) { x *= 2; dx *= 2; }                        /* mode 4: the 512 dots (physical x = 2 x game x) */
    sy = 1;
    if (dy < 0) { dy = -dy; sy = -1; }
    if (dx >= dy) {                                     /* one point for each x */
        e = dx >> 1;
        for (n = dx; n >= 0; n--) {
            if (hr) gfx_xor_pixel_hr(x, y, c); else gfx_xor_pixel(x, y, c);
            e -= dy;
            if (e < 0) { y += sy; e += dx; }
            x++;
        }
    } else {                                            /* one point for each y */
        e = dy >> 1;
        for (n = dy; n >= 0; n--) {
            if (hr) gfx_xor_pixel_hr(x, y, c); else gfx_xor_pixel(x, y, c);
            e -= dx;
            if (e < 0) { x++; e += dy; }
            y += sy;
        }
    }
}

#ifdef ASM_LINE
extern void ql_line(long x1, long y1, long x2, long y2, long c, long scr);   /* ql_line.asm */
#endif

void loin_draw(u8 x1, u8 y1, u8 x2, u8 y2, u8 c)
{
    if (loin_hook) loin_hook(x1, y1, x2, y2);
#ifdef ASM_LINE
    ql_line((long)x1, (long)y1, (long)x2, (long)y2, (long)c, (long)screen);
#else
    loin_bres_c(x1, y1, x2, y2, c);
#endif
}

void (*lines_hloin_hook)(u8 x1, u8 x2, u8 y, u8 c) = 0;

void lines_hloin(u8 x1, u8 x2, u8 y, u8 c)
{
    if (lines_hloin_hook) lines_hloin_hook(x1, x2, y, c);
    gfx_hloin(x1, x2, y, c);
}

void nlin2(u8 y) { lines_hloin(0, 255, y, C_BLUE); }        /* STA Y1; COL = BLUE; X1 = 0; X2 = 255; JMP HLOIN */

/* NLIN4: in the Apple II text mode (the text views) NLI4 puts dashes on row 2 under the characters of row 1;
   the QL has no text mode (the text views are drawn on the graphics screen), so it always goes through NLIN2 with y = 19,
   which falls in the middle of row 2 */
void nlin4(void) { nlin2(19); }
