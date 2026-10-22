/* Volcado para tools/test_ll9asm.py: casos al azar de ship_ll9_faces_c y ship_ll9_verts_c con sus entradas (variables globales y planos
   reales de las 32 naves) y sus salidas, en texto. uso: ll9_dump N semilla */
#include <stdio.h>
#include <stdlib.h>
#include "../src/ship3d.h"

static unsigned long rs;
static unsigned rnd(void) { rs = (rs * 1103515245UL + 12345UL) & 0xFFFFFFFFUL; return (unsigned)((rs >> 8) & 0xFFFF); }
static void hex(const char *k, const u8 *p, int n) { int i; printf(" %s=", k); for (i = 0; i < n; i++) printf("%02x", p[i]); }
static void rbytes(u8 *p, int n) { int i; for (i = 0; i < n; i++) p[i] = (u8)rnd(); }
static const u8 *bp_off(const u8 *bp, u8 lo, u8 hi)
{
    long off = (long)hi * 256L + lo;
    if (off >= 32768L) off -= 65536L;
    return bp + off;
}

/* ---- aristas: LL75..LL81 con el recorte y el dibujo reales (ship_clip2, ship_lsput) y las lineas registradas ---- */
static unsigned char lines_log[600][4]; static int nlines;
static void rec_loin(void) { if (!(ship_dbg_skip & 2) && nlines < 600) { lines_log[nlines][0] = zx15[0]; lines_log[nlines][1] = zx15[1]; lines_log[nlines][2] = zx15[2]; lines_log[nlines][3] = zx15[3]; nlines++; } }
static int edge_idx[80], edge_acc[80], edge_n; static unsigned char edge_xy[80][4], edge_raw[80][8];

/* copia de ship_ll9_edges_c que ademas registra, por arista visible, el resultado de ship_clip2 y la linea (X1, Y1, X2, Y2) que deja */
static void edges_logged(const u8 *v)
{
    u8 a, x;
ll75:
    if (v[0] < zxx4) goto ll78;
    a = v[1];
    if (xx2[a & 0x0F] != 0) goto ll79;
    if (xx2[a >> 4] == 0) goto ll78;
ll79:
    x = v[2];
    zx15[0] = xx3[x]; zx15[1] = xx3[x + 1]; zx15[2] = xx3[x + 2]; zx15[3] = xx3[x + 3];
    x = v[3];
    zx15[4] = xx3[x]; zx12[0] = xx3[x + 2]; zx12[1] = xx3[x + 3]; zx15[5] = xx3[x + 1];
    {
        int r;
        for (r = 0; r < 6 && edge_n < 80; r++) edge_raw[edge_n][r] = zx15[r];
        if (edge_n < 80) { edge_raw[edge_n][6] = zx12[0]; edge_raw[edge_n][7] = zx12[1]; }
        r = ship_clip2(zx15[5]);
        if (edge_n < 80) { edge_idx[edge_n] = zxx17; edge_acc[edge_n] = !r; edge_xy[edge_n][0] = zx15[0]; edge_xy[edge_n][1] = zx15[1]; edge_xy[edge_n][2] = zx15[2]; edge_xy[edge_n][3] = zx15[3]; edge_n++; }
        if (r) goto ll78;
    }
    ship_lsput();
ll78:
    if (zlsnum >= zcnt) return;
    v += 4;
    zxx17++;
    if (zxx17 < zxx20) goto ll75;
}

