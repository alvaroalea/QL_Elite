/* Flight: stardust in the 4 views, radar with contacts, sight, tunnel, missiles and bulbs with pseudo-random controls. */
#include "platform.h"
#include "text.h"
#include "dash.h"
#include "space.h"
#include "stars.h"
#include "sun.h"
#include "chart.h"
#include "game.h"
#include "state.h"
#include "emath.h"
#include "keys.h"
#include "shipdata.h"
#include "ships.h"
#include "combat.h"
#include "damage.h"
#include "equip.h"
#include "dockit.h"
#include "cmdr.h"
typedef unsigned short u16s;
static u16s hv[60];
static u16s rs = 9876;
static unsigned rnd(void) { rs = (u16s)(rs * 25173u + 13849u); return (unsigned)(rs >> 4); }
static int n = 0;
static u8 dm[16];
static u8 de[16];                                   /* equipment: docking computer, bomb, WARP */                                   /* damage results */
static void hash(void)
{
    long j;
    u16s h = 0;
    for (j = 0; j < 32768L; j++) h = (u16s)(((h << 5) | (h >> 11)) ^ screen[j]);
    if (n < 58) hv[n++] = h;
}
int main(void)
{
    int view, f, i;
    struct controls c;
    plat_init(); txt_init();
    for (view = 0; view < 4; view++) {
        game_new();
        em_rand[0] = 3; em_rand[1] = 5; em_rand[2] = 7; em_rand[3] = (u8)(11 + view);
        gfx_clear(C_BLACK);
        space_launch();
        dash_show(); space_scanner_reset(); dash_msblob(); dash_spblb();
        chart_sight((u8)view);
        stars_new();
        for (i = 0; i < 3; i++) space_spawn((u8)(SHIPT_VIPER + i));
        combat_reset();
        space_view = (u8)view;                                 /* PLUT in the view axes */
        ship_zinf();                                           /* still target on the view axis */
        if (view == 0) inwk[7] = 2; else if (view == 1) { inwk[7] = 2; inwk[8] = 0x80; }
        else if (view == 2) { inwk[1] = 2; inwk[2] = 0x80; } else inwk[1] = 2;
        i = ship_new(SHIPT_SIDEWINDER); if (i >= 0) state_slot(i)[32] = 0;
        laser[0] = LASER_POW; laser[1] = LASER_BEAM; laser[2] = LASER_MIL; laser[3] = LASER_MINING;
        for (f = 0; f < 200; f++) {
            c.speed_up = (u8)((rnd() & 3) == 0); c.slow_down = (u8)((rnd() & 7) == 0);
            c.roll_left = (u8)((rnd() & 3) == 0 && f > 60); c.roll_right = (u8)((rnd() & 3) == 1 && f > 60);
            c.pitch_up = (u8)((rnd() & 3) == 0 && f > 60); c.pitch_down = (u8)((rnd() & 3) == 2 && f > 60);
            c.fire = c.bomb = c.escape_pod = c.target = c.unarm = c.missile = c.ecm = c.jump = c.dock = c.undock = 0;
            c.fire = (u8)((rnd() & 3) != 0);
            space_controls(&c);
            laser_fire((u8)view, c.fire);
            if (space_step() & SPACE_EV_STATION) dash_spblb();
            laser_frame_end();
            laser_cool();
            stars_set_speed(zDELTA);
            stars_update((u8)view);
            dash_update();
            if (f % 50 == 49) hash();
        }
        ships_explode_nearest(); ships_explode_nearest();      /* explosions: cloud and end */
        for (f = 0; f < 90; f++) {
            c.speed_up = c.slow_down = c.roll_left = c.roll_right = c.pitch_up = c.pitch_down = 0;
            c.fire = c.bomb = c.escape_pod = c.target = c.unarm = c.missile = c.ecm = c.jump = c.dock = c.undock = 0;
            space_controls(&c);
            space_step();
            stars_set_speed(zDELTA); stars_update((u8)view); dash_update();
            if (f == 30 || f == 89) hash();
        }
    }
    /* planet head-on and sun head-on */
    for (view = 0; view < 2; view++) {
        game_new(); gfx_clear(C_BLACK); sun_forget(); space_view = (u8)view; space_launch();
        for (i = 0; i < NI; i++) state_slot(0)[i] = 0;
        state_slot(0)[8] = 2; zDELTA = 3;
        for (f = 0; f < 4; f++) space_step();
        hash();
        ships_arrive();
        for (i = 0; i < NI; i++) state_slot(1)[i] = 0;
        state_slot(1)[8] = 1; state_slot(1)[2] = 0; state_slot(1)[5] = 0;
        for (f = 0; f < 4; f++) space_step();
        hash();
    }
    gfx_clear(C_BLACK); sun_col = C_WHITE; tunnel_draw(4); hash();
    gfx_clear(C_BLACK); tunnel_draw(8); hash();
    /* damage: head-on collision, collision without shield, nearby sun and DEATH debris */
    {
        int k;
        game_new(); combat_reset(); player_dead = 0; ships_arrive();
        ship_zinf(); inwk[6] = 40; i = ship_new(SHIPT_VIPER); if (i >= 0) state_slot(i)[32] = 0;
        zDELTA = 0; alpha = beta = 0; space_view = 0;
        for (f = 0; f < 40; f++) space_step();
        dm[0] = fsh; dm[1] = ash; dm[2] = energy; dm[3] = player_dead;
        fsh = 0; energy = 60; ship_zinf(); inwk[6] = 40; i = ship_new(SHIPT_ANACONDA); if (i >= 0) state_slot(i)[32] = 0;
        dm[4] = space_step(); dm[5] = player_dead; dm[6] = energy;
        ships_clear(); player_dead = 0; bst = 0xFF; qq14 = 10; zDELTA = 40;
        state_slot(1)[1] = 100; state_slot(1)[4] = 60; state_slot(1)[7] = 30;
        dm[7] = damage_checks(20); dm[8] = cabtmp; dm[9] = qq14;
        ships_clear(); ships_death_debris();
        dm[10] = frin[0]; dm[11] = frin[4]; dm[12] = state_slot(4)[31]; dm[13] = em_rand[0]; dm[14] = em_rand[1]; dm[15] = mcnt;
        (void)k;
    }
    {
        struct controls cc; int k;
        ships_clear(); sspr = 1; dock_auto = 0xFF; zDELTA = 30; jstx = 128; jsty = 128;
        for (k = 0; k < NI; k++) state_slot(1)[k] = (u8)(k * 37 + 11);
        state_slot(1)[1] = 3; state_slot(1)[2] = 0x80; state_slot(1)[4] = 1; state_slot(1)[5] = 0; state_slot(1)[7] = 4; state_slot(1)[8] = 0;
        state_slot(0)[1] = 50; state_slot(0)[7] = 90; state_slot(0)[8] = 1;
        for (f = 0; f < 4; f++) {
            for (k = 0; k < (int)sizeof cc; k++) ((u8 *)&cc)[k] = 0;
            dokey_auto(&cc);
            de[f * 2] = (u8)(jstx ^ (jsty << 1) ^ zDELTA);
            de[f * 2 + 1] = (u8)(cc.slow_down | cc.speed_up << 1 | cc.roll_left << 2 | cc.roll_right << 3 | cc.pitch_up << 4 | cc.pitch_down << 5);
            state_slot(1)[1]++;
        }
        dock_auto = 0; sspr = 0;
        bomb_on(0);
        for (k = 0; k < 10; k++) { de[8] = (u8)(de[8] + bombtbx[k]); de[9] = (u8)(de[9] ^ bombtby[k]); }
        ships_clear(); state_slot(0)[8] = 3; state_slot(1)[8] = 0x83;
        de[10] = (u8)warp(); de[11] = state_slot(0)[8]; de[12] = state_slot(1)[8];
        de[13] = em_rand[0]; de[14] = em_rand[2]; de[15] = zK[0];
    }
    gfx_clear(C_BLACK);
    for (i = 0; i < n; i++) { screen[2 * i] = (u8)hv[i]; screen[2 * i + 1] = (u8)(hv[i] >> 8); }
    screen[200] = (u8)n;
    screen[201] = tally[0]; screen[202] = tally[1]; screen[203] = tallyl; screen[204] = gntmp; screen[205] = energy;
    for (i = 0; i < 16; i++) screen[220 + i] = dm[i];
    for (i = 0; i < 16; i++) screen[240 + i] = de[i];
    screen[206] = cash[3]; screen[207] = cash[2]; screen[208] = fist;
    plat_present();
    return 0;
}
