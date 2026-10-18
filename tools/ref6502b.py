# Transcripcion literal (con acarreo explicito) de mas rutinas aritmeticas de Elite Apple II.
import math,sys
sys.path.insert(0,'tools')
exec(open('tools/gen_tables.py').read().split("def arr")[0])
LOG=[x>>8 for x in log]; LOGL=[x&255 for x in log]; ALOG=[x>>8 for x in alog]
SNE=[255 if abs(math.sin(i/64*2*math.pi))>=1 else int(256*abs(math.sin(i/64*2*math.pi))+0.5) for i in range(32)]
def ll5(R,Q):
    Y=R; S=Q; X=0; Q=0
    for _ in range(8):
        if X<Q: c=0
        elif X>Q or Y>=64:
            c=1
            c2=1 if Y>=64 else 0; Y=(Y-64)&255
            X=(X-Q-(1-c2))&255
        else: c=0
        Q=((Q<<1)|c)&255
        for _ in range(2):
            c1=S>>7; S=(S<<1)&255
            c2=Y>>7; Y=((Y<<1)|c1)&255
            X=((X<<1)|c2)&255
    return Q
def ll28(A,Q,check=True):
    # devuelve (R, carry)
    if check and A>=Q: return 255,1
    if A==0: return 0,0
    lo=LOGL[A]-LOGL[Q]; b=1 if lo<0 else 0
    hi=LOG[A]-LOG[Q]-b
    if hi>=0: return 255,1
    return ALOG[hi&255],0
def ll61(A,Q,U):
    if Q==0: return 50,50
    X=0
    while True:
        A>>=1; X+=1
        if A<Q: break
    S=X; R,_=ll28(A,Q)
    A=R; X=S
    while True:
        c=A>>7; A=(A<<1)&255
        nu=((U<<1)|c); U=nu&255
        if U&0x80: return 50,50
        X=(X-1)&255
        if X==0: break
    return A,U
def ll38(A,S,Q,R):
    # devuelve (A,S,carry)
    if not ((A^S)&0x80):
        s=Q+R; return s&255,S,s>>8
    d=R-Q
    if d>=0: return d&255,S,0
    S^=0x80; a=(d&255)^255; s=a+1
    return s&255,S,s>>8
def dvid4(A,Q):
    # devuelve (P, R)
    c=A>>7; P=(A<<1)&255; A=0
    for _ in range(8):
        # ROL A
        nc=A>>7; A=((A<<1)|c)&255; c=nc
        if A>=Q: A=(A-Q)&255; c=1
        else: c=0
        # ROL P
        nc=P>>7; P=((P<<1)|c)&255; c=nc
    # ROL A siguiente iteracion usa c: se maneja en el bucle; tras la ultima, c es el bit sacado de P
    R,_=ll28(A,Q,False) if A!=0 or True else (0,0)
    return P,R
def dvidt(A,P,Q):
    # (P+1 A) = (A P)/Q  signo-magnitud, Q con signo
    P1=A; T=(A^Q)&0x80
    A=0
    c=P>>7; P=(P<<1)&255; P1=((P1<<1)|c)&255
    c2=0                       # ASL Q; LSR Q deja C = 0: el bit que sale de P+1 se pierde
    Q=Q&0x7F
    for _ in range(16):
        nc=A>>7; A=((A<<1)|c2)&255; c2=nc
        if A>=Q: A=(A-Q)&255; c=1
        else: c=0
        nc=P>>7; P=((P<<1)|c)&255; c=nc
        nc=P1>>7; P1=((P1<<1)|c)&255; c2=nc
    return P1,(P|T)
def mult3(A,P1,P0,Q):
    sign=(A^Q)&0x80
    mq=Q&0x7F
    if mq==0: return 0
    mag=(((A&0x7F)<<16)|(P1<<8)|P0)*mq
    return (mag|(sign<<24))&0xFFFFFFFF
