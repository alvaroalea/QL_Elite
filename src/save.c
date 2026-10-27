#include <stdio.h>
#include "save.h"
#include "cmdr.h"
#include "state.h"

#ifdef __GNUC__
char save_dev[SAVE_DEV_MAX + 1] = "";
#else
char save_dev[SAVE_DEV_MAX + 1] = "flp1_";
#endif
u8 save_seed = 0x5A;

static const char *devs[4] = { "flp1_", "flp2_", "mdv1_", "mdv2_" };
static int dev_idx = 0;

static void scopy(char *d, const char *s, int max)
{
    int n = 0;
    while (s[n] && n < max - 1) { d[n] = s[n]; n++; }
    d[n] = 0;
}

void save_set_device(const char *dev)
{
    int i;
    scopy(save_dev, dev, SAVE_DEV_MAX + 1);
    for (i = 0; i < 4; i++)                        /* save_next_device continues from the chosen drive */
        if (save_dev[0] == devs[i][0] && save_dev[1] == devs[i][1] && save_dev[2] == devs[i][2] && save_dev[3] == devs[i][3] && !save_dev[5])
            dev_idx = i;
}

/* 3 letters, 1 digit, '_' and then anything (letters, digits, '_'), ending in '_' */
int save_valid_device(const char *d)
{
    int i, n = 0;
    while (d[n]) n++;
    if (n < 5 || n > SAVE_DEV_MAX || d[n - 1] != '_') return 0;
    for (i = 0; i < 3; i++)
        if (!((d[i] >= 'A' && d[i] <= 'Z') || (d[i] >= 'a' && d[i] <= 'z'))) return 0;
    if (d[3] < '0' || d[3] > '9' || d[4] != '_') return 0;
    for (i = 5; i < n; i++)
        if (!((d[i] >= 'A' && d[i] <= 'Z') || (d[i] >= 'a' && d[i] <= 'z') || (d[i] >= '0' && d[i] <= '9') || d[i] == '_')) return 0;
    return 1;
}

int save_next_device(void)
{
    dev_idx = (dev_idx + 1) & 3;
    scopy(save_dev, devs[dev_idx], SAVE_DEV_MAX + 1);
    return dev_idx;
}

/* file name: <device>elite_<name in lower case> */
void save_filename(const char *name, char *out)
{
    int n = 0, i = 0;
    const char *p = "elite_";
    while (save_dev[i] && n < SAVE_DEV_MAX) out[n++] = save_dev[i++];
    while (*p) out[n++] = *p++;
    for (i = 0; name[i] && i < CMDR_NAMELEN; i++) {
        char ch = name[i];
        if (ch >= 'A' && ch <= 'Z') ch = (char)(ch - 'A' + 'a');
        out[n++] = ch;
    }
    out[n] = 0;
}

/* encryption sequence: a 16-bit linear congruential generator from the seed; only its high byte is used */
static void cipher(u8 *p, int n, u8 seed)
{
    unsigned long s = 0x1234UL + (unsigned long)seed * 257UL;
    while (n-- > 0) {
        s = (s * 25173UL + 13849UL) & 0xFFFFUL;
        *p++ ^= (u8)(s >> 8);
    }
}

int save_encode(const u8 *blk, const char *name, u8 *f)
{
    int i, n = 0;
    if (!name[0]) return SAVE_E_NAME;
    f[0] = 'E'; f[1] = 'L'; f[2] = 'Q'; f[3] = 'L'; f[4] = 1;
    f[5] = save_seed; f[6] = (u8)(save_seed ^ 0xA5); f[7] = (u8)(save_seed + 0x3C); f[8] = (u8)(~save_seed);
    for (i = 0; i < 8; i++) f[9 + i] = 0;
    while (name[n] && n < CMDR_NAMELEN) { f[9 + n] = (u8)name[n]; n++; }
    for (i = 0; i < CMDR_SIZE; i++) f[17 + i] = blk[i];
    cipher(f + 9, 8 + CMDR_SIZE, f[5]);
    return SAVE_OK;
}

int save_decode(const u8 *file, char *name, u8 *blk)
{
    u8 f[SAVE_FILE_SIZE];
    int i;
    for (i = 0; i < SAVE_FILE_SIZE; i++) f[i] = file[i];
    if (f[0] != 'E' || f[1] != 'L' || f[2] != 'Q' || f[3] != 'L' || f[4] != 1) return SAVE_E_FORMAT;
    if (f[6] != (u8)(f[5] ^ 0xA5) || f[7] != (u8)(f[5] + 0x3C) || f[8] != (u8)(~f[5])) return SAVE_E_FORMAT;
    cipher(f + 9, 8 + CMDR_SIZE, f[5]);
    for (i = 0; i < CMDR_NAMELEN; i++) name[i] = (char)f[9 + i];
    name[CMDR_NAMELEN] = 0;
    for (i = 0; i < CMDR_SIZE; i++) blk[i] = f[17 + i];
    return SAVE_OK;
}

static int write_file(const char *fn, const u8 *f)
{
    FILE *fp = fopen(fn, "wb");
    int ok;
    if (!fp) return SAVE_E_OPEN;
    ok = (fwrite(f, 1, SAVE_FILE_SIZE, fp) == SAVE_FILE_SIZE);
    if (fclose(fp) != 0) ok = 0;
    return ok ? SAVE_OK : SAVE_E_IO;
}

int save_commander(void)
{
    u8 blk[CMDR_SIZE], f[SAVE_FILE_SIZE];
    char fn[48];
    int r;
    if (!cmdr_name[0]) return SAVE_E_NAME;
    svc >>= 1;                                   /* LSR SVC of the original */
    cmdr_pack(blk);
    cmdr_seal(blk);
    r = save_encode(blk, cmdr_name, f);
    if (r != SAVE_OK) return r;
    save_filename(cmdr_name, fn);
    return write_file(fn, f);
}

int load_commander(const char *name)
{
    u8 f[SAVE_FILE_SIZE], blk[CMDR_SIZE];
    char fn[48], nm[CMDR_NAMELEN + 1];
    FILE *fp;
    long got;
    int r;
    if (!name[0]) return SAVE_E_NAME;
    save_filename(name, fn);
    fp = fopen(fn, "rb");
    if (!fp) return SAVE_E_OPEN;
    got = (long)fread(f, 1, SAVE_FILE_SIZE, fp);
    fclose(fp);
    if (got != SAVE_FILE_SIZE) return got < 4 ? SAVE_E_FORMAT : SAVE_E_IO;
    r = save_decode(f, nm, blk);
    if (r != SAVE_OK) return r;
    if (cmdr_unpack(blk) != 0) return SAVE_E_CHECK;
    cmdr_set_name(nm);
    return SAVE_OK;
}

const char *save_errstr(int err)
{
    switch (err) {
    case SAVE_OK:       return "OK";
    case SAVE_E_OPEN:   return "CANNOT OPEN FILE";
    case SAVE_E_IO:     return "DISK ERROR";
    case SAVE_E_FORMAT: return "NOT AN ELITE FILE";
    case SAVE_E_CHECK:  return "FILE CORRUPTED";
    case SAVE_E_NAME:   return "NO NAME";
    }
    return "ERROR";
}
