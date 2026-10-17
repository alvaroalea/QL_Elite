/* Compara con el 6502 original (tests/vectors_missions.txt, de tools/gen_missions.py):
   T  motor de fichas (detok.c): DETOK, DETOK3 y TT27 con la salida de CHPR (letra, columna, fila) y los ganchos
   P  PDESC (missions.c)
   M  decision de DOENTRY, BRIEF2/BRIEF3/DEBRIEF/DEBRIEF2 y el bucle de BRIEF
   E  partes 2 a 4 del bucle principal (encounters.c) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "detok.h"
#include "missions.h"
#include "encounters.h"
#include "state.h"
#include "emath.h"
#include "cmdr.h"
#include "ships.h"
#include "ship3d.h"
#include "shipdata.h"
#include "space.h"
#include "game.h"
#include "input.h"
#include "move.h"
#include "universe.h"
#include "config.h"
#include "space.h"
#include "sound.h"

static int total, fails;
static char line[200000], got[200000], copy[400];
static int glen;

static void add(const char *s)
{
    int n = (int)strlen(s);
    if (glen) got[glen++] = ';';
    memcpy(got + glen, s, (size_t)n + 1);
    glen += n;
}
static void ev_letter(u8 ch, u8 xc, u8 yc) { char b[32]; sprintf(b, "L%d,%d,%d", ch, xc, yc); add(b); }
static void ev_clss(void) { add("C"); }
static void ev_beep(void) { add("B"); }
static void ev_tt66(u8 v) { char b[8]; sprintf(b, "T%d", v); add(b); }
static void ev_pause2(void) { add("P"); }
static void ev_delay(u8 n) { char b[8]; sprintf(b, "D%d", n); add(b); }
static void ev_nlin4(void) { add("N"); }
static void ev_clyns(void) { add("Y"); }
static char nm_sel[32], nm_cur[32];
static void ev_name(u8 which)
{
    const char *s = which == 3 ? nm_sel : nm_cur;
    for (; *s; s++) dt_tt27((u8)*s);
}

static void hooks(void)
{
    dt_letter_hook = ev_letter; dt_clss_hook = ev_clss; dt_beep_hook = ev_beep; dt_tt66_hook = ev_tt66;
    dt_pause2_hook = ev_pause2; dt_delay_hook = ev_delay; dt_nlin4_hook = ev_nlin4; dt_clyns_hook = ev_clyns;
    dt_name_hook = ev_name;
}

static void hexb(const char *h, u8 *b, int n) { int i; unsigned v; for (i = 0; i < n; i++) { sscanf(h + 2 * i, "%2x", &v); b[i] = (u8)v; } }

static int tok_n(char **a, char *s)
{
    int n = 0;
    char *p = strtok(s, " \n");
    while (p) { a[n++] = p; p = strtok(0, " \n"); }
    return n;
}

static void t_case(char **a)
{
    /* T kind tok qq17 dtw1 dtw2 dtw3 dtw4 dtw5 dtw6 dtw8 xc yc gc mj rnd inwk k3 sel cur nm ev | fin*10 k3 rnd buf */
    char kind = a[1][0];
    int tok = atoi(a[2]), i, ok;
    u8 *v[10];
    char fin[200], buf[600];
    v[0] = &qq17; v[1] = &dtw1; v[2] = &dtw2; v[3] = &dtw3; v[4] = &dtw4; v[5] = &dtw5; v[6] = &dtw6; v[7] = &dtw8; v[8] = &dt_xc; v[9] = &dt_yc;
    for (i = 0; i < 10; i++) *v[i] = (u8)atoi(a[3 + i]);
    gcnt = (u8)atoi(a[13]); mj = (u8)atoi(a[14]);
    hexb(a[15], em_rand, 4);
    hexb(a[16], inwk, 37);
    hexb(a[17], zk3, 14);
    strcpy(nm_sel, strcmp(a[18], "-") ? a[18] : ""); strcpy(nm_cur, strcmp(a[19], "-") ? a[19] : "");
    strcpy(cmdr_name, a[20]);
    glen = 0; got[0] = 0;
    if (kind == 'E') detok((u8)tok); else if (kind == 'R') detok3((u8)tok); else dt_tt27((u8)tok);
    fin[0] = 0;
    for (i = 0; i < 10; i++) sprintf(fin + strlen(fin), "%s%d", i ? " " : "", *v[i]);
    sprintf(fin + strlen(fin), " %d", zk3[0]);
    sprintf(fin + strlen(fin), " %02x%02x%02x%02x", em_rand[0], em_rand[1], em_rand[2], em_rand[3]);
    buf[0] = 0;
    for (i = 0; i < dtw5; i++) sprintf(buf + 2 * i, "%02x", dt_buf[i]);
    if (!dtw5) strcpy(buf, "-");
    {
        char exp[200];
        exp[0] = 0;
        for (i = 0; i < 12; i++) sprintf(exp + strlen(exp), "%s%s", i ? " " : "", a[23 + i]);
        ok = !strcmp(strcmp(a[21], "-") ? a[21] : "", got) && !strcmp(exp, fin) && !strcmp(a[35], buf);
        total++;
        if (!ok) {
            fails++;
            if (fails <= 5) {
                printf("FALLO T %c %d\n  6502: %.300s\n     C: %.300s\n  6502: %s %s\n     C: %s %s\n", kind, tok, a[21], got, exp, a[35], fin, buf);
                if (strcmp(a[21], got)) { int k = 0; while (a[21][k] && a[21][k] == got[k]) k++; printf("  difiere en %d: ...%.60s | ...%.60s\n", k, a[21] + (k > 20 ? k - 20 : 0), got + (k > 20 ? k - 20 : 0)); }
            }
        }
    }
}


