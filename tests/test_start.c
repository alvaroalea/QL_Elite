/* RESET y RES2 (space_reset_all, space_reset) contra el codigo original en el emulador (tools/gen_start.py): estado de
   las variables despues y llamadas a SPBLB, ECBLB (de ECMOF) y WPSHPS (WPSHPS lo hace quien llama: clear_view). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "space.h"
#include "ships.h"
#include "stars.h"
#include "sun.h"
#include "tactics.h"
#include "combat.h"
#include "damage.h"
#include "encounters.h"
#include "keys.h"
#include "state.h"
#include "cmdr.h"
#include "emath.h"

extern unsigned short orig_slsp;
static char line[4000], names[2000];
static char *nm[200];
static int nn;

static u8 *ref(const char *n)
{
    int k;
    static u8 dummy;
    if (n[0] == 'Z') {
        k = atoi(n + 1);
        if (k <= NOSH) return &frin[k];
        k -= NOSH + 1;
        if (k <= NTY) return &many[k];
        switch (k - NTY - 1) {
        case 0: return &junk_count; case 1: return &dock_auto; case 2: return &ecmp; case 3: return &mj; case 4: return &cabtmp;
        case 5: return &las2; case 6: return &msar; case 7: return &space_view; case 8: return &lasct; case 9: return &gntmp;
        case 11: return &enc_ev; case 12: return &dly; case 13: return &mess_de;
        }
        return 0;                                            /* HFX: no existe en el port */
    }
    if (n[0] == 'I') return &inwk[atoi(n + 1)];
    if (!strcmp(n, "SLSPL") || !strcmp(n, "SLSPH")) return 0;
    if (!strcmp(n, "BETA")) return &beta;
    if (!strcmp(n, "BET1")) return &bet1;
    if (!strcmp(n, "QQ22")) return &qq22[0];
    if (!strcmp(n, "QQ22H")) return &qq22[1];
    if (!strcmp(n, "ECMA")) return &ecma;
    if (!strcmp(n, "ALP1")) return &alp1;
    if (!strcmp(n, "ALP2")) return &alp2[0];
    if (!strcmp(n, "ALP2H")) return &alp2[1];
    if (!strcmp(n, "BET2")) return &bet2[0];
    if (!strcmp(n, "BET2H")) return &bet2[1];
    if (!strcmp(n, "MSTG")) return &mstg;
    if (!strcmp(n, "DELTA")) return &zDELTA;
    if (!strcmp(n, "MCNT")) return &mcnt;
    if (!strcmp(n, "ALPHA")) return &alpha;
    if (!strcmp(n, "QQ12")) return &qq12;
    if (!strcmp(n, "JSTX")) return &jstx;
    if (!strcmp(n, "JSTY")) return &jsty;
    if (!strcmp(n, "FSH")) return &fsh;
    if (!strcmp(n, "ASH")) return &ash;
    if (!strcmp(n, "ENERGY")) return &energy;
    if (!strcmp(n, "NOSTM")) return &stars_nostm;
    if (!strcmp(n, "LSX2")) return &circ_lsx2[0];
    if (!strcmp(n, "LSY2")) return &circ_lsy2[0];
    if (!strcmp(n, "JSTGY")) return &jstgy;
    (void)dummy;
    return 0;                                                /* dontclip, Yx2M1 (constantes) y KEY2 */
}

