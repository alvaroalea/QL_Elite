#include "combat.h"
#include "state.h"
#include "emath.h"
#include "ship_math.h"
#include "ship3d.h"
#include "shipdata.h"
#include "ships.h"
#include "space.h"
#include "cmdr.h"
#include "sound.h"
#include "damage.h"
#include "tactics.h"
#include "lines.h"
#include "move.h"

u8 las, las2, lasct, lasx, lasy, dly;
u8 combat_view_on = 1;
void (*combat_mess_hook)(u8 token) = 0;
void (*combat_mess_off_hook)(void) = 0;

#define VIEW_X 128                       /* X: centre of the 256 x 136 space view */
#define VIEW_Y view_cy                   /* Y (68; 80 in the native view) */
#define MLAS 50                          /* Mlas: mining laser power */

/* KWL% and KWH%: what each ship type adds to TALLY (1..32), fractional part (/256) and integer part */
static const u8 kwl[32] = {
    149, 0, 16, 10, 10, 6, 8, 10, 16, 17, 234, 170, 213, 0, 85, 26,
    85, 128, 85, 90, 85, 170, 50, 42, 21, 42, 64, 192, 170, 33, 85, 0
};
static const u8 kwh[32] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 2, 0, 5, 0
};

/* DOENTRY/RES2: on launch from the station everything is full and cold */
void combat_reset(void)
{
    energy = 255; fsh = 255; ash = 255;
    gntmp = 0;
    las = las2 = lasct = 0;
    dly = 0;
    combat_view_on = 1;
}

/* TTX66 sets LAS2 = 0: the view has been erased and LASLI2 must not draw (XOR) lines that are no longer there */
void laser_forget(void) { las2 = 0; }

/* DENGY: DEC ENERGY; if it reaches 0 it stays at 1. Returns the Z flag of the DEC */
u8 dengy(void)
{
    energy--;
    if (energy == 0) { energy++; return 1; }
    return 0;
}

/* SHD: INX; if it wraps, DEX and return (shield full); otherwise fall into DENGY: charging the shield costs 1 energy */
u8 shd(u8 x)
{
    x++;
    if (x == 0) return 255;
    dengy();
    return x;
}

/* las: two lines from (LASX, LASY) to (a, 2Y-1) and to (y, 2Y-1) with LL30 (= LOIN, XOR) */
static void las_pair(u8 a, u8 y)
{
    loin_draw(lasx, lasy, a, 2 * VIEW_Y - 1, LASER_COL);
    loin_draw(lasx, lasy, y, 2 * VIEW_Y - 1, LASER_COL);
}

void lasli2(void)
{
    if (!combat_view_on) return;         /* LDA QQ11; BNE RE1 */
    las_pair(32, 224);
    las_pair(48, 208);
}

void lasli(void)
{
    u8 a;
    a = em_dornd();
    lasy = (u8)((a & 7) + (VIEW_Y - 4) + (em_rand_c & 1));     /* AND #7; ADC #Y-4 (carry from DORND) */
    em_rand_c = 0;                                              /* the ADC does not carry: the next DORND enters with C = 0 */
    a = em_dornd();
    lasx = (u8)((a & 7) + (VIEW_X - 4) + (em_rand_c & 1));     /* AND #7; ADC #X-4 */
    em_rand_c = 0;
    gntmp = (u8)(gntmp + 8);                                    /* LDA GNTMP; ADC #8 (the previous ADC does not carry) */
    dengy();
    lasli2();
}

/* Part 3 of the flight loop (MA68): LAS = 0; if LASCT = 0, fire is pressed, GNTMP < 242 and the view has a laser:
   LAS = LAS2 = power (bits 0-6), sound, lines, and LASCT = power AND %11111010 for pulse lasers (0 if bit 7) */
void laser_fire(u8 view, u8 fire)
{
    u8 l;
    las = 0;
    if (lasct) return;
    if (!fire) return;
    if (gntmp >= 242) return;
    l = laser[view & 3];
    if (!l) return;
    las = las2 = (u8)(l & 0x7F);
    snd_lasnoise();
    lasli();
    lasct = (u8)((l & 0x80) ? 0 : (l & 0xFA));
}

/* Part 16 (MA23): with LAS2 != 0 and LASCT < 8 the lines are erased and LAS2 = 0. With beam and military (LASCT = 0) they are erased in the
   same frame they are drawn: the flicker is the original's */
void laser_frame_end(void)
{
    if (las2 == 0) return;
    if (lasct >= 8) return;
    lasli2();
    las2 = 0;
}

/* MLOOP: the laser cools 1 per iteration and LASCT drops by 2 (from 1 to 0 if odd) */
void laser_cool(void)
{
    u8 x;
    if (gntmp) gntmp--;
    x = lasct;
    if (x) { x--; if (x) x--; lasct = x; }
}

/* HITCH: the ship in INWK (already in view axes, after PLUT) is ahead (positive z), is not a planet or sun, is not exploding,
   x_hi = y_hi = 0 and x_lo^2 + y_lo^2 <= the plane's hit area (bytes 1 and 2). Returns the carry */
u8 hitch(void)
{
    u8 s, r, x;
    unsigned u;
    if (inwk[8]) return 0;
    if (ztype & 0x80) return 0;
    if ((inwk[31] & 0x20) | inwk[1] | inwk[4]) return 0;
    s = em_squa2(inwk[0]); r = zP;
    x = em_squa2(inwk[3]);
    u = (unsigned)zP + r; r = (u8)u;                 /* SQUA2 returns with C = 0 */
    u = (unsigned)x + s + (u >> 8);
    if (u > 255) return 0;                           /* BCS TN10 */
    s = (u8)u;
    if (zxx0[2] != s) return (u8)(zxx0[2] > s);      /* CMP S; BNE HI1 */
    return (u8)(zxx0[1] >= r);                       /* CMP R */
}

