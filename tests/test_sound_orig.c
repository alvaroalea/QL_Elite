/* Sound against the original code in the emulator (tools/gen_sound.py): the DORNDs each routine consumes (SOEXPL, SOHISS and
   SOBOMB, only with sound on), RAND and the carry on exit. The sound itself (QL BEEP) is our own. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sound.h"
#include "emath.h"

static int cnt;
static void hook(void) { cnt++; em_dornd(); }

int main(int argc, char **argv)
{
    FILE *f = fopen(argc > 1 ? argv[1] : "tests/vectors_sound.txt", "r");
    char line[300], ent[20];
    long cases = 0, fails = 0;
    if (!f) { printf("sin vectores\n"); return 2; }
    snd_init();
    snd_dornd_hook = hook;
    while (fgets(line, sizeof line, f)) {
        unsigned r[4], w[4];
        int dn, c, x, y, wc, wcnt, i, bad = 0;
        char rs[20], ws[20];
        if (sscanf(line, "%19s %8s %d %d %d %d : %d %8s %d", ent, rs, &dn, &c, &x, &y, &wcnt, ws, &wc) != 9) continue;
        for (i = 0; i < 4; i++) { char b[3]; b[0] = rs[2 * i]; b[1] = rs[2 * i + 1]; b[2] = 0; r[i] = (unsigned)strtol(b, 0, 16); b[0] = ws[2 * i]; b[1] = ws[2 * i + 1]; w[i] = (unsigned)strtol(b, 0, 16); em_rand[i] = (u8)r[i]; }
        em_rand_c = (u8)c;
        snd_enable(!(dn & 0x80));
        cnt = 0;
        if (!strcmp(ent, "EXNO")) snd_exno();
        else if (!strcmp(ent, "EXNO3")) snd_exno3();
        else if (!strcmp(ent, "SOEXPL")) snd_soexpl(y);
        else if (!strcmp(ent, "SOHISS")) snd_sohiss(y);
        else if (!strcmp(ent, "SOBOMB")) snd_sobomb();
        else if (!strcmp(ent, "BEEP")) snd_beep();
        else if (!strcmp(ent, "BOOP")) snd_boop();
        else if (!strcmp(ent, "SOBEEP")) snd_sobeep(x, y);
        else if (!strcmp(ent, "SOBLIP")) snd_soblip(x, y);
        else if (!strcmp(ent, "LASNOISE")) snd_lasnoise();
        if (cnt != wcnt) bad = 1;
        for (i = 0; i < 4; i++) if (em_rand[i] != w[i]) bad = 1;
        if (cnt == 0 ? em_rand_c != c : em_rand_c != wc) bad = 1;
        cases++;
        if (bad) { if (fails < 8) printf("FALLO %s", line), printf("  port: %d DORND, rand %02x%02x%02x%02x c=%d\n", cnt, em_rand[0], em_rand[1], em_rand[2], em_rand[3], em_rand_c); fails++; }
    }
    printf("  %ld casos, %ld fallos\n", cases, fails);
    return fails ? 1 : 0;
}
