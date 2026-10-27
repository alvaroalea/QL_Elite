/* Elite QL: entry point. */
#include "config.h"
#include "platform.h"
#include "text.h"
#include "input.h"
#include "sound.h"
#include "app.h"
#include "dash.h"
#include "save.h"

/* dir ending in '_' (QDOS) into out; 1 if it is a valid device (on the PC any prefix is valid, e.g. "data/") */
static int norm_dir(const char *dir, char *out)
{
    int n = 0;
    while (dir[n] && n < SAVE_DEV_MAX - 1) { out[n] = dir[n]; n++; }
    out[n] = 0;
#ifdef __GNUC__
    return 1;
#else
    if (n > 0 && out[n - 1] != '_') { out[n++] = '_'; out[n] = 0; }
    return save_valid_device(out);
#endif
}

static int try_dir(const char *dir)
{
    char d[SAVE_DEV_MAX + 1];
    if (!dir[0] || !norm_dir(dir, d)) return 0;
    save_set_device(d);
    return dash_load() != DASH_E_OPEN;
}

/* Game drive (panel and commanders): 1) the argument, EXEC_W flp1_elite;"flp1_" (Toolkit II, SMSQ/E);
   2) otherwise, where elite_dash is: Toolkit II DATA_USE and PROG_USE and then the usual drives.
   If it does not appear, flp1_ (the panel is drawn without the background). Reads the panel on the way. */
static void find_drive(int argc, char **argv)
{
    static const char *drives[] = { "flp1_", "flp2_", "win1_", "mdv1_", "mdv2_", 0 };
    char dir[SAVE_DEV_MAX + 1], first[SAVE_DEV_MAX + 1];
    int i;
    for (i = 0; i < SAVE_DEV_MAX + 1; i++) first[i] = save_dev[i];
    if (argc > 1 && norm_dir(argv[1], dir)) { save_set_device(dir); dash_load(); return; }
    for (i = 0; i < 2; i++)
        if (plat_default_dir(i, dir, SAVE_DEV_MAX + 1) && try_dir(dir)) return;
    if (try_dir(first)) return;
#ifndef __GNUC__
    for (i = 0; drives[i]; i++)
        if (try_dir(drives[i])) return;
#else
    (void)drives;
#endif
    save_set_device(first);
}

int main(int argc, char **argv)
{
    plat_init();
    txt_init();
    plat_mode(gfx_hires_opt);                       /* by default it starts in mode 8; with the 512 option (flight options menu), title and flight go in mode 4 (512 x 256, red, green and white) */
    in_init();
    config_apply();                                 /* CTL1 joystick (cursors and space) always selected */
    snd_init();
    find_drive(argc, argv);                           /* first thing: finds the drive and reads the panel just once (no messages); it stays in memory */
    game_run();
    snd_off();
    plat_shutdown();
    plat_exit();                                      /* QL: without "Press a key to exit" */
    return 0;
}
