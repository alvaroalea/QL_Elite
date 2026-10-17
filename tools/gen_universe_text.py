# Extrae la tabla de descripciones ("goat soup") de TXTELITE.C 1.5 (Ian Bell) a src/universe_text.h, sin retocar las cadenas.
import re
src=open('tests/oracle/TXTELITE.C',encoding='latin1').read().replace('\r','')
a=src.index('static struct desc_choice desc_list[] =')
b=src.index('};',a)
body=src[a:b].split('\n',1)[1]
open('src/universe_text.h','w').write("""/* Generado por tools/gen_universe_text.py desde TXTELITE.C 1.5 (C) Ian Bell: tabla de descripciones de planetas
   (adaptada por Bell de las fuentes de Christian Pinder). Tokens: 0x81..0xA4 = elegir opcion, 0xB0..0xB2 = nombres. */
static const char *uni_desc_list[][5] =
"""+body+"};\n")
print(src[a:b].count('{"'),'entradas')
