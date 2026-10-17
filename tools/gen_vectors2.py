import sys,random
sys.path.insert(0,'tools')
from ref6502b import *
random.seed(11)
B=lambda: random.choice([0,1,2,3,0x3F,0x40,0x7F,0x80,0x81,0xFF,random.randrange(256),random.randrange(256),random.randrange(256)])
out=[]
for _ in range(40000):
    A,Q,U,S,R,P0,P1,P2=B(),B(),B(),B(),B(),B(),B(),B()
    out.append("ll5 %d %d %d"%(A,Q,ll5(A,Q)))
    if Q:
        r,c=ll28(A,Q); out.append("ll28 %d %d %d %d"%(A,Q,r,c))
        a2=A%Q if True else A
        r,c=ll28(a2,Q,False); out.append("ll28n %d %d %d %d"%(a2,Q,r,c))
        r,u=ll61(A if A>=Q else Q,Q,0); out.append("ll61 %d %d %d %d"%(A if A>=Q else Q,Q,r,u))
        p,r=dvid4(A,Q); out.append("dvid4 %d %d %d %d"%(A,Q,p,r))
        p1,a=dvidt(A,P0,Q); out.append("dvidt %d %d %d %d %d"%(A,P0,Q,p1,a)) if (Q&0x7F) else None
    a,s,c=ll38(A,S&0x80,Q,R); out.append("ll38 %d %d %d %d %d %d %d"%(A,S&0x80,Q,R,a,s,c))
    k=mult3_literal(A,P1,P0,Q); out.append("mult3 %d %d %d %d %d"%(A,P1,P0,Q,k))
    kk=dvid3b(P0,P1,P2,Q|1,R,S); out.append("dvid3b %d %d %d %d %d %d %d %d %d %d"%(P0,P1,P2,Q|1,R,S,*kk))
    if A:
        pass
    r,c=fmltu(Q,SNE[A&31]); out.append("fmltu2 %d %d %d %d"%(A,Q,r,c))
    m=(A&0x7F)*Q; out.append("mlu2 %d %d %d %d"%(A,Q,m>>8,m&255))
open('tests/vectors2.txt','w').write('\n'.join(x for x in out if x)+'\n')
print(len(out),'vectores')
