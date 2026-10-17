#include "keys.h"

static u8 cur[8], prev[8];
static u8 rowmask = 0xFE;           /* por defecto filas 1..7 (todas las de vuelo); la 0 solo con joystick 2 */
static int queue[8];
static int qn;
int ctl_joystick = 0;

/* ASCII UK por posicion (fila*8+bit). 0 = sin caracter. Provisional: comprobar en el QL real. */
static const char plain[64] = {
    /* f0 */ 0,0,'5',0,0,0,'4','7',
    /* f1 */ 13,0,0,27,0,'\\',' ',0,
    /* f2 */ ']','z','.','c','b','`','m','\'',
    /* f3 */ '[',0,'k','s','f','=','g',';',
    /* f4 */ 'l','3','h','1','a','p','d','j',
    /* f5 */ '9','w','i',9,'r','-','y','o',
    /* f6 */ '8','2','6','q','e','0','t','u',
    /* f7 */ 0,0,0,'x','v','/','n',','
};
static const char shifted[64] = {
    /* f0 */ 0,0,'%',0,0,0,'$','&',
    /* f1 */ 13,0,0,27,0,'|',' ',0,
    /* f2 */ '}','Z','>','C','B','`','M','"',
    /* f3 */ '{',0,'K','S','F','+','G',':',
    /* f4 */ 'L','#','H','!','A','P','D','J',
    /* f5 */ '(','W','I',9,'R','_','Y','O',
    /* f6 */ '*','@','^','Q','E',')','T','U',
    /* f7 */ 0,0,0,'X','V','?','N','<'
};

void key_init(void)
{
    int i;
    for (i = 0; i < 8; i++) cur[i] = prev[i] = 0;
    qn = 0;
    rowmask = 0xFE;
    ctl_joystick = 0;
}

void key_set_rows(u8 mask) { rowmask = mask; }

static void enqueue(int ch) { if (qn < 8) queue[qn++] = ch; }

void key_scan(void)
{
    int r, b;
    u8 edge;
    for (r = 0; r < 8; r++) {
        prev[r] = cur[r];
        if (rowmask & (1 << r)) cur[r] = (u8)ql_read_row(r);
        /* filas no leidas conservan el ultimo valor */
    }
    /* caracteres: nuevas pulsaciones (con la matriz completa en rowmask) */
    for (r = 0; r < 8; r++) {
        edge = (u8)(cur[r] & ~prev[r]);
        if (!edge) continue;
        for (b = 0; b < 8; b++) {
            char ch;
            if (!(edge & (1 << b))) continue;
            if (r == 7 && b <= 2) continue;                      /* modificadores */
            if (r == 1 && b == 1 && (cur[7] & 2)) { enqueue(8); continue; }   /* CTRL+Izq = borrar */
            ch = (cur[7] & 1) ? shifted[r * 8 + b] : plain[r * 8 + b];
            if (ch) enqueue((u8)ch);
        }
    }
}

int key_down(u8 k) { return (cur[KEY_ROW(k)] >> KEY_BIT(k)) & 1; }
int key_pressed(u8 k) { return ((cur[KEY_ROW(k)] & ~prev[KEY_ROW(k)]) >> KEY_BIT(k)) & 1; }
int key_released(u8 k) { return ((prev[KEY_ROW(k)] & ~cur[KEY_ROW(k)]) >> KEY_BIT(k)) & 1; }

int key_raw(int row) { return cur[row & 7]; }

int key_any(void)
{
    int r;
    for (r = 0; r < 8; r++) {
        u8 v = cur[r];
        if (r == 7) v &= 0xF8;                                   /* ignora SHIFT, CTRL, ALT */
        if (v) return 1;
    }
    return 0;
}

int key_getchar(void)
{
    int ch, i;
    if (qn == 0) return -1;
    ch = queue[0];
    for (i = 1; i < qn; i++) queue[i - 1] = queue[i];
    qn--;
    return ch;
}

int joy_read(int port)
{
    int j = 0;
    if (port == 1) {
        if (key_down(K_UP)) j |= JOY_UP;
        if (key_down(K_DOWN)) j |= JOY_DOWN;
        if (key_down(K_LEFT)) j |= JOY_LEFT;
        if (key_down(K_RIGHT)) j |= JOY_RIGHT;
        if (key_down(K_SPACE)) j |= JOY_FIRE;
    } else {
        if (key_down(K_F4)) j |= JOY_UP;
        if (key_down(K_F2)) j |= JOY_DOWN;
        if (key_down(K_F1)) j |= JOY_LEFT;
        if (key_down(K_F3)) j |= JOY_RIGHT;
        if (key_down(K_F5)) j |= JOY_FIRE;
    }
    return j;
}

void ctl_read(struct controls *c)
{
    int j = 0;
    c->slow_down  = (u8)key_down(K_SLASH);
    c->speed_up   = (u8)key_down(K_SPACE);
    c->roll_left  = (u8)key_down(K_COMMA);
    c->roll_right = (u8)key_down(K_PERIOD);
    c->pitch_up   = (u8)key_down(K_X);
    c->pitch_down = (u8)key_down(K_S);
    c->fire       = (u8)key_down(K_A);
    c->bomb       = (u8)key_down(K_B);
    c->escape_pod = (u8)key_down(K_ESC);
    c->target     = (u8)key_down(K_T);
    c->unarm      = (u8)key_down(K_U);
    c->missile    = (u8)key_down(K_M);
    c->ecm        = (u8)key_down(K_E);
    c->jump       = (u8)key_down(K_J);
    c->dock       = (u8)key_down(K_C);
    c->undock     = (u8)key_down(K_P);
    if (ctl_joystick) {
        j = joy_read(ctl_joystick);
        /* CTL1 usa el espacio como disparo: para no acelerar al disparar, el "acelerar" pasa a ENTER */
        if (ctl_joystick == 1) c->speed_up = (u8)key_down(K_ENTER);
        if (j & JOY_LEFT)  c->roll_left = 1;
        if (j & JOY_RIGHT) c->roll_right = 1;
        if (j & JOY_UP)    c->pitch_down = 1;       /* palanca hacia delante = picar */
        if (j & JOY_DOWN)  c->pitch_up = 1;
        if (j & JOY_FIRE)  c->fire = 1;
    }
}
