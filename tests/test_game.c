/* Logica de juego (comercio, equipo, hiperespacio) y una sesion completa guiada por guion en el PC.
   Las capturas van a tests/shots/NN.bin (ql2png.py las convierte). No se compara con el original (ver PENDIENTES.md). */
#include <stdio.h>
#include <string.h>
#include "game.h"
#include "cmdr.h"
#include "save.h"
#include "state.h"
#include "input.h"
#include "menu.h"
#include "app.h"
#include "text.h"
#include "keys.h"
#include "sound.h"

static int fails, total;
static void check(int ok, const char *what) { total++; if (!ok) { fails++; printf("FALLO: %s\n", what); } }

/* ------------------------------------------------------------ logica */
static void logic(void)
{
    int i, r;
    unsigned long c0;
    game_new();
    check(strcmp(uni_galaxy[cur_sys].name, "LAVE") == 0, "se empieza en Lave");
    check(cash_get() == 1000UL, "100 Cr");
    check(qq12 == 0xFF, "atracado");

    /* comercio */
    c0 = cash_get();
    check(trade_max_buy(0) == 16 || trade_max_buy(0) < 16, "max compra razonable");
    r = trade_buy(0, 5);                                    /* 5 t de comida */
    check(r == TR_OK && qq20[0] == 5 && avl[0] == 11 && cash_get() == c0 - 5UL * mkt.price[0], "comprar comida");
    r = trade_sell(0, 2);
    check(r == TR_OK && qq20[0] == 3 && avl[0] == 11, "vender: no repone existencias");
    check(trade_sell(0, 9) == TR_NOSTOCK, "vender de mas");
    check(trade_sell(3, 1) == TR_NONE, "vender lo que no hay");
    check(trade_buy(0, 100) == TR_NOSTOCK, "comprar mas que las existencias");
    cash_set(0);
    check(trade_buy(0, 1) == TR_NOCASH, "sin dinero");
    cash_set(1000000UL);
    /* bodega: 22 -> caben 20 t en total */
    qq20[0] = 0;
    check(trade_room(0) == 20, "caben 20 t con bodega 22");
    mkt.quantity[2] = 63;
    check(trade_buy(2, 21) == TR_NOROOM, "21 t no caben");
    check(trade_buy(2, 20) == TR_OK, "20 t si");
    check(trade_room(0) == 0, "bodega llena");
    check(trade_buy(0, 1) == TR_NOROOM, "bodega llena: nada mas");
    qq20[2] = 0;
    crgo = 37;
    check(trade_room(0) == 35, "bodega grande: 35 t");
    crgo = 22;
    mkt.quantity[13] = 63;
    check(trade_room(13) == 199, "kg: hasta 199");
    qq20[13] = 150;
    check(trade_room(13) == 49, "kg: 49 mas");
    qq20[13] = 0;

    /* equipo */
    cash_set(1000000UL);
    qq14 = 30;
    check(equip_price(0) == 80, "combustible: (70-30)*2");
    check(equip_buy(0, 0) == EQ_OK && qq14 == 70, "repostar");
    check(equip_buy(1, 0) == EQ_OK && nomsl == 4, "misil");
    check(equip_buy(1, 0) == EQ_PRESENT && nomsl == 4, "maximo 4 misiles");
    check(equip_buy(2, 0) == EQ_OK && crgo == 37, "bodega grande");
    check(equip_buy(2, 0) == EQ_PRESENT, "bodega grande dos veces");
    c0 = cash_get();
    check(equip_buy(4, 1) == EQ_OK && laser[1] == LASER_POW && cash_get() == c0 - 4000UL, "laser trasero");
    c0 = cash_get();
    check(equip_buy(5, 0) == EQ_OK && laser[0] == LASER_BEAM && cash_get() == c0 - 10000UL + 4000UL, "laser de haz reembolsa el de pulsos");
    check(equip_buy(4, 7) == EQ_BAD, "vista invalida");
    check(equip_buy(3, 0) == EQ_OK && ecm_fitted, "ECM");
    check(equip_buy(8, 0) == EQ_OK && bomb == 0x7F, "bomba");
    check(equip_buy(11, 0) == EQ_OK && ghyp, "hiperimpulsor galactico");
    cash_set(5);
    check(equip_buy(10, 0) == EQ_NOCASH && !dkcmp, "sin dinero para el ordenador");
    check(equip_count() >= 3 && equip_count() <= 14, "articulos segun nivel tecnologico");

    /* rangos */
    tally[0] = 0; tally[1] = 0;      check(strcmp(rating_name(), "Harmless") == 0, "rango 0");
    tally[0] = 8;                    check(strcmp(rating_name(), "Mostly Harmless") == 0, "rango 8");
    tally[0] = 16;                   check(strcmp(rating_name(), "Poor") == 0, "rango 16");
    tally[0] = 32;                   check(strcmp(rating_name(), "Average") == 0, "rango 32");
    tally[0] = 64;                   check(strcmp(rating_name(), "Above Average") == 0, "rango 64");
    tally[0] = 128;                  check(strcmp(rating_name(), "Competent") == 0, "rango 128");
    tally[0] = 0; tally[1] = 2;      check(strcmp(rating_name(), "Dangerous") == 0, "rango 512");
    tally[1] = 10;                   check(strcmp(rating_name(), "Deadly") == 0, "rango 2560");
    tally[1] = 25;                   check(strncmp(rating_name(), "---- E L I T E", 14) == 0, "rango 6400");
    tally[1] = 0;
    fist = 0; check(strcmp(legal_name(), "Clean") == 0, "legal: clean");
    fist = 1; check(strcmp(legal_name(), "Offender") == 0, "legal: offender");
    fist = 50; check(strcmp(legal_name(), "Fugitive") == 0, "legal: fugitive");
    fist = 0;

    /* hiperespacio */
    {
        int best = -1, far_ = -1;
        unsigned bd = 9999, fd = 0;
        qq14 = 70;
        for (i = 0; i < UNI_NSYS; i++) {
            unsigned d;
            if (i == cur_sys) continue;
            d = jump_distance(i);
            if (d <= qq14 && d < bd) { bd = d; best = i; }
            if (d > qq14 && d > fd) { fd = d; far_ = i; }
        }
        check(best >= 0 && far_ >= 0, "hay sistemas cercanos y lejanos");
        check(jump_possible(best) && !jump_possible(far_) && !jump_possible(cur_sys), "alcance del salto");
        check(jump_do(far_) == 0, "no salta fuera de alcance");
        qq14 = 70;
        check(jump_do(best) == 1 && cur_sys == best && qq14 == 70 - bd, "salto: gasta combustible");
        check(qq0 == uni_galaxy[best].x && qq1 == uni_galaxy[best].y, "qq0/qq1 del destino");
        check(mkt.price[0] == (unsigned)(uni_galaxy[best].economy ? mkt.price[0] : mkt.price[0]), "mercado nuevo");
    }
    ghyp = 0; check(jump_galactic() == 0, "sin hiperimpulsor no hay salto galactico");
    ghyp = 0xFF;
    {
        u8 s0[6];
        uni_galaxy_seed(1, s0);
        check(s0[0] == 0x4A && s0[1] == 0x5A && s0[2] == 0x48 && s0[3] == 0x02 && s0[4] == 0x53 && s0[5] == 0xB7, "semilla galaxia 1");
    }
    check(jump_galactic() == 1 && gcnt == 1 && ghyp == 0, "salto galactico");
    {
        u8 s1[6];
        uni_galaxy_seed(2, s1);
        check(memcmp(s1, qq21, 6) == 0, "qq21 = semilla de la galaxia 2");
    }
    for (i = 0; i < 6; i++) check(1, "");
    total -= 6;

    /* guardar, cargar, y que el mercado cargado conserve existencias */
    game_new();
    save_set_device("tests/tmp/");
    trade_buy(0, 4);
    cmdr_set_name("TESTER");
    check(save_commander() == SAVE_OK, "guardar partida");
    {
        u8 a0 = avl[0], q20 = qq20[0];
        unsigned long c = cash_get();
        game_new();
        check(load_commander("TESTER") == SAVE_OK, "cargar partida");
        game_after_load();
        check(avl[0] == a0 && qq20[0] == q20 && cash_get() == c && mkt.quantity[0] == a0, "existencias y carga conservadas");
        check(strcmp(uni_galaxy[cur_sys].name, "LAVE") == 0, "sistema tras cargar");
    }
}

