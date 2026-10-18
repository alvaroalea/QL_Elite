/* MVT1, MVS4, MVS5, MVT6 y MVEIT (con MV40, MV45, TIDY...) contra el codigo original en el emulador (tools/gen_move.py):
   INWK a la salida, las llamadas a SCAN (con INWK+0..8, INWK+31 y TYPE) y a TACTICS. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "move.h"
#include "space.h"
#include "state.h"
#include "emath.h"
#include "ship_math.h"
#include "ships.h"
#include "ship3d.h"

static char line[2000], got[1000];
static void hx(char *d, const u8 *b, int n) { int i; for (i = 0; i < n; i++) sprintf(d + 2 * i, "%02x", b[i]); }
static void scan(u8 type)
{
    char *p = got + strlen(got);
    if (*got) *p++ = ' ';
    *p++ = 's'; hx(p, inwk, 9); p += 18;
    sprintf(p, "%02x%02x", inwk[31], type);
}
static void tact(void) { strcat(got, got[0] ? " t" : "t"); }
static void rdh(const char *s, u8 *b, int n) { int i; char t[3]; t[2] = 0; for (i = 0; i < n; i++) { t[0] = s[2 * i]; t[1] = s[2 * i + 1]; b[i] = (u8)strtol(t, 0, 16); } }

int main(int argc, char **argv)
{
    FILE *f = fopen(argc > 1 ? argv[1] : "tests/vectors_move.txt", "r");
    long cases[7] = { 0 }, fails[7] = { 0 };
    const char *kinds = "TVWXMYZ";
    int k;
    if (!f) { printf("sin vectores\n"); return 2; }
    plat_init();
    space_scan_hook = scan; move_tactics_hook = tact;
    while (fgets(line, sizeof line, f)) {
        char *c1 = strchr(line, ':'), *c2, ib[100], ob[100], buf[400], want[400];
        u8 iw[NI];
        int v[16], n;
        if (!c1) continue;
        *c1 = 0; c2 = strchr(c1 + 1, ':');
        k = (int)(strchr(kinds, line[0]) - kinds);
        got[0] = 0; buf[0] = 0; want[0] = 0;
        if (k == 0) {
            sscanf(line + 2, "%99s %d %d %d", ib, &v[0], &v[1], &v[2]);
            rdh(ib, inwk, NI); zR = (u8)v[2]; mvt1((u8)v[0], (u8)v[1]);
            hx(buf, inwk, NI); sscanf(c1 + 1, "%399s", want);
        } else if (k == 1) {
            sscanf(line + 2, "%99s %d %d %d", ib, &v[0], &v[1], &v[2]);
            rdh(ib, inwk, NI); alpha = (u8)v[0]; beta = (u8)v[1]; mvs4((u8)v[2]);
            hx(buf, inwk, NI); sscanf(c1 + 1, "%399s", want);
        } else if (k == 2) {
            sscanf(line + 2, "%99s %d %d %d", ib, &v[0], &v[1], &v[2]);
            rdh(ib, inwk, NI); move_rat2 = (u8)v[2]; mvs5((u8)v[0], (u8)v[1]);
            hx(buf, inwk, NI); sprintf(buf + 2 * NI, " %02x%02x", zK[0], zK[1]);
            { char *e = c1 + 1; while (*e == ' ') e++; strcpy(want, e); want[strcspn(want, "\n")] = 0; }
        } else if (k == 3) {
            u8 r;
            sscanf(line + 2, "%99s %d %d %d %d", ib, &v[0], &v[1], &v[2], &v[3]);
            rdh(ib, inwk, NI); zP1 = (u8)v[2]; move_p2 = (u8)v[3];
            r = mvt6((u8)v[0], (u8)v[1]);
            sprintf(buf, "%d %d %d", r, zP1, move_p2);
            { char *e = c1 + 1; while (*e == ' ') e++; strcpy(want, e); want[strcspn(want, "\n")] = 0; }
        } else if (k == 6) {
            sscanf(line + 2, "%d %d %99s", &v[0], &v[1], ib);
            rdh(ib, inwk, NI); alpha = (u8)v[0]; beta = (u8)v[1];
            mv40_pre();
            hx(buf, inwk, NI); sscanf(c1 + 1, "%399s", want);
        } else if (k == 5) {
            sscanf(line + 2, "%d %d %d %d %99s", &v[0], &v[1], &v[2], &v[3], ib);
            rdh(ib, inwk, NI); ztype = (u8)v[0]; zDELTA = (u8)v[1]; alpha = (u8)v[2]; beta = (u8)v[3];
            mv45();
            hx(buf, inwk, NI);
            { char *a = c1 + 1, *b2; b2 = strchr(a, ':'); *b2 = 0; while (*a == ' ') a++; n = (int)strlen(a); while (n && a[n - 1] == ' ') a[--n] = 0;
              if (strcmp(a, got)) { cases[k]++; fails[k]++; if (fails[k] < 6) printf("FALLO Y log\n  port: %s\n  orig: %s\n", got, a); continue; }
              b2++; while (*b2 == ' ') b2++; strcpy(want, b2); want[strcspn(want, "\n")] = 0; }
        } else {
            sscanf(line + 2, "%d %d %d %d %d %d %d %d %d %d %d %d %d %99s", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6], &v[7], &v[8], &v[9], &v[10], &v[11], &v[12], ib);
            rdh(ib, inwk, NI);
            ztype = (u8)v[0]; move_xsav = (u8)v[1]; mcnt = (u8)v[2]; zDELTA = (u8)v[3]; alpha = (u8)v[4]; beta = (u8)v[5];
            alp1 = (u8)v[6]; alp2[0] = (u8)v[7]; alp2[1] = (u8)v[8]; bet1 = (u8)v[9]; bet2[0] = (u8)v[10]; bet2[1] = (u8)v[11];
            mveit();
            hx(buf, inwk, NI);
            { char *a = c1 + 1, *b2; b2 = strchr(a, ':'); *b2 = 0; while (*a == ' ') a++; n = (int)strlen(a); while (n && a[n - 1] == ' ') a[--n] = 0;
              if (strcmp(a, got)) { cases[k]++; fails[k]++; if (fails[k] < 6) printf("FALLO M log\n  port: %s\n  orig: %s\n", got, a); continue; }
              b2++; while (*b2 == ' ') b2++; strcpy(want, b2); want[strcspn(want, "\n")] = 0; }
        }
        (void)iw; (void)c2;
        cases[k]++;
        if (strcmp(buf, want)) { if (fails[k] < 6) printf("FALLO %c %.60s...\n  port: %s\n  orig: %s\n", line[0], line + 2, buf, want); fails[k]++; }
    }
    for (k = 0; k < 7; k++) printf("  %c  %ld casos, %ld fallos\n", kinds[k], cases[k], fails[k]);
    for (k = 0; k < 7; k++) if (fails[k]) return 1;
    return 0;
}
