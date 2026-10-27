/* Dashboard against the original code in the emulator (tools/gen_dash.py): full DIALS (DIS1, DIS2, DIS5, COMPAS, SP1, SP2, DOT),
   MSBAR, msblob, SCAN, cntr, BUMP2 and REDU2. The primitives (HLOIN, VLOIN, PIXEL, CPIX) are compared by their arguments. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "dash.h"
#include "space.h"
#include "state.h"
#include "emath.h"
#include "tactics.h"
#include "combat.h"
#include "ship_math.h"
#include "ship3d.h"

static char line[8000], got[4000];
static void tr(char k, u8 a, u8 b, u8 c, u8 d)
{
    char s[40];
    if (k == 'h') sprintf(s, "h %d %d %d %d", a, b, c, d);
    else if (k == 'v') sprintf(s, "v %d %d %d %d", a, b, c, d);
    else if (k == 'p') sprintf(s, "p %d %d %d", a, b, c);
    else sprintf(s, "c %d %d %d", a, b, d);
    if (got[0]) strcat(got, " | ");
    strcat(got, s);
}
static void hexs(char *d, const u8 *b, int n) { int i; for (i = 0; i < n; i++) sprintf(d + 2 * i, "%02x", b[i]); }
static void rdh(const char *s, u8 *b, int n) { int i; char t[3]; t[2] = 0; for (i = 0; i < n; i++) { t[0] = s[2 * i]; t[1] = s[2 * i + 1]; b[i] = (u8)strtol(t, 0, 16); } }
static char *trim(char *s) { char *e; while (*s == ' ') s++; e = s + strlen(s); while (e > s && (e[-1] == ' ' || e[-1] == '\n')) *--e = 0; return s; }
/* removes from an original trace the calls with colour 0 (the port does not draw what cannot be seen) */
static void drop0(char *t)
{
    char out[4000], *p = t, *q;
    out[0] = 0;
    while (*p) {
        char seg[80];
        int n;
        q = strstr(p, " | ");
        n = q ? (int)(q - p) : (int)strlen(p);
        memcpy(seg, p, (size_t)n); seg[n] = 0;
        { int len = (int)strlen(seg); if (!(seg[0] != 'p' && len >= 2 && seg[len - 2] == ' ' && seg[len - 1] == '0')) { if (out[0]) strcat(out, " | "); strcat(out, seg); } }
        p += n; if (q) p += 3;
    }
    strcpy(t, out);
}

