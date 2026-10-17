#include "space.h"
#include "state.h"
#include "emath.h"
#include "ship_math.h"
#include "universe.h"
#include "game.h"

u8 jstx = 128, jsty = 128;
u8 alpha, beta, alp1, bet1;
u8 alp2[2], bet2[2];
u8 mcnt, flh = 0, damp = 0, djd = 0;
u8 energy = 255, fsh = 255, ash = 255;
u8 altit = 255, cabtmp = 30, gntmp = 0;
static u8 auto_pilot = 0;              /* auto: piloto automatico del atraque (aun no existe) */
static u8 k2[4];                       /* K2..K2+3 */

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

/* MVT1: (x_sg x_hi x_lo) = (x_sg x_hi x_lo) +/- (S R); R = zR; A = signo de la cantidad (bit 7); X = 0, 3 o 6 */
static void mvt1(u8 a, u8 x)
{
    u8 s, t;
    unsigned u;
    int d, cc;
    s = (u8)(a & 0x7F);                          /* ASL A; STA S ... LSR S */
    t = (u8)(a & 0x80);                          /* LDA #0; ROR A */
    if (!((t ^ inwk[x + 2]) & 0x80)) {           /* misma senal: suma */
        u = (unsigned)zR + inwk[x]; inwk[x] = (u8)u;
        u = (unsigned)s + inwk[x + 1] + (u >> 8); inwk[x + 1] = (u8)u;
        u = (unsigned)inwk[x + 2] + (u >> 8);
        inwk[x + 2] = (u8)((u8)u | t);
        return;
    }
    d = (int)inwk[x] - zR; cc = (d >= 0); inwk[x] = (u8)d;                    /* MV10: resta */
    d = (int)inwk[x + 1] - s - (1 - cc); cc = (d >= 0); inwk[x + 1] = (u8)d;
    d = (int)(inwk[x + 2] & 0x7F) - (1 - cc); cc = (d >= 0);
    inwk[x + 2] = (u8)((((u8)d) | 0x80) ^ t);
    if (cc) return;                                                           /* BCS MV11 */
    d = 1 - (int)inwk[x] - 1; cc = (d >= 0); inwk[x] = (u8)d;                 /* cruzo el cero: negar */
    d = 0 - (int)inwk[x + 1] - (1 - cc); cc = (d >= 0); inwk[x + 1] = (u8)d;
    d = 0 - (int)inwk[x + 2] - (1 - cc);
    inwk[x + 2] = (u8)(((u8)d & 0x7F) | t);
}

/* MV40 (+ MV45): gira la posicion del planeta o la estacion segun nuestro alabeo y cabeceo y la traslada por la velocidad */
static void mv40(void)
{
    u8 a, t;
    unsigned s;
    int d, cc;
    zQ = (u8)(alpha ^ 0x80);
    zP = inwk[0]; zP1 = inwk[1];
    em_mult3(inwk[2]);
    sm_mvt3(3);
    k2[1] = zK[1]; zP = zK[1]; k2[2] = zK[2]; zP1 = zK[2];
    zQ = beta;
    k2[3] = zK[3];
    em_mult3(zK[3]);
    sm_mvt3(6);
    zP = zK[1]; inwk[6] = zK[1]; zP1 = zK[2]; inwk[7] = zK[2]; inwk[8] = zK[3];
    a = (u8)(zK[3] ^ 0x80);
    em_mult3(a);
    t = (u8)(zK[3] & 0x80);
    a = (u8)(t ^ k2[3]);
    if (a & 0x80) goto mv1;
    s = (unsigned)zK[0] + k2[0];
    s = (unsigned)zK[1] + k2[1] + (s >> 8); inwk[3] = (u8)s;
    s = (unsigned)zK[2] + k2[2] + (s >> 8); inwk[4] = (u8)s;
    s = (unsigned)zK[3] + k2[3] + (s >> 8); a = (u8)s;
    goto mv2;
mv1:
    d = (int)zK[0] - k2[0]; cc = (d >= 0);
    d = (int)zK[1] - k2[1] - (1 - cc); cc = (d >= 0); inwk[3] = (u8)d;
    d = (int)zK[2] - k2[2] - (1 - cc); cc = (d >= 0); inwk[4] = (u8)d;
    zP = (u8)(k2[3] & 0x7F);
    d = (int)(zK[3] & 0x7F) - zP - (1 - cc); cc = (d >= 0); zP = (u8)d; a = zP;
    if (cc) goto mv2;
    d = 1 - (int)inwk[3] - 1; cc = (d >= 0); inwk[3] = (u8)d;
    d = 0 - (int)inwk[4] - (1 - cc); cc = (d >= 0); inwk[4] = (u8)d;
    d = 0 - (int)zP - (1 - cc);
    a = (u8)((u8)d | 0x80);
mv2:
    a = (u8)(a ^ t);
    inwk[5] = a;
    zQ = alpha;
    zP = inwk[3]; zP1 = inwk[4];
    em_mult3(inwk[5]);
    sm_mvt3(0);
    inwk[0] = zK[1]; inwk[1] = zK[2]; inwk[2] = zK[3];
    zR = zDELTA;                                 /* MV45: z -= DELTA */
    mvt1(0x80, 6);
}

static void move_slot(int n)
{
    u8 *p = state_slot(n);
    int i;
    for (i = 0; i < NI; i++) inwk[i] = p[i];
    mv40();
    for (i = 0; i < NI; i++) p[i] = inwk[i];
}

/* Planeta (ranura 0) y estacion (ranura 1) respecto a la nave. El planeta sigue las formulas de SOLAR: z, x e y con byte alto
   (w0hi AND 3) + 3 para z y la mitad para x e y. La estacion arranca detras de la nave (acabamos de salir de ella). */
void space_launch(void)
{
    u8 *p = state_slot(0), *st = state_slot(1);
    u8 ztop;
    int i;
    for (i = 0; i < NI; i++) { p[i] = 0; st[i] = 0; }
    ztop = (u8)((((uni_galaxy[cur_sys].seed0 >> 8) & 3)) + 3);
    p[8] = ztop; p[2] = (u8)(ztop >> 1); p[5] = (u8)(ztop >> 1);
    st[4] = 1; st[5] = 0x80;                     /* y = -1 * 256: algo por debajo */
    st[7] = 6; st[8] = 0x80;                     /* z = -6 * 256: detras */
    sspr = 1;
    space_reset();
}

void space_step(void)
{
    u8 a;
    int d;
    mcnt++;
    move_slot(0);
    if (sspr) {
        move_slot(1);
        if ((state_slot(1)[7] > 0x30) || (state_slot(1)[8] & 0x7F)) sspr = 0;     /* lejos de la estacion: la brujula apunta al planeta */
    }
    if ((mcnt & 31) == 10) {                     /* MA22: altitud cada 32 fotogramas */
        altit = 255;
        if (sm_mas2m(0) != 0) return;            /* algun byte alto distinto de cero: lejos */
        a = sm_mas3(0);
        if (a == 0xFF) return;                   /* desbordo */
        d = (int)a - 37;                         /* SBC #36 con C = 0 */
        if (d < 0) { altit = 0; return; }        /* el original muere aqui (DEATH) */
        zR = (u8)d; zQ = 0;
        altit = em_ll5();
    }
}
