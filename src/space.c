#include "space.h"
#include "state.h"
#include "emath.h"
#include "ship_math.h"
#include "universe.h"
#include "game.h"
#include "shipdata.h"
#include "ships.h"
#include "combat.h"
#include "damage.h"
#include "stars.h"
#include "sun.h"
#include "tactics.h"
#include "dash.h"
#include "keys.h"

u8 jstx = 128, jsty = 128;
u8 alpha, beta, alp1, bet1;
u8 alp2[2], bet2[2];
u8 mcnt, flh = 0, damp = 0, djd = 0;
u8 space_view = 0;                     /* VIEW: current view (0 front, 1 rear, 2 left, 3 right) */
u8 energy = 255, fsh = 255, ash = 255;
u8 altit = 255, cabtmp = 30, gntmp = 0;
#define auto_pilot dock_auto               /* auto: the docking computer is in charge (flight.c) */

int space_signed(u8 v) { return (v & 0x80) ? -(int)(v & 0x7F) : (int)v; }

/* RES2: initial values of the turns and speed, dust, MSTG, sentinels of the last circle, and switches off the S bulb and the ECM if
   they were on. Clearing the screen (WPSHPS) is done by the caller (clear_view); JSTX is not touched, like the original */
void space_reset(void)
{
    stars_nostm = NOST;
    circ_lsx2[0] = 0xFF;
    circ_lsy2[0] = 0xFF;
    mstg = 0xFF;
    jsty = 128;
    alp2[0] = 128; bet2[0] = 128;
    beta = 0; bet1 = 0;
    alp2[1] = 0; bet2[1] = 0;
    mcnt = 0;
    zDELTA = 3; alpha = 3; alp1 = 3;
    if (sspr) dash_spblb();
    if (ecma) ecm_off();
    ships_zero();                                /* WPSHPS (caller) and ZERO */
    ships_slsp_reset();                          /* SLSP = LS% */
    ship_zinf();                                 /* ZINF */
}

/* RESET: ZERO, turns and hyperspace counters to 0, JSTGY and QQ12 to $FF (docked), shields and energy full, and RES2 */
void space_reset_all(void)
{
    ships_zero();
    beta = 0; bet1 = 0; qq22[0] = 0; qq22[1] = 0; ecma = 0; alp1 = 0; alp2[0] = 0;
    jstgy = 0xFF;
    qq12 = 0xFF;
    fsh = 0xFF; ash = 0xFF; energy = 0xFF;
    space_reset();
}

/* BUMP2: X += A (saturates at 255); if it ends in the lower half and DJD = 0, it stops at the centre (128) */
u8 space_bump2(u8 x, u8 a)
{
    unsigned s = (unsigned)x + a;
    x = (s > 255) ? 255 : (u8)s;
    if (x & 0x80) return x;                      /* BPL djd1 not taken */
    if (djd) return x;                           /* LDA DJD; BNE RE2+2 */
    return 128;                                  /* LDX #128 */
}

/* REDU2: X -= A (minimum 1); if it is still in the upper half and DJD = 0, it stops at the centre */
u8 space_redu2(u8 x, u8 a)
{
    if (x >= a) x = (u8)(x - a); else x = 1;     /* SEC; SBC T; BCS RE3; LDX #1 */
    if (!(x & 0x80)) return x;                   /* BPL RE2+2 */
    if (djd) return x;
    return 128;
}

/* cntr: one step towards the centre (128), except with damping off and no autopilot */
u8 space_cntr(u8 x)
{
    if (auto_pilot == 0 && damp != 0) return x;  /* LDA auto; BNE cnt2; LDA DAMP; BNE RE1 */
    if (!(x & 0x80)) goto bump;                  /* TXA; BPL BUMP */
    x--;                                         /* DEX */
    if (x & 0x80) return x;                      /* BMI RE1 */
bump:
    x++;                                         /* INX */
    if (x != 0) return x;                        /* BNE RE1 */
    x--;                                         /* REDU: DEX */
    if (x == 0) goto bump;                       /* BEQ BUMP */
    return x;
}

