import sys,zlib,struct
d=open(sys.argv[1],'rb').read()
pal=[(0,0,0),(0,0,255),(255,0,0),(255,0,255),(0,255,0),(0,255,255),(255,255,0),(255,255,255)]
sc=3
rows=[]
for y in range(256):
    row=bytearray()
    for x in range(256):
        p=y*128+(x>>2)*2; s=2*(3-(x&3))
        c=(((d[p]>>(s+1))&1)<<2)|(((d[p+1]>>(s+1))&1)<<1)|((d[p+1]>>s)&1)
        row+=bytes(pal[c])*sc
    rows+=[b'\0'+bytes(row)]*sc
raw=b''.join(rows)
def ch(t,b): c=struct.pack('>I',len(b))+t+b; return c+struct.pack('>I',zlib.crc32(t+b)&0xffffffff)
png=b'\x89PNG\r\n\x1a\n'+ch(b'IHDR',struct.pack('>IIBBBBB',256*sc,256*sc,8,2,0,0,0))+ch(b'IDAT',zlib.compress(raw))+ch(b'IEND',b'')
open(sys.argv[2],'wb').write(png)