static u8 heap[256], heap0[256];
static void edges_case(const u8 *bp, int type, int mode)
{
    static const int chs[3] = { 136, 160, 134 };
    const u8 *v = bp_off(bp, bp[3], bp[16]);
    int i;
    zxx0 = bp; zxx20 = bp[9]; zcnt = bp[5]; zxx4 = (u8)(rnd() % 32); zxx17 = 0;
    if (zxx20 == 0) return;
    clip_h = chs[rnd() % 3];
    rbytes(xx3, 258); rbytes(zk3, 16); rbytes(heap0, 256); rbytes(zx12, 6); rbytes(zx15, 6);
    for (i = 0; i < 16; i++) if (rnd() % 5) zk3[i] = (u8)(1 + rnd() % 9);
    if (mode == 0)                                       /* casi todo dentro de la vista */
        for (i = 0; i + 3 < 258; i += 4) {
            if (rnd() % 10) xx3[i + 1] = 0;
            if (rnd() % 10) xx3[i + 3] = 0;
            if (rnd() % 10) xx3[i + 2] = (u8)(rnd() % clip_h);
            else if (rnd() & 1) xx3[i + 2] = (u8)(clip_h - 2 + (int)(rnd() % 4));     /* justo en el borde (clip_h - 2 .. clip_h + 1) */
        }
    zlsnum2 = (u8)((rnd() % 4) ? 1 + 4 * (rnd() % 12) : 0);
    zlsnum = 1;
    for (i = 0; i < 256; i++) heap[i] = heap0[i];
    zxx19 = heap;
    printf("CASE 2 %d %d %ld 0 %d %d zxx4=%d zxx17=0 zxx20=%d zcnt=%d zlsnum2=%d", type, (int)ship_off[type], (long)(v - ship_blob), clip_h, mode, zxx4, zxx20, zcnt, zlsnum2);
    hex("xx3", xx3, 258); hex("zk3", zk3, 16); hex("heap", heap0, 256); hex("zx12", zx12, 6); hex("zx15", zx15, 6);
    printf("\n");
    ship_dbg_skip = (u8)((rnd() % 4) == 0 ? 2 : 0);
    ship_loin_hook = rec_loin; nlines = 0; edge_n = 0;
    edges_logged(v);
    printf("OUT skip=%d zlsnum=%d zxx17=%d nl=%d", ship_dbg_skip, zlsnum, zxx17, nlines);
    hex("heap", heap, 256);
    printf(" lines=");
    for (i = 0; i < nlines; i++) printf("%02x%02x%02x%02x", lines_log[i][0], lines_log[i][1], lines_log[i][2], lines_log[i][3]);
    printf(" edges=");
    for (i = 0; i < edge_n; i++) printf("%02x%d%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", edge_idx[i], edge_acc[i], edge_xy[i][0], edge_xy[i][1], edge_xy[i][2], edge_xy[i][3],
                                        edge_raw[i][0], edge_raw[i][1], edge_raw[i][2], edge_raw[i][3], edge_raw[i][4], edge_raw[i][5], edge_raw[i][6], edge_raw[i][7]);
    printf("\n");
    /* comprobacion de la propia prueba: ship_ll9_edges_c da lo mismo que la copia con registro */
    {
        u8 heap1[256]; int nl1 = nlines, z1 = zlsnum, x17 = zxx17, bad = 0;
        static unsigned char lines1[600][4];
        for (i = 0; i < 256; i++) heap1[i] = heap[i];
        for (i = 0; i < nl1; i++) { lines1[i][0] = lines_log[i][0]; lines1[i][1] = lines_log[i][1]; lines1[i][2] = lines_log[i][2]; lines1[i][3] = lines_log[i][3]; }
        for (i = 0; i < 256; i++) heap[i] = heap0[i];
        zlsnum = 1; zxx17 = 0; nlines = 0; ship_ll9_edges_c(v);
        if (nlines != nl1 || zlsnum != z1 || zxx17 != x17) bad = 1;
        for (i = 0; i < 256; i++) if (heap[i] != heap1[i]) bad = 1;
        for (i = 0; i < nl1 && !bad; i++) if (lines_log[i][0] != lines1[i][0] || lines_log[i][1] != lines1[i][1] || lines_log[i][2] != lines1[i][2] || lines_log[i][3] != lines1[i][3]) bad = 1;
        if (bad) fprintf(stderr, "AUTOCOMPROBACION: ship_ll9_edges_c != copia con registro\n");
    }
}

