/* Compares trade_room / trade_buy / trade_sell with the ORIGINAL code (tnpr, TT219, TT208, GCASH, LCASH, MCASH) run
   in the 6502 emulator: vectors from tests/vectors_trade.txt (tools/gen_trade.py).
   "Reachable" domain (field 1): states that occur when playing (valid hold, quantity <= stock/hold). */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "game.h"
#include "state.h"

static int hexv(char c) { return c <= '9' ? c - '0' : c - 'a' + 10; }
static void unhex(const char *s, u8 *d, int n) { int i; for (i = 0; i < n; i++) d[i] = (u8)(hexv(s[2 * i]) * 16 + hexv(s[2 * i + 1])); }

int main(void)
{
    FILE *f = fopen("tests/vectors_trade.txt", "r");
    char line[400], hold[40], hold2[40], cashs[16], cash2[16];
    int reach, g, qty, avl_, price, trib, crgo_, carry, a, ok, y, x;
    unsigned long c0, c1;
    u8 h[17], eh[17];
    long nN = 0, nB = 0, nS = 0;
    long badB_gnum = 0, badN[2] = {0, 0}, badB[2] = {0, 0}, badS_noover = 0, badS_over = 0, sat = 0, over_cases = 0, badS_unreach = 0;
    int i;
    if (!f) { printf("falta tests/vectors_trade.txt (python3 tools/gen_trade.py)\n"); return 2; }
    game_new();
    while (fgets(line, sizeof line, f)) {
        if (line[0] == 'N') {
            sscanf(line, "N %d %d %39s %d %d %d %d %d", &reach, &g, hold, &trib, &crgo_, &qty, &carry, &a);
            unhex(hold, h, 17);
            memcpy(qq20, h, 17); tribble[1] = (u8)trib; crgo = (u8)crgo_;
            ok = trade_fits(g, (unsigned)qty);
            nN++;
            if (qty != 0 && (carry == 0) != ok) badN[reach]++;     /* qty 0 is never bought (TT219 jumps with BEQ) */
        } else if (line[0] == 'B') {
            char cx[4];
            sscanf(line, "B %d %d %d %d %d %39s %d %d %lu %3s %d %d %15s %39s %d", &reach, &g, &qty, &avl_, &price, hold, &trib, &crgo_, &c0, cx, &x, &y, cash2, hold2, &a);
            unhex(hold, h, 17); unhex(hold2, eh, 17);
            c1 = strtoul(cash2, 0, 16);
            memcpy(qq20, h, 17); tribble[1] = (u8)trib; crgo = (u8)crgo_;
            for (i = 0; i < NGOODS; i++) { mkt.price[i] = 0; mkt.quantity[i] = 0; avl[i] = 0; }
            mkt.price[g] = (unsigned)price * 4; mkt.quantity[g] = (unsigned)avl_; avl[g] = (u8)avl_;
            cash_set(c0);
            ok = trade_buy(g, (unsigned)qty);
            nB++;
            if (qty == 0) ok = TR_OK;
            {
                int bad = 0;
                if (x == 0 && ok != TR_OK) bad = 1;                                   /* the original bought, C did not */
                if (x == 1 && y == 206 && ok != TR_NOROOM) bad = 1;
                if (x == 1 && y == 197 && ok != TR_NOCASH) bad = 1;
                if (cash_get() != c1 || memcmp(qq20, eh, 17) != 0 || avl[g] != a) bad = 1;
                if (bad) { if (qty > avl_) badB_gnum++; else badB[reach]++; }     /* qty > stock: the original's gnum does not let it be requested */
            }
        } else if (line[0] == 'S') {
            sscanf(line, "S %d %d %d %d %39s %15s %15s %39s", &reach, &g, &qty, &price, hold, cashs, cash2, hold2);
            unhex(hold, h, 17); unhex(hold2, eh, 17);
            c0 = strtoul(cashs, 0, 16); c1 = strtoul(cash2, 0, 16);
            memcpy(qq20, h, 17);
            for (i = 0; i < NGOODS; i++) mkt.price[i] = 0;
            mkt.price[g] = (unsigned)price * 4;
            cash_set(c0);
            trade_sell(g, (unsigned)qty);
            nS++;
            if (cash_get() != c1 || memcmp(qq20, eh, 17) != 0) {
                unsigned long prod = (unsigned long)price * (unsigned long)qty * 4UL;
                if (qty > h[g]) badS_unreach++;                          /* qty > hold: gnum does not let it be requested */
                else if (c0 + prod > 0xFFFFFFFFUL) sat++;
                else badS_noover++;
            }
            if ((unsigned long)price * (unsigned long)qty * 4UL >= 65536UL && reach) over_cases++;   /* with TRADE_FAITHFUL they must also match */
        }
    }
    fclose(f);
    printf("tnpr  (trade_room): %ld casos; discrepancias alcanzables %ld, fuera de dominio %ld\n", nN, badN[1], badN[0]);
    printf("TT219 (trade_buy) : %ld casos; discrepancias con cantidad <= existencias %ld (alcanzables %ld); con cantidad > existencias (gnum lo impide) %ld\n", nB, badB[0] + badB[1], badB[1], badB_gnum);
    printf("TT208 (trade_sell): %ld casos; discrepancias: sin desbordamiento %ld, por importe de 16 bits %ld (de %ld casos con importe >= 65536: ya incluidos en las coincidencias),\n"
           "                    por saturar la caja %ld, con cantidad > bodega (gnum lo impide) %ld\n", nS, badS_noover, badS_over, over_cases, sat, badS_unreach);
    return (badN[0] || badN[1] || badB[0] || badB[1] || badS_noover) ? 1 : 0;
}
