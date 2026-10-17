elite_dash  Fondo del panel de instrumentos (tira de 256 x 56 pixeles en modo 8) para cargar en el QL.
            Copiar al disco o microdrive con el nombre  elite_dash  (flp1_elite_dash, mdv1_elite_dash...): el juego
            lo busca en la unidad seleccionada en el menu de disco (por defecto flp1_), igual que las partidas.
            Sin el fichero el juego dibuja un panel minimo.
            Se regenera con  make dash_file  (python3 tools/gen_dashboard.py) a partir de A.SCREEN.bin del Apple II
            (variable ELITE_SRC, ver tools/README_ref.txt). Formato: 'E','L','Q','D', 1, 56, 0, 136 y 56 lineas de 128 bytes.
            Para editar la cabina en un programa de dibujo del PC:
              make dash_png        exporta data/elite_dash a data/cabina.png (1024 x 112: cada pixel del QL es un
                                   bloque de 4 x 2, con la proporcion del modo 8)
              (editar data/cabina.png con los 8 colores del QL: negro, azul, rojo, magenta, verde, cian, amarillo, blanco)
              make dash_from_png   vuelve a generar data/elite_dash
            Tambien directamente: python3 tools/png2dash.py imagen.png [salida] (cualquier tamano multiplo de 256 x 56)
            y python3 tools/png2dash.py --export [elite_dash] [imagen.png] [--scale K] (K = 0: 256 x 56; K: 512K x 56K).
            El juego dibuja encima las barras, la brujula, el radar, los misiles y las bombillas: conviene respetar esas
            zonas de la plantilla.
