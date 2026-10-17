#include "detok.h"
#include "state.h"
#include "emath.h"
#include "cmdr.h"
#include "tokens_data.h"

u8 dt_xc = 1, dt_yc = 1;
u8 dtw1 = 0x20, dtw2 = 0, dtw3 = 0, dtw4 = 0, dtw5 = 0, dtw6 = 0, dtw8 = 0xFF;
u8 dt_buf[256];

void (*dt_letter_hook)(u8 ch, u8 xc, u8 yc) = 0;
void (*dt_clss_hook)(void) = 0;
void (*dt_beep_hook)(void) = 0;
void (*dt_tt66_hook)(u8 view) = 0;
void (*dt_pause2_hook)(void) = 0;
void (*dt_delay_hook)(u8 n) = 0;
void (*dt_nlin4_hook)(void) = 0;
void (*dt_clyns_hook)(void) = 0;
void (*dt_name_hook)(u8 which) = 0;

#define DTW7 'A'                                 /* MT16: DTW7 apunta al operando de LDA #'A' (letra de la unidad) */

void dt_reset(void)
{
    dtw1 = 0x20; dtw2 = 0; dtw3 = 0; dtw4 = 0; dtw5 = 0; dtw6 = 0; dtw8 = 0xFF;
}

u8 dt_rupla(u8 i) { return tok_rupla[i - 1]; }
u8 dt_rugal(u8 i) { return tok_rugal[i - 1]; }
u8 dt_nru(void) { return TOK_NRU; }

/* ---------------------------------------------------------------- CHPR */

/* RRX2 / RRX1: 12 y los demas codigos < 32 (salvo 7 y 10) ponen XC = 1; todos salvo 13 bajan una fila */
static void rrx2(u8 a) { dt_xc = 1; if (a != 13) dt_yc++; }

void dt_chpr(u8 a)
{
    zk3[0] = a;                                  /* STA K3 (CHPR guarda A en K3) */
    if (qq17 == 255) return;                     /* impresion desactivada */
    for (;;) {                                   /* RRafter */
        if (a == 7) { if (dt_beep_hook) dt_beep_hook(); return; }
        if (a < 32) {
            if (a == 10) { dt_yc++; return; }    /* RRX1 con A = 10 */
            rrx2(a);
            return;
        }
        if (dt_xc >= 31) rrx2(12);               /* RR1: fuera de la linea: salto de linea (JSR RRX2 con A >= 32) */
        if (dt_yc >= 24) {                       /* clss: borra y vuelve a RRafter con el mismo caracter */
            if (dt_clss_hook) dt_clss_hook();
            dt_xc = 0; dt_yc = 0;                /* cleargrap; en modo texto el original no mueve el cursor (se colgaria) */
            continue;
        }
        if (dt_letter_hook) dt_letter_hook(a, dt_xc, dt_yc);
        dt_xc++;                                 /* RR6 */
        return;
    }
}

/* ---------------------------------------------------------------- DASC / TT26 */

static void das1(u8 x)                           /* DAS1: imprime los X primeros caracteres de BUF */
{
    u8 y = 0;
    do { dt_chpr(dt_buf[y]); y++; x--; } while (x);
}

static u8 sc1;                                   /* SC+1: mascara de los espacios que se ensanchan en cada pasada */

/* DA1: fin de parrafo (CR) con el texto justificado: cada LL caracteres se ensanchan espacios hasta que el caracter LL
   de BUF sea un espacio, se imprime la linea y se sigue con el resto */
