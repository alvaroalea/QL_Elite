#include "emath.h"
#include "platform.h"
typedef unsigned short u16s;
static u16s s;
static void mix(u8 v) { s = (u16s)(((s << 3) | (s >> 13)) ^ v); }
static void mixg(void)
{
    mix(zP); mix(zQ); mix(zR); mix(zS); mix(zT); mix(zU);
    mix(zK[0]); mix(zK[1]); mix(zK[2]); mix(zK[3]); mix(em_x); mix(em_carry);
}
static void put(int k) { screen[2 * k] = (u8)s; screen[2 * k + 1] = (u8)(s >> 8); s = 0; }
int main(void)
{
    int a, q, k = 0;
    u8 r;
    plat_init();
    em_rand[0] = 1; em_rand[1] = 2; em_rand[2] = 3; em_rand[3] = 4;
#define LOOP2(BODY) for (a = 0; a < 256; a += 5) for (q = 0; q < 256; q += 7) { zP = (u8)(a ^ 0x5A); zQ = (u8)q; zR = (u8)(a * 3); zS = (u8)(q * 5); zT = 0; zU = 0; zK[0] = (u8)q; zK[1] = (u8)a; zK[2] = 0; zK[3] = 0; em_carry = (u8)(a & 1); BODY; mixg(); } put(k++);
    LOOP2(r = em_multu(); mix(r))
    LOOP2(r = em_mu11((u8)(q | 1)); mix(r))
    LOOP2(r = em_squa2((u8)a); mix(r))
    LOOP2(r = em_squa((u8)a); mix(r))
    LOOP2(r = em_mult1((u8)a); mix(r))
    LOOP2(em_mult12((u8)a))
    LOOP2(r = em_mad((u8)a); mix(r))
    LOOP2(r = em_add((u8)a); mix(r))
    LOOP2(r = em_mults((u8)a); mix(r))
    LOOP2(r = em_fmltu((u8)a); mix(r))
    LOOP2(r = em_mltu2((u8)a); mix(r))
    LOOP2(r = em_tis2((u8)a); mix(r))
    LOOP2(r = em_dvid96((u8)a); mix(r))
    LOOP2(r = em_tis1((u8)q, (u8)a); mix(r))
    LOOP2(zR = (u8)(a); zQ = (u8)q; r = em_ll5(); mix(r))
    LOOP2(r = em_ll28((u8)a); mix(r))
    LOOP2(zU = 0; r = em_ll61((u8)a); mix(r))
    LOOP2(r = em_ll38((u8)a); mix(r))
    LOOP2(r = em_dvid4((u8)a); mix(r))
    LOOP2(zDELTA = (u8)q; r = em_dv41((u8)a); mix(r))
    LOOP2(zDELTA = (u8)q; r = em_dv42((u8)a); mix(r))
    LOOP2(r = em_dvidt((u8)a); mix(r))
    LOOP2(em_mult3((u8)a))
    LOOP2(r = em_fmltu2((u8)a); mix(r))
    LOOP2(r = em_mlu2((u8)a); mix(r))
    LOOP2(em_dvid3b((u8)a, (u8)q, (u8)(a + 3), (u8)(q & 0x80)))
    screen[1000] = (u8)k;
    plat_present();
    return 0;
}
