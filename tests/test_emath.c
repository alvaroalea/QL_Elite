#include <stdio.h>
#include <string.h>
#include "emath.h"
u8 *screen;
void plat_init(void){} void plat_present(void){} void plat_shutdown(void){}
static int fails, total;
#define CHECK(cond, ...) do { total++; if (!(cond)) { if (fails++ < 10) { printf("FALLO: "); printf(__VA_ARGS__); printf("\n"); } } } while (0)
int main(int argc, char **argv)
{
    FILE *f = fopen(argc > 1 ? argv[1] : "tests/vectors.txt", "r");
    char op[16];
    int a, b, c, d, e, g, h;
    int n[4];
    if (!f) { printf("sin vectores\n"); return 2; }
    memset(em_rand, 0, 4); em_rand[0]=0x11; em_rand[1]=0x22; em_rand[2]=0x33; em_rand[3]=0x44;
    while (fscanf(f, "%15s", op) == 1) {
        if (!strcmp(op, "add")) { fscanf(f, "%d %d %d %d %d %d", &a,&b,&c,&d,&e,&g);
            zP=b; zS=c; zR=d; { u8 r = em_add(a); CHECK(r==e && em_x==g, "add %d %d %d %d -> %d,%d esperado %d,%d", a,b,c,d,r,em_x,e,g); } }
        else if (!strcmp(op, "mad")) { fscanf(f, "%d %d %d %d %d %d", &a,&b,&c,&d,&e,&g);
            zQ=b; zS=c; zR=d; { u8 r = em_mad(a); CHECK(r==e && em_x==g, "mad %d %d %d %d -> %d,%d esperado %d,%d", a,b,c,d,r,em_x,e,g); } }
        else if (!strcmp(op, "mult1")) { fscanf(f, "%d %d %d %d", &a,&b,&c,&d);
            zQ=b; { u8 r = em_mult1(a); CHECK(r==c && zP==d, "mult1 %d %d", a,b); } }
        else if (!strcmp(op, "mults")) { fscanf(f, "%d %d %d %d", &a,&b,&c,&d);
            zP=a; { u8 r = em_mults(b); CHECK(r==c && zP==d, "mults %d %d -> %d,%d esperado %d,%d", a,b,r,zP,c,d); } }
        else if (!strcmp(op, "tis2")) { fscanf(f, "%d %d %d", &a,&b,&c);
            zQ=b; { u8 r = em_tis2(a); CHECK(r==c, "tis2 %d %d -> %d esperado %d", a,b,r,c); } }
        else if (!strcmp(op, "dvid96")) { fscanf(f, "%d %d", &a,&b);
            { u8 r = em_dvid96(a); CHECK(r==b, "dvid96 %d -> %d esperado %d", a,r,b); } }
        else if (!strcmp(op, "fmltu")) { fscanf(f, "%d %d %d %d", &a,&b,&c,&d);
            zQ=b; em_carry = 0; { u8 r = em_fmltu(a); CHECK(r==c && em_carry==d, "fmltu %d %d -> %d,%d esperado %d,%d", a,b,r,em_carry,c,d); } }
        else if (!strcmp(op, "mltu2")) { fscanf(f, "%d %d %d %d %d %d", &a,&b,&c,&d,&e,&g);
            zP=b; zQ=c; { u8 r = em_mltu2(a); CHECK(r==d && zP1==e && zP==g, "mltu2 %d %d %d", a,b,c); } }
        else if (!strcmp(op, "rand")) { fscanf(f, "%d %d %d %d %d", &a,&b,&c,&d,&e);
            em_rand_c = a; { u8 r = em_dornd(); CHECK(r==b && em_rand_x==c && em_rand_c==d && em_rand_v==e, "rand %d -> %d,%d,%d,%d esperado %d,%d,%d,%d", a,r,em_rand_x,em_rand_c,em_rand_v,b,c,d,e); } }
        else { printf("op desconocida %s\n", op); return 2; }
    }
    (void)g; (void)h; (void)n;
    printf("%d comprobaciones, %d fallos\n", total, fails);
    return fails != 0;
}