int main(int argc, char **argv)
{
    int n = argc > 1 ? atoi(argv[1]) : 100, k, i, kind;
    rs = argc > 2 ? (unsigned long)atol(argv[2]) : 1UL;
    printf("BLOB ");
    for (i = 0; i < SHIP_BLOB_SIZE; i++) printf("%02x", ship_blob[i]);
    printf("\n");
    for (k = 0; k < n; k++) {
        int type = 1 + (int)(rnd() % SHIP_TYPES), mode = (int)(rnd() % 7), custom = 0;
        const u8 *bp = SHIP_BP(type), *v;
        static u8 cplan[48];
        kind = k % 3;                                    /* 0 = caras, 1 = vertices, 2 = aristas */
        if (kind == 2) mode = (int)(rnd() % 2);
        if (mode == 6 && kind == 1) {                    /* plano propio: dos vertices con coordenadas diminutas (profundidades 0..8) */
            int nv = 1 + (int)(rnd() % 3);
            for (i = 0; i < 48; i++) cplan[i] = 0;
            for (i = 0; i < nv; i++) {
                cplan[20 + 6 * i] = (u8)(rnd() % 4); cplan[21 + 6 * i] = (u8)(rnd() % 4); cplan[22 + 6 * i] = (u8)(rnd() % 4);
                cplan[23 + 6 * i] = (u8)((rnd() & 0xE0) | (rnd() % 8)); cplan[24 + 6 * i] = (u8)rnd(); cplan[25 + 6 * i] = (u8)rnd();
            }
            cplan[8] = (u8)(6 * nv);
            bp = cplan; custom = 1; type = 0;
        } else if (mode == 6) mode = 0;
        zxx0 = bp;
        zxx4 = (u8)(rnd() % 40);
        zxx17 = (u8)(rnd() % 8);
        rbytes(zxx16, 18); rbytes(zxx18, 9); rbytes(zk3, 16); rbytes(zx12, 6); rbytes(zx15, 6); rbytes(inwk, NI); rbytes(xx3, 258);
        if (mode == 1) for (i = 0; i < 9; i++) zxx18[i] = (u8)((zxx18[i] & 0x8F) % 0x90);    /* valores pequenos: rara vez desborda */
        if (mode == 2) for (i = 0; i < 16; i++) zk3[i] = (u8)((rnd() & 1) ? 0 : zk3[i]);       /* caras ocultas y visibles */
        if (mode == 3) { for (i = 0; i < 18; i++) zxx16[i] = (u8)(zxx16[i] % 0x61 | (zxx16[i] & 0x80 ? 0x80 : 0)); inwk[7] = (u8)(rnd() % 6); inwk[8] = 0; inwk[1] &= 3; inwk[4] &= 3; }
        if (mode == 4) {                                 /* todo diminuto: la profundidad y las posiciones caen en 0, 4, 5... (casos limite) */
            for (i = 0; i < 18; i++) zxx16[i] = (u8)((rnd() % 4) | (rnd() & 0x80));
            for (i = 0; i < 9; i++) inwk[i] = (u8)(rnd() % 9);
            inwk[2] = (u8)((rnd() & 1) ? 0x80 : 0); inwk[5] = (u8)((rnd() & 1) ? 0x80 : 0); inwk[8] = (u8)((rnd() & 1) ? 0x80 : 0);
            if (rnd() & 1) { inwk[6] = (u8)(rnd() % 8); inwk[7] = 0; }
        }
        if (mode == 5) {                                 /* matriz a cero y bytes altos de la posicion en {0, $80}: valores de 16 bits con solo el bit 15 */
            for (i = 0; i < 18; i++) zxx16[i] = (u8)((rnd() & 3) ? 0 : (rnd() & 0x80));
            inwk[1] = (u8)((rnd() & 1) ? 0x80 : 0); inwk[4] = (u8)((rnd() & 1) ? 0x80 : 0); inwk[7] = (u8)((rnd() & 1) ? 0x80 : 0);
            inwk[0] = (u8)(rnd() % 3 ? 0 : rnd()); inwk[3] = (u8)(rnd() % 3 ? 0 : rnd()); inwk[6] = (u8)(rnd() % 3 ? 0 : rnd());
        }
        if (custom) {
            for (i = 0; i < 18; i++) zxx16[i] = (u8)((rnd() % 5) | (rnd() & 0x80));
            for (i = 0; i < 9; i++) inwk[i] = (u8)(rnd() % 12);
            inwk[2] = (u8)((rnd() & 1) ? 0x80 : 0); inwk[5] = (u8)((rnd() & 1) ? 0x80 : 0); inwk[8] = (u8)((rnd() & 1) ? 0x80 : 0);
            inwk[7] = (u8)((rnd() % 5) ? 0 : 1);
            for (i = 0; i < 16; i++) zk3[i] = (u8)((rnd() % 4) ? 1 + rnd() % 9 : 0);
        }
        view_yscale = (int)(rnd() & 1); { static const int cys[5] = { 68, 80, 92, 100, 127 }; view_cy = cys[rnd() % 5]; }
        if (kind == 2) { edges_case(bp, type, mode); continue; }
        if (kind == 0) {
            zxx20 = bp[12];
            if (zxx20 == 0) { k--; continue; }
            v = bp_off(bp, bp[4], bp[17]);
        } else {
            zxx20 = bp[8];
            v = 0;
        }
        printf("CASE %d %d %d %ld %d %d %d zxx4=%d zxx17=%d zxx20=%d", kind, type, (int)ship_off[type], v ? (long)(v - ship_blob) : 0L, view_yscale, view_cy, mode,
               zxx4, zxx17, zxx20);
        hex("zxx16", zxx16, 18); hex("zxx18", zxx18, 9); hex("zk3", zk3, 16); hex("zx12", zx12, 6); hex("zx15", zx15, 6);
        hex("inwk", inwk, NI); hex("xx3", xx3, 258);
        if (custom) hex("plan", cplan, 48);
        printf("\n");
        if (kind == 0) ship_ll9_faces_c(v); else ship_ll9_verts_c();
        printf("OUT zcnt=%d zxx17=%d", zcnt, zxx17);
        hex("zk3", zk3, 16); hex("zxx18", zxx18, 9); hex("xx3", xx3, 258);
        printf("\n");
    }
    return 0;
}
