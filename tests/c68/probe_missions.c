/* c68 probe: token engine (all the TKN1 and RUTOK tokens, and the normal ones), PDESC, the DOENTRY decision and the
   appearance of ships, with fixed states; the hashes go to the "screen" to compare the PC with c68 */
#include "platform.h"
#include "text.h"
#include "detok.h"
#include "missions.h"
#include "encounters.h"
#include "state.h"
#include "emath.h"
#include "ships.h"
#include "shipdata.h"
#include "cmdr.h"
#include "game.h"
typedef unsigned short u16s;
static u16s hv[64];
static int nh = 0;
static u16s hh;
static void hb(u8 b) { hh = (u16s)(((hh << 5) | (hh >> 11)) ^ b); }
static void fin(void) { hv[nh++] = hh; hh = 0; }
static void let(u8 c, u8 x, u8 y) { hb(c); hb(x); hb(y); }
static void ev(void) { hb(0xEE); }
static void ev1(u8 a) { hb(0xED); hb(a); }
static void name(u8 w) { const char *s = w == 3 ? "LAVE" : "DISO"; for (; *s; s++) dt_tt27((u8)*s); }
static void st(int i)
{
    qq17 = (u8)(i & 1 ? 0x80 : 0); dtw1 = 0x20; dtw2 = 0xFF; dtw3 = 0; dtw4 = (u8)(i & 2 ? 0x80 : 0); dtw5 = 0; dtw6 = 0;
    dtw8 = 0xFF; dt_xc = (u8)(i % 31); dt_yc = (u8)(i % 24); gcnt = (u8)(i % 3); mj = 0;
}
int main(void)
{
    int i, k;
    plat_init(); txt_init();
    state_clear();
    dt_letter_hook = let; dt_clss_hook = ev; dt_beep_hook = ev; dt_tt66_hook = ev1; dt_pause2_hook = ev; dt_delay_hook = ev1;
    dt_name_hook = name;
    em_rand[0] = 1; em_rand[1] = 2; em_rand[2] = 3; em_rand[3] = 4;
    for (i = 1; i < 256; i++) { st(i); detok((u8)i); hb(dtw5); hb(dt_xc); hb(dt_yc); hb(em_rand[0]); }
    fin();
    for (i = 1; i < 27; i++) { st(i); detok3((u8)i); hb(dtw5); hb(em_rand[1]); }
    fin();
    for (i = 2; i < 256; i++) { if (i == 5 || i == 119 || i == 125 || i == 199 || i == 225) continue; st(i); dt_tt27((u8)i); hb(dtw5); hb(qq17); }
    fin();
    for (i = 1; i <= dt_nru(); i++) { st(0); tp = 1; qq8[0] = qq8[1] = 0; qq12 = 0x80; gcnt = (u8)(dt_rugal((u8)i) & 0x7F); hb((u8)pdesc(dt_rupla((u8)i), 0)); }
    fin();
    for (i = 0; i < 256; i++) { tp = (u8)i; tally[1] = (u8)(i * 7); gcnt = (u8)(i % 4); qq0 = (u8)(i & 1 ? 215 : 63); qq1 = (u8)(i & 1 ? 84 : 72); hb((u8)missions_choose()); }
    fin();
    for (i = 0; i < 400; i++) {
        ships_clear();
        frin[0] = 0x81; sspr = (u8)(i % 7 == 0); mj = 0; junk_count = (u8)(i % 4); tp = (u8)(i % 3 ? 1 : 8); gcnt = 1;
        qq0 = 144; qq1 = 33; qq4 = (u8)(i % 8); qq20[3] = (u8)i; qq20[6] = 0; qq20[10] = (u8)(i >> 2); fist = (u8)(i * 3); enc_ev = (u8)(i % 5 ? 0 : 0xFF);
        encounters_tick((u8)(i & 1));
        for (k = 0; k < NOSH; k++) { int j; hb(frin[k]); for (j = 0; j < NI; j++) hb(state_slot(k)[j]); }
        hb(enc_ev); hb(junk_count); hb(enc_c);
    }
    fin();
    { u8 k4[4]; for (i = 0; i < 200; i++) { k4[0] = (u8)(i * 7); k4[1] = (u8)(i * 13); k4[2] = (u8)(i * 29); k4[3] = (u8)i; st(i); dt_bprnt(k4, (u8)(1 + i % 11), (u8)(i & 1)); } }
    fin();
    for (i = 0; i < 32; i++) { u8 q[6]; q[0] = (u8)i; q[1] = (u8)(i * 3); q[2] = (u8)(i * 5); q[3] = (u8)(i * 7); q[4] = (u8)(i * 37); q[5] = (u8)(i * 11);
        st(0); qq12 = 0x80; qq8[0] = (u8)i; qq8[1] = 0; tt25_raw(q, (u8)(i & 7), (u8)((i >> 1) & 7), (u8)(i % 15), (u8)(i * 2), (u8)i, 1, (u8)i, 0, (u8)i); }
    fin();
    gfx_clear(C_BLACK);
    for (i = 0; i < nh; i++) { screen[2 * i] = (u8)hv[i]; screen[2 * i + 1] = (u8)(hv[i] >> 8); }
    screen[200] = (u8)nh;
    plat_present();
    return 0;
}
