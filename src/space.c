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

u8 jstx = 128, jsty = 128;
u8 alpha, beta, alp1, bet1;
u8 alp2[2], bet2[2];
u8 mcnt, flh = 0, damp = 0, djd = 0;
u8 space_view = 0;                     /* VIEW: vista actual (0 delante, 1 atras, 2 izquierda, 3 derecha) */
u8 energy = 255, fsh = 255, ash = 255;
u8 altit = 255, cabtmp = 30, gntmp = 0;
static u8 auto_pilot = 0;              /* auto: piloto automatico del atraque (aun no existe) */

int space_signed(u8 v) { return (v & 0x80) ? -(int)(v & 0x7F) : (int)v; }

/* RES2: lo que toca a los giros y la velocidad */
void space_reset(void)
{
    jstx = 128;
    jsty = 128;
    alp2[0] = 128; bet2[0] = 128;
    beta = 0; bet1 = 0;
    alp2[1] = 0; bet2[1] = 0;
    mcnt = 0;
    zDELTA = 3; alpha = 3; alp1 = 3;
}

/* BUMP2: X += A (satura en 255); si queda en la mitad baja y DJD = 0, se detiene en el centro (128) */
u8 space_bump2(u8 x, u8 a)
{
    unsigned s = (unsigned)x + a;
    x = (s > 255) ? 255 : (u8)s;
    if (x & 0x80) return x;                      /* BPL djd1 no se toma */
    if (djd) return x;                           /* LDA DJD; BNE RE2+2 */
    return 128;                                  /* LDX #128 */
}

/* REDU2: X -= A (minimo 1); si sigue en la mitad alta y DJD = 0, se detiene en el centro */
u8 space_redu2(u8 x, u8 a)
{
    if (x >= a) x = (u8)(x - a); else x = 1;     /* SEC; SBC T; BCS RE3; LDX #1 */
    if (!(x & 0x80)) return x;                   /* BPL RE2+2 */
    if (djd) return x;
    return 128;
}

/* cntr: un paso hacia el centro (128), salvo con la amortiguacion desactivada y sin piloto automatico */
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

    /* DOKEY: BUMP2/REDU2 con A = 20 */
    x = jstx;
    if (c->roll_left)  x = space_bump2(x, 20);   /* KY3: , */
    if (c->roll_right) x = space_redu2(x, 20);   /* KY4: . */
    jstx = x;
    x = jsty;
    if (c->pitch_up)   x = space_redu2(x, 20);   /* KY5: X (subir) */
    if (c->pitch_down) x = space_bump2(x, 20);   /* KY6: S (picar) */
    jsty = x;

    /* velocidad: espacio acelera hasta 40, / frena hasta 1 */
    if (c->speed_up && zDELTA < 40) zDELTA++;
    if (c->slow_down) { zDELTA--; if (zDELTA == 0) zDELTA = 1; }

    /* bucle principal: alabeo */
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

    /* cabeceo (el ADC #4 usa el acarreo que dejo el alabeo) */
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

/* scacol: color de cada tipo de nave en el radar (B azul, R rojo, C cian, M magenta, F "fuzzy", 0 no se dibuja) */
static const char scacol[] = "0BBRRRRRRCCCMMMRCCCCCCCBCCMCCFCC0C";

/* el COL del original para el tipo (BLUE 16, RED 20, FUZZY 24; CYAN y MAG son WHITE, 12, en el Apple II): para las pruebas */
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

/* SCAN: dibuja (XOR) la nave de INWK en el radar si su bit 4 de INWK+31 esta puesto: un punto de 4 de ancho y un palo hasta el
   plano del radar. Planetas y soles (tipo con bit 7) no salen; tampoco lo que esta a 16384 unidades o mas en algun eje. */
