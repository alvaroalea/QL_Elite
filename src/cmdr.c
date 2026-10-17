#include "cmdr.h"
#include "state.h"

char cmdr_name[CMDR_NAMELEN + 1] = "JAMESON";

/* CHECK: A = NT-3; para x = NT-3 .. 1:  A = A + b[x-1] + C;  A ^= b[x]  (C es el acarreo del ADC anterior, empieza en 0) */
u8 cmdr_check(const u8 *b)
{
    unsigned a = CMDR_NT - 3, c = 0, t;
    int x;
    for (x = CMDR_NT - 3; x > 0; x--) {
        t = a + b[x - 1] + c;
        c = (t >> 8) & 1;
        a = (t & 0xFF) ^ b[x];
    }
    return (u8)a;
}

/* CHECK2: igual, pero antes de sumar hace A ^= x y un ROR A (el bit 0 sale al acarreo y el acarreo entra por el bit 7) */
u8 cmdr_check2(const u8 *b)
{
    unsigned a = CMDR_NT - 3, c = 0, t, nc;
    int x;
    for (x = CMDR_NT - 3; x > 0; x--) {
        a ^= (unsigned)x;
        nc = a & 1;
        a = (a >> 1) | (c << 7);
        c = nc;
        t = a + b[x - 1] + c;
        c = (t >> 8) & 1;
        a = (t & 0xFF) ^ b[x];
    }
    return (u8)a;
}

static void cp(u8 *d, const u8 *s, int n) { while (n-- > 0) *d++ = *s++; }

void cmdr_pack(u8 *b)
{
    int i;
    for (i = 0; i < CMDR_SIZE; i++) b[i] = 0;
    b[CB_TP] = tp; b[CB_QQ0] = qq0; b[CB_QQ1] = qq1;
    cp(b + CB_SEED, qq21, 6);
    cp(b + CB_CASH, cash, 4);
    b[CB_FUEL] = qq14; b[CB_COK] = cok; b[CB_GCNT] = gcnt;
    cp(b + CB_LASER, laser, 4);
    b[CB_CRGO] = crgo;
    cp(b + CB_HOLD, qq20, 17);
    b[CB_ECM] = ecm_fitted; b[CB_BST] = bst; b[CB_BOMB] = bomb; b[CB_ENGY] = engy;
    b[CB_DKCMP] = dkcmp; b[CB_GHYP] = ghyp; b[CB_ESCP] = escp;
    b[CB_NOMSL] = nomsl; b[CB_FIST] = fist;
    cp(b + CB_AVL, avl, 17);
    b[CB_QQ26] = qq26;
    b[CB_TALLY] = tally[0]; b[CB_TALLY + 1] = tally[1];
    b[CB_SVC] = svc;
}

void cmdr_seal(u8 *b)
{
    b[CB_CHK3] = cmdr_check2(b);
    b[CB_CHK] = cmdr_check(b);
    b[CB_CHK2] = (u8)(b[CB_CHK] ^ 0xA9);
}

int cmdr_unpack(const u8 *b)
{
    u8 c = cmdr_check(b);
    u8 k;
    if (c != b[CB_CHK]) return -1;
    if (cmdr_check2(b) != b[CB_CHK3]) return -1;
    k = b[CB_COK];
    if ((u8)(c ^ 0xA9) != b[CB_CHK2]) k |= 0x80;     /* tZ de DFAULT: marca de datos alterados */
    k |= 0x08;
    tp = b[CB_TP]; qq0 = b[CB_QQ0]; qq1 = b[CB_QQ1];
    cp(qq21, b + CB_SEED, 6);
    cp(cash, b + CB_CASH, 4);
    qq14 = b[CB_FUEL]; cok = k; gcnt = b[CB_GCNT];
    cp(laser, b + CB_LASER, 4);
    crgo = b[CB_CRGO];
    cp(qq20, b + CB_HOLD, 17);
    ecm_fitted = b[CB_ECM]; bst = b[CB_BST]; bomb = b[CB_BOMB]; engy = b[CB_ENGY];
    dkcmp = b[CB_DKCMP]; ghyp = b[CB_GHYP]; escp = b[CB_ESCP];
    nomsl = b[CB_NOMSL]; fist = b[CB_FIST];
    cp(avl, b + CB_AVL, 17);
    qq26 = b[CB_QQ26];
    tally[0] = b[CB_TALLY]; tally[1] = b[CB_TALLY + 1];
    svc = b[CB_SVC];
    return 0;
}

/* Bloque de fabrica de JAMESON (Q% = FALSE en la fuente): 100 Cr, 70 de combustible, laser de pulsos frontal, bodega de 22 */
static const u8 default_blk[CMDR_SIZE] = {
    0,                                  /* TP */
    20, 173,                            /* Lave */
    0x4A, 0x5A, 0x48, 0x02, 0x53, 0xB7, /* semilla de la galaxia 1 */
    0x00, 0x00, 0x03, 0xE8,             /* 1000 decimas = 100 Cr */
    70, 0, 0,                           /* combustible, COK, galaxia */
    LASER_POW, 0, 0, 0,
    0, 0,
    22,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,                /* ECM, BST, BOMB, ENGY, DKCMP, GHYP, ESCP */
    0, 0, 0, 0,
    3, 0,                               /* misiles, FIST */
    16, 15, 17, 0, 3, 28, 14, 0, 0, 10, 0, 17, 58, 7, 9, 8, 0,
    0,                                  /* QQ26 */
    0, 0,                               /* TALLY */
    128,                                /* SVC */
    0xAA, 0x27, 0x03                    /* CHK2, CHK3, CHK de fabrica */
};

void cmdr_default(void)
{
    cmdr_unpack(default_blk);           /* los tres checksums del original estan en el bloque: la prueba los verifica */
    cmdr_name[0] = 0;
    cmdr_set_name("JAMESON");
}

int cmdr_set_name(const char *s)
{
    int n = 0;
    char ch;
    while (*s && n < CMDR_NAMELEN) {
        ch = *s++;
        if (ch >= 'a' && ch <= 'z') ch = (char)(ch - 'a' + 'A');
        if ((ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_' || ch == '-') cmdr_name[n++] = ch;
    }
    cmdr_name[n] = 0;
    return n;
}
