/* Laser y danos contra el 6502 ORIGINAL (vectores de tools/gen_laser.py, que ejecuta la fuente en el emulador):
   HITCH, ANGRY, LASLI, disparo (MA68), MLOOP, recarga (MA77 + SHD + DENGY), danos (MA47..MA8 con SPIN, SFS1, EXNO2,
   ANGRY), recompensa (MA15..MAC1) y SFS1/SFS2. Ademas, pruebas propias: el pulso, el haz y que LASLI2 borra lo dibujado. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "combat.h"
#include "state.h"
#include "emath.h"
#include "ship3d.h"
#include "ships.h"
#include "space.h"
#include "shipdata.h"
#include "game.h"
#include "cmdr.h"

static int fails, total;
static char cur[2048];
static void check(int ok, const char *what)
{
    total++;
    if (!ok) { if (fails++ < 15 || getenv("ALLF")) printf("FALLO %s: %.*s\n", what, getenv("ALLF") ? 2000 : 150, cur); }
}

static int hexb(const char *s, u8 *out, int n)
{
    int i; unsigned v;
    for (i = 0; i < n; i++) { if (sscanf(s + 2 * i, "%2x", &v) != 1) return 0; out[i] = (u8)v; }
    return 1;
}

static u8 bp[32];
static u8 messes[16]; static int nmess;
static void mhook(u8 t) { if (nmess < 16) messes[nmess++] = t; }

static void slots_for_children(u8 type)
{
    ships_clear();
    frin[0] = 128; frin[1] = SHIPT_CORIOLIS; frin[2] = type;      /* los hijos van a las ranuras 3.. */
}