void space_scan(u8 type)
{
    u8 a, x1, y2, col;
    unsigned s;
    if (!(inwk[31] & 0x10)) return;
    if (type & 0x80) return;
    if (type >= sizeof scacol - 1) return;
    col = scan_ql(scacol[type]);
    if (col == C_BLACK) return;
    if ((inwk[1] | inwk[4] | inwk[7]) & 0xC0) return;
    a = inwk[1];
    if (inwk[2] & 0x80) a = (u8)((a ^ 0xFF) + 1);            /* x negativo: EOR #$FF; ADC #1; CLC */
    s = (unsigned)a + 125;                                    /* SC2: ADC #125 */
    x1 = (u8)((u8)s & 0xFE);
    a = (u8)(inwk[7] >> 2);                                   /* z: LSR A; LSR A; CLC */
    s = (unsigned)a;
    if (inwk[8] & 0x80) s = (unsigned)(u8)(a ^ 0xFF) + 1;     /* z negativo (detras): EOR #$FF; SEC */
    s = s + 91;                                               /* SC3: ADC #91 */
    y2 = (u8)((u8)s ^ 0xFF);
    a = (u8)(inwk[4] >> 1);                                   /* y: LSR A; CLC */
    s = (unsigned)a;
    if (!(inwk[5] & 0x80)) s = (unsigned)(u8)(a ^ 0xFF) + 1;  /* y positivo (encima): EOR #$FF; SEC */
    s = s + y2;                                               /* SCD6: ADC Y2 */
    a = (u8)s;
    if (a < 146) a = 146;                                     /* CMP #146; BCS ...; LDA #146 */
    if (a >= 191) a = 190;                                    /* CMP #191; BCC ...; LDA #190 */
    gfx_cpix((int)x1 - 2, a, col);                            /* cabeza (CPIX con X = X1 - 2) */
    gfx_vloin(x1, a, y2, col);                                /* palo (VLOIN) */
}

static void load_slot(int n) { u8 *p = state_slot(n); int i; for (i = 0; i < NI; i++) inwk[i] = p[i]; }
static void store_slot(int n) { u8 *p = state_slot(n); int i; for (i = 0; i < NI; i++) p[i] = inwk[i]; }

/* Tras repintar el panel el radar esta vacio: ninguna nave figura como dibujada */
void space_scanner_reset(void)
{
    int n;
    for (n = 0; n <= NOSH; n++) state_slot(n)[I_STATE] = (u8)(state_slot(n)[I_STATE] & 0xEF);
}

/* Planeta (ranura 0) y estacion (ranura 1) respecto a la nave al despegar (ships.c: SOLAR y NWSPS); reinicia los giros y la velocidad */
void space_launch(void)
{
    space_reset();                               /* RES2 */
    ships_launch();                              /* SOS1 y NWSPS */
    game_tt111();                                /* TT110: TT111 al despegar */
    zDELTA = 12;                                 /* velocidad de lanzamiento */
    fist = (u8)(fist | space_bad());             /* JSR BAD; ORA FIST */
}

/* Llegada a un sistema tras el hiperespacio: planeta lejos delante y el sol, sin estacion cerca */
void space_arrive(void)
{
    space_reset();                               /* RES2 */
    ships_arrive();                              /* SOLAR */
}

/* BAD: (esclavos + narcoticos) * 2 + armas (8 bits) */
u8 space_bad(void) { return (u8)((u8)((u8)(qq20[3] + qq20[6]) << 1) + qq20[10] + (u8)((u8)(qq20[3] + qq20[6]) >> 7)); }

/* Borra del radar todo lo que figura como dibujado (antes de cambiar de sistema) */
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

/* Un contacto de prueba (ships.c): una nave de verdad, con su plano, orientacion, velocidad y giros */
int space_spawn(u8 type) { return ships_spawn(type); }

/* Un fotograma del espacio local: cada nave se mueve (MVEIT), se dibuja (LL9) y se quita si hace falta (ships.c). Devuelve los SPACE_EV_* */
u8 space_step(void)
{
    u8 ev = 0, e;
    e = ships_step(space_view);
    if (e & SHIPS_EV_STATION) ev |= SPACE_EV_STATION;
    if (e & SHIPS_EV_DOCK) return (u8)(ev | SPACE_EV_DOCK);      /* el original salta a DOENTRY sin acabar el fotograma */
    if (e & SHIPS_EV_DEATH) return (u8)(ev | SPACE_EV_DEATH);
    energy_tick(mcnt);                           /* MA77: escudos y energia cada 8 fotogramas */
    if (ships_part14(mcnt)) ev |= SPACE_EV_STATION;   /* MA22: la estacion aparece (SPBLB) */
    if (damage_checks(mcnt)) ev |= SPACE_EV_DEATH;   /* MA22/MA93: ENERGY LOW, altitud, cabina y combustible */
    return ev;
}
