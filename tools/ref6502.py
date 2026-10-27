# Reference: literal (instruction by instruction) transcription of arithmetic routines of Apple II Elite.
class R:  # state with carry
    pass
def adc(a,b,c):
    s=a+b+c; return s&255, s>>8
def sbc(a,b,c):
    s=a-b-(1-c); return s&255, 1 if s>=0 else 0
def ror(v,c): return ((v>>1)|(c<<7))&255, v&1
def lsr(v): return v>>1, v&1
def mult1(A,Q):
    X=A; A&=0x7F; A,c=lsr(A); P=A
    T=(X^Q)&0x80
    A=Q&0x7F
    if A==0: return 0,0           # mu10: A=0, P=0
    T1=A-1; A=0
    for _ in range(7):
        if c: A,c=adc(A,T1,c)
        A,c=ror(A,c); P,c=ror(P,c)
    A,c=lsr(A); P,c=ror(P,c)
    return A|T,P
def mls(Pin,A):  # MULTS with P=Pin (0..31)
    X=A; T=A&0x80; A&=0x7F
    if A==0: return 0,0
    T1=A-1; P=Pin; A=0
    P,c=lsr(P)
    for k in range(5):
        if c: A,c=adc(A,T1,c)
        A,c=ror(A,c); P,c=ror(P,c)
    for k in range(3):
        A,c=lsr(A); P,c=ror(P,c)
    return A|T,P
def add(A,P,S,R_):
    T1=A; T=A&0x80
    if (A^S)&0x80:
        U=S&0x7F
        X,c=sbc(P,R_,1)
        A,c=sbc(T1&0x7F,U,c)
        if not c:
            U=A
            X,c=adc(X^0xFF,1,0)
            A,c=sbc(0,U,c)
            A|=0x80
        return A^T,X
    X,c=adc(R_,P,0)
    A,c=adc(S,T1,c)
    return A|T,X
def tis2(A,Q):
    Y=A; A&=0x7F
    if A>=Q: return (Y&0x80)|96
    T=0xFE
    while True:
        A=(A<<1)&255  # note: ASL may overflow C; the original handles it: A<=127 and Q>A
        # ASL produces a carry if A>=128; CMP Q with A (8 bits) -> comparison on 8-bit A
        if A>=Q: A-=Q
        T2=(T<<1)|1 if True else 0
        c=(T>>7)&1; T=((T<<1)&255)|1 if True else 0
        # ROL T uses the carry from CMP/SBC: c_in = 1 if A>=Q (before subtracting)
        break
    return None
if __name__=='__main__':
    import itertools
    # MULT1: find the closed form
    bad=0
    for A in range(256):
        for Q in range(256):
            a,p=mult1(A,Q)
            mag=((a&0x7F)<<8)|p; s=(A^Q)&0x80
            cand=((A&0x7F)*(Q&0x7F))*2
            exp=cand if (A&0x7F)*(Q&0x7F) else 0
            if mag!=(exp&0x7FFF) or (a&0x80)!=(s if exp else 0 if False else (a&0x80)):
                bad+=1
                if bad<6: print('MULT1 dif',A,Q,hex(mag),hex(cand))
    print('MULT1 forma |A|*|Q|*2 -> fallos',bad)
