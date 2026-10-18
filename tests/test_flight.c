/* PLUT (move.c) y, en su momento, ESCAPE, MJP y TT110 contra el codigo original en el emulador (tools/gen_flight.py). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "move.h"
#include "state.h"

static char line[4000];
static void hexs(char *d, const u8 *b, int n) { int i; for (i = 0; i < n; i++) sprintf(d + 2 * i, "%02x", b[i]); }
static void rdh(const char *s, u8 *b, int n) { int i; char t[3]; t[2] = 0; for (i = 0; i < n; i++) { t[0] = s[2 * i]; t[1] = s[2 * i + 1]; b[i] = (u8)strtol(t, 0, 16); } }

int main(int argc, char **argv)
{
    FILE *f = fopen(argc > 1 ? argv[1] : "tests/vectors_flight.txt", "r");
    long cases[8] = { 0 }, fails[8] = { 0 };
    if (!f) { printf("sin vectores\n"); return 2; }
    plat_init();
    while (fgets(line, sizeof line, f)) {
        char *c1 = strchr(line, ':');
        if (!c1) continue;
        *c1 = 0;
        if (line[0] == 'U') {
            int v;
            char ib[100], buf[100], want[100];
            sscanf(line + 2, "%d %99s", &v, ib);
            rdh(ib, inwk, NI);
            plut((u8)v);
            hexs(buf, inwk, NI);
            sscanf(c1 + 1, "%99s", want);
            cases[0]++;
            if (strcmp(buf, want)) { if (fails[0] < 4) printf("FALLO U vista %d\n  port: %s\n  orig: %s\n", v, buf, want); fails[0]++; }
        }
    }
    printf("  U  %ld casos, %ld fallos\n", cases[0], fails[0]);
    return fails[0] ? 1 : 0;
}
