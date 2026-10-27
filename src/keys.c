#include "keys.h"

static u8 cur[8], prev[8];
static u8 rowmask = 0xFE;           /* rows read on EVERY key_scan (by default 1..7) */
static u8 slowmask = 0;             /* rows read one at a time, rotating (one per key_scan) */
static u8 slowpos = 0;
static int queue[8];
static int qn;
int ctl_joystick = 0;
u8 jstgy = 0xFF, jste = 0;

/* UK ASCII by position (row*8+bit). 0 = no character. Provisional: check on a real QL. */
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
    rowmask = 0xFE; slowmask = 0; slowpos = 0;
    ctl_joystick = 0;
}

void key_set_rows(u8 mask) { rowmask = mask; slowmask = 0; }

/* Each row costs one IPC call (slow on a real QL): the "fast" rows are read on every scan and the "slow" ones one at a
   time, rotating, so a key in a slow row is detected in at most (number of slow rows) scans. */
void key_set_scan(u8 fast, u8 slow) { rowmask = fast; slowmask = (u8)(slow & ~fast); slowpos = 0; }

/* Menus: row 1 (cursors, ENTER, ESC, space) and row 7 (SHIFT, CTRL) always; the rest (letters and digits) rotating: 3 reads per scan */
void key_ctx_menu(void) { key_set_scan(0x82, 0x7D); }

/* Flight: row 1 (cursors, space, ENTER, ESC), row 3 (F accelerate, G) and row 4 (A, D brake, H, J, P, 1, 3) always; rows 0, 2, 5, 6 and 7
   rotating (4, 5, 7, C, B, M, 9, E, T, U, 2, 6, 8, SHIFT...): 4 reads per scan. With joystick 2 row 0 (F1..F5) is fast (5). */
void key_ctx_flight(void)
{
    u8 fast = 0x1A, slow = 0xE5;
    if (ctl_joystick == 2) { fast |= 1; slow &= (u8)~1; }
#ifdef DEBUG
    fast |= 0x80; slow &= (u8)~0x80;                         /* SHIFT (row 7) without delay for SHIFT-C/P/H/1/2/3 */
#endif
    key_set_scan(fast, slow);
}

static void enqueue(int ch) { if (qn < 8) queue[qn++] = ch; }

void key_scan(void)
{
    int r, b;
    u8 edge;
    for (r = 0; r < 8; r++) {
        prev[r] = cur[r];
        if (rowmask & (1 << r)) cur[r] = (u8)ql_read_row(r);
        /* rows not read keep their last value */
    }
    if (slowmask) {                                           /* one slow row per scan, rotating */
        for (b = 1; b <= 8; b++) {
            r = (slowpos + b) & 7;
            if (slowmask & (1 << r)) { cur[r] = (u8)ql_read_row(r); slowpos = (u8)r; break; }
        }
    }
    /* characters: new key presses (with the full matrix in rowmask) */
    for (r = 0; r < 8; r++) {
        edge = (u8)(cur[r] & ~prev[r]);
        if (!edge) continue;
        for (b = 0; b < 8; b++) {
            char ch;
            if (!(edge & (1 << b))) continue;
            if (r == 7 && b <= 2) continue;                      /* modifiers */
            if (r == 1 && b == 1 && (cur[7] & 2)) { enqueue(8); continue; }   /* CTRL+Left = delete */
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
        if (r == 7) v &= 0xF8;                                   /* ignores SHIFT, CTRL, ALT */
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
    c->slow_down  = (u8)key_down(K_D);           /* F accelerates and D brakes (formerly space and /) */
    c->speed_up   = (u8)key_down(K_F);
    /* The cursors turn the ship (and are the port 1 joystick): left/right = roll; up = dive (stick pushed
       forward), down = pitch up. They replace , . S X of the original to read fewer keyboard rows. */
    c->roll_left  = (u8)key_down(K_LEFT);
    c->roll_right = (u8)key_down(K_RIGHT);
    c->pitch_down = (u8)key_down(K_UP);
    c->pitch_up   = (u8)key_down(K_DOWN);
    c->fire       = (u8)(key_down(K_A) | key_down(K_SPACE));   /* A or space (the CTL1 joystick button) */
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
        if (jste & 0x80) j ^= JOY_UP | JOY_DOWN | JOY_LEFT | JOY_RIGHT;     /* EOR JSTE: both axes reversed */
        if (!(jstgy & 0x80)) j ^= JOY_UP | JOY_DOWN;                        /* EOR JSTGY: Y axis reversed */
        if ((j & (JOY_UP | JOY_DOWN)) == (JOY_UP | JOY_DOWN)) j &= ~(JOY_UP | JOY_DOWN);
        if ((j & (JOY_LEFT | JOY_RIGHT)) == (JOY_LEFT | JOY_RIGHT)) j &= ~(JOY_LEFT | JOY_RIGHT);
        if (j & JOY_LEFT)  c->roll_left = 1;
        if (j & JOY_RIGHT) c->roll_right = 1;
        if (j & JOY_UP)    c->pitch_down = 1;       /* stick forward = dive */
        if (j & JOY_DOWN)  c->pitch_up = 1;
        if (j & JOY_FIRE)  c->fire = 1;
    }
}
