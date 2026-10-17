/* Pruebas de cmdr.c y save.c: sumas de comprobacion contra el 6502 literal, ida y vuelta por fichero, corrupcion. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmdr.h"
#include "save.h"
#include "state.h"

static int fails, total;
static void check(int ok, const char *what) { total++; if (!ok) { fails++; printf("FALLO: %s\n", what); } }

static int hexv(char c) { return c <= '9' ? c - '0' : c - 'a' + 10; }

static void snap(u8 *b) { cmdr_pack(b); }

int main(void)
{
    FILE *f;
    char line[400], nm[16], fn[64];
    u8 blk[CMDR_SIZE], b1[CMDR_SIZE], b2[CMDR_SIZE], file[SAVE_FILE_SIZE], tmp[SAVE_FILE_SIZE];
    int i, k, n = 0, r;
    unsigned chk, chk2;

    /* 1. CHECK y CHECK2 contra vectores del 6502 literal (incluye el comandante de fabrica: $03 y $27) */
    f = fopen("tests/vectors_chk.txt", "r");
    if (!f) { printf("falta tests/vectors_chk.txt (make test_save lo genera)\n"); return 2; }
    while (fgets(line, sizeof line, f)) {
        for (i = 0; i < CMDR_SIZE; i++) blk[i] = (u8)(hexv(line[2 * i]) * 16 + hexv(line[2 * i + 1]));
        sscanf(line + 2 * CMDR_SIZE, "%x %x", &chk, &chk2);
        check(cmdr_check(blk) == chk, "CHECK");
        check(cmdr_check2(blk) == chk2, "CHECK2");
        n++;
    }
    fclose(f);
    check(n > 3000, "vectores leidos");

    /* 2. el comandante por defecto sellado reproduce las tres constantes de la fuente */
    state_clear();
    cmdr_default();
    snap(b1); b1[CB_COK] = 0;                                  /* la fabrica lleva COK = 0; cok = 8 solo vive en memoria */
    cmdr_seal(b1);
    check(b1[CB_CHK] == 0x03 && b1[CB_CHK3] == 0x27 && b1[CB_CHK2] == 0xAA, "default: CHK=03 CHK3=27 CHK2=AA");
    check(cash[2] == 3 && cash[3] == 0xE8 && cash[0] == 0 && cash[1] == 0, "cash 100 Cr (1000 decimas, big-endian)");
    check(qq14 == 70 && laser[0] == LASER_POW && crgo == 22 && nomsl == 3 && svc == 128, "default: fuel, laser, bodega, misiles, svc");
    check(qq0 == 20 && qq1 == 173 && qq21[0] == 0x4A && qq21[5] == 0xB7, "default: Lave y semilla");
    check(avl[0] == 16 && avl[12] == 58 && avl[16] == 0, "default: mercado");
    check(strcmp(cmdr_name, "JAMESON") == 0, "default: nombre");
    check(cok == 0x08, "default: cok = 8 (CHK2 correcta)");

    /* 3. pack/unpack redondo con valores arbitrarios */
    srand(7);
    for (k = 0; k < 200; k++) {
        for (i = 0; i < CMDR_SIZE; i++) b1[i] = (u8)(rand() & 255);
        b1[CB_COK] &= 0x7F;
        cmdr_seal(b1);
        check(cmdr_unpack(b1) == 0, "unpack de bloque sellado");
        snap(b2);
        b1[CB_COK] |= 0x08;                                  /* DFAULT pone el bit 3 */
        for (i = 0; i <= CB_SVC; i++) {
            if (i >= 47 && i <= 50) continue;                /* bytes sin uso: no se guardan */
            if (i == 20 || i == 21) continue;
            if (b1[i] != b2[i]) { check(0, "pack(unpack(b)) == b"); break; }
        }
    }

    /* 4. CHK o CHK3 mal: rechazado y sin tocar el estado; solo CHK2 mal: aceptado con cok bit 7 */
    state_clear(); cmdr_default();
    snap(b1); cmdr_seal(b1);
    b1[CB_CHK] ^= 1; check(cmdr_unpack(b1) == -1, "CHK mal"); b1[CB_CHK] ^= 1;
    b1[CB_CHK3] ^= 0x80; check(cmdr_unpack(b1) == -1, "CHK3 mal"); b1[CB_CHK3] ^= 0x80;
    b1[CB_CHK2] ^= 4; r = cmdr_unpack(b1); check(r == 0 && (cok & 0x80), "CHK2 mal: acepta y marca cok"); b1[CB_CHK2] ^= 4;
    {   /* CHECK/CHECK2 del original NO detectan todos los cambios de un bit (cada byte se suma en un paso y se combina por XOR
           en el siguiente, y a veces se cancelan): se mide la tasa, y se exige que sea la del algoritmo, no mas baja */
        int det = 0, tot = 0;
        for (i = 0; i <= CB_SVC; i++) {
            for (k = 0; k < 8; k++) {
                u8 save1 = b1[i];
                b1[i] ^= (u8)(1 << k);
                tot++;
                if (cmdr_unpack(b1) != 0) det++;
                b1[i] = save1;
            }
        }
        printf("cambios de un bit detectados: %d de %d\n", det, tot);
        check(det * 100 / tot >= 50, "al menos la mitad de los cambios de un bit se detectan");
    }

    /* 5. nombres */
    check(cmdr_set_name("bob smith!") == 7 && strcmp(cmdr_name, "BOBSMIT") == 0, "nombre recortado a 7 y limpio");
    check(cmdr_set_name("  ") == 0 && cmdr_name[0] == 0, "nombre vacio");
    check(cmdr_set_name("a_b-9") == 5 && strcmp(cmdr_name, "A_B-9") == 0, "caracteres permitidos");

    /* 6. nombre de fichero */
    save_set_device("flp1_"); save_filename("JAMESON", fn);
    check(strcmp(fn, "flp1_elite_jameson") == 0, "nombre de fichero QL");
    save_set_device("tests/tmp/"); save_filename("BOB", fn);
    check(strcmp(fn, "tests/tmp/elite_bob") == 0, "nombre de fichero PC");
    save_set_device("");
    check(save_next_device() == 1 && strcmp(save_dev, "flp2_") == 0, "siguiente dispositivo");
    check(save_next_device() == 2 && strcmp(save_dev, "mdv1_") == 0, "siguiente dispositivo 2");
    save_next_device(); save_next_device();
    check(strcmp(save_dev, "flp1_") == 0, "los dispositivos dan la vuelta");
    save_set_device("tests/tmp/");

    /* 7. ida y vuelta por fichero */
    state_clear(); cmdr_default();
    cmdr_set_name("Cmdr7");
    cash[2] = 0x12; cash[3] = 0x34; qq14 = 33; qq20[3] = 5; qq20[12] = 200; ecm_fitted = 255; laser[1] = LASER_BEAM;
    tally[0] = 0x80; tally[1] = 2; qq0 = 99; qq1 = 7; svc = 128; tp = 3; fist = 17; gcnt = 2;
    save_seed = 0x9C;
    r = save_commander();
    check(r == SAVE_OK, "guardar");
    check(svc == 64, "svc se divide por 2 al guardar (LSR SVC)");
    snap(b1);
    state_clear(); cmdr_default();
    r = load_commander("CMDR7");
    check(r == SAVE_OK, "cargar");
    snap(b2);
    check(memcmp(b1, b2, CB_SVC + 1) == 0, "el estado cargado es identico al guardado");
    check(strcmp(cmdr_name, "CMDR7") == 0, "el nombre se carga");
    check(load_commander("cmdr7") == SAVE_OK, "nombre en minusculas");
    check(load_commander("NOEXISTE") == SAVE_E_OPEN, "fichero inexistente");
    check(load_commander("") == SAVE_E_NAME, "nombre vacio al cargar");
    cmdr_set_name("");
    check(save_commander() == SAVE_E_NAME, "no guarda sin nombre");
    cmdr_set_name("X");
    save_set_device("no_existe_dir/");
    check(save_commander() == SAVE_E_OPEN, "directorio inexistente");
    save_set_device("tests/tmp/");

    /* 8. el fichero esta cifrado y distintas semillas dan ficheros distintos */
    state_clear(); cmdr_default();
    cmdr_pack(b1); cmdr_seal(b1);
    save_seed = 1; save_encode(b1, "AB", file);
    save_seed = 2; save_encode(b1, "AB", tmp);
    check(memcmp(file + 9, tmp + 9, 85) != 0, "semillas distintas -> contenido distinto");
    check(memcmp(file + 17, b1, CMDR_SIZE) != 0, "el bloque no va en claro");
    check(save_decode(file, nm, b2) == SAVE_OK && memcmp(b1, b2, CMDR_SIZE) == 0 && strcmp(nm, "AB") == 0, "decode(encode(x)) == x");

    /* 9. corrupcion de un fichero: el cifrado no pierde ni inventa deteccion (el veredicto es el del bloque en claro) */
    {
        int bad_format = 0, mism = 0, kk;
        for (i = 0; i < SAVE_FILE_SIZE; i++) {
            for (kk = 0; kk < 8; kk++) {
                u8 sv[CMDR_SIZE], pl[CMDR_SIZE];
                int via_file, direct;
                memcpy(tmp, file, SAVE_FILE_SIZE);
                tmp[i] ^= (u8)(1 << kk);
                r = save_decode(tmp, nm, sv);
                if (r == SAVE_E_FORMAT) { bad_format++; continue; }
                if (i < 17) continue;                                             /* nombre: sin comprobacion */
                memcpy(pl, b1, CMDR_SIZE);
                pl[i - 17] ^= (u8)(1 << kk);
                via_file = cmdr_unpack(sv);
                direct = cmdr_unpack(pl);
                if (via_file != direct) mism++;
            }
        }
        printf("corrupcion de fichero: %d bits de cabecera rechazados por formato, %d discrepancias\n", bad_format, mism);
        check(mism == 0, "un bit cambiado en el fichero = el mismo bit en el bloque");
        check(bad_format >= 8 * 9 - 8, "cabecera protegida");
    }

    /* 10. ficheros truncados o ajenos */
    save_set_device("tests/tmp/");
    f = fopen("tests/tmp/elite_corto", "wb"); fwrite("ELQL", 1, 4, f); fclose(f);
    check(load_commander("CORTO") == SAVE_E_IO, "fichero truncado");
    f = fopen("tests/tmp/elite_basura", "wb"); for (i = 0; i < SAVE_FILE_SIZE; i++) fputc(i * 7, f); fclose(f);
    check(load_commander("BASURA") == SAVE_E_FORMAT, "fichero ajeno");
    f = fopen("tests/tmp/elite_vacio", "wb"); fclose(f);
    check(load_commander("VACIO") == SAVE_E_FORMAT, "fichero vacio");

    printf("%d comprobaciones, %d fallos\n", total, fails);
    return fails ? 1 : 0;
}
