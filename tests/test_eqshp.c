/* equip_buy / equip_price (Equipment) contra el codigo original en el emulador (tools/gen_eqshp.py): EQSHP con eq, prx, refund,
   PRXS, LCASH y MCASH reales. Resultado (comprado / sin dinero / ya lo tienes), dinero y equipo a la salida. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "game.h"
#include "state.h"

static char line[1000];
static void setst(const char *s)
{
    int v[10], i;
    unsigned lz[4];
    unsigned long c;
    char hz[16];
    sscanf(s, "%d %d %d %d %d %d %d %d %d %d %8s %lx", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6], &v[7], &v[8], &v[9], hz, &c);
    qq14 = (u8)v[0]; nomsl = (u8)v[1]; crgo = (u8)v[2]; ecm_fitted = (u8)v[3]; bst = (u8)v[4]; escp = (u8)v[5]; bomb = (u8)v[6];
    engy = (u8)v[7]; dkcmp = (u8)v[8]; ghyp = (u8)v[9];
    for (i = 0; i < 4; i++) { char t[3]; t[0] = hz[2 * i]; t[1] = hz[2 * i + 1]; t[2] = 0; lz[i] = (unsigned)strtol(t, 0, 16); laser[i] = (u8)lz[i]; }
    cash_set(c);
}
static void getst(char *d)
{
    sprintf(d, "%d %d %d %d %d %d %d %d %d %d %02x%02x%02x%02x %08lx", qq14, nomsl, crgo, ecm_fitted, bst, escp, bomb, engy, dkcmp, ghyp,
            laser[0], laser[1], laser[2], laser[3], cash_get());
}

int main(int argc, char **argv)
{
    FILE *f = fopen(argc > 1 ? argv[1] : "tests/vectors_eqshp.txt", "r");
    long cases[2] = { 0 }, fails[2] = { 0 };
    if (!f) { printf("sin vectores\n"); return 2; }
    while (fgets(line, sizeof line, f)) {
        char *c1 = strchr(line, ':'), *c2;
        if (!c1) continue;
        if (line[0] == 'Q') {
            int tek, k, view, dummy, r, want_r;
            char pre[200], post[200], end[16], got[200], *p;
            *c1 = 0;
            c2 = strchr(c1 + 1, ':'); *c2 = 0;
            sscanf(line + 2, "%d %d %d %d", &tek, &k, &view, &dummy);
            p = strchr(line + 2, ' '); p = strchr(p + 1, ' '); p = strchr(p + 1, ' '); p = strchr(p + 1, ' ');
            strcpy(pre, p + 1);
            sscanf(c1 + 1, "%15s", end);
            { char *q = strchr(c1 + 1, ' '); (void)q; }
            p = strstr(c1 + 1, "197");
            want_r = !strcmp(end, "ok") ? EQ_OK : !strcmp(end, "bay") ? -1 : (p && p < c2) ? EQ_NOCASH : EQ_PRESENT;
            strcpy(post, c2 + 1); post[strcspn(post, "\n")] = 0;
            { char *a = post; while (*a == ' ') a++; memmove(post, a, strlen(a) + 1); }
            if (want_r < 0) continue;
            cases[0]++;
            setst(pre);
            r = equip_buy(k - 1, view);
            getst(got);
            if (r != want_r || strcmp(got, post)) {
                if (fails[0] < 6) printf("FALLO Q tek=%d item=%d vista=%d\n  pre:      %s\n  port:  %d %s\n  orig:  %d %s\n", tek, k - 1, view, pre, r, got, want_r, post);
                fails[0]++;
            }
        } else if (line[0] == 'P') {
            int q, i, bad = 0;
            char *p = c1 + 1;
            *c1 = 0;
            sscanf(line + 2, "%d", &q);
            qq14 = (u8)q;
            for (i = 0; i < 14; i++) { long v = strtol(p, &p, 10); if ((long)equip_price(i) != v) bad = 1; }
            cases[1]++;
            if (bad) { if (fails[1] < 5) printf("FALLO P QQ14=%d\n", q); fails[1]++; }
        }
    }
    printf("  Q  %ld casos, %ld fallos\n  P  %ld casos, %ld fallos\n", cases[0], fails[0], cases[1], fails[1]);
    return (fails[0] || fails[1]) ? 1 : 0;
}
