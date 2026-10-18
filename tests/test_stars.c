/* Polvo estelar (STARS, nWq, FLIP con PIXEL2) contra el codigo original en el emulador (tools/gen_stars.py): cada PIXEL
   (X, A, ZZ), las tablas SX..SZL, RAND, ALPHA, ALP2 y BET2 a la salida. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "stars.h"
#include "space.h"
#include "emath.h"
#include "state.h"

static char line[20000], got[20000], exp1[2000], gs[2000];
static int glen;
static void rec(u8 x, u8 y, u8 zz) { char s[32]; sprintf(s, "%s%d %d %d", glen ? " " : "", x, y, zz); strcpy(got + glen, s); glen += (int)strlen(s); }
static void hexb(const char *p, u8 *d, int n) { int i; unsigned v; for (i = 0; i < n; i++) { sscanf(p + 2 * i, "%2x", &v); d[i] = (u8)v; } }
static void hexs(char *s, const u8 *b, int n) { int i; for (i = 0; i < n; i++) sprintf(s + 2 * i, "%02x", b[i]); }
static char *trim(char *s) { char *e; while (*s == ' ') s++; e = s + strlen(s); while (e > s && (e[-1] == ' ' || e[-1] == '\n')) *--e = 0; return s; }

int main(int argc, char **argv)
{
    FILE *f = fopen(argc > 1 ? argv[1] : "tests/vectors_stars.txt", "r");
    long cases = 0, fails = 0;
    u8 *tabs[6];
    static u8 dummy;
    if (!f) { printf("sin vectores\n"); return 2; }
    tabs[0] = sx; tabs[1] = sy; tabs[2] = sz; tabs[3] = sxl; tabs[4] = syl; tabs[5] = szl;
    screen = (u8 *)calloc(32768, 1);
    stars_pixel_hook = rec;
    while (fgets(line, sizeof line, f)) {
        char entry[16], rnd[16], *p, *c1 = strchr(line, ':'), *c2, *want;
        int v[11], i, used;
        if (!c1) continue;
        *c1 = 0; c2 = strchr(c1 + 1, ':'); *c2 = 0;
        strcpy(exp1, trim(c1 + 1)); want = trim(c2 + 1);
        sscanf(line, "%15s %d %d %d %d %d %d %d %d %d %d %d %15s%n", entry, &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6], &v[7], &v[8], &v[9], &v[10], rnd, &used);
        p = line + used;
        for (i = 0; i < 6; i++) { p = trim(p); hexb(p, tabs[i], NOST + 1); p += 2 * (NOST + 1); }
        stars_nostm = (u8)v[1]; alpha = (u8)v[2]; alp1 = (u8)v[3]; alp2[0] = (u8)v[4]; alp2[1] = (u8)(v[4] ^ 0x80);
        beta = (u8)v[5]; bet1 = (u8)v[6]; bet2[0] = (u8)v[7]; bet2[1] = (u8)(v[7] ^ 0x80); delt4[0] = (u8)v[8]; delt4[1] = (u8)v[9];
        hexb(rnd, em_rand, 4);
        em_rand_c = (u8)v[10];
        glen = 0; got[0] = 0;
        if (!strcmp(entry, "STARS")) stars_update((u8)v[0]);
        else if (!strcmp(entry, "nWq")) stars_new();
        else stars_flip();
        gs[0] = 0;
        for (i = 0; i < 6; i++) { hexs(gs + strlen(gs), tabs[i], NOST + 1); strcat(gs, " "); }
        hexs(gs + strlen(gs), em_rand, 4); strcat(gs, " ");
        sprintf(gs + strlen(gs), "%02x%02x%02x%02x%02x%02x", alpha, alp2[0], alp2[1], bet2[0], bet2[1], beta);
        cases++;
        if (strcmp(gs, exp1) || strcmp(got, want)) {
            if (fails < 5) printf("FALLO %.60s\n  estado port %s\n  estado orig %s\n  puntos port %.300s\n  puntos orig %.300s\n", line, gs, exp1, got, want);
            fails++;
        }
    }
    (void)dummy;
    printf("STARS/nWq/FLIP: %ld casos, %ld fallos\n", cases, fails);
    return fails ? 1 : 0;
}