static void check(int ok, const char *what, const char *l)
{
    total++;
    if (!ok) { fails++; if (fails <= 12) printf("FALLO %s: %.300s\n", what, l); }
}

static void p_case(char **a, const char *l)
{
    /* P zz gc tp q8 q12 rnd name | printed ev rnd */
    int r, q8 = atoi(a[4]), zz = atoi(a[1]);
    char rs[16];
    gcnt = (u8)atoi(a[2]); tp = (u8)atoi(a[3]); qq8[0] = (u8)q8; qq8[1] = (u8)(q8 >> 8); qq12 = (u8)atoi(a[5]);
    hexb(a[6], em_rand, 4);
    strcpy(nm_sel, strcmp(a[7], "-") ? a[7] : "");
    a++;
    qq17 = 0x80; dtw1 = 0x20; dtw2 = 0xFF; dtw3 = 0; dtw4 = 0; dtw5 = 0; dtw6 = 0; dtw8 = 0xFF; dt_xc = 1; dt_yc = 10; mj = 0;
    memset(inwk, 0, NI); memset(zk3, 0, 14);
    glen = 0; got[0] = 0;
    r = pdesc((u8)zz);
    sprintf(rs, "%02x%02x%02x%02x", em_rand[0], em_rand[1], em_rand[2], em_rand[3]);
    if (atoi(a[8]) == 0) check(r == 0, "PDESC (descripcion normal)", l);
    else {
        check(r == 1 && !strcmp(a[9], got) && !strcmp(a[10], rs), "PDESC (descripcion especial)", l);
        if (fails <= 2 && strcmp(a[9], got)) { int k = 0; while (a[9][k] && a[9][k] == got[k]) k++; printf("  r %d difiere en %d: ...%.80s | ...%.80s\n", r, k, a[9] + (k > 20 ? k - 20 : 0), got + (k > 20 ? k - 20 : 0)); }
    }
}

static void m_case(char **a, const char *l)
{
    /* M tp t1 gc q0 q1 cash | who tp engy t1 cash tok */
    int c, want;
    char cs[16];
    tp = (u8)atoi(a[1]); tally[1] = (u8)atoi(a[2]); gcnt = (u8)atoi(a[3]); qq0 = (u8)atoi(a[4]); qq1 = (u8)atoi(a[5]);
    hexb(a[6], cash, 4); engy = 1;
    if (!strcmp(a[8], "BRIEF")) want = MIS_BRIEF;
    else if (!strcmp(a[8], "BAY")) want = MIS_NONE;
    else switch (atoi(a[13])) { case 15: want = MIS_DEBRIEF; break; case 11: want = MIS_BRIEF2; break;
                                case 222: want = MIS_BRIEF3; break; case 223: want = MIS_DEBRIEF2; break; default: want = -1; }
    c = missions_choose();
    check(c == want, "DOENTRY: sesion", l);
    glen = 0;
    if (c == MIS_DEBRIEF) debrief(); else if (c == MIS_BRIEF2) brief2(); else if (c == MIS_BRIEF3) brief3(); else if (c == MIS_DEBRIEF2) debrief2();
    if (c == MIS_BRIEF) return;
    sprintf(cs, "%02x%02x%02x%02x", cash[0], cash[1], cash[2], cash[3]);
    check(tp == atoi(a[9]) && engy == atoi(a[10]) && tally[1] == atoi(a[11]) && !strcmp(cs, a[12]), "BRIEF2/3, DEBRIEF/2: estado", l);
}

