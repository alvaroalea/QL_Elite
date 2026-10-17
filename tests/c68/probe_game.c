#include "platform.h"
#include "text.h"
#include "universe.h"
#include "game.h"
#include "cmdr.h"
#include "save.h"
#include "state.h"
#include "emath.h"
typedef unsigned short u16s;
static u16s hv[64];
static int nh = 0;
static u16s hh;
static void hb(u8 b) { hh = (u16s)(((hh << 5) | (hh >> 11)) ^ b); }
static void hl(unsigned long v) { hb((u8)v); hb((u8)(v >> 8)); hb((u8)(v >> 16)); hb((u8)(v >> 24)); }
static void hs(void) { long j; hh = 0; for (j = 0; j < 32768L; j++) hb(screen[j]); hv[nh++] = hh; hh = 0; }
static void fin(void) { hv[nh++] = hh; hh = 0; }
static char buf[600]; static int bl;
static void cap(int c) { if (bl < 599) buf[bl++] = (char)c; }
int main(void)
{
    int i, j, g, x, y;
    u8 blk[CMDR_SIZE], f[SAVE_FILE_SIZE];
    plat_init(); txt_init();
    /* 1. gfx: lineas en todas las direcciones (normal y XOR), rectangulos, pixeles */
    gfx_clear(C_BLACK);
    for (i = 0; i < 256; i += 9) { gfx_line(128, 128, i, 0, (u8)(1 + i % 7), 0); gfx_line(128, 128, 255 - i, 255, (u8)(1 + i % 7), 1); gfx_line(128, 128, 0, i, 3, 0); gfx_line(128, 128, 255, 255 - i, 5, 1); }
    for (i = 0; i < 10; i++) gfx_rect(i * 23, i * 19, 31, 17, (u8)(i % 8));
    gfx_line(5, 200, 5, 250, 2, 0); gfx_line(10, 250, 200, 250, 6, 0); gfx_line(0, 0, 255, 255, 7, 1);
    for (i = 0; i < 40; i++) { gfx_pixel(i * 6, 100 + i, (u8)(i % 8)); gfx_xor_pixel(i * 6 + 1, 100 + i, 7); }
    hs();
    /* 2. texto */
    gfx_clear(C_BLACK);
    txt_color(C_YELLOW, C_BLUE); txt_clear_rows(0, 1, C_BLUE); txt_print_at(3, 0, "HOLA ELITE QL 0123456789");
    txt_color(C_WHITE, C_BLACK); txt_print_at(0, 5, "The quick brown fox jumps over");
    txt_color(C_GREEN, TXT_TRANSPARENT); txt_print_at(4, 7, "transparente"); txt_color(C_RED, C_BLACK);
    txt_at(30, 20); txt_puts("ajuste de linea largo"); txt_putc('\b'); txt_putc('X');
    hs();
    /* 3. universo: 8 galaxias */
    for (g = 1; g <= 8; g++) {
        uni_build_galaxy(g);
        for (i = 0; i < UNI_NSYS; i++) {
            const uni_system *p = &uni_galaxy[i];
            hb(p->x); hb(p->y); hb(p->economy); hb(p->govtype); hb(p->techlev); hb(p->population); hl(p->productivity); hl(p->radius);
            for (j = 0; p->name[j]; j++) hb((u8)p->name[j]);
        }
        fin();
    }
    uni_build_galaxy(3);
    for (i = 0; i < 256; i += 3) for (j = 0; j < 256; j += 5) hl(uni_distance(&uni_galaxy[i], &uni_galaxy[j]));
    fin();
    for (i = 0; i < 256; i += 7) { uni_market m; uni_gen_market((u8)(i * 3), &uni_galaxy[i], &m); for (j = 0; j < 17; j++) { hl(m.price[j]); hl(m.quantity[j]); } }
    fin();
    for (i = 0; i < 256; i += 11) { bl = 0; uni_putc = cap; uni_describe(&uni_galaxy[i]); for (j = 0; j < bl; j++) hb((u8)buf[j]); }
    fin();
    for (x = 0; x < 256; x += 17) for (y = 0; y < 256; y += 19) hb((u8)uni_nearest((u8)x, (u8)y));
    fin();
    /* 4. juego: comercio y equipo */
    game_new();
    em_rand[0] = 1; em_rand[1] = 2; em_rand[2] = 3; em_rand[3] = 4;
    for (i = 0; i < 17; i++) { hl(trade_buy(i, (unsigned)(i % 5 + 1))); hl(cash_get()); }
    for (i = 0; i < 17; i++) { hl(trade_sell(i, 1)); hl(cash_get()); hl(trade_max_buy(i)); }
    for (i = 0; i < 14; i++) { hl(equip_buy(i, i & 3)); hl(cash_get()); hl(equip_price(i)); }
    for (i = 0; i < 17; i++) hb(qq20[i]);
    for (i = 0; i < 17; i++) hb(avl[i]);
    fin();
    for (i = 0; i < 256; i++) if (jump_possible(i)) { hl(jump_distance(i)); }
    fin();
    for (i = 0; i < 12; i++) { int k = 0; for (j = 0; j < 256 && k < 3; j++) if (jump_possible(j)) { jump_do(j); hl(cash_get()); hb(qq14); hb(qq26); hb(cur_sys); k++; break; } qq14 = 70; }
    fin();
    cash_set(123456789UL); hl(cash_get()); cash_earn(4000000000UL); hl(cash_get()); cash_spend(77UL); hl(cash_get());
    fin();
    /* 5. comandante y fichero */
    game_new();
    cmdr_pack(blk); cmdr_seal(blk);
    for (i = 0; i < CMDR_SIZE; i++) hb(blk[i]);
    save_seed = 0x9C; save_encode(blk, "TESTER", f);
    for (i = 0; i < SAVE_FILE_SIZE; i++) hb(f[i]);
    { char nm[8]; u8 b2[CMDR_SIZE]; hb((u8)save_decode(f, nm, b2)); for (i = 0; i < CMDR_SIZE; i++) hb(b2[i]); hb((u8)cmdr_unpack(b2)); }
    for (i = 0; i < 4; i++) { hb(rating_name()[0]); tally[1] = (u8)(i * 9); }
    fin();
    for (i = 0; i < 256; i++) { hb((u8)em_dornd()); }
    fin();
    gfx_clear(C_BLACK);
    for (i = 0; i < nh; i++) { screen[2 * i] = (u8)hv[i]; screen[2 * i + 1] = (u8)(hv[i] >> 8); }
    screen[200] = (u8)nh;
    plat_present();
    return 0;
}
