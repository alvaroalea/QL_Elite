#include "equip.h"
#include "state.h"
#include "emath.h"
#include "ship_math.h"
#include "combat.h"
#include "sound.h"

u8 bombtbx[10], bombtby[10];
u8 bomb_col = C_WHITE;
void (*bomb_line_hook)(u8 x1, u8 y1, u8 x2, u8 y2) = 0;

/* BOMBPOS: x de cada vertice del rayo (de derecha a izquierda) */
static const u8 bombpos[10] = { 224, 224, 192, 160, 128, 96, 64, 32, 0, 0 };

/* BOMBOFF: dibuja (XOR) el rayo en zigzag de la bomba con los 10 vertices de BOMBTBX/BOMBTBY; solo en la vista espacial */
void bomb_off(void)
{
    u8 y, x1, y1;
    if (!combat_view_on) return;                 /* LDA QQ11; BNE BOMBR1 */
    x1 = bombtbx[0]; y1 = bombtby[0];            /* XX12 */
    for (y = 1; y < 10; y++) {                   /* CLICK: el chasquido del altavoz no se porta */
        if (bomb_line_hook) bomb_line_hook(x1, y1, bombtbx[y], bombtby[y]);
        gfx_line(x1, y1, bombtbx[y], bombtby[y], bomb_col, 1);
        x1 = bombtbx[y]; y1 = bombtby[y];
    }
}

/* BOMBON: vertices nuevos al azar (y entre 3 y 131, x = BOMBPOS + 0..31; el primero en x = 255 y el ultimo en x = 0) y los
   dibuja (cae en BOMBOFF). c = acarreo con que entra el primer DORND; los demas entran con C = 0 (lo deja el CPY #10) */
void bomb_on(u8 c)
{
    u8 y, a;
    for (y = 0; y < 10; y++) {
        em_rand_c = (u8)(y == 0 ? (c & 1) : 0);
        a = em_dornd();
        bombtby[y] = (u8)((a & 127) + 3 + (em_rand_c & 1));     /* AND #127; ADC #3 con el acarreo de DORND */
        bombtbx[y] = (u8)((em_rand_x & 31) + bombpos[y]);       /* TXA; AND #31; CLC; ADC BOMBPOS,Y */
    }
    bombtbx[9] = 0;
    bombtbx[0] = 0xFF;
    em_rand_c = 1;                               /* el ultimo CPY #10 deja C = 1 (BCS BOMBOFF) */
    bomb_off();
}

/* BOMBEFF: sonido, borra el rayo y dibuja otro */
void bomb_eff(void)
{
    snd_sobomb();
    bomb_off();
    bomb_on(1);                                  /* BOMBOFF termina con C = 1 (CPY #10) */
}

/* BOMBEFF2: JSR P%+3 / JSR BOMBEFF / BOMBEFF: cuatro rayos seguidos */
void bomb_eff2(void) { bomb_eff(); bomb_eff(); bomb_eff(); bomb_eff(); }

/* Parte 3 (MA24..MA76): con la tecla de la bomba, ASL BOMB y, si queda algo, BOMBON. Version publicada (IB disk): si la
   bomba ya esta explotando (BOMB negativo) la tecla no hace nada (la version de la fuente la volvia a lanzar sin borrar) */
void bomb_key(void)
{
    u8 c;
    if (bomb & 0x80) return;
    c = (u8)(bomb >> 7);
    bomb = (u8)(bomb << 1);
    if (bomb == 0) return;
    bomb_on(c);
}

/* Parte 16 (MA18..MA77): mientras BOMB es negativo, cuatro rayos por fotograma; al volverse positivo se borra el ultimo */
void bomb_frame(void)
{
    if (!(bomb & 0x80)) return;
    bomb_eff2();
    bomb = (u8)(bomb << 1);
    if (bomb & 0x80) return;
    bomb_off();
}

/* WARP: salto dentro del sistema (tecla J). Solo sin otras naves que no sean basura (FRIN+2+JUNK), sin estacion y fuera
   del hiperespacio, y si el planeta y el sol que esten delante (z positiva) estan lejos (MAS2 >= 2): acerca los dos 65.536
   unidades en z (ADD de $81 $81 al byte alto). Devuelve 1 si salta (el llamador repinta la vista: LOOK1 con QQ11 = 1) */
int warp(void)
{
    u8 a;
    a = (u8)(frin[(u8)(2 + junk_count)] | sspr | mj);
    if (a) { snd_boop(); return 0; }
    if (!(ks[8] & 0x80)) {
        if (sm_mas2(0, 0) < 2) { snd_boop(); return 0; }
    }
    if (!(ks[NI + 8] & 0x80)) {
        if (sm_mas2(0, NI) < 2) { snd_boop(); return 0; }
    }
    zS = 0x81; zR = 0x81; zP = 0x81;
    ks[8] = em_add(ks[8]);
    ks[NI + 8] = em_add(ks[NI + 8]);
    return 1;
}
