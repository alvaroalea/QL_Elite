#include "equip.h"
#include "state.h"
#include "emath.h"
#include "ship_math.h"
#include "combat.h"
#include "sound.h"
#include "lines.h"

u8 bombtbx[10], bombtby[10];
u8 bomb_col = C_WHITE;
void (*bomb_line_hook)(u8 x1, u8 y1, u8 x2, u8 y2) = 0;

/* BOMBPOS: x of each vertex of the bolt (from right to left) */
static const u8 bombpos[10] = { 224, 224, 192, 160, 128, 96, 64, 32, 0, 0 };

/* BOMBOFF: draws (XOR) the bomb's zigzag bolt with the 10 vertices of BOMBTBX/BOMBTBY; only in the space view */
void bomb_off(void)
{
    u8 y, x1, y1;
    if (!combat_view_on) return;                 /* LDA QQ11; BNE BOMBR1 */
    x1 = bombtbx[0]; y1 = bombtby[0];            /* XX12 */
    for (y = 1; y < 10; y++) {                   /* CLICK: the speaker click is not ported */
        if (bomb_line_hook) bomb_line_hook(x1, y1, bombtbx[y], bombtby[y]);
        loin_draw(x1, (u8)view_ty(y1), bombtbx[y], (u8)view_ty(bombtby[y]), bomb_col);   /* original y -> screen */
        x1 = bombtbx[y]; y1 = bombtby[y];
    }
}

/* BOMBON: new random vertices (y between 3 and 131, x = BOMBPOS + 0..31; the first at x = 255 and the last at x = 0) and
   draws them (falls through to BOMBOFF). c = carry on entry to the first DORND; the others enter with C = 0 (left by CPY #10) */
void bomb_on(u8 c)
{
    u8 y, a;
    for (y = 0; y < 10; y++) {
        em_rand_c = (u8)(y == 0 ? (c & 1) : 0);
        a = em_dornd();
        bombtby[y] = (u8)((a & 127) + 3 + (em_rand_c & 1));     /* AND #127; ADC #3 with the carry from DORND */
        bombtbx[y] = (u8)((em_rand_x & 31) + bombpos[y]);       /* TXA; AND #31; CLC; ADC BOMBPOS,Y */
    }
    bombtbx[9] = 0;
    bombtbx[0] = 0xFF;
    em_rand_c = 1;                               /* the last CPY #10 leaves C = 1 (BCS BOMBOFF) */
    bomb_off();
}

/* BOMBEFF: sound, erases the bolt and draws another */
void bomb_eff(void)
{
    u8 c;
    snd_sobomb();                                /* with sound: 25 DORND (leaves its carry in em_rand_c) */
    c = combat_view_on ? 1 : em_rand_c;          /* BOMBOFF ends with C = 1 (CPY #10); outside the space view it does not touch it */
    bomb_off();
    bomb_on(c);
    em_rand_c = 1;                               /* BOMBON -> BOMBOFF: C = 1 on return (CPY #10) */
}

/* BOMBEFF2: JSR P%+3 / JSR BOMBEFF / BOMBEFF: four bolts in a row */
void bomb_eff2(void) { bomb_eff(); bomb_eff(); bomb_eff(); bomb_eff(); }

/* Part 3 (MA24..MA76): with the bomb key, ASL BOMB and, if anything is left, BOMBON. Published version (IB disk): if the
   bomb is already exploding (BOMB negative) the key does nothing (the source version relaunched it without erasing) */
void bomb_key(void)
{
    u8 c;
    if (bomb & 0x80) return;
    c = (u8)(bomb >> 7);
    bomb = (u8)(bomb << 1);
    if (bomb == 0) return;
    bomb_on(c);
}

/* Part 16 (MA18..MA77): while BOMB is negative, four bolts per frame; when it turns positive the last one is erased */
void bomb_frame(void)
{
    if (!(bomb & 0x80)) return;
    bomb_eff2();
    bomb = (u8)(bomb << 1);
    if (bomb & 0x80) return;
    bomb_off();
}

/* WARP: jump within the system (J key). Only with no ships other than junk (FRIN+2+JUNK), no station and outside
   hyperspace, and if the planet and the sun ahead (positive z) are far (MAS2 >= 2): brings both 65,536
   units closer in z (ADD of $81 $81 to the high byte). Returns 1 if it jumps (the caller redraws the view: LOOK1 with QQ11 = 1) */
int warp_blocked(void)
{
    if (frin[(u8)(2 + junk_count)] | sspr | mj) return 1;
    if (!(ks[8] & 0x80) && sm_mas2(0, 0) < 2) return 1;
    if (!(ks[NI + 8] & 0x80) && sm_mas2(0, NI) < 2) return 1;
    return 0;
}

int warp(void)
{
    if (warp_blocked()) { snd_boop(); return 0; }
    zS = 0x81; zR = 0x81; zP = 0x81;
    ks[8] = em_add(ks[8]);
    ks[NI + 8] = em_add(ks[NI + 8]);
    return 1;
}
