#include <stdio.h>
#include <string.h>
#include "emath.h"
u8 *screen;
void plat_init(void){} void plat_present(void){} void plat_shutdown(void){}
static int fails, total;
#define CHECK(cond, ...) do { total++; if (!(cond)) { if (fails++ < 12) { printf("FALLO: "); printf(__VA_ARGS__); printf("\n"); } } } while (0)
int main(int argc, char **argv)
{
    FILE *f = fopen(argc > 1 ? argv[1] : "tests/vectors2.txt", "r");
    char op[16];
    int a[16];
    unsigned long k;
    u8 r;
    if (!f) { printf("sin vectores\n"); return 2; }
    while (fscanf(f, "%15s", op) == 1) {
        if (!strcmp(op, "ll5")) { fscanf(f, "%d %d %d", &a[0],&a[1],&a[2]);
            zR = a[0]; zQ = a[1]; r = em_ll5(); CHECK(r == a[2] && zQ == a[2], "ll5 %d %d -> %d esperado %d", a[0],a[1],r,a[2]); }
        else if (!strcmp(op, "ll28")) { fscanf(f, "%d %d %d %d", &a[0],&a[1],&a[2],&a[3]);
            zQ = a[1]; r = em_ll28(a[0]); CHECK(r == a[2] && zR == a[2] && em_carry == a[3], "ll28 %d %d -> %d,%d esperado %d,%d", a[0],a[1],r,em_carry,a[2],a[3]); }
        else if (!strcmp(op, "ll28n")) { fscanf(f, "%d %d %d %d", &a[0],&a[1],&a[2],&a[3]);
            zQ = a[1]; r = em_ll28_nocheck(a[0]); CHECK(r == a[2] && em_carry == a[3], "ll28n %d %d -> %d,%d esperado %d,%d", a[0],a[1],r,em_carry,a[2],a[3]); }
        else if (!strcmp(op, "ll61")) { fscanf(f, "%d %d %d %d", &a[0],&a[1],&a[2],&a[3]);
            zQ = a[1]; zU = 0; em_ll61(a[0]); CHECK(zR == a[2] && zU == a[3], "ll61 %d %d -> %d,%d esperado %d,%d", a[0],a[1],zR,zU,a[2],a[3]); }
        else if (!strcmp(op, "ll38")) { fscanf(f, "%d %d %d %d %d %d %d", &a[0],&a[1],&a[2],&a[3],&a[4],&a[5],&a[6]);
            zS = a[1]; zQ = a[2]; zR = a[3]; r = em_ll38(a[0]); CHECK(r == a[4] && zS == a[5] && em_carry == a[6], "ll38 %d %d %d %d -> %d,%d,%d esperado %d,%d,%d", a[0],a[1],a[2],a[3],r,zS,em_carry,a[4],a[5],a[6]); }
        else if (!strcmp(op, "dvid4")) { fscanf(f, "%d %d %d %d", &a[0],&a[1],&a[2],&a[3]);
            zQ = a[1]; r = em_dvid4(a[0]); CHECK(r == a[2] && zP == a[2] && zR == a[3], "dvid4 %d %d -> %d,%d esperado %d,%d", a[0],a[1],r,zR,a[2],a[3]); }
        else if (!strcmp(op, "dvidt")) { fscanf(f, "%d %d %d %d %d", &a[0],&a[1],&a[2],&a[3],&a[4]);
            zP = a[1]; zQ = a[2]; r = em_dvidt(a[0]); CHECK(zP1 == a[3] && r == a[4], "dvidt %d %d %d -> %d,%d esperado %d,%d", a[0],a[1],a[2],zP1,r,a[3],a[4]); }
        else if (!strcmp(op, "mult3")) { fscanf(f, "%d %d %d %d %lu", &a[0],&a[1],&a[2],&a[3],&k);
            zP1 = a[1]; zP = a[2]; zQ = a[3]; em_mult3(a[0]);
            CHECK((((unsigned long)zK[3] << 24) | ((unsigned long)zK[2] << 16) | ((unsigned long)zK[1] << 8) | zK[0]) == k, "mult3 %d %d %d %d", a[0],a[1],a[2],a[3]); }
        else if (!strcmp(op, "dvid3b")) { fscanf(f, "%d %d %d %d %d %d %d %d %d %d", &a[0],&a[1],&a[2],&a[3],&a[4],&a[5],&a[6],&a[7],&a[8],&a[9]);
            zP = a[0]; zP1 = a[1]; em_dvid3b(a[2], a[3], a[4], a[5]);
            CHECK(zK[0] == a[6] && zK[1] == a[7] && zK[2] == a[8] && zK[3] == a[9], "dvid3b %d %d %d %d %d %d -> %d,%d,%d,%d esperado %d,%d,%d,%d", a[0],a[1],a[2],a[3],a[4],a[5],zK[0],zK[1],zK[2],zK[3],a[6],a[7],a[8],a[9]); }
        else if (!strcmp(op, "fmltu2")) { fscanf(f, "%d %d %d %d", &a[0],&a[1],&a[2],&a[3]);
            zK[0] = a[1]; em_carry = 0; r = em_fmltu2(a[0]); CHECK(r == a[2] && em_carry == a[3], "fmltu2 %d %d -> %d,%d esperado %d,%d", a[0],a[1],r,em_carry,a[2],a[3]); }
        else if (!strcmp(op, "mlu2")) { fscanf(f, "%d %d %d %d", &a[0],&a[1],&a[2],&a[3]);
            zQ = a[1]; r = em_mlu2(a[0]); CHECK(r == a[2] && zP == a[3], "mlu2 %d %d", a[0],a[1]); }
        else { printf("op desconocida %s\n", op); return 2; }
    }
    printf("%d comprobaciones, %d fallos\n", total, fails);
    return fails != 0;
}