static char blog[20000];
static int bcnt;
static void b_ll9(void)
{
    char b[48];
    sprintf(b, "%s%d.%d.%d.%d.%d.%d", bcnt ? ";" : "", inwk[0], inwk[3], inwk[6], inwk[7], inwk[29], inwk[30]);
    strcat(blog, b); bcnt++;
}
static void b_mveit(void) { }
static int b_script(void) { return 'x'; }

static void b_case(char **a, const char *l)
{
    /* B tp | tp type tok count log */
    state_clear(); ships_clear();
    tp = (u8)atoi(a[1]);
    blog[0] = 0; bcnt = 0; glen = 0;
    mis_ll9_hook = b_ll9; mis_mveit_hook = b_mveit; in_script = b_script;
    brief();
    in_script = 0;
    check(tp == atoi(a[3]) && frin[0] == atoi(a[4]) && bcnt == atoi(a[6]) && !strcmp(blog, a[7]), "BRIEF", l);
}

static void e_case(char **a, const char *l)
{
    /* E rnd c0 mj junk sspr cops con tp gc q0 q1 gov c3 c6 c10 fist ev occ made... | end rnd ev junk cops con c */
    int occ, k, i, bar;
    char exp[4000], g[4000], rs[16];
    u8 was[NOSH];
    state_clear(); ships_clear();
    hexb(a[1], em_rand, 4);
    mj = (u8)atoi(a[3]); junk_count = (u8)atoi(a[4]);
    memset(many, 0, sizeof many);
    sspr = (u8)atoi(a[5]); many[SHIPT_VIPER] = (u8)atoi(a[6]); many[SHIPT_CONSTRICTOR] = (u8)atoi(a[7]);
    tp = (u8)atoi(a[8]); gcnt = (u8)atoi(a[9]); qq0 = (u8)atoi(a[10]); qq1 = (u8)atoi(a[11]); qq4 = (u8)atoi(a[12]);
    qq20[3] = (u8)atoi(a[13]); qq20[6] = (u8)atoi(a[14]); qq20[10] = (u8)atoi(a[15]); fist = (u8)atoi(a[16]); enc_ev = (u8)atoi(a[17]);
    occ = atoi(a[18]);
    for (k = 0; k < NOSH; k++) { frin[k] = (u8)((occ >> k) & 1 ? (k == 0 ? 0x81 : 7) : 0); was[k] = frin[k]; }
    frin[NOSH] = 0;
    encounters_tick((u8)atoi(a[2]));
    exp[0] = 0;
    for (bar = 19; strcmp(a[bar], "|"); bar++)
        if (strcmp(a[bar], "-") && a[bar][strlen(a[bar]) - 1] != '-') { if (exp[0]) strcat(exp, " "); strcat(exp, a[bar]); }
    g[0] = 0;
    for (k = 0; k < NOSH; k++) {
        u8 *p = state_slot(k);
        char b[120];
        if (was[k] || !frin[k]) continue;
        sprintf(b, "%s%d:%d:", g[0] ? " " : "", frin[k], k);
        for (i = 0; i < 37; i++) sprintf(b + strlen(b), "%02x", (i == 31 || i == 35) ? 0 : p[i]);
        strcat(g, b);
    }
    sprintf(rs, "%02x%02x%02x%02x", em_rand[0], em_rand[1], em_rand[2], em_rand[3]);
    check(!strcmp(exp, g), "encuentros: naves creadas", l);
    check(!strcmp(rs, a[bar + 2]) && enc_ev == atoi(a[bar + 3]) && junk_count == atoi(a[bar + 4]) &&
          many[SHIPT_VIPER] == atoi(a[bar + 5]) && many[SHIPT_CONSTRICTOR] == atoi(a[bar + 6]), "encuentros: estado", l);
    if (fails && fails <= 3 && strcmp(exp, g)) printf("  6502: %.400s\n     C: %.400s\n", exp, g);
}

static int cmp_slot(int n, const char *hexs, int planet)
{
    u8 b[NI];
    int i;
    hexb(hexs, b, 37);
    for (i = 0; i < 37; i++) {
        if (i == 31 || i == 33 || i == 34 || i == 35) continue;
        if (planet && i == 36) continue;
        if (state_slot(n)[i] != b[i]) return 0;
    }
    return 1;
}

static void s_case(char **a, const char *l)
{
    /* S planeta inwk mj mc sspr tek | wpls nwshp */
    int r;
    char *p;
    state_clear(); ships_clear();
    hexb(a[1], state_slot(0), NI); hexb(a[2], inwk, NI);
    mj = (u8)atoi(a[3]); sspr = (u8)atoi(a[5]); qq5 = (u8)atoi(a[6]);
    frin[0] = 128; frin[1] = 129;
    r = ships_part14((u8)atoi(a[4]));
    check(r == atoi(a[8]), "parte 14: aparece la estacion", l);
    if (r && strcmp(a[9], "-")) {
        p = strchr(a[9], ':');
        check(frin[1] == atoi(a[9]) && sspr == 1 && cmp_slot(1, p + 1, 0), "parte 14: NWSPS", l);
    }
}

