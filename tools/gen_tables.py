# Genera src/math_tables.h: tablas log/logL/alogh con las mismas formulas que el original (tablas matematicas).
import math
log=[0]+[int(0x2000*math.log(i)/math.log(2)+0.5) for i in range(1,256)]
alog=[int(2**((i/2+128)/16)+0.5) for i in range(256)]
def arr(name,v): return "static const u8 %s[256] = {\n%s\n};\n"%(name,',\n'.join('    '+','.join('0x%02X'%x for x in v[i:i+16]) for i in range(0,256,16)))
open('src/math_tables.h','w').write("/* Generado por tools/gen_tables.py */\n"+arr('em_log',[x>>8 for x in log])+arr('em_logL',[x&255 for x in log])+arr('em_alogh',[x>>8 for x in alog]))