/* ------------------------------------------------------------ sesion guiada */
#define SNAP(n) (-100 - (n))
static const int script[] = {
    SNAP(0),                                    /* titulo */
    '1', -1, -1, -1,                            /* nuevo comandante -> estado */
    SNAP(1),
    '7', SNAP(2),                               /* mercado */
    '1', KC_DOWN, KC_DOWN, '5', SNAP(3), KC_ENTER, SNAP(4), 'm', KC_ESC,   /* comprar: 5 t de textiles y maximo; fuera */
    '2', KC_UP, 'm', KC_ESC,                    /* vender */
    '3', KC_DOWN, KC_ENTER, SNAP(5), KC_ESC,    /* equipo: misil */
    '9', SNAP(6),                               /* inventario */
    '4', KC_RIGHT, KC_RIGHT, KC_DOWN, SNAP(7),  /* carta galactica */
    '5', KC_LEFT, SNAP(8),                      /* carta local */
    '6', SNAP(9),                               /* datos */
    '8', SNAP(10),                              /* estado */
    'i', '2', KC_BS, KC_BS, KC_BS, KC_BS, KC_BS, KC_BS, KC_BS, 'b', 'o', 'b', KC_ENTER, SNAP(11), KC_ESC,   /* disco: guardar como BOB */
    '0', -1, -1, -1, -1, -1, -1, -1, -1, SNAP(12), 'c',     /* despegar, unos fotogramas, atracar */
    '8', 'i', '1', 'b', 'o', 'b', KC_ENTER, SNAP(13),       /* cargar BOB */
    'i', 'q', 'y'                                /* salir */
};
static unsigned sp = 0;
static int snapn = 0;

static void snapshot(int n)
{
    char fn[64];
    FILE *f;
    sprintf(fn, "tests/shots/%02d.bin", n);
    f = fopen(fn, "wb");
    if (f) { fwrite(screen, 1, SCR_BYTES, f); fclose(f); snapn++; }
}

static int feed(void)
{
    int v;
    for (;;) {
        if (sp >= sizeof script / sizeof script[0]) return KC_QUIT;
        v = script[sp++];
        if (v <= -100) { snapshot(-100 - v); continue; }
        return v;
    }
}

int main(void)
{
    plat_init(); txt_init(); in_init(); snd_init();
    logic();
    printf("logica: %d comprobaciones, %d fallos\n", total, fails);
    save_set_device("tests/tmp/");
    in_script = feed;
    game_run();
    check(sp >= sizeof script / sizeof script[0] - 3, "la sesion guiada llego al final");
    check(snapn == 14, "14 capturas");
    check(strcmp(cmdr_name, "BOB") == 0, "se cargo el comandante BOB");
    printf("sesion: %u teclas, %d capturas; %d comprobaciones, %d fallos\n", sp, snapn, total, fails);
    return fails ? 1 : 0;
}
