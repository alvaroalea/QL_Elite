/* QL backend (XTC68 / qcc). UNTESTED on hardware: validate on a real QL or emulator.

   Console: the c68 runtime opens a window (centred, with a red border and cursor). The game does not use it
   (it draws directly at $20000 and reads the keyboard with KEYROW), but the IPC still puts every key press into the
   keyboard queue of that window, which has the focus. If nobody reads it, on exit SuperBASIC receives
   all the keys. Hence: (1) the window is redefined as full screen, black and without cursor,
   (2) the queue is emptied at start-up, on every plat_present() and on exit (atexit). */
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <qdos.h>

u8 *screen = (u8 *)0x20000L;               /* standard QL screen 1 */

/* ql_ipc.asm: generic trap #3; returns D0 (0 = ok, negative = QDOS error) */
extern long ql_trap3(long d0, long d1, long d2, long d3, long a0, long a1);
extern long ql_trap1(long d0, long d1, long d2, long d3);

/* trap #3 operations used */
#define IO_FBYTE 0x01L                      /* D3 = timeout; D1.B = byte; ERR.NC (-1) if there is nothing */
#define SD_WDEF  0x0DL                      /* D1.B = border colour, D2.W = border width, A1 -> 4 words w,h,x,y */
#define SD_CURE  0x0EL                      /* enables the cursor */
#define SD_CURS  0x0FL                      /* suppresses the cursor */
#define SD_CLEAR 0x20L                      /* clears the whole window */
#define FLUSH_MAX 512                       /* cap on bytes per flush, in case the channel returned endless data */

static long con_in = -1L;                   /* QDOS stdin channel (read) */
static long con_out = -1L;                  /* QDOS stdout channel (window) */
extern void ql_tick_on(void);
extern void ql_tick_off(void);
extern long ql_ticks(void);
static int tick_up = 0;
static long last_tick = 0;
static int  con_up = 0;                     /* plat_init done and plat_shutdown pending */
static short win_def[4];                    /* width, height, x, y */
static short orig_mode = -1;                /* screen mode the program started with (0 = mode 4, 8 = mode 8; -1 = could not be read): restored on exit */

/* Empties the console keyboard queue without waiting (timeout 0). */
void plat_flush_input(void)
{
    int n;
    if (con_in < 0L) return;
    for (n = 0; n < FLUSH_MAX; n++)
        if (ql_trap3(IO_FBYTE, 0L, 0L, 0L, con_in, 0L) != 0L) break;
}

/* Console window full screen, black and without cursor (after start-up and after each mode change) */
static void con_setup(void)
{
    if (con_out >= 0L) {
        win_def[0] = 512; win_def[1] = 256; win_def[2] = 0; win_def[3] = 0;   /* full screen */
        ql_trap3(SD_WDEF, 0L, 0L, -1L, con_out, (long)win_def);              /* black border, width 0 */
        ql_trap3(SD_CLEAR, 0L, 0L, -1L, con_out, 0L);
        ql_trap3(SD_CURS, 0L, 0L, -1L, con_out, 0L);                         /* no cursor */
    }
}

void plat_init(void)
{
    short mode = -1, tv = -1;              /* -1 = read the current mode */
    mt_dmode(&mode, &tv);
    if (orig_mode < 0 && (mode == 0 || mode == 8)) orig_mode = mode;
    mode = 8; tv = -1;                     /* the demos start in mode 8, without changing TV (the game asks for mode 4 with plat_mode) */
    mt_dmode(&mode, &tv);
    /* NOTE: writing directly to $20000 may require supervisor mode (trap #0) or SD.EXTOP */

    con_in  = (long)fgetchid(stdin);
    con_out = (long)fgetchid(stdout);

    con_setup();
    plat_flush_input();
    if (!con_up) { con_up = 1; atexit(plat_shutdown); }
#ifndef NOTICK
    if (!tick_up) { ql_tick_on(); tick_up = 1; last_tick = ql_ticks(); }
#endif
}

/* Changes the screen mode (MT.DMODE: D1 = 0 is the 512 x 256 mode 4 and 8 is mode 8; without changing TV), leaves the console full
   screen again and clears the screen to black. A change to the same mode does nothing */
void plat_mode(int m4)
{
    short mode = m4 ? 0 : 8, tv = -1;
    if ((m4 ? 1 : 0) == (int)gfx_m4) return;
    mt_dmode(&mode, &tv);
    con_setup();
    gfx_set_mode(m4);
    gfx_clear(C_BLACK);
}

/* Key wait for the demos: without it the c68 runtime clears the screen when returning to QDOS.
   Reads the matrix with the same IPC command as keys_ql.c (KEYROW), but without depending on keys.c. */
extern long ql_ipc(char *blk);
static long wk_blk[2];                      /* long: even address */

static int wk_row(int row)
{
    char *b = (char *)wk_blk;
    long r;
    b[0] = 9; b[1] = 1; b[2] = 0; b[3] = 0; b[4] = 0; b[5] = 0; b[6] = (char)row; b[7] = 2;
    r = ql_ipc(b);
    return (r < 0L) ? 0 : (int)(r & 0xFF);
}

static int wk_any(void)                      /* any key (except SHIFT/CTRL/ALT) */
{
    int r, v;
    for (r = 0; r < 8; r++) {
        v = wk_row(r);
        if (r == 7) v &= 0xF8;
        if (v) return 1;
    }
    return 0;
}