void space_controls(const struct controls *c)
{
    u8 x, a, y, carry;

    /* DOKEY: BUMP2/REDU2 with A = 20 */
    x = jstx;
    if (c->roll_left)  x = space_bump2(x, 20);   /* KY3: , */
    if (c->roll_right) x = space_redu2(x, 20);   /* KY4: . */
    jstx = x;
    x = jsty;
    if (c->pitch_up)   x = space_redu2(x, 20);   /* KY5: X (climb) */
    if (c->pitch_down) x = space_bump2(x, 20);   /* KY6: S (dive) */
    jsty = x;

    /* speed: space accelerates up to 40, / brakes down to 1 */
    if (c->speed_up && zDELTA < 40) zDELTA++;
    if (c->slow_down) { zDELTA--; if (zDELTA == 0) zDELTA = 1; }

    /* main loop: roll */
    x = space_cntr(jstx); x = space_cntr(x);
    a = (u8)(x ^ 0x80); y = a;
    alp2[0] = (u8)(a & 0x80);
    jstx = x;
    alp2[1] = (u8)((a & 0x80) ^ 0x80);
    a = y;
    if (a & 0x80) a = (u8)((a ^ 0xFF) + 1);      /* BPL P%+7; EOR #$FF; CLC; ADC #1 */
    a = (u8)(a >> 2);
    carry = (u8)(a >= 8);                        /* CMP #8; BCS P%+3 */
    if (!carry) { carry = (u8)(a & 1); a = (u8)(a >> 1); }
    alp1 = a;
    alpha = (u8)(a | alp2[0]);

    /* pitch (the ADC #4 uses the carry left by the roll) */
    x = space_cntr(jsty);
    a = (u8)(x ^ 0x80); y = a;
    a = (u8)(a & 0x80);
    jsty = x;
    bet2[1] = a;
    bet2[0] = (u8)(a ^ 0x80);
    a = y;
    if (a & 0x80) a = (u8)(a ^ 0xFF);            /* BPL P%+4; EOR #$FF */
    a = (u8)((unsigned)a + 4 + carry);           /* ADC #4 */
    a = (u8)(a >> 4);
    if (a < 3) a = (u8)(a >> 1);                 /* CMP #3; BCS P%+3; LSR A */
    bet1 = a;
    beta = (u8)(a | bet2[0]);
}

/* ---------------------------------------------------------------- radar (SCAN) */

/* scacol: colour of each ship type on the radar (B blue, R red, C cyan, M magenta, F "fuzzy", 0 not drawn) */
static const char scacol[] = "0BBRRRRRRCCCMMMRCCCCCCCBCCMCCFCC0C";

/* the original's COL for the type (BLUE 16, RED 20, FUZZY 24; CYAN and MAG are WHITE, 12, on the Apple II): for the tests */
u8 space_scacol(u8 type)
{
    if (type >= sizeof scacol - 1) return 0xFF;
    switch (scacol[type]) {
    case 'B': return 16;
    case 'R': return 20;
    case 'F': return 24;
    case 'C': case 'M': return 12;
    }
    return 0;
}

static u8 scan_ql(char k)
{
    switch (k) {
    case 'B': return C_BLUE;
    case 'R': return C_RED;
    case 'C': return C_CYAN;
    case 'M': return C_MAGENTA;
    case 'F': return C_WHITE;
    }
    return C_BLACK;
}

/* SCAN: draws (XOR) the ship in INWK on the radar if its bit 4 of INWK+31 is set: a dot 4 wide and a stick down to the
   radar plane. Planets and suns (type with bit 7) do not show; neither does anything 16384 units or more away on some axis. */
void (*space_scan_hook)(u8 type) = 0;

