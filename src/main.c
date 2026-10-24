/* Elite QL: punto de entrada. */
#include "config.h"
#include "platform.h"
#include "text.h"
#include "input.h"
#include "sound.h"
#include "app.h"
#include "dash.h"
#include "save.h"

/* dir terminado en '_' (QDOS) en out; 1 si es un dispositivo valido (en el PC vale cualquier prefijo, p. ej. "data/") */
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

/* Unidad del juego (panel y comandantes): 1) el argumento, EXEC_W flp1_elite;"flp1_" (Toolkit II, SMSQ/E);
   2) si no hay, donde este elite_dash: DATA_USE y PROG_USE de Toolkit II y luego las unidades habituales.
   Si no aparece, flp1_ (el panel se dibuja sin el fondo). Lee el panel de paso. */
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
    in_init();
    config_apply();                                 /* joystick CTL1 (cursores y espacio) siempre seleccionado */
    snd_init();
    find_drive(argc, argv);                           /* lo primero: busca la unidad y lee el panel una sola vez (sin mensajes); se queda en memoria */
    game_run();
    snd_off();
    plat_shutdown();
    plat_exit();                                      /* QL: sin "Press a key to exit" */
    return 0;
}