static void k_case(char **a, const char *l)
{
    /* K inwk | frin1 sspr spblb tipo inwk */
    state_clear(); ships_clear();
    frin[0] = 128; frin[1] = SHIPT_CORIOLIS; sspr = 1; mstg = 0xFF;
    hexb(a[1], inwk, 37);
    ship_kill(1);
    check(atoi(a[3]) == 0 && sspr == atoi(a[4]) && frin[1] == atoi(a[6]) && cmp_slot(1, a[7], 1), "KS4", l);
}

static void o_case(char **a, const char *l)
{
    /* O qq15 fist tek tribble rnd c | fist tribble qq20 qq20+6 rnd nwshp1 nwshp2 */
    u8 q[6];
    char rs[16], tr[8];
    char *p1, *p2;
    state_clear(); ships_clear();
    hexb(a[1], q, 6);
    uni_galaxy[0].seed0 = (u16)((q[1] << 8) | q[0]); uni_galaxy[0].goatsoupseed.b = q[3]; uni_galaxy[0].goatsoupseed.d = q[5];
    cur_sys = 0; fist = (u8)atoi(a[2]); qq5 = (u8)atoi(a[3]); hexb(a[4], tribble, 2); hexb(a[5], em_rand, 4); em_rand_c = (u8)atoi(a[6]);
    qq20[0] = 9; qq20[6] = 9;
    ships_arrive();
    sprintf(rs, "%02x%02x%02x%02x", em_rand[0], em_rand[1], em_rand[2], em_rand[3]);
    sprintf(tr, "%02x%02x", tribble[0], tribble[1]);
    p1 = strchr(a[13], ':'); p2 = strchr(a[14], ':');
    check(fist == atoi(a[8]) && !strcmp(tr, a[9]) && qq20[0] == atoi(a[10]) && qq20[6] == atoi(a[11]) && !strcmp(rs, a[12]) &&
          frin[0] == atoi(a[13]) && cmp_slot(0, p1 + 1, 1) && frin[1] == atoi(a[14]) && cmp_slot(1, p2 + 1, 1), "SOLAR", l);
}

static const char *frz = "dxQ-";
static int frz_i;
static int frz_feed(void) { return frz[frz_i] ? frz[frz_i++] : -1; }

static void config_checks(void)
{
    u8 d0 = damp, p0 = patg;
    int r;
    dks3('D', 0); check(damp == (u8)(d0 ^ 0xFF), "DKS3: D invierte DAMP", "dks3");
    dks3('A', 0); check(damp == (u8)(d0 ^ 0xFF), "DKS3: otra tecla no toca DAMP", "dks3");
    dks3('D', 0);
    frz_i = 0; in_script = frz_feed;
    r = freeze();
    in_script = 0;
    check(r == 0 && damp == (u8)(d0 ^ 0xFF) && patg == (u8)(p0 ^ 0xFF) && !snd_enabled(), "FREEZE: D, X, Q y - (fin de la pausa)", "freeze");
    snd_enable(1); damp = d0; patg = p0;
}

int main(void)
{
    FILE *f = fopen("tests/vectors_missions.txt", "r");
    char *a[64];
    int cnt[128];
    memset(cnt, 0, sizeof cnt);
    if (!f) { printf("falta tests/vectors_missions.txt (make test_missions)\n"); return 1; }
    hooks();
    config_checks();
    while (fgets(line, sizeof line, f)) {
        int na;
        memcpy(copy, line, sizeof copy - 1);
        na = tok_n(a, line);
        if (na < 2) continue;
        cnt[(int)a[0][0]]++;
        if (a[0][0] == 'T') t_case(a);
        else if (a[0][0] == 'P') p_case(a, copy);
        else if (a[0][0] == 'M') m_case(a, copy);
        else if (a[0][0] == 'B') b_case(a, copy);
        else if (a[0][0] == 'E') e_case(a, copy);
        else if (a[0][0] == 'S') s_case(a, copy);
        else if (a[0][0] == 'K') k_case(a, copy);
        else if (a[0][0] == 'O') o_case(a, copy);
    }
    fclose(f);
    printf("S %d, K %d, O %d; ", cnt['S'], cnt['K'], cnt['O']);
    printf("T %d, P %d, M %d, B %d, E %d casos; %d comprobaciones, %d fallos\n", cnt['T'], cnt['P'], cnt['M'], cnt['B'], cnt['E'], total, fails);
    return fails != 0;
}
