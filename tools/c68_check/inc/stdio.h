typedef struct _F FILE;
extern FILE *fopen(const char *, const char *);
extern unsigned long fwrite(const void *, unsigned long, unsigned long, FILE *);
extern unsigned long fread(void *, unsigned long, unsigned long, FILE *);
extern int fclose(FILE *);
