# QL_Elite

[![English](https://img.shields.io/badge/Language-English-blue)](README.eng.md)
[![Español](https://img.shields.io/badge/Language-Español-red)](README.md)


Port de **Elite (versión Apple II, Firebird 1985)** al **Sinclair QL** (68008; modo 8: 256 × 256 con 8 colores; opcionalmente título y vuelo en modo 4: 512 × 256 con rojo, verde y blanco, desde las opciones de la pausa), escrito en C89 con
XTC68 (`qcc`) y un poco de ensamblador. Es un port fiel: usa la aritmética de enteros de 8 bits del original y cada rutina se ha
comparado, cuando se ha podido, con el código 6502 original en un emulador (estado en `docs/INVENTARIO.md`).

El juego original es de Ian Bell y David Braben. El código de referencia es el de Mark Moxon (`elite-source-code-apple-ii`), que se
usa solo para consultar y verificar; el código del port es propio.

![Captura 1](screenshot_1.png)
![Captura 2](screenshot_2.png)

## Cómo ejecutarlo en el QL (o en un emulador)

1. Generar la imagen de disco: `make elite.img` (`build/elite.img`, disco DS/DD de 720 KB con `elite`, `boot`, `elite_scr` y `elite_dash`).
2. Montar `build/elite.img` como `flp1_` en el emulador (sQLux, QPC2, uQLx...) o copiarla a un disco real, y arrancar con
   `LRUN flp1_boot`. Para instalarlo en otra unidad o directorio, copiar los cuatro ficheros y cambiar `dev$` en `boot`; el juego busca sus datos en `flp1_`, `flp2_`, `win1_`, `mdv1_`, `mdv2_` (y en `DATA_USE`/`PROG_USE` con Toolkit II, o en la unidad que se le pase con `EXEC_W dev$&"elite";dev$`).
3. Las partidas se guardan en la misma unidad como `elite_<nombre>` (menú de disco, tecla `I` estando atracado).

Sin el fichero `elite_dash` el juego dibuja un panel mínimo y avisa con «NO ELITE_DASH FILE».

## Cómo compilarlo

- Dependencias: `python3` (con `Pillow` para algunas herramientas), `gcc`, `make`, `git`, `meson` y `ninja`.
- Compilador del QL: `tools/install_xtc68.sh` instala XTC68 y el runtime c68 en `/tmp/qlsdk`. Después, `make elite`
  produce el ejecutable QDOS `build/elite`.
- Versión de PC con la pantalla en memoria: `make host_game`.
- Pruebas: `make test_all` (no necesita nada más). Las que comparan con el 6502 original (`make test_orig`, `test_move`, `test_dash`,
  `test_arith`, `test_flight`...) necesitan la fuente de Moxon:
  `git clone https://github.com/markmoxon/elite-source-code-apple-ii ref/moxon-appleII` (ruta por defecto; si está en otro sitio,
  `export ELITE_SRC=<ruta>/1-source-files/main-sources`).
- La documentación del proyecto está en `CLAUDE.md` (memoria completa) e `docs/INVENTARIO.md` (estado de las 528 rutinas del original).

## Teclas

### Pantalla de título

| Tecla | Acción |
|---|---|
| `1` | Nuevo comandante (Jameson) |
| `2` | Cargar comandante |
| `Q` | Salir |

### Pantallas (iguales en vuelo y atracado)

| Tecla | Pantalla |
|---|---|
| `5` | Carta galáctica |
| `6` | Carta de corto alcance |
| `7` | Datos del sistema |
| `8` | Precios del mercado |
| `9` | Estado (Status) |
| `0` | Inventario |

`ESC` vuelve de una pantalla al estado (atracado) o a la vista del espacio (en vuelo).

### En vuelo

| Tecla | Acción |
|---|---|
| Cursores | Alabeo (izquierda/derecha) y cabeceo (arriba = picar, abajo = subir) |
| `F` / `D` | Acelerar / frenar |
| `A` o espacio | Disparar el láser (mantener pulsado) |
| `1` `2` `3` `4` | Vista frontal, trasera, izquierda, derecha |
| `T` / `M` / `U` | Armar misil / disparar misil / desarmar misil |
| `E` | ECM |
| `B` | Bomba de energía |
| `J` | Salto dentro del sistema |
| `H` | Hiperespacio (hace falta un sistema objetivo al alcance) |
| `G` | Salto galáctico |
| `C` / `P` | Encender / apagar el ordenador de atraque (si se lleva) |
| `ESC` | Cápsula de escape (si se lleva) |
| `=` | Pausa (muestra «PAUSED» en pantalla; `-` sigue) |

También se atraca entrando por la ranura de la estación.

Con el juego en **pausa** (`=`): `D` amortiguación, `A` recentrado del teclado, `X` autores y salto fallido a mano (con `TAB`
durante el hiperespacio), `F` barras parpadeantes, `Y` eje Y del joystick, `J` ejes del joystick, `K` joystick (CTL1, cursores y
espacio), `U` solo mayúsculas en los textos de las misiones, `T` cinta/disco, `Q` / `S` sonido apagado / encendido, `-` sigue
jugando y `ESC` vuelve al título.

### Atracado

| Tecla | Acción |
|---|---|
| `1` | Comprar mercancías |
| `2` | Vender mercancías |
| `3` | Equipo (comprar equipo y láseres) |
| `4` | No hace nada |
| `5`–`0` | Pantallas (ver arriba) |
| `U` | Despegar |
| `I` | Menú de disco (guardar y cargar comandante) |
| `H` / `G` | Avisan «DOCKED» (no se puede saltar atracado) |

### Mercado y equipo

Cursores arriba/abajo para elegir artículo, cifras para la cantidad, `M` para el máximo, `ENTER` para confirmar y `ESC` para salir.

### Cartas

Los cursores mueven la mira; `F` busca un sistema por su nombre (solo atracado); `D` selecciona el sistema más cercano a la mira; `O`
vuelve al sistema actual.

### Teclado del QL

Los cursores y el espacio equivalen al joystick CTL1. En un emulador, `Mayúsculas + ESC` es la salida de emergencia. El teclado se lee
por fila (`KEYROW`), por lo que tres teclas pulsadas en un rectángulo de la matriz pueden dar una fantasma.

## Datos del juego (`data/`)

- `elite_dash`: fondo del panel de instrumentos (256 × 56 en modo 8; en el vuelo se convierte a modo 4: rojo y verde se conservan, el resto sale blanco). Se puede editar en el PC: `make dash_png` lo exporta a
  `asset/cabina.png`, se edita con los 8 colores del QL y `make dash_from_png` lo vuelve a generar. Más detalles en `data/README.txt`.
- `boot`: programa de arranque de SuperBASIC. `elite_scr`: pantalla de carga.

## Créditos

Elite: Ian Bell y David Braben. Código documentado de referencia: Mark Moxon. Port al QL: A. Alea y Claude, 2026.
