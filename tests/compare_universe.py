# Compara el volcado del oraculo (TXTELITE.C 1.5) con el de universe.c.
# Sistemas (S), descripciones (D) y mercados (M): deben ser identicos.
# Distancias (T): universe.c sigue el algoritmo del 6502; se comprueba contra una version independiente en Python
# y se informa de cuanto se aparta el calculo en coma flotante de Text Elite.
import math,sys,collections
o=open(sys.argv[1]).read().split('\n'); u=open(sys.argv[2]).read().split('\n')
ok=True
for t,name in (('S','sistemas'),('D','descripciones'),('M','mercados')):
    a=[l for l in o if l.startswith(t+' ')]; b=[l for l in u if l.startswith(t+' ')]
    bad=sum(1 for x,y in zip(a,b) if x!=y)+abs(len(a)-len(b))
    print('%-14s %6d registros, %d diferencias'%(name,len(a),bad)); ok&=(bad==0)
sysd={}
for l in o:
    if l.startswith('S '):
        p=l.split(); sysd[(int(p[1]),int(p[2]))]=(int(p[4]),int(p[5]))
def d6502(a,b):
    dx=abs(a[0]-b[0]); dy=abs(a[1]-b[1])>>1
    s=dx*dx+dy*dy
    if s>65535: s=255*256+(s&255)
    return 4*math.isqrt(s)
to=[l.split() for l in o if l.startswith('T ')]; tu=[l.split() for l in u if l.startswith('T ')]
bad=0; eq=0; mx=0
for a,b in zip(to,tu):
    g,i,j=int(a[1]),int(a[2]),int(a[3]); do=int(a[4]); du=int(b[4])
    if du!=d6502(sysd[(g,i)],sysd[(g,j)]): bad+=1
    if do==du: eq+=1
    mx=max(mx,abs(do-du))
print('%-14s %6d pares, %d diferencias con el algoritmo 6502 independiente'%('distancias',len(tu),bad)); ok&=(bad==0)
print('  (informativo) Text Elite en coma flotante coincide con el 6502 en %.1f%% de los pares; diferencia maxima %d'%(100*eq/len(to),mx))
print('OK' if ok else 'FALLO'); sys.exit(0 if ok else 1)