static void da1(void)
{
    u8 x, y, s, a;
    for (;;) {                                   /* DA5 */
        x = dtw5;
        if (x == 0) break;                       /* DA6+3 */
        if (x < DT_LL + 1) { das1(x); break; }   /* DA6 */
        sc1 = (u8)(sc1 >> 1);                    /* LSR SC+1 */
da11:
        if (!(sc1 & 0x80)) sc1 = 0x40;
        y = DT_LL - 1;
dal1:
        if (dt_buf[DT_LL] == ' ') goto da2;
dal2:
        y--;
        if ((y & 0x80) || y == 0) goto da11;     /* BMI DA11; BEQ DA11 */
        if (dt_buf[y] != ' ') goto dal2;
        sc1 = (u8)(sc1 << 1);                    /* ASL SC+1 */
        if (sc1 & 0x80) goto dal2;
        s = y;
        y = dtw5;
        do {                                     /* DAL6: se abre un hueco en S */
            a = dt_buf[y];
            dt_buf[(u8)(y + 1)] = a;
            y--;
        } while (y >= s && y != 0xFF);           /* CPY SC; BCS DAL6 (S >= 1) */
        dtw5++;
        for (;;) {                               /* DAL3: A = ' ' (el ultimo LDA BUF,Y fue el espacio de la posicion S) */
            if (dt_buf[y] != a) goto dal1;
            y--;
            if (y & 0x80) goto da11;
        }
da2:
        das1(DT_LL);
        dt_chpr(12);
        dtw5 = (u8)(dtw5 - DT_LL - 1);           /* SBC #LL con C = 0 (CHPR vuelve con CLC) */
        x = dtw5;
        if (x == 0) break;
        x++;
        y = 0;
        do { dt_buf[y] = dt_buf[(u8)(DT_LL + 1 + y)]; y++; x--; } while (x);   /* DAL4 */
    }
    dtw5 = 0;                                    /* STX DTW5 (X = 0) */
    dt_chpr(12);
}

void dt_dasc(u8 a)
{
    u8 x = 0xFF;
    dtw8 = 0xFF;                                 /* {19} solo vale para una letra */
    if (!(a == '.' || a == ':' || a == 10 || a == 12 || a == ' ')) x = 0;
    dtw2 = x;                                    /* $FF: no estamos dentro de una palabra */
    if (!(dtw4 & 0x80)) { dt_chpr(a); return; }
    if (!(dtw4 & 0x40) && a == 12) { da1(); return; }
    dt_buf[dtw5] = a;
    dtw5++;
}

/* ---------------------------------------------------------------- TT27: fichas normales */

static void tt26(u8 a) { dt_dasc(a); }

static void ex(u8 a)                             /* ex: ficha A de QQ18 */
{
    unsigned i = 0;
    u8 x = a;
    if (x) {
        for (;;) {                               /* TT51 / TT49 / TT59 */
            if (tok_qq18[i] == 0) { i++; x--; if (x == 0) break; continue; }
            i++;
        }
    }
    do {                                         /* TT50 */
        dt_tt27((u8)(tok_qq18[i] ^ TOK_RE));
        i++;
    } while (tok_qq18[i] != 0);
}

static void cmn(void)
{
    int y;
    for (y = 0; cmdr_name[y] && y < CMDR_NAMELEN; y++) tt26((u8)cmdr_name[y]);
}

