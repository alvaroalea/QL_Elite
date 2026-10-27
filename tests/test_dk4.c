/* FREEZE (DK4 with '=') and DKS3 (config.c) against the original code in the emulator (tools/gen_keys.py): the 9 configuration
   options (DAMP..DISK), DNOIZ, how the pause ends ('-' or ESC = DEATH2) and how many keys were read. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "config.h"
#include "input.h"
#include "sound.h"
#include "keys.h"
#include "space.h"

static char line[400];
static int seq[40], ns, si, lower;
static int feed(void)
{
    int k;
    if (si >= ns) return -1;
    k = seq[si++];
    if (k == 0) return -1;
    if (lower && k >= 'A' && k <= 'Z') k += 32;
    return k;
}
static int rdx(const char *s, int i) { char t[3]; t[0] = s[2 * i]; t[1] = s[2 * i + 1]; t[2] = 0; return (int)strtol(t, 0, 16); }

int main(int argc, char **argv)
{
    FILE *f = fopen(argc > 1 ? argv[1] : "tests/vectors_dk4.txt", "r");
    long cases = 0, fails = 0;
    if (!f) { printf("sin vectores\n"); return 2; }
    plat_init();
    snd_init();
    while (fgets(line, sizeof line, f)) {
        char cfg[40], sq[100], oc[40], end[8], *c1 = strchr(line, ':');
        int dn, odn, keys, i, bad = 0, r;
        if (!c1) continue;
        *c1 = 0;
        if (sscanf(line + 2, "%39s %x %99s", cfg, &dn, sq) != 3) continue;
        if (sscanf(c1 + 1, "%39s %x %7s %d", oc, &odn, end, &keys) != 4) continue;
        for (i = 0; i < 9; i++) *config_opt((u8)i) = (u8)rdx(cfg, i);
        snd_enable(dn == 0);
        ns = (int)strlen(sq) / 2;
        for (i = 0; i < ns; i++) seq[i] = rdx(sq, i);
        si = 0; lower = cases & 1;
        in_script = feed; in_quit = 0;
        r = freeze();
        in_script = 0;
        for (i = 0; i < 9; i++) if (*config_opt((u8)i) != (u8)rdx(oc, i)) bad = 1;
        if (snd_enabled() != (odn == 0)) bad = 1;
        if (r != !strcmp(end, "esc")) bad = 1;
        if (si != keys) bad = 1;
        cases++;
        if (bad) { if (fails < 5) printf("FALLO %s\n", line); fails++; }
    }
    printf("  %ld casos, %ld fallos\n", cases, fails);
    return fails ? 1 : 0;
}