/* AN2: the station (slot 1) turns hostile (NEWB bit 2) */
static void an2(void) { state_slot(1)[I_NEWB] = (u8)(state_slot(1)[I_NEWB] | 0x04); }

/* ANGRY: writes to the ship's K% block (INF), not to INWK */
void angry(u8 type)
{
    u8 *p = zinf;
    if (type == SHIPT_CORIOLIS) { an2(); return; }
    if (p[I_NEWB] & 0x20) an2();                     /* the station protects it */
    if (p[I_AI] == 0) return;                        /* no AI: nothing to anger */
    p[I_AI] = (u8)(p[I_AI] | 0x80);
    p[28] = 2;                                       /* accelerates */
    p[30] = 4;                                       /* pitches */
    if (ztype >= SHIPT_COBRA_MK_3) p[I_NEWB] = (u8)(p[I_NEWB] | 0x04);   /* LDA TYPE: the ship being processed */
}

/* EXNO2: TALLY(1 0) + fraction; if the high byte rises, "RIGHT ON COMMANDER!" */
void exno2(u8 type)
{
    unsigned s;
    if (type >= 1 && type <= 32) {
        s = (unsigned)tallyl + kwl[type - 1]; tallyl = (u8)s;
        s = (unsigned)tally[0] + kwh[type - 1] + (s >> 8); tally[0] = (u8)s;
        if (s > 255) { tally[1]++; mess(MESS_RIGHT_ON); }
    }
    snd_exno2();                                     /* SOEXPL with Y = 55 */
}

/* Part 11 of the flight loop, after PLUT. No missiles yet: the missile's target warning (MSAR) is omitted */
void combat_hit(void)
{
    u8 type = ztype;
    int d;
    if (!hitch()) return;                            /* BCC MA8 */
    missile_lock(move_xsav);                         /* LDA MSAR; BEQ MA47; JSR BEEP; LDX XSAV; LDY #RED; JSR ABORT2 */
    if (las == 0) return;                            /* MA47 */
    snd_exno();                                      /* EXNO: hit */
    if (type == SHIPT_CORIOLIS) goto mad;            /* the station does not suffer, but gets angry */
    if (type >= SHIPT_CONSTRICTOR) {
        if (las != (LASER_MIL & 0x7F)) goto mad;     /* only the military laser damages it... */
        las = (u8)(las >> 2);                        /* ...to a quarter (LSR LAS twice: stays like that this frame) */
    }
    d = (int)inwk[I_ENERGY] - las;                   /* BURN */
    if (d >= 0) { inwk[I_ENERGY] = (u8)d; goto mad; }
    inwk[31] = (u8)(inwk[31] | 0x80);                /* destroyed: LL9 makes it explode */
    /* the carry with which the first DORND enters comes from the original's comparisons (CMP #AST, CMP #Mlas) */
    if (type == SHIPT_ASTEROID) {
        if (las == MLAS) { em_rand_c = 1; spin2((u8)(em_dornd() & 3), SHIPT_SPLINTER); }
        else em_rand_c = (u8)(las >= MLAS);
    } else em_rand_c = (u8)(type >= SHIPT_ASTEROID);
    spin(SHIPT_PLATE);
    spin(SHIPT_CANISTER);
    exno2(type);
    inwk[I_ENERGY] = 0;                              /* the original stores what the sound leaves in A; the ship no longer counts */
mad:
    angry(type);
}

/* Part 12 (MA15..KS1S). With NEWB bit 7 (scooped or docked) it is just removed. When the explosion ends: FIST |= bit 6 of
   NEWB (police) and, if there is no message on screen (DLY) and we are not in hyperspace (MJ), the plane's bounty (bytes
   10 and 11; nothing if byte 10 is 0) and the message with the money */
int combat_killed(void)
{
    u8 x;
    if (inwk[I_NEWB] & 0x80) return 1;
    if (!(inwk[31] & 0x80) || !(inwk[31] & 0x20)) return 0;
    fist = (u8)(fist | (inwk[I_NEWB] & 0x40));
    if (dly | mj) return 1;
    if (ztype & 0x80) return 1;
    x = zxx0[10];
    if (x == 0) return 1;
    sm_mcash(x, zxx0[11]);
    mess(MESS_CASH);
    return 1;
}

/* Part 16 (MA77..paen1): every 8 frames, with energy >= 128 the shields are recharged and energy rises by ENGY + 1 */
void energy_tick(u8 mc)
{
    unsigned s;
    if (mc & 7) return;
    if (energy & 0x80) { ash = shd(ash); fsh = shd(fsh); }
    s = (unsigned)engy + energy + 1;                 /* SEC; LDA ENGY; ADC ENERGY */
    if (s <= 255) energy = (u8)s;                    /* BCS paen1 */
}

void mess(u8 token)
{
    if (combat_mess_hook) combat_mess_hook(token);
    dly = MESS_DLY;
    mess_de = 0;                                 /* " DESTROYED" is valid for a single message (consumed by MESS and its erasure) */
}

/* TT100: DEC DLY; when it reaches 0 the message is erased; if it goes negative it returns to 0 */
void dly_tick(void)
{
    if (dly == 0) return;
    dly--;
    if (dly == 0 && combat_mess_off_hook) combat_mess_off_hook();
}