int main(int argc, char **argv)
{
    FILE *f = fopen(argc > 1 ? argv[1] : "tests/vectors_start.txt", "r"), *g = fopen("tests/vectors_start.addr", "r");
    long cases[3] = { 0 }, fails[3] = { 0 };
    char *p;
    if (!f || !g) { printf("sin vectores\n"); return 2; }
    if (!fgets(names, sizeof names, g)) return 2;
    for (p = strtok(names, " \n"); p; p = strtok(0, " \n")) nm[nn++] = p;
    plat_init();
    while (fgets(line, sizeof line, f)) {
        char *c1 = strchr(line, ':'), *c2, *q;
        int k = line[0] == 'S', i, bad = 0, sp, em, v;
        u8 before[200];
        char msg[300] = "";
        if (!c1) continue;
        *c1 = 0; c2 = strchr(c1 + 1, ':'); if (c2) *c2 = 0; else c2 = c1;
        if (line[0] == 'F') {                                 /* DFAULT: cmdr_unpack y cmdr_pack */
            u8 b[CMDR_SIZE], o[CMDR_SIZE];
            q = line + 2;
            for (i = 0; i < CMDR_SIZE; i++) b[i] = (u8)strtol(q, &q, 10);
            cases[2]++;
            if (cmdr_unpack(b) != 0) { fails[2]++; printf("FALLO F: rechaza un bloque valido\n"); continue; }
            cmdr_pack(o);
            q = c2 + 1;
            for (i = 0; i < 74; i++) {
                v = (int)strtol(q, &q, 10);
                if ((i >= 47 && i <= 50) || i == 20 || i == 21) continue;
                if (o[i] != (u8)v) { bad = 1; if (strlen(msg) < 100) sprintf(msg + strlen(msg), " b%d port=%d orig=%d", i, o[i], v); }
            }
            if (bad) { fails[2]++; if (fails[2] < 5) printf("FALLO F:%s\n", msg); }
            continue;
        }
        q = line + 2;
        for (i = 0; i < nn; i++) { before[i] = (u8)strtol(q, &q, 10);
    if (ref(nm[i])) *ref(nm[i]) = before[i]; }
        sp = many[2] != 0; em = ecma != 0;                    /* SSPR = many[SHIP_SST]; el original mira SSPR antes de RES2 */
        if (k) space_reset(); else space_reset_all();
        /* llamadas: SPBLB si SSPR != 0 (solo RES2), ECMOF si ECMA != 0 (solo RES2), y WPSHPS (de quien llama) */
        if (strstr(c1 + 1, "SPBLB") != 0 && !(k && sp)) bad = 1, strcat(msg, " SPBLB");
        if (strstr(c1 + 1, "SPBLB") == 0 && (k && sp)) bad = 1, strcat(msg, " noSPBLB");
        if (strstr(c1 + 1, "ECBLB") != 0 && !(k && em)) bad = 1, strcat(msg, " ECBLB");
        if (strstr(c1 + 1, "ECBLB") == 0 && (k && em)) bad = 1, strcat(msg, " noECBLB");
        q = c2 + 1;
        for (i = 0; i < nn; i++) {
            v = (int)strtol(q, &q, 10);
            if (!strcmp(nm[i], "dontclip")) { if (v != 0) bad = 1; continue; }
            if (!strcmp(nm[i], "Yx2M1")) { if (v != SUN_YX2M1) bad = 1; continue; }
            if (!strcmp(nm[i], "SLSPL")) { if ((orig_slsp & 255) != v) bad = 1; continue; }
            if (!strcmp(nm[i], "SLSPH")) { if ((orig_slsp >> 8) != v) bad = 1; continue; }
            if (!ref(nm[i])) continue;
            if (*ref(nm[i]) != (u8)v) { bad = 1;
    if (strlen(msg) < 200) sprintf(msg + strlen(msg), " %s port=%d orig=%d", nm[i], *ref(nm[i]), v); }
        }
        cases[k]++;
        if (bad) { if (fails[k] < 5) printf("FALLO %c:%s\n", k ? 'S' : 'R', msg); fails[k]++; }
    }
    printf("  R  %ld casos, %ld fallos\n  S  %ld casos, %ld fallos\n  F  %ld casos, %ld fallos\n", cases[0], fails[0], cases[1], fails[1], cases[2], fails[2]);
    return (fails[0] || fails[1] || fails[2]) ? 1 : 0;
}