int main(int argc, char **argv)
{
    FILE *f = fopen(argc > 1 ? argv[1] : "tests/vectors_dash.txt", "r");
    long cases[9] = { 0 }, fails[9] = { 0 };
    int pflag;
    const char *kinds = "DMBSCURE";
    int k;
    if (!f) { printf("sin vectores\n"); return 2; }
    plat_init();
    dash_trace = tr;
    while (fgets(line, sizeof line, f)) {
        char *c1 = strchr(line, ':'), *c2, *want, *wst, st[200], ws2[200];
        int v[24], kk;
        if (!c1) continue;
        *c1 = 0; c2 = strchr(c1 + 1, ':');
        if (line[0] == 'P') line[0] = 'E', pflag = 1; else pflag = 0;
        k = (int)(strchr(kinds, line[0]) - kinds);
        got[0] = 0; st[0] = 0;
        if (k == 7) {                                   /* bulbs: E (ECBLB) and S (SPBLB) in XOR on a black screen */
            char exp[2000] = "", have[2000] = "";
            int x, y;
            gfx_clear(C_BLACK);
            if (pflag) dash_spblb(); else dash_ecblb();
            for (x = 0; x < 256; x++) for (y = 0; y < 192; y++) if (gfx_get(x, y) != C_BLACK) sprintf(have + strlen(have), "%s%02x%02x", have[0] ? " " : "", x, y);
            { char *e = strchr(c1 + 1, ':'); strcpy(exp, trim(e ? e + 1 : c1 + 1)); }
            cases[7]++;
            if (strcmp(have, exp)) { if (fails[7] < 3) printf("FALLO bombilla %d\n  port: %s\n  orig: %s\n", pflag, have, exp); fails[7]++; }
            continue;
        }
        if (c2) { *c2 = 0; want = trim(c1 + 1); wst = trim(c2 + 1); } else { want = trim(c1 + 1); wst = want; }
        kk = k;
        if (k == 0) {
            char dh[64], ch[64], ksb[400];
            u8 b[13];
            sscanf(line + 2, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %63s %63s %d %d %d %399s", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6], &v[7], &v[8], &v[9], &v[10], &v[11], &v[12], &v[13], &v[14], dh, ch, &v[15], &v[16], &v[17], ksb);
            zDELTA = (u8)v[0]; alp1 = (u8)v[1]; bet1 = (u8)v[2]; energy = (u8)v[3]; fsh = (u8)v[4]; ash = (u8)v[5]; qq14 = (u8)v[6]; altit = (u8)v[7];
            cabtmp = (u8)v[8]; gntmp = (u8)v[9]; mcnt = (u8)v[10]; alp2[1] = (u8)v[11]; bet2[0] = (u8)v[12]; flh = (u8)v[13]; sspr = (u8)v[14];
            rdh(dh, dash_dials, 13); rdh(ch, dash_dialc, 13);
            dash_comx = (u8)v[15]; dash_comy = (u8)v[16]; dash_comc = (u8)v[17];
            { u8 kb[2 * NI]; int i; rdh(ksb, kb, 2 * NI); for (i = 0; i < NI; i++) { state_slot(0)[i] = kb[i]; state_slot(1)[i] = kb[NI + i]; } }
            (void)b;
            dash_update();
            hexs(st, dash_dials, 13); strcat(st, " "); hexs(st + strlen(st), dash_dialc, 13);
            sprintf(st + strlen(st), " %d %d %d", dash_comx, dash_comy, dash_comc);
            strcpy(ws2, wst);
        } else if (k == 1) {
            char mh[20];
            int x, y;
            sscanf(line + 2, "%d %d %19s", &x, &y, mh);
            rdh(mh, dash_mscol, 4);
            dash_msbar((u8)x, (u8)y);
            hexs(st, dash_mscol, 4); strcpy(ws2, wst);
        } else if (k == 2) {
            char mh[20];
            int nm;
            sscanf(line + 2, "%d %19s", &nm, mh);
            rdh(mh, dash_mscol, 4); nomsl = (u8)nm;
            dash_msblob();
            hexs(st, dash_mscol, 4); strcpy(ws2, wst);
        } else if (k == 3) {
            char ib[100];
            int t;
            sscanf(line + 2, "%d %99s", &t, ib);
            rdh(ib, inwk, NI); ztype = (u8)t;
            space_scan((u8)t);
            drop0(want);
            st[0] = 0; ws2[0] = 0;
        } else {
            int x, a, d1, d2, r;
            if (k == 4) {
                sscanf(line + 2, "%d %d %d %d", &x, &v[0], &v[1], &v[2]);
                damp = (u8)v[0]; djd = (u8)v[1]; dock_auto = (u8)v[2];
                r = space_cntr((u8)x);
            } else if (k == 5) {
                sscanf(line + 2, "%d %d %d", &x, &a, &d1); djd = (u8)d1; r = space_bump2((u8)x, (u8)a);
            } else {
                sscanf(line + 2, "%d %d %d", &x, &a, &d1); djd = (u8)d1; r = space_redu2((u8)x, (u8)a);
            }
            (void)d2;
            sprintf(st, "%d", r); strcpy(ws2, want); want = ""; got[0] = 0;
        }
        cases[kk]++;
        if (strcmp(got, want) || strcmp(st, ws2)) {
            if (fails[kk] < 4) printf("FALLO %c %.70s...\n  port: %s : %s\n  orig: %s : %s\n", line[0], line + 2, got, st, want, ws2);
            fails[kk]++;
        }
    }
    for (k = 0; k < 8; k++) printf("  %c  %ld casos, %ld fallos\n", kinds[k], cases[k], fails[k]);
    for (k = 0; k < 8; k++) if (fails[k]) return 1;
    return 0;
}