void space_scan(u8 type)
{
    u8 a, x1, y2, col;
    unsigned s;
    if (space_scan_hook) { space_scan_hook(type); return; }
    if (!(inwk[31] & 0x10)) return;
    if (type & 0x80) return;
    if (type >= sizeof scacol - 1) return;
    col = scan_ql(scacol[type]);
    if (col == C_BLACK) return;
    if ((inwk[1] | inwk[4] | inwk[7]) & 0xC0) return;
    a = inwk[1];
    if (inwk[2] & 0x80) a = (u8)((a ^ 0xFF) + 1);            /* negative x: EOR #$FF; ADC #1; CLC */
    s = (unsigned)a + 125;                                    /* SC2: ADC #125 */
    x1 = (u8)((u8)s & 0xFE);
    a = (u8)(inwk[7] >> 2);                                   /* z: LSR A; LSR A; CLC */
    s = (unsigned)a;
    if (inwk[8] & 0x80) s = (unsigned)(u8)(a ^ 0xFF) + 1;     /* negative z (behind): EOR #$FF; SEC */
    s = s + 91;                                               /* SC3: ADC #91 */
    y2 = (u8)((u8)s ^ 0xFF);
    a = (u8)(inwk[4] >> 1);                                   /* y: LSR A; CLC */
    s = (unsigned)a;
    if (!(inwk[5] & 0x80)) s = (unsigned)(u8)(a ^ 0xFF) + 1;  /* positive y (above): EOR #$FF; SEC */
    s = s + y2;                                               /* SCD6: ADC Y2 */
    a = (u8)s;
    if (a < 146) a = 146;                                     /* CMP #146; BCS ...; LDA #146 */
    if (a >= 191) a = 190;                                    /* CMP #191; BCC ...; LDA #190 */
    if (dash_trace) { dash_trace('c', (u8)(x1 - 2), a, 0, space_scacol(type)); dash_trace('v', x1, a, y2, space_scacol(type)); }
    gfx_yoff_set(view_native ? DASH_Y0 - 136 : 0);            /* the radar is in the panel */
    gfx_cpix((int)x1 - 2, a, col);                            /* head (CPIX with X = X1 - 2) */
    gfx_vline_xor((int)x1, a, y2, col);                       /* stick: own, 1 dot wide (VLOIN of the original: 2) with the column in assembly */
    gfx_yoff_set(0);
}

static void load_slot(int n) { u8 *p = state_slot(n); int i; for (i = 0; i < NI; i++) inwk[i] = p[i]; }
static void store_slot(int n) { u8 *p = state_slot(n); int i; for (i = 0; i < NI; i++) p[i] = inwk[i]; }

/* After repainting the panel the radar is empty: no ship counts as drawn */
void space_scanner_reset(void)
{
    int n;
    for (n = 0; n <= NOSH; n++) state_slot(n)[I_STATE] = (u8)(state_slot(n)[I_STATE] & 0xEF);
}

/* Planet (slot 0) and station (slot 1) relative to the ship on launch (ships.c: SOLAR and NWSPS); resets the turns and speed */
void space_launch(void)
{
    space_reset();                               /* RES2 */
    ships_launch();                              /* SOS1 y NWSPS */
    game_tt111();                                /* TT110: TT111 on launch */
    zDELTA = 12;                                 /* launch speed */
    fist = (u8)(fist | space_bad());             /* JSR BAD; ORA FIST */
}

/* Arrival in a system after hyperspace: planet far ahead and the sun, with no station nearby */
void space_arrive(void)
{
    space_reset();                               /* RES2 */
    ships_arrive();                              /* SOLAR */
}

/* BAD: (slaves + narcotics) * 2 + firearms (8 bits) */
u8 space_bad(void) { return (u8)((u8)((u8)(qq20[3] + qq20[6]) << 1) + qq20[10] + (u8)((u8)(qq20[3] + qq20[6]) >> 7)); }

/* Clears from the radar everything that counts as drawn (before changing system) */
void space_scanner_clear(void)
{
    int n;
    for (n = 0; n < NOSH; n++) {
        if (frin[n] == 0) continue;
        load_slot(n);
        space_scan(frin[n]);
        inwk[I_STATE] = (u8)(inwk[I_STATE] & 0xEF);
        store_slot(n);
    }
}

/* A test contact (ships.c): a real ship, with its plane, orientation, speed and turns */
int space_spawn(u8 type) { return ships_spawn(type); }

/* One frame of local space: each ship moves (MVEIT), is drawn (LL9) and removed if needed (ships.c). Returns the SPACE_EV_* */
u8 space_step(void)
{
    u8 ev = 0, e;
    e = ships_step(space_view);
    if (e & SHIPS_EV_STATION) ev |= SPACE_EV_STATION;
    if (e & SHIPS_EV_DOCK) return (u8)(ev | SPACE_EV_DOCK);      /* the original jumps to DOENTRY without finishing the frame */
    if (e & SHIPS_EV_DEATH) return (u8)(ev | SPACE_EV_DEATH);
    energy_tick(mcnt);                           /* MA77: shields and energy every 8 frames */
    if (ships_part14(mcnt)) ev |= SPACE_EV_STATION;   /* MA22: the station appears (SPBLB) */
    if (damage_checks(mcnt)) ev |= SPACE_EV_DEATH;   /* MA22/MA93: ENERGY LOW, altitude, cockpit and fuel */
    return ev;
}
