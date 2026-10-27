/* Commander block (77 bytes, bytes #0..#76 of the original) and its three checksums.
   Layout and algorithms of CHECK / CHECK2 / SVE / DFAULT of Apple II Elite; the code is our own. */
#ifndef CMDR_H
#define CMDR_H
#include "platform.h"

#define CMDR_SIZE    77            /* bytes #0..#76: TP ... CHK */
#define CMDR_NAMELEN 7             /* up to 7 letters, like the DFS name limit */
#define CMDR_NT      76            /* NT% of the original = SVC + 3 - TP */

/* offsets inside the block */
#define CB_TP     0
#define CB_QQ0    1
#define CB_QQ1    2
#define CB_SEED   3                /* 6 bytes */
#define CB_CASH   9                /* 4 bytes, big-endian */
#define CB_FUEL   13
#define CB_COK    14
#define CB_GCNT   15
#define CB_LASER  16               /* 4 bytes: front, rear, left, right */
#define CB_CRGO   22
#define CB_HOLD   23               /* 17 bytes */
#define CB_ECM    40
#define CB_BST    41
#define CB_BOMB   42
#define CB_ENGY   43
#define CB_DKCMP  44
#define CB_GHYP   45
#define CB_ESCP   46
#define CB_NOMSL  51
#define CB_FIST   52
#define CB_AVL    53               /* 17 bytes */
#define CB_QQ26   70
#define CB_TALLY  71               /* 2 bytes, low byte first */
#define CB_SVC    73
#define CB_CHK2   74
#define CB_CHK3   75
#define CB_CHK    76

/* weapon values (POW etc. of the original) */
#define LASER_POW    15            /* pulse laser */
#define LASER_BEAM   (128 + 15)    /* beam laser */
#define LASER_MIL    151           /* military: INT(128.5 + 1.5 * POW) */
#define LASER_MINING 50

extern char cmdr_name[CMDR_NAMELEN + 1];     /* current name, 0-terminated */

u8   cmdr_check(const u8 *blk);              /* CHECK: main checksum (bytes #0..#73) */
u8   cmdr_check2(const u8 *blk);             /* CHECK2: third sum (Apple II / C64) */
void cmdr_default(void);                     /* JAMESON: the default commander, at Lave; calls cmdr_unpack with the factory block */
void cmdr_pack(u8 *blk);                     /* game variables -> block (without sums: set them with cmdr_seal) */
void cmdr_seal(u8 *blk);                     /* computes CHK3, CHK and CHK2 = CHK ^ $A9 (like SVE) */
/* block -> game variables. Returns 0 if valid, -1 if CHK or CHK3 do not match (touches nothing).
   If only CHK2 fails, cok bit 7 is set (tampered data), like DFAULT; cok bit 3 is always set. */
int  cmdr_unpack(const u8 *blk);
int  cmdr_set_name(const char *s);           /* cleans and trims the name (letters, digits, '_' and '-'); returns the length, 0 if it ends up empty */
#endif
