# Parchea TXTELITE.C 1.5 lo minimo para compilarlo en PC y le anade un volcado del universo (oraculo de pruebas).
import re
s=open('tests/oracle/TXTELITE.C',encoding='latin1').read().replace('\r','')
for inc in ['#include <conio.h>','#include <graph.h>','#include <malloc.h>']:
    s=s.replace(inc,'')
# quitar toupper/tolower propios (chocan con ctype.h)
s=re.sub(r'\nint tolower\(char c\)[\s\S]*?\n}\n','\n',s)
s=re.sub(r'\nint toupper\(char c\)[\s\S]*?\n}\n','\n',s)
s=re.sub(r'\buint\b','oracle_uint',s)
s=s.replace('int main(','int txtelite_main_unused(')
head='''#include <ctype.h>
#include <stdarg.h>
#include <string.h>
static char oraclebuf[4096]; static int oraclelen;
static int oracle_printf(const char *f, ...){ va_list ap; va_start(ap,f); oraclelen+=vsnprintf(oraclebuf+oraclelen,sizeof(oraclebuf)-oraclelen,f,ap); va_end(ap); return 0; }
#define printf oracle_printf
'''
tail=r'''
#undef printf
int main(void){
  int g,i,j,k; unsigned fl[4]={0x00,0x37,0xAB,0xFF};
  for(g=1;g<=8;g++){
    galaxynum=g; buildgalaxy(g);
    for(i=0;i<galsize;i++){
      plansys p=galaxy[i];
      printf("S %d %d %s %u %u %u %u %u %u %u %u %u\n",g,i,p.name,(unsigned)p.x,(unsigned)p.y,(unsigned)p.economy,(unsigned)p.govtype,(unsigned)p.techlev,(unsigned)p.population,(unsigned)p.productivity,(unsigned)p.radius,0u);
      oraclelen=0; oraclebuf[0]=0; rnd_seed=p.goatsoupseed; goat_soup("\x8F is \x97.",&p);
      printf("D %d %d %s\n",g,i,oraclebuf);
      for(k=0;k<4;k++){ markettype m=genmarket(fl[k],p); printf("M %d %d %u",g,i,fl[k]); for(j=0;j<=lasttrade;j++) printf(" %u/%u",(unsigned)m.quantity[j],(unsigned)m.price[j]); printf("\n"); }
    }
  }
  for(g=1;g<=8;g+=7){ galaxynum=g; buildgalaxy(g);
    for(i=0;i<galsize;i++) for(j=0;j<galsize;j++) printf("T %d %d %d %u\n",g,i,j,(unsigned)distance(galaxy[i],galaxy[j])); }
  return 0;
}
'''
open('tests/oracle/oracle.c','w').write(head+s+tail)
