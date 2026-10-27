/* TT66/TTX66/TTX66K/BOX and CLYNS (missions.c) against the original code in the emulator (tools/gen_screen.py): the same
   sequence (MT2, graphic or text clearing, BOX, FLFLLS, ee3, tokens with XC/YC, spaces, clearrow) and the variables. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "missions.h"
#include "detok.h"
#include "lines.h"
#include "sun.h"
#include "combat.h"
#include "damage.h"
#include "state.h"

static char line[20000], got[20000];
static int glen;
static void put(const char *s) { size_t n = strlen(s); if (glen) got[glen++] = ' '; memcpy(got + glen, s, n); glen += (int)n; got[glen] = 0; }
static void tr(char k, u8 a) { char s[16]; if (k == 'e') sprintf(s, "e %d", a); else sprintf(s, "%c", k); put(s); }
static void clr(int g) { put(g ? "g" : "x"); if (g) { put("b"); scr_box(); } }
static void hl(u8 x1, u8 x2, u8 y, u8 c) { char s[32]; sprintf(s, "h %d %d %d %d", x1, x2, y, c == C_BLUE ? 16 : 99); put(s); }
static void t27(u8 a) { char s[32]; sprintf(s, "t %d %d %d", a, dt_xc, dt_yc); put(s); }
static void chp(u8 a) { char s[32]; sprintf(s, "c %d %d %d", a, dt_xc, dt_yc); put(s); }
static void row(u8 r) { char s[16]; sprintf(s, "r %d", r); put(s); }
static char *trim(char *s) { char *e; while (*s == ' ') s++; e = s + strlen(s); while (e > s && (e[-1] == ' ' || e[-1] == '\n')) *--e = 0; return s; }

int main(int argc, char **argv)
{
    FILE *f = fopen(argc > 1 ? argv[1] : "tests/vectors_screen.txt", "r");
    long cases[3] = { 0 }, fails[3] = { 0 };
    const char *kinds = "TCB";
    int k;
    if (!f) { printf("sin vectores\n"); return 2; }
    plat_init();
    scr_trace = tr; scr_clear_hook = clr; lines_hloin_hook = hl; scr_tt27 = t27; scr_chpr = chp; scr_clearrow_hook = row;
    while (fgets(line, sizeof line, f)) {
        char *c1 = strchr(line, ':'), *c2, *want, *wst, st[200], buf[300];
        int v[4];
        if (!c1) continue;
        *c1 = 0; c2 = strchr(c1 + 1, ':'); *c2 = 0;
        want = trim(c1 + 1); wst = trim(c2 + 1);
        k = (int)(strchr(kinds, line[0]) - kinds);
        glen = 0; got[0] = 0;
        circ_lsp = 0x55; qq17 = 0x55; dtw2 = 0x55; dt_xc = 7; dt_yc = 7; las2 = 9; dly = 9; mess_de = 9;
        if (k == 0) {
            sscanf(line + 2, "%d %d %d", &v[0], &v[1], &v[2]);
            scr_viewdir = (u8)v[1]; qq22[1] = (u8)v[2];
            tt66((u8)v[0]);
            sprintf(st, "%d %d %d %d %d %d %d %d %d %d", qq11, circ_lsp, qq17, dtw2, dt_xc, dt_yc, las2, dly, mess_de, scr_text & 0x80);
            { int a[10], i; char *p = wst, w2[200]; for (i = 0; i < 10; i++) a[i] = (int)strtol(p, &p, 10);
              sprintf(w2, "%d %d %d %d %d %d %d %d %d %d", a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8], a[9] & 0x80); strcpy(buf, w2); }
        } else if (k == 1) {
            sscanf(line + 2, "%d", &v[0]);
            scr_text = (u8)v[0];
            clyns();
            sprintf(st, "%d %d %d %d %d %d", dt_xc, dt_yc, dtw2, qq17, dly, mess_de);
            strcpy(buf, wst);
        } else {
            gfx_clear(C_BLACK); put("b"); scr_box();
            sprintf(st, "%s", wst);               /* the corners: %10101010 in bytes 1 and 37 (dot 254 lit) */
            strcpy(buf, wst);
            if (!gfx_get(254, 0) || !gfx_get(0, 0)) strcpy(st, "esquina");
        }
        cases[k]++;
        if (strcmp(got, want) || strcmp(st, buf)) {
            if (fails[k] < 5) printf("FALLO %s\n  port:     %s : %s\n  original: %s : %s\n", line, got, st, want, buf);
            fails[k]++;
        }
    }
    for (k = 0; k < 3; k++) printf("  %c  %ld casos, %ld fallos\n", kinds[k], cases[k], fails[k]);
    for (k = 0; k < 3; k++) if (fails[k]) return 1;
    return 0;
}
