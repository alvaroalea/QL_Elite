# Genera src/sne_table.h: tabla de senos del original (formula de elite-data.asm), i = 0..31
import math
sne=[255 if abs(math.sin(i/64*2*math.pi))>=1 else int(256*abs(math.sin(i/64*2*math.pi))+0.5) for i in range(32)]
open('src/sne_table.h','w').write("/* Generado por tools/gen_sne.py: |sin(i/64 * 2*pi)| * 256, i = 0..31 */\nstatic const u8 em_sne[32] = {\n    "+",".join("0x%02X"%x for x in sne[:16])+",\n    "+",".join("0x%02X"%x for x in sne[16:])+"\n};\n")
