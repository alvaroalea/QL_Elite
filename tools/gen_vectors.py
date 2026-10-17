# Genera tests/vectors.txt con la salida de la transcripcion literal de las rutinas (referencia).
import sys,random,math; sys.path.insert(0,'tools')
from ref6502 import *
exec(open('tools/gen_tables.py').read().split("def arr")[0])
LOG=[x>>8 for x in log]; LOGL=[x&255 for x in log]; ALOG=[x>>8 for x in alog]
def fmltu(A,Q):
    if A==0 or Q==0: return 0,0
    lo,c=adc(LOGL[A],LOGL[Q],0)
    hi,c=adc(LOG[Q],LOG[A],c)
    if not c: return 0,0
    return ALOG[hi],1
def tis2(A,Q):
    Y=A; A&=0x7F
    if A>=Q: return (Y&0x80)|96
    T=0xFE
    while True:
        A=(A<<1)&255
        c=0
        if A>=Q: A-=Q; c=1
        co=(T>>7)&1; T=((T<<1)&255)|c
        if not co: break
    t=T>>2; r=(t>>1)+t+(t&1)
    return (Y&0x80)|r
def dvid96(A):
    T=A&0x80; A&=0x7F; T1=254
    while True:
        A=(A<<1)&255; c=0
        if A>=96: A-=96; c=1
        co=(T1>>7)&1; T1=((T1<<1)&255)|c
        if not co: break
    return T1|T
def mltu2(A,P,Q):
    A^=0xFF; A,c=lsr(A); P1=A; A=0; X=16
    P,c=ror(P,c)
    while True:
        if not c: A,c=adc(A,Q,c); A,c=ror(A,c); P1,c=ror(P1,c); P,c=ror(P,c)
        else: A,c=lsr(A); P1,c=ror(P1,c); P,c=ror(P,c)
        X-=1
        if X==0: break
    return A,P1,P
def mad(A,Q,S,R_):
    a,p=mult1(A,Q); return add(a,p,S,R_)
random.seed(7)
out=[]
B=lambda: random.choice([0,1,2,0x7F,0x80,0x81,0xFF,random.randrange(256),random.randrange(256),random.randrange(256)])
for _ in range(60000):
    A,P,S,R_,Q=B(),B(),B(),B(),B()
    a,x=add(A,P,S,R_); out.append("add %d %d %d %d %d %d"%(A,P,S,R_,a,x))
    a,x=mad(A,Q,S,R_); out.append("mad %d %d %d %d %d %d"%(A,Q,S,R_,a,x))
    a,p=mult1(A,Q); out.append("mult1 %d %d %d %d"%(A,Q,a,p))
    a,p=mls(P,A); out.append("mults %d %d %d %d"%(P,A,a,p))
    out.append("tis2 %d %d %d"%(A,Q,tis2(A,Q)) if Q else "")
    out.append("dvid96 %d %d"%(A,dvid96(A)))
    a,c=fmltu(A,Q); out.append("fmltu %d %d %d %d"%(A,Q,a,c))
    a,p1,p0=mltu2(A,P,Q); out.append("mltu2 %d %d %d %d %d %d"%(A,P,Q,a,p1,p0))
# aleatorio: semilla y 300 salidas
st=[0x11,0x22,0x33,0x44]
def dornd(st,c):
    a=st[0]; carry=(a>>7)&1; a=((a<<1)|c)&255; x=a
    s=a+st[2]+carry; a=s&255; carry=s>>8; st[0]=a; st[2]=x
    a=st[1]; x=a; s=a+st[3]+carry; res=s&255; carry=s>>8
    v=1 if ((a^res)&(st[3]^res)&0x80) else 0
    st[1]=res; st[3]=x
    return res,x,carry,v
c=0
for i in range(300):
    r=dornd(st,c); out.append("rand %d %d %d %d %d"%(c,*r)); c=r[2] if i%3 else 0
open('tests/vectors.txt','w').write('\n'.join(l for l in out if l)+'\n')
print(len(out),'vectores')