def dvid3b(P0,P1,P2,Qin,Rin,Sin):
    P0|=1; T=(P2^Sin)&0x80; Y=0
    A=P2&0x7F
    while True:
        if A>=64: break
        c=P0>>7; P0=(P0<<1)&255
        c2=P1>>7; P1=((P1<<1)|c)&255
        A=((A<<1)|c2)&255
        Y=(Y+1)&255
        if Y==0: break
    P2=A; A=Sin&0x7F; Q=Qin; R=Rin
    while True:
        Y=(Y-1)&255
        c=Q>>7; Q=(Q<<1)&255
        c2=R>>7; R=((R<<1)|c)&255
        A=((A<<1)|c2)&255
        if A&0x80: break
    Q=A; R=254; A=P2
    while True:
        c=A>>7; A=(A<<1)&255
        if c:
            A=(A-Q)&255; cc=1
            c0=R>>7; R=((R<<1)|cc)&255
            if c0: continue
            Rres=R; break
        cc=1 if A>=Q else 0
        if cc: A=(A-Q)&255
        c0=R>>7; R=((R<<1)|cc)&255
        if c0: continue
        break
    K=[0,0,0,0]
    if Y&0x80:
        A=R
        while True:
            c=A>>7; A=(A<<1)&255
            c1=K[1]>>7; K[1]=((K[1]<<1)|c)&255
            c2=K[2]>>7; K[2]=((K[2]<<1)|c1)&255
            K[3]=((K[3]<<1)|c2)&255
            Y=(Y+1)&255
            if Y==0: break
        K[0]=A; K[3]|=T
    elif Y==0:
        K[0]=R; K[3]=T
    else:
        A=R
        while True:
            A>>=1; Y=(Y-1)&255
            if Y==0: break
        K[0]=A; K[3]=T
    return K
if __name__=='__main__':
    # formas cerradas
    bad=sum(1 for v in range(65536) if ll5(v>>8,v&255)!=math.isqrt(v)); print('LL5 = isqrt(R*256+Q): fallos',bad)
    bad=0
    for A in range(256):
        for Q in range(1,256):
            P,R=dvid4(A,Q)
            if (P,)!=((A*256//Q)>>8 if False else (A//Q)&255,): bad+=1
    print('DVID4 P = A/Q: fallos',bad)
    import random; random.seed(3); bad=0
    for _ in range(100000):
        A=random.randrange(256);P1=random.randrange(256);P0=random.randrange(256);Q=random.randrange(256)
        if mult3(A,P1,P0,Q)!=mult3(A,P1,P0,Q): bad+=1
    print('ok literal')

def mult3_literal(A,P1,P0,Q):
    R=A; K2=A&0x7F
    mq=Q&0x7F
    if mq==0: return 0
    T=mq-1
    # LSR K+2 ; ROR A(=P1) ; STA K+1 ; LDA P ; ROR A ; STA K
    c=K2&1; K2>>=1
    a=P1; nc=a&1; a=((a>>1)|(c<<7))&255; c=nc; K1=a
    a=P0; nc=a&1; a=((a>>1)|(c<<7))&255; c=nc; K0=a
    A_=0
    for _ in range(24):
        if c:
            s=A_+T+c; A_=s&255; c=s>>8     # ADC T con C=1
        # ROR A, ROR K2, ROR K1, ROR K0
        nc=A_&1; A_=((A_>>1)|(c<<7))&255; c=nc
        nc=K2&1; K2=((K2>>1)|(c<<7))&255; c=nc
        nc=K1&1; K1=((K1>>1)|(c<<7))&255; c=nc
        nc=K0&1; K0=((K0>>1)|(c<<7))&255; c=nc
    T=A_
    k3=(((R^Q)&0x80)|T)&255
    return (k3<<24)|(K2<<16)|(K1<<8)|K0
def fmltu(A,Q):
    if A==0 or Q==0: return 0,0
    lo=LOGL[A]+LOGL[Q]; c=lo>>8
    hi=LOG[Q]+LOG[A]+c
    if hi<256: return 0,0
    return ALOG[hi&255],1
if __name__=='__main__' and len(sys.argv)>1 and sys.argv[1]=='mult3':
    import random; random.seed(5); bad=0
    for _ in range(300000):
        A=random.choice([0,1,0x7F,0x80,0xFF,random.randrange(256)]);P1=random.randrange(256);P0=random.randrange(256);Q=random.choice([0,1,2,0x7F,0x80,0xFF,random.randrange(256)])
        a=mult3_literal(A,P1,P0,Q); b=mult3(A,P1,P0,Q)
        if a!=b:
            bad+=1
            if bad<6: print('MULT3',hex(A),hex(P1),hex(P0),hex(Q),hex(a),hex(b))
    print('MULT3 forma cerrada fallos',bad)