int main(void)
{
    FILE *f = fopen("tests/vectors_laser.txt", "r");
    char tok[16][700];
    int cnt[10];
    memset(cnt, 0, sizeof cnt);
    if (!f) { printf("falta tests/vectors_laser.txt (make test_laser con ELITE_SRC)\n"); return 1; }
    plat_init();
    state_clear();
    combat_mess_hook = mhook;
    while (fgets(cur, sizeof cur, f)) {
        int nt = 0; char *p = cur;
        while (nt < 16 && sscanf(p, "%699s", tok[nt]) == 1) { p = strstr(p, tok[nt]) + strlen(tok[nt]); nt++; }
        if (nt == 0) continue;
        switch (tok[0][0]) {
        case 'H': {
            u8 iw[9]; int st, ty, a1, a2, c;
            hexb(tok[1], iw, 9); st = atoi(tok[2]); ty = atoi(tok[3]); a1 = atoi(tok[4]); a2 = atoi(tok[5]); c = atoi(tok[6]);
            memset(inwk, 0, NI); memcpy(inwk, iw, 9); inwk[31] = (u8)st; ztype = (u8)ty;
            bp[1] = (u8)a1; bp[2] = (u8)a2; zxx0 = bp;
            check(hitch() == c, "HITCH"); cnt[0]++;
            break; }
        case 'A': {
            u8 sl[NI], ex[NI]; int ty = atoi(tok[1]), s36 = atoi(tok[3]), e36 = atoi(tok[5]);
            hexb(tok[2], sl, NI); hexb(tok[4], ex, NI);
            memcpy(state_slot(2), sl, NI); state_slot(1)[36] = (u8)s36; zinf = state_slot(2); ztype = (u8)ty;
            angry((u8)ty);
            check(memcmp(state_slot(2), ex, NI) == 0 && state_slot(1)[36] == e36, "ANGRY"); cnt[1]++;
            break; }
        case 'L': {
            u8 r[4], r2[4];
            hexb(tok[1], r, 4); hexb(tok[9], r2, 4);
            memcpy(em_rand, r, 4); em_rand_c = (u8)atoi(tok[2]); gntmp = (u8)atoi(tok[3]); energy = (u8)atoi(tok[4]);
            combat_view_on = 1;
            memset(screen, 0, 32768L);
            lasli();
            check(lasx == atoi(tok[5]) && lasy == atoi(tok[6]) && gntmp == atoi(tok[7]) && energy == atoi(tok[8]) &&
                  memcmp(em_rand, r2, 4) == 0, "LASLI");
            lasli2();
            { long i; int clean = 1; for (i = 0; i < 32768L; i++) if (screen[i]) { clean = 0; break; } check(clean, "LASLI2 borra"); }
            cnt[2]++;
            break; }
        case 'F': {
            u8 lz[4]; int g = atoi(tok[3]), called;
            hexb(tok[5], lz, 4); memcpy(laser, lz, 4);
            lasct = (u8)atoi(tok[1]); gntmp = (u8)g; las2 = (u8)atoi(tok[6]); las = 99; energy = 200;
            combat_view_on = 0;
            laser_fire((u8)atoi(tok[4]), (u8)(atoi(tok[2]) != 0));
            called = (gntmp != (u8)g);
            check(las == atoi(tok[7]) && las2 == atoi(tok[8]) && lasct == atoi(tok[9]) && called == atoi(tok[10]), "disparo MA68");
            cnt[3]++;
            break; }
        case 'M':
            gntmp = (u8)atoi(tok[1]); lasct = (u8)atoi(tok[2]);
            laser_cool();
            check(gntmp == atoi(tok[3]) && lasct == atoi(tok[4]), "MLOOP"); cnt[4]++;
            break;
        case 'E':
            energy = (u8)atoi(tok[1]); ash = (u8)atoi(tok[2]); fsh = (u8)atoi(tok[3]); engy = (u8)atoi(tok[4]);
            energy_tick((u8)atoi(tok[5]));
            check(energy == atoi(tok[6]) && ash == atoi(tok[7]) && fsh == atoi(tok[8]), "MA77 recarga"); cnt[5]++;
            break;
        case 'D': {
            /* D typ las e35 st newb ai b0 st36 rnd c tl t0 t1 | LAS slot_in 35 31 TALLYL TALLY TALLY+1 st36 0 slot_out log rnd_out */
            u8 sl[NI], so[NI], r[4], ro[4];
            int ty = atoi(tok[1]), st = atoi(tok[4]), destroyed, k, ok;
            char *q, *lg;
            /* hay mas de 16 campos: se vuelven a leer con sscanf sobre la linea */
            char a[30][700]; int na = 0; char *pp = cur;
            while (na < 30 && sscanf(pp, "%699s", a[na]) == 1) { pp = strstr(pp, a[na]) + strlen(a[na]); na++; }
            hexb(a[9], r, 4); hexb(a[15], sl, NI); hexb(a[23], so, NI); hexb(a[25], ro, 4); lg = a[24];
            slots_for_children((u8)ty);
            memcpy(state_slot(2), sl, NI); state_slot(1)[36] = (u8)atoi(a[8]);
            memset(bp, 0, sizeof bp); bp[0] = (u8)atoi(a[7]); bp[1] = 255; bp[2] = 255;
            zxx0 = bp; zinf = state_slot(2); ztype = (u8)ty;
            memset(inwk, 0, NI); inwk[35] = (u8)atoi(a[3]); inwk[31] = (u8)st; inwk[36] = (u8)atoi(a[5]);
            las = (u8)atoi(a[2]); tallyl = (u8)atoi(a[11]); tally[0] = (u8)atoi(a[12]); tally[1] = (u8)atoi(a[13]);
            memcpy(em_rand, r, 4); em_rand_c = (u8)atoi(a[10]); dly = 0; nmess = 0;
            combat_hit();
            destroyed = !(st & 0x80) && (inwk[31] & 0x80);
            ok = las == atoi(a[14]) && inwk[31] == atoi(a[17]) && tallyl == atoi(a[18]) && tally[0] == atoi(a[19]) &&
                 tally[1] == atoi(a[20]) && state_slot(1)[36] == atoi(a[21]) && memcmp(state_slot(2), so, NI) == 0 &&
                 memcmp(em_rand, ro, 4) == 0;
            if (!destroyed) ok = ok && inwk[35] == atoi(a[16]);
            /* hijos (NWSHP) en las ranuras 3..NOSH-1 y mensajes */
            k = 3; q = lg;
            {
                int nm = 0;
                while (q && *q && *q != '-') {
                    char nm_[8]; int v1, v2;
                    if (sscanf(q, "%7[^:]:%d:%d", nm_, &v1, &v2) != 3) break;
                    if (strcmp(nm_, "NWSHP") == 0) { if (k < NOSH) { ok = ok && frin[k] == v1; k++; } }
                    else if (strcmp(nm_, "MESS") == 0) { ok = ok && nm < nmess && messes[nm] == v1; nm++; }
                    q = strchr(q, ','); if (q) q++;
                }
                ok = ok && nm == nmess;
                if (k < NOSH) ok = ok && frin[k] == 0;
            }
            if (!ok && getenv("ALLF")) printf("  C: las %d 31 %d 35 %d tal %d %d %d st36 %d slot %d rnd %d frin3.. %d %d %d nmess %d\n", las, inwk[31], inwk[35],
                tallyl, tally[0], tally[1], state_slot(1)[36], memcmp(state_slot(2), so, NI) == 0, memcmp(em_rand, ro, 4) == 0, frin[3], frin[4], frin[5], nmess);
            check(ok, "danos MA47");
            cnt[6]++;
            break; }
        case 'K': {
            unsigned long cash_in, cash_out; int r;
            u8 *pc = cash;
            inwk[36] = (u8)atoi(tok[1]); inwk[31] = (u8)atoi(tok[2]); fist = (u8)atoi(tok[3]);
            dly = (u8)atoi(tok[4]); mj = (u8)atoi(tok[5]);
            memset(bp, 0, sizeof bp); bp[10] = (u8)atoi(tok[6]); bp[11] = (u8)atoi(tok[7]); zxx0 = bp; ztype = 11;
            sscanf(tok[8], "%lx", &cash_in); sscanf(tok[11], "%lx", &cash_out);
            pc[0] = (u8)(cash_in >> 24); pc[1] = (u8)(cash_in >> 16); pc[2] = (u8)(cash_in >> 8); pc[3] = (u8)cash_in;
            nmess = 0;
            r = combat_killed();
            check(r == atoi(tok[9]) && fist == atoi(tok[10]) && cash_get() == cash_out &&
                  ((tok[12][0] == '-' && nmess == 0) || (nmess == 1 && messes[0] == atoi(tok[12]))), "recompensa MA15");
            mj = 0;
            cnt[7]++;
            break; }
        case 'S': {
            u8 sl[NI], got[NI], r[4], ro[4], keep[NI];
            int par = atoi(tok[1]), ch = atoi(tok[2]), i, ok;
            hexb(tok[4], sl, NI); hexb(tok[5], r, 4); hexb(tok[8], got, NI); hexb(tok[9], ro, 4);
            slots_for_children((u8)par);
            memcpy(state_slot(2), sl, NI); zinf = state_slot(2); ztype = (u8)par;
            for (i = 0; i < NI; i++) keep[i] = inwk[i] = (u8)(i * 7 + 1);
            memcpy(em_rand, r, 4); em_rand_c = (u8)atoi(tok[6]);
            sfs1((u8)atoi(tok[3]), (u8)ch);
            ok = frin[3] == ch && atoi(tok[7]) == ch && memcmp(inwk, keep, NI) == 0 && memcmp(em_rand, ro, 4) == 0;
            for (i = 0; i < NI; i++) if (i != 31 && i != 33 && i != 34 && i != 35 && i != 36) ok = ok && state_slot(3)[i] == got[i];
            if (!ok && getenv("ALLF")) { printf("  C: frin3 %d rnd %02x%02x%02x%02x keep %d slot3 ", frin[3], em_rand[0], em_rand[1], em_rand[2], em_rand[3], memcmp(inwk, keep, NI) == 0);
                for (i = 0; i < NI; i++) printf("%02x", state_slot(3)[i]);
                printf("\n"); }
            check(ok, "SFS1");
            cnt[8]++;
            break; }
        }
    }
    fclose(f);
    printf("6502: HITCH %d, ANGRY %d, LASLI %d, disparo %d, MLOOP %d, recarga %d, danos %d, recompensa %d, SFS1 %d\n",
           cnt[0], cnt[1], cnt[2], cnt[3], cnt[4], cnt[5], cnt[6], cnt[7], cnt[8]);

    /* ---- pruebas propias: ritmo del pulso y del haz (el bucle de vuelo: disparo, naves, MA23, MLOOP) */
    {
        int fr, on = 0, shots = 0, vis = 0;
        memset(screen, 0, 32768L);
        combat_reset(); combat_view_on = 1;
        laser[0] = LASER_POW;
        for (fr = 0; fr < 20; fr++) {
            laser_fire(0, 1);
            if (las) shots++;
            laser_frame_end();
            on = (las2 != 0); vis += on;
            laser_cool();
        }
        strcpy(cur, "pulsos");
        check(shots == 4 && vis == 8, "pulsos: un disparo cada 5 fotogramas, visible 2");
        check(gntmp == 4 * 8 - 20, "pulsos: +8 por disparo, -1 por fotograma");
        laser_frame_end(); if (las2) { lasct = 0; laser_frame_end(); }
        combat_reset(); combat_view_on = 1; laser[0] = LASER_BEAM; shots = 0;
        memset(screen, 0, 32768L);
        for (fr = 0; fr < 20; fr++) { laser_fire(0, 1); if (las) shots++; laser_frame_end(); laser_cool(); }
        { long i; int clean = 1; for (i = 0; i < 32768L; i++) if (screen[i]) clean = 0;
          strcpy(cur, "haz"); check(shots == 20 && clean, "haz: dispara en cada fotograma y se borra en el mismo"); }
        check(gntmp == 20 * 7 && energy == 255 - 20, "haz: +8 -1 por fotograma de calor, -1 de energia por disparo");
        /* sobrecalentamiento: a partir de 242 no dispara */
        gntmp = 242; laser_fire(0, 1); check(las == 0, "a 242 no dispara");
        gntmp = 241; laser_fire(0, 1); check(las == 15, "a 241 dispara"); laser_frame_end();
        /* SHD gasta energia al cargar el escudo */
        energy = 200; check(shd(254) == 255 && energy == 199, "SHD carga y gasta 1");
        energy = 200; check(shd(255) == 255 && energy == 200, "SHD lleno no gasta");
    }
    /* ---- integracion: una Sidewinder quieta delante, laser de pulsos, bucle de vuelo hasta que explota y se quita */
    {
        int fr, n, alive = 1, frames = 0;
        unsigned long c0;
        u8 t0 = tally[0], t1 = tally[1], tl0 = tallyl;
        const u8 *bpw = SHIP_BP(SHIPT_SIDEWINDER);
        game_new();
        combat_reset(); combat_view_on = 1; dly = 0; mj = 0;
        laser[0] = LASER_POW;
        ships_arrive();
        ship_zinf(); inwk[7] = 3; inwk[27] = 0; inwk[29] = 0; inwk[30] = 0;      /* x = y = 0, z = 768 */
        n = ship_new(SHIPT_SIDEWINDER);
        state_slot(n)[I_AI] = 0;                                              /* sin IA (TACTICS aun no existe) */
        t0 = tally[0]; t1 = tally[1]; tl0 = tallyl; c0 = cash_get();
        zDELTA = 0; alpha = beta = 0;
        memset(screen, 0, 32768L);
        for (fr = 0; fr < 400 && alive; fr++) {
            laser_fire(0, 1);
            space_step();
            laser_frame_end();
            laser_cool();
            dly_tick();
            mcnt--;                                                           /* TT100: DEC MCNT (lo hace el bucle de vuelo) */
            alive = 0;
            for (n = 0; n < NOSH; n++) if (frin[n] == SHIPT_SIDEWINDER) alive = 1;
            frames++;
        }
        strcpy(cur, "integracion");
        check(!alive, "la Sidewinder acaba destruida y quitada");
        check(((unsigned)tally[1] << 16 | (unsigned)tally[0] << 8 | tallyl) == (((unsigned)t1 << 16 | (unsigned)t0 << 8 | tl0) + 85),
              "TALLY suma KWL% de la Sidewinder (85/256)");
        check(cash_get() == c0 + bpw[10] + 256UL * bpw[11], "recompensa del plano (bytes 10 y 11)");
        check(energy < 255, "los disparos gastan energia");
        printf("integracion: %d fotogramas hasta quitar la nave; recompensa %d decimas; energia %d, GNTMP %d\n",
               frames, bpw[10] + 256 * bpw[11], energy, gntmp);
    }
    printf("%d comprobaciones, %d fallos\n", total, fails);
    return fails != 0;
}