void dt_tt27(u8 a)
{
    u8 x;
    if (a == 0) return;                          /* csh: dinero (no se usa en las fichas de las misiones) */
    if (a & 0x80) {                              /* TT43 */
        if (a >= 160) { ex((u8)(a - 160)); return; }    /* TT47 */
        x = (u8)((a & 127) << 1);
        dt_tt27(tok_tkn2[26 + x]);
        if (tok_tkn2[26 + x + 1] == '?') return;
        dt_tt27(tok_tkn2[26 + x + 1]);
        return;
    }
    switch (a) {
    case 1: return;                              /* tal: numero de galaxia (no se usa aqui) */
    case 2: if (!(mj & 0x80) && dt_name_hook) dt_name_hook(2); return;   /* ypl */
    case 3: if (dt_name_hook) dt_name_hook(3); return;                   /* cpl */
    case 4: cmn(); return;
    case 5: return;                              /* fwl: combustible y dinero (no se usa aqui) */
    case 6: qq17 = 0x80; return;
    case 8: qq17 = 0; return;
    case 9: dt_xc = 21; dt_tt27(':'); return;    /* crlf + TT73 */
    }
    if (a >= 96) { ex(a); return; }
    if (a >= 14 && a < 32) { ex((u8)(a + 114)); return; }   /* qw */
    x = qq17;
    if (x == 0) { tt26(a); return; }             /* TT74 */
    if (x & 0x80) {                              /* TT41 */
        if (x & 0x40) {                          /* TT45 */
            if (x == 255) return;
            if (a >= 'A') goto tt42;
            qq17 = (u8)(x & 0xBF);               /* TT46 */
            tt26(a);
            return;
        }
        if (a < 'A') { tt26(a); return; }
        qq17 = (u8)(x | 0x40);
        tt26(a);
        return;
    }
    if (x & 0x40) { qq17 = (u8)(x & 0xBF); tt26(a); return; }      /* BVS TT46 */
tt42:
    if (a >= 'A' && a <= 'Z') a = (u8)(a + 32);
    tt26(a);
}

/* ---------------------------------------------------------------- DETOK */

static void dt_print(const u8 *t, u8 a)          /* DETOK/DTEN: la ficha A de la tabla t (delimitador: VE) */
{
    unsigned i = 0;
    u8 x = a, c;
    for (;;) {                                   /* DTL1 */
        if ((u8)(t[i] ^ TOK_VE) == 0) { x--; if (x == 0) break; }
        i++;
    }
    for (;;) {                                   /* DTL2 */
        i++;
        c = (u8)(t[i] ^ TOK_VE);
        if (c == 0) return;
        detok2(c);
    }
}

void detok(u8 a) { dt_print(tok_tkn1, a); }
void detok3(u8 a) { dt_print(tok_rutok, a); }

void dt_dts(u8 a)
{
    if (a >= 'A') {
        if ((dtw6 & 0x80) || !(dtw2 & 0x80)) a = (u8)(a | dtw1);   /* DT10 */
        a = (u8)(a & dtw8);                      /* DT5 */
    }
    dt_dasc(a);                                  /* DT9 */
}

static void mt13(void) { dtw6 = 0x80; dtw1 = 0x20; }
static void mt19(void) { dtw8 = 0xDF; }
void dt_mt2(void) { dtw1 = 0x20; dtw6 = 0; }
void dt_mt14(void) { dtw4 = 0x80; dtw5 = 0; }
void dt_mt15(void) { dtw4 = 0; dtw5 = 0; }

/* Byte a de la pagina cero del Apple II, para el fallo de MT17. Solo se modela lo que puede ser una letra: RAND (2-5),
   INWK (12-48) y K3 (56-69: K3 es el ultimo caracter de CHPR); el resto (T1-T4, SC, P, XC, COL, YC, QQ17...) nunca da
   una vocal en ese momento (direcciones de pantalla y de la fuente, columnas, banderas) */
static u8 zp_peek(u8 a)
{
    if (a >= 2 && a <= 5) return em_rand[a - 2];
    if (a >= 12 && a <= 48) return inwk[a - 12];
    if (a >= 56 && a <= 69) return zk3[a - 56];
    return 0;
}

static u8 vowel(u8 a)
{
    a = (u8)(a | 0x20);
    return (u8)(a == 'a' || a == 'e' || a == 'i' || a == 'o' || a == 'u');
}

static void pause2(void) { if (dt_pause2_hook) dt_pause2_hook(); }
static void tt66(u8 v) { if (dt_tt66_hook) dt_tt66_hook(v); }