/* Replaces _envsetup of the c68 4.24f runtime, which Cinit calls before main. The original copies the Toolkit II default
   directories (pointers to QDOS strings at SV+$AC, $B0 and $B4) to PROG_USE, DATA_USE and SPL_USE without checking anything:
   without Toolkit II the pointers are 0 and it reads the string at ROM address 0, whose first word is the length.
   On the JS ROM it is 3 (3 bytes of garbage), on Minerva $4C02: it copies 19458 bytes into a 50-byte stack buffer and the
   program hangs before reaching main. This one does the same as the original (environment list of the parent or of the
   SuperBASIC *ENV* extension), but only accepts an even pointer inside RAM and a length from 1 to 49. */
static void tk2_default(char *name, long off)
{
    char *q = *(char **)(_sys_var + off);
    char *s, *e;
    int n, i;
    e = getenv(name);
    if (e && *e) return;                                   /* the environment rules, as in the original */
    if (q <= _sys_var || q >= *(char **)(_sys_var + 0x20) || ((long)q & 1L)) return;   /* outside RAM (SV_RAMT) */
    n = *(short *)q;
    if (n < 1 || n > 49) return;
    s = (char *)malloc(strlen(name) + n + 2);
    if (!s) return;
    strcpy(s, name);
    e = s + strlen(s);
    *e++ = '=';
    for (i = 0; i < n; i++) *e++ = q[2 + i];
    *e = 0;
    putenv(s);
}

void _envsetup(char *env)
{
    char *p = 0, *bv, *nt, *ntb, *nl;
    if (env && !((long)env & 1L) && *(short *)env == 0x4AFC)
        p = *(char **)(env + 2);                           /* environment passed by the parent job */
    else {
        _super();                                          /* so that SuperBASIC does not move its tables while they are walked */
        bv  = *(char **)(_sys_var + 0x10) + 0x68;          /* SV_BASIC: BASIC variables */
        ntb = bv + *(long *)(bv + 0x18);                   /* BV_NTBAS */
        nt  = bv + *(long *)(bv + 0x1C) - 8;               /* BV_NTP: last entry */
        nl  = bv + *(long *)(bv + 0x20);                   /* BV_NLBAS */
        for (; nt >= ntb; nt -= 8)
            if (nt[0] == 8 && nt[1] == 0 && !strncmp("*ENV*", nl + *(short *)(nt + 2) + 1, 5)) {
                p = *(char **)(*(char **)(nt + 4) + 2);  /* *ENV* function of the ENV extension */
                break;
            }
        _user();
    }
    putenv("");
    if (p)
        for (; *p; p += strlen(p) + 1) putenv(p);
    tk2_default("PROG_USE", 0xACL);
    tk2_default("DATA_USE", 0xB0L);
    tk2_default("SPL_USE", 0xB4L);
}

/* Toolkit II default directories (DATA_USE / PROG_USE), which _envsetup leaves in the environment at start-up;
   without Toolkit II they are empty. (c68's getcdd reads SPL_USE, not DATA_USE: hence getenv.) */
int plat_default_dir(int prog, char *buf, int max)
{
    char *e = getenv(prog ? "PROG_USE" : "DATA_USE");
    int n = 0;
    buf[0] = 0;
    if (!e) return 0;
    while (e[n] && n < max - 1) { buf[n] = e[n]; n++; }
    buf[n] = 0;
    return buf[0] != 0;
}

void plat_wait_key(void)
{
    while (wk_any()) plat_flush_input();    /* first wait for the key used to launch the program to be released */
    while (!wk_any()) plat_flush_input();
    while (wk_any()) plat_flush_input();    /* and released: so SuperBASIC does not receive it */
    plat_flush_input();
}

/* Called once per frame from the loop: takes the opportunity to empty the keyboard queue. */

unsigned long plat_ticks(void) { return tick_up ? (unsigned long)ql_ticks() : 0UL; }

/* 50 frames per second limit: if the frame lasted less than one tick (20 ms), wait for the next. A frame that
   already lasted one tick or more (a normal QL: 5-10 fps) waits for nothing; it only slows down a fast QL (Super Gold Card or similar) */
void plat_present(void)
{
    plat_flush_input();
    if (tick_up) {
        while (ql_ticks() == last_tick) { }
        last_tick = ql_ticks();
    }
}

void plat_shutdown(void)
{
    if (tick_up) { ql_tick_off(); tick_up = 0; }      /* before finishing: the clock task is in our code */
    if (!con_up) return;                    /* idempotent: atexit also calls it */
    con_up = 0;
    plat_flush_input();
    if (orig_mode >= 0 && orig_mode != (gfx_m4 ? 0 : 8)) {      /* the mode the user started with */
        short mode = orig_mode, tv = -1;
        mt_dmode(&mode, &tv);
        con_setup();
    }
    if (con_out >= 0L) {
        ql_trap3(SD_CLEAR, 0L, 0L, -1L, con_out, 0L);
        ql_trap3(SD_CURE, 0L, 0L, -1L, con_out, 0L);                         /* gives the cursor back */
    }
}

/* Exit without the c68 runtime's "Press a key to exit": the console is cleaned and the job removes itself
   with MT.FRJOB (trap #1, D0=5, D1=-1 = this job, D3 = error code 0). It does not return. If for some reason it fails,
   plat_exit returns and the program ends through the normal path. */
void plat_exit(void)
{
    plat_shutdown();
    ql_trap1(5L, -1L, 0L, 0L);
}
