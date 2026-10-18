/* Vuelo (PROVISIONAL): vista espacial con polvo estelar, panel de instrumentos basico, controles de velocidad,
   cambio de vista, hiperespacio y atraque. Todavia no hay MVEIT, TACTICS, STARS ni DIALS del original:
   el polvo es un sustituto simple y el atraque no exige llegar a la estacion. */
#ifndef FLIGHT_H
#define FLIGHT_H
#define FL_DOCK 1                    /* el piloto ha atracado */
#define FL_QUIT 2                    /* salida de emergencia */
#define FL_DEATH 3                   /* hemos muerto (DEATH): se vuelve al titulo (DEATH2) */
extern int flight_testkeys;          /* pruebas: N (nave), K (explotar) y C (atraque sin ordenador) */
int flight_run(void);
void misjump(void);                  /* MJP: salto fallido al espacio entre sistemas (para las pruebas) */
#endif
