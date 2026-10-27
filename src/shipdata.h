#ifndef SHIPDATA_H
#define SHIPDATA_H
#include "platform.h"
#define SHIP_TYPES 32
#define SHIP_BLOB_SIZE 7154L
/* ship_blob: all the blueprints; ship_off[type] (1..32): offset of the type's blueprint inside the block */
extern const u8 ship_blob[7154];
extern const unsigned short ship_off[SHIP_TYPES + 1];
#define SHIP_BP(t) (ship_blob + ship_off[t])
/* ship types (names from the source, without the SHIP_ prefix) */
#define SHIPT_MISSILE 1
#define SHIPT_CORIOLIS 2
#define SHIPT_ESCAPE_POD 3
#define SHIPT_PLATE 4
#define SHIPT_CANISTER 5
#define SHIPT_BOULDER 6
#define SHIPT_ASTEROID 7
#define SHIPT_SPLINTER 8
#define SHIPT_SHUTTLE 9
#define SHIPT_TRANSPORTER 10
#define SHIPT_COBRA_MK_3 11
#define SHIPT_PYTHON 12
#define SHIPT_BOA 13
#define SHIPT_ANACONDA 14
#define SHIPT_ROCK_HERMIT 15
#define SHIPT_VIPER 16
#define SHIPT_SIDEWINDER 17
#define SHIPT_MAMBA 18
#define SHIPT_KRAIT 19
#define SHIPT_ADDER 20
#define SHIPT_GECKO 21
#define SHIPT_COBRA_MK_1 22
#define SHIPT_WORM 23
#define SHIPT_COBRA_MK_3_P 24
#define SHIPT_ASP_MK_2 25
#define SHIPT_PYTHON_P 26
#define SHIPT_FER_DE_LANCE 27
#define SHIPT_MORAY 28
#define SHIPT_THARGOID 29
#define SHIPT_THARGON 30
#define SHIPT_CONSTRICTOR 31
#define SHIPT_DODO 32
#endif
