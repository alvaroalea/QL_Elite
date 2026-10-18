/* JAMESON (NA2%): el comandante de fabrica del original (tools/gen_jameson.py) contra cmdr_default + cmdr_pack + cmdr_seal. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "platform.h"
#include "cmdr.h"

int main(int argc, char **argv)
{
    FILE *f = fopen(argc > 1 ? argv[1] : "tests/vectors_jameson.txt", "r");
    char line[400];
    u8 want[97], b[CMDR_SIZE];
    int i, bad = 0;
    if (!f || !fgets(line, sizeof line, f)) { printf("sin vectores\n"); return 2; }
    for (i = 0; i < 97; i++) { char t[3]; t[0] = line[2 * i]; t[1] = line[2 * i + 1]; t[2] = 0; want[i] = (u8)strtol(t, 0, 16); }
    cmdr_default();
    cmdr_pack(b);
    if (strcmp(cmdr_name, "JAMESON")) { printf("nombre: %s\n", cmdr_name); bad++; }
    for (i = 0; i < 74; i++) {                       /* COK: DFAULT (cmdr_unpack) le pone el bit 3 */
        u8 w = want[8 + i];
        if (i == 14) w = (u8)(w | 0x08);
        if (b[i] != w) { printf("byte %d: port %02x original %02x\n", i, b[i], w); bad++; }
    }
    /* las tres sumas de comprobacion de fabrica (CHK2 = CHK EOR $A9, CHK3, CHK) son las del bloque original */
    if (cmdr_check(want + 8) != want[8 + 76]) { printf("CHK: %02x original %02x\n", cmdr_check(want + 8), want[8 + 76]); bad++; }
    if (cmdr_check2(want + 8) != want[8 + 75]) { printf("CHK3: %02x original %02x\n", cmdr_check2(want + 8), want[8 + 75]); bad++; }
    if ((u8)(want[8 + 76] ^ 0xA9) != want[8 + 74]) { printf("CHK2\n"); bad++; }
    printf("  JAMESON: %d diferencias\n", bad);
    return bad ? 1 : 0;
}
