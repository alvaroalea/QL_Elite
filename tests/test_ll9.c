/* Compara ship3d.c (LL9, recorte, LL51) y sm_proj con los vectores generados ejecutando el codigo ORIGINAL en un emulador de 6502.
   Formato de cada linea:  op in:nombre=hex,... out:nombre=hex,...   */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ship3d.h"
u8 *screen;
void plat_init(void){} void plat_present(void){} void plat_shutdown(void){}
void loin_draw(u8 a, u8 b, u8 c, u8 d, u8 e) { (void)a; (void)b; (void)c; (void)d; (void)e; }

static int fails, total, nops;
static char opcount[64][16]; static long opn[64], opf[64];

typedef struct { const char *name; u8 *p; int len; } var_t;
static u8 r_ia, r_ty, r_bp, r_ci, r_sw;
static u8 o_ra, o_rc;
static u8 heap[256];
static u8 dlog[1 + 4 * 256], slog[1 + 16];
static void rec_loin(void) { int n = dlog[0]; if (n < 255) { memcpy(dlog + 1 + 4 * n, zx15, 4); dlog[0]++; } }
static void rec_doexp(void) { if (slog[0] < 15) { slog[1 + slog[0]] = 1; slog[0]++; } }
static void rec_planet(void) { if (slog[0] < 15) { slog[1 + slog[0]] = 2; slog[0]++; } }
static var_t vars[] = {
    {"inwk", inwk, NI}, {"ks", ks, 2 * NI}, {"k", zK, 4}, {"k3", zk3, 16}, {"x15", zx15, 6}, {"x12", zx12, 6},
    {"xx16", zxx16, 18}, {"xx3", xx3, 200}, {"xx2", zk3, 16}, {"heap", heap, 128}, {"rand", em_rand, 4},
    {"ty", &r_ty, 1}, {"bp", &r_bp, 1}, {"ci", &r_ci, 1}, {"sw", &r_sw, 1},
    {"dlog", dlog, 1 + 4 * 255}, {"slog", slog, 17}, {"ia", &r_ia, 1},
    {"ra", &o_ra, 1}, {"rc", &o_rc, 1},
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
            else if (v->p[i] != b) {
                if (!bad && fails < 8) printf("  %s %s[%d]: original=%02x puerto=%02x\n", op, v->name, i, b, v->p[i]);
                bad = 1;
            }
        }
        if (compare && bad && fails < 8) { printf("FALLO %s (caso %d): variable %s\n", op, total + 1, v->name); }
        s = e + n; if (*s == ',') s++;
    }
    return bad;
}

static void run(const char *op)
{
    o_ra = o_rc = 0;
    memset(dlog, 0, sizeof dlog); memset(slog, 0, sizeof slog);
    ship_loin_hook = rec_loin; ship_doexp_hook = rec_doexp; ship_planet_hook = rec_planet;
    if (!strcmp(op, "ll9")) {
        ztype = r_ty; zxx0 = SHIP_BP(r_bp); zxx19 = heap; zinf = ks; ship_c_in = r_ci;
        ship_ll9();
    }
    else if (!strcmp(op, "clip")) { zswap = r_sw; o_rc = ship_clip(); r_sw = zswap; }
    else if (!strcmp(op, "clip2")) { zswap = r_sw; o_rc = ship_clip2(zx15[5]); r_sw = zswap; }
    else if (!strcmp(op, "ll51")) ship_ll51();
    else if (!strcmp(op, "proj")) { o_rc = sm_proj(); o_ra = sm_a; }
    else { printf("operacion desconocida: %s\n", op); exit(2); }
}
static char line[16384];
int main(int argc, char **argv)
{
    FILE *f = fopen(argc > 1 ? argv[1] : "tests/vectors_ll9.txt", "r");
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
