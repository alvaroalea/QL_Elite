/* Saving and loading commanders to a file (QDOS: flp1_, mdv1_...; PC: current directory or the one given).
   Own format (94 bytes), with the original's 77-byte block and its checksums:
     0..3   "ELQL"      4   version (1)      5..8  encryption seed      9..16  name (8 bytes, padded with 0)
     17..93 commander block (77 bytes)
   From byte 9 everything is encrypted with XOR against a sequence generated with the seed (the same style as MUTILATE:
   it only prevents editing by eye, it is not security). */
#ifndef SAVE_H
#define SAVE_H
#include "platform.h"

#define SAVE_FILE_SIZE 94

#define SAVE_OK        0
#define SAVE_E_OPEN   (-1)       /* could not be opened (does not exist, disk not ready, protected...) */
#define SAVE_E_IO     (-2)       /* read or write error (disk full...) */
#define SAVE_E_FORMAT (-3)       /* not an Elite QL file */
#define SAVE_E_CHECK  (-4)       /* the checksums do not match */
#define SAVE_E_NAME   (-5)       /* empty name */

#define SAVE_DEV_MAX 31
extern char save_dev[SAVE_DEV_MAX + 1];        /* device prefix: "flp1_" on the QL, "" on the PC */
extern u8   save_seed;           /* encryption seed for the next save (the game sets a random byte) */

void save_set_device(const char *dev);
int  save_valid_device(const char *dev);           /* 1 if it is AAAn_[text_]: 3 letters, digit, _ and ends in _ */
int  save_next_device(void);                       /* cycles flp1_ -> flp2_ -> mdv1_ -> mdv2_ ; returns the new index */
void save_filename(const char *name, char *out);   /* out of at least 48 bytes */
int  save_encode(const u8 *blk, const char *name, u8 *file);          /* block (already sealed) -> 94 bytes */
int  save_decode(const u8 *file, char *name, u8 *blk);                /* 94 bytes -> name + block; SAVE_E_FORMAT if not Elite's */
int  save_commander(void);                         /* saves the current commander: svc/2, seals and writes */
int  load_commander(const char *name);             /* reads, validates (CHK, CHK3) and applies */
const char *save_errstr(int err);
#endif
