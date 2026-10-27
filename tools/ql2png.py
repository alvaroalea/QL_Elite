import sys,zlib,struct
# usage: ql2png.py capture.bin output.png [4]    (the 4 = mode 4 capture: 512 x 256, black/red/green/white; by default mode 8: 256 x 256, 8 colours)
d=open(sys.argv[1],'rb').read()
m4=len(sys.argv)>3 and sys.argv[3]=='4'
pal=[(0,0,0),(0,0,255),(255,0,0),(255,0,255),(0,255,0),(0,255,255),(255,255,0),(255,255,255)]
rows=[]
if m4:
    sx,sy,W=2,3,512                       # physical dots of half the width: 1024 x 768 (4:3)
    for y in range(256):
        row=bytearray()
        for x in range(W):
            p=y*128+(x>>3)*2; b=7-(x&7)
            g=(d[p]>>b)&1; r=(d[p+1]>>b)&1
            row+=bytes(pal[(g<<2)|(r<<1)|(g&r)])*sx
        rows+=[b'\0'+bytes(row)]*sy
else:
    sx,sy,W=3,3,256
    for y in range(256):
        row=bytearray()
        for x in range(256):
            p=y*128+(x>>2)*2; s=2*(3-(x&3))
            c=(((d[p]>>(s+1))&1)<<2)|(((d[p+1]>>(s+1))&1)<<1)|((d[p+1]>>s)&1)
            row+=bytes(pal[c])*sx
        rows+=[b'\0'+bytes(row)]*sy
raw=b''.join(rows)
def ch(t,b): c=struct.pack('>I',len(b))+t+b; return c+struct.pack('>I',zlib.crc32(t+b)&0xffffffff)
png=b'\x89PNG\r\n\x1a\n'+ch(b'IHDR',struct.pack('>IIBBBBB',W*sx,256*sy,8,2,0,0,0))+ch(b'IDAT',zlib.compress(raw))+ch(b'IEND',b'')
open(sys.argv[2],'wb').write(png)
