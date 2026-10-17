/* Compara ship_math.c con los vectores generados desde la transcripcion literal del 6502.
   Formato de cada linea:  op in:nombre=hex,... out:nombre=hex,...   */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ship_math.h"
u8 *screen;
void plat_init(void){} void plat_present(void){} void plat_shutdown(void){}

static int fails, total, nops;
static char opcount[64][16]; static long opn[64], opf[64];

typedef struct { const char *name; u8 *p; int len; } var_t;
static u8 r_ia, r_ix, r_iy, r_iv, r_ic;           /* registros de entrada */
static u8 o_ra, o_rx, o_ry, o_rc;                 /* registros de salida */
static var_t vars[] = {
    {"inwk", inwk, NI}, {"ks", ks, 2 * NI}, {"k", zK, 4}, {"k3", zk3, 16}, {"x15", zx15, 6},
    {"q", &zQ, 1}, {"r", &zR, 1}, {"s", &zS, 1}, {"t", &zT, 1}, {"u", &zU, 1},
    {"p", &zP, 1}, {"p1", &zP1, 1}, {"p2", &zP2, 1}, {"cash", cash, 4},
    {"ia", &r_ia, 1}, {"ix", &r_ix, 1}, {"iy", &r_iy, 1}, {"iv", &r_iv, 1}, {"ic", &r_ic, 1},
    {"ra", &o_ra, 1}, {"rx", &o_rx, 1}, {"ry", &o_ry, 1}, {"rc", &o_rc, 1},
    {0, 0, 0}
};
static int hexv(int c) { return c <= '9' ? c - '0' : c - 'a' + 10; }
static var_t *find(const char *n, int len)
{
    var_t *v;
    for (v = vars; v->name; v++) if ((int)strlen(v->name) == len && !strncmp(v->name, n, len)) return v;
    return 0;
}

/* recorre "a=hex,b=hex"; modo 0 = cargar, modo 1 = comparar; devuelve 0 si todo coincide */
static int apply(const char *s, int compare, const char *op, const char *line)
{
    int bad = 0;
    while (*s && *s != ' ' && *s != '\n') {
        const char *eq = strchr(s, '=');
        const char *e;
        var_t *v;
        int i, n;
        if (!eq) break;
        v = find(s, (int)(eq - s));
        e = eq + 1;
        n = 0; while (e[n] && e[n] != ',' && e[n] != ' ' && e[n] != '\n') n++;
        if (!v) { printf("variable desconocida en %s\n", op); exit(2); }
        for (i = 0; i < n / 2; i++) {
            u8 b = (u8)((hexv(e[2 * i]) << 4) | hexv(e[2 * i + 1]));
            if (!compare) v->p[i] = b;
            else if (v->p[i] != b) { bad = 1; }
        }
        if (compare && bad && fails < 8) { printf("FALLO %s: variable %s\n  %.200s\n", op, v->name, line); }
        s = e + n; if (*s == ',') s++;
    }
    return bad;
}

static void run(const char *op)
{
    o_ra = o_rx = o_ry = o_rc = 0;
    if (!strcmp(op, "mvt3")) { o_ra = sm_mvt3(r_ix); o_rc = sm_c; }
    else if (!strcmp(op, "mas1")) o_ra = sm_mas1(r_iy, r_ix);
    else if (!strcmp(op, "mas2")) o_ra = sm_mas2(r_ia, r_iy);
    else if (!strcmp(op, "mas2m")) o_ra = sm_mas2m(r_iy);
    else if (!strcmp(op, "mas3")) o_ra = sm_mas3(r_iy);
    else if (!strcmp(op, "mas4")) o_ra = sm_mas4(r_ia);
    else if (!strcmp(op, "vcsub")) sm_vcsub(ks + r_iv);
    else if (!strcmp(op, "tas1")) sm_tas1(ks + r_iv, r_iy);
    else if (!strcmp(op, "tas2")) sm_tas2();
    else if (!strcmp(op, "norm")) sm_norm();
    else if (!strcmp(op, "tas3")) { o_ra = sm_tas3(r_iy); o_rx = em_x; }
    else if (!strcmp(op, "tas4")) { o_ra = sm_tas4(r_iy); o_rx = em_x; }
    else if (!strcmp(op, "tas6")) sm_tas6();
    else if (!strcmp(op, "tis3")) o_ra = sm_tis3(r_ia, r_ix, r_iy);
    else if (!strcmp(op, "tidy")) sm_tidy();
    else if (!strcmp(op, "sps1")) sm_sps1();
    else if (!strcmp(op, "sps4")) sm_sps4();
    else if (!strcmp(op, "sps3")) sm_sps3(r_ix);
    else if (!strcmp(op, "sps2")) { o_rx = sm_sps2(r_ia); o_ry = sm_y; }
    else if (!strcmp(op, "pls6")) { o_rc = sm_pls6(r_ia); o_rx = sm_x; }
    else if (!strcmp(op, "proj")) o_rc = sm_proj();
    else if (!strcmp(op, "lcash")) o_rc = sm_lcash(r_ix, r_iy);
    else if (!strcmp(op, "mcash")) sm_mcash(r_ix, r_iy);
    else if (!strcmp(op, "gcash")) { o_rx = sm_gcash(); o_ry = sm_y; }
    else if (!strcmp(op, "scaley2")) { o_ra = sm_scaley2(r_ia); o_rc = sm_c; }
    else if (!strcmp(op, "scaley")) { o_ra = sm_scaley(r_ia); o_rc = sm_c; }
    else if (!strcmp(op, "scalex")) { o_ra = sm_scalex(r_ia); o_rc = sm_c; }
    else { printf("operacion desconocida: %s\n", op); exit(2); }
}

static char line[8192];
int main(int argc, char **argv)
{
    FILE *f = fopen(argc > 1 ? argv[1] : "tests/vectors3.txt", "r");
    int i;
    if (!f) { printf("sin vectores\n"); return 2; }
    while (fgets(line, sizeof line, f)) {
        char op[16];
        char *in, *out;
        int bad, k;
        if (sscanf(line, "%15s", op) != 1) continue;
        in = strstr(line, " in:"); out = strstr(line, " out:");
        if (!in || !out) continue;
        state_clear();
        apply(in + 4, 0, op, line);
        /* mvt3/tas1 etc. leen K tal como esta; los registros de entrada ya estan cargados */
        run(op);
        bad = apply(out + 5, 1, op, line);
        total++;
        for (k = 0; k < nops; k++) if (!strcmp(opcount[k], op)) break;
        if (k == nops) { strcpy(opcount[nops++], op); }
        opn[k]++;
        if (bad) { fails++; opf[k]++; }
    }
    fclose(f);
    for (i = 0; i < nops; i++) printf("  %-8s %5ld casos, %ld fallos\n", opcount[i], opn[i], opf[i]);
    printf("%d comprobaciones, %d fallos\n", total, fails);
    return fails ? 1 : 0;
}