/* JMTB: fichas de salto 1..31 (DT3) */
static void jump(u8 a)
{
    u8 y, x;
    switch (a) {
    case 1: dtw1 = 0; dtw6 = 0; break;                        /* MT1 */
    case 2: dt_mt2(); break;
    case 3: case 4: dt_tt27(a); break;                        /* nombre del sistema seleccionado / del comandante */
    case 5: dtw3 = 0; break;                                  /* MT5 */
    case 6: qq17 = 0x80; dtw3 = 0xFF; break;                  /* MT6 */
    case 8: dt_xc = 6; dtw2 = 0xFF; break;                    /* MT8 */
    case 9: dt_xc = 1; tt66(1); break;                        /* MT9 */
    case 11: if (dt_nlin4_hook) dt_nlin4_hook(); break;
    case 13: mt13(); break;
    case 14: dt_mt14(); break;
    case 15: dt_mt15(); break;
    case 16: dt_dasc(DTW7); break;                            /* MT16 */
    case 17:                                                  /* MT17: adjetivo del sistema ("...IAN") */
        qq17 = (u8)(qq17 & 0xBF);
        dt_tt27(3);
        /* LDA BUF-1,X: BUF = $0100, asi que BUF-1 = $FF y BeebAsm lo ensambla en pagina cero (LDA $FF,X, B5 FF en el
           binario): VOWEL mira el byte DTW5 - 1 de la pagina cero, no la ultima letra (ver zp_peek) */
        if (vowel(zp_peek((u8)(dtw5 - 1)))) dtw5--;
        detok(153);
        break;
    case 18:                                                  /* MT18: 1 a 4 fichas de dos letras al azar */
        mt19();
        em_rand_c = 0;                                        /* C = 0: DT3 (ASL/LSR de un numero < 128) */
        y = (u8)(em_dornd() & 3);
        do {
            x = (u8)(em_dornd() & 62);                        /* 1.a vuelta: acarreo del DORND anterior; luego 0 (DASC) */
            dt_dts(tok_tkn2[2 + x]);
            dt_dts(tok_tkn2[3 + x]);
            em_rand_c = 0;                                    /* DASC vuelve con C = 0 */
            y--;
        } while (!(y & 0x80));
        break;
    case 19: mt19(); break;
    case 21: if (dt_clyns_hook) dt_clyns_hook(); break;       /* CLYNS */
    case 22: pause2(); tt66(1); break;                        /* PAUSE: PAUSE2 y PAS1 */
    case 23: dt_yc = 9; mt13(); break;                        /* MT23 */
    case 24: pause2(); break;
    case 25: detok(216); if (dt_delay_hook) dt_delay_hook(100); break;   /* BRIS */
    case 26: break;                                           /* MT26: nombre de fichero (menu de disco propio) */
    case 27: detok((u8)(217 + gcnt)); break;                  /* MT27 */
    case 28: detok((u8)(220 + gcnt)); break;                  /* MT28 */
    case 29: dt_yc = 5; mt13(); break;                        /* MT29 */
    case 30: case 31: break;                                  /* FILEPR, OTHERFILEPR: disco */
    default: dt_dasc(a); break;                               /* 7, 10, 12, 20: DASC */
    }
}

void detok2(u8 a)
{
    u8 x;
    if (a < 32) { jump(a); return; }             /* DT3 */
    if (dtw3 & 0x80) { dt_tt27(a); return; }     /* fichas normales */
    if (a < '[') { dt_dts(a); return; }          /* DT8 */
    if (a < 129) {                               /* DT6: ficha al azar de MTIN */
        em_rand_c = 0;                           /* C = 0 tras CMP #129 */
        x = em_dornd();
        detok((u8)(tok_mtin[a - 91] + (x >= 51) + (x >= 102) + (x >= 153) + (x >= 204)));
        return;
    }
    if (a < 215) { detok(a); return; }
    x = (u8)((a - 215) << 1);                    /* fichas de dos letras de TKN2 */
    dt_dts(tok_tkn2[x]);
    dt_dts(tok_tkn2[x + 1]);
}
