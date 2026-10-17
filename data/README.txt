elite_dash  Fondo del panel de instrumentos (tira de 256 x 56 pixeles en modo 8) para cargar en el QL.
            Copiar al disco o microdrive con el nombre  elite_dash  (flp1_elite_dash, mdv1_elite_dash...): el juego
            lo busca en la unidad seleccionada en el menu de disco (por defecto flp1_), igual que las partidas.
            Sin el fichero el juego dibuja un panel minimo.
            Se regenera con  make dash_file  (python3 tools/gen_dashboard.py) a partir de A.SCREEN.bin del Apple II
            (variable ELITE_SRC, ver tools/README_ref.txt). Formato: 'E','L','Q','D', 1, 56, 0, 136 y 56 lineas de 128 bytes.
