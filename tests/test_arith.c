/* Maths (Arithmetic) contra el codigo original en el emulador (tools/gen_arith.py): cada rutina con A, X, Y, C y la pagina cero al
   azar; se comparan sus salidas (las que declara el contrato de emath.h) y, si el port toca em_carry, el acarreo. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "emath.h"

u8 *screen;
void plat_init(void) {}
void plat_present(void) {}
void plat_shutdown(void) {}

static char line[1000];

int main(int argc, char **argv)
{
    FILE *f = fopen(argc > 1 ? argv[1] : "tests/vectors_arith.txt", "r");
    long cases = 0, fails = 0;
    char op[16];
    if (!f) { printf("sin vectores\n"); return 2; }
    while (fgets(line, sizeof line, f)) {
        int in[21], o[17], i, bad = 0, n = 0;
        char *p = line, *c;
        u8 a, x, y, ra = 0, y1 = 0;
        char msg[200];
        if (sscanf(p, "%15s", op) != 1) continue;
        p = strchr(p, ' ');
        for (i = 0; i < 21; i++) in[i] = (int)strtol(p, &p, 10);
        c = strchr(p, ':'); p = c + 1;
        for (i = 0; i < 17; i++) o[i] = (int)strtol(p, &p, 10);
        a = (u8)in[0]; x = (u8)in[1]; y = (u8)in[2];
        zP = (u8)in[4]; zP1 = (u8)in[5]; zQ = (u8)in[6]; zR = (u8)in[7]; zS = (u8)in[8]; zT = (u8)in[9]; zU = (u8)in[10];
        zDELTA = (u8)in[12];
        for (i = 0; i < 4; i++) zK[i] = (u8)in[17 + i];
        em_carry = 0xAA; em_x = 0xAA;
        msg[0] = 0;
#define NEED(cond, what) do { if (!(cond)) { bad = 1; if (n++ < 3) sprintf(msg + strlen(msg), " %s", what); } } while (0)
        if (!strcmp(op, "MULTU")) { ra = em_multu(); NEED(ra == o[0], "A"); NEED(zP == o[4], "P"); }
        else if (!strcmp(op, "MU11")) { ra = em_mu11(x); NEED(ra == o[0], "A"); NEED(zP == o[4], "P"); }
        else if (!strcmp(op, "SQUA")) { ra = em_squa(a); NEED(ra == o[0], "A"); NEED(zP == o[4], "P"); }
        else if (!strcmp(op, "SQUA2")) { ra = em_squa2(a); NEED(ra == o[0], "A"); NEED(zP == o[4], "P"); }
        else if (!strcmp(op, "MULT1")) { ra = em_mult1(a); NEED(ra == o[0], "A"); NEED(zP == o[4], "P"); }
        else if (!strcmp(op, "MULT12")) { em_mult12(a); NEED(zR == o[7], "R"); NEED(zS == o[8], "S"); }
        else if (!strcmp(op, "MAD")) { ra = em_mad(a); NEED(ra == o[0], "A"); NEED(em_x == o[1], "X"); }
        else if (!strcmp(op, "ADD")) { ra = em_add(a); NEED(ra == o[0], "A"); NEED(em_x == o[1], "X"); }
        else if (!strcmp(op, "TIS1")) { ra = em_tis1(x, a); NEED(ra == o[0], "A"); }
        else if (!strcmp(op, "TIS2")) { ra = em_tis2(a); NEED(ra == o[0], "A"); }
        else if (!strcmp(op, "DVID4")) { ra = em_dvid4(a); NEED(zP == o[4], "P"); NEED(zR == o[7], "R"); }
        else if (!strcmp(op, "DV41")) { em_dv41(a); NEED(zP == o[4], "P"); NEED(zR == o[7], "R"); }
        else if (!strcmp(op, "DV42")) { em_dv42(a); NEED(zP == o[4], "P"); NEED(zR == o[7], "R"); }
        else if (!strcmp(op, "MLTU2")) { ra = em_mltu2(a); NEED(ra == o[0], "A"); NEED(zP1 == o[5], "P1"); NEED(zP == o[4], "P"); }
        else if (!strcmp(op, "MULT3")) { em_mult3(a); for (i = 0; i < 4; i++) NEED(zK[i] == o[12 + i], "K"); }
        else if (!strcmp(op, "MLU1")) { ra = em_mlu1((u8)in[13], &y1); NEED(ra == o[0], "A"); NEED(zP == o[4], "P"); NEED(y1 == o[16], "Y1"); }
        else if (!strcmp(op, "MLU2")) { ra = em_mlu2(a); NEED(ra == o[0], "A"); NEED(zP == o[4], "P"); }
        else if (!strcmp(op, "LL38")) { ra = em_ll38(a); NEED(ra == o[0], "A"); NEED(zS == o[8], "S"); }
        else if (!strcmp(op, "DVID3B2")) { em_dvid3b(a, (u8)in[14], (u8)in[15], (u8)in[16]); for (i = 0; i < 4; i++) NEED(zK[i] == o[12 + i], "K"); }
        else continue;
        if (em_carry != 0xAA) NEED(em_carry == o[3], "C");
        cases++;
        if (bad) { if (fails < (getenv("ALL") ? 100000 : 12)) printf("FALLO %s:%s   (entrada %.60s)\n", op, msg, line); fails++; }
    }
    printf("  %ld casos, %ld fallos\n", cases, fails);
    return fails ? 1 : 0;
}
