#include "sound.h"

/* Apple II: CPU a 1.0205 MHz; cada clic invierte el altavoz, asi que frecuencia = 1.0205e6 / (2 * ciclos_por_clic).
   QL: frecuencia ~ 11447 / (10.6 + pitch) Hz  =>  pitch = 0.022434 * ciclos - 10.6.
   Duracion en unidades de 72 us: clics * ciclos * 1.0205e6^-1 / 72e-6 = clics * ciclos * 0.01361. */
#define SND_NOISE_FUZZ 12   /* ajustar de oido en el QL real */
#define SND_NOISE_RND  12

static int enabled = 1;
void (*snd_dornd_hook)(void) = 0;

/* los DORND que gastan los sonidos con retardo aleatorio, solo si el sonido esta activo (BIT DNOIZ; BMI SOUR) */
static void dornds(long n) { if (enabled && snd_dornd_hook) while (n--) snd_dornd_hook(); }
static long clock_ms = 0;
static long busy_until = 0;
static int busy_prio = 0;

long snd_pitch_from_cycles(long cycles)
{
    long n = cycles * 22434L + 500000L - 10600000L;
    long p;
    if (n < 0) return SND_MIN_PITCH;
    p = n / 1000000L;
    if (p < SND_MIN_PITCH) p = SND_MIN_PITCH;
    if (p > 255) p = 255;
    return p;
}

long snd_units_from(long clicks, long cycles)
{
    long u = clicks * cycles * 1361L / 100000L;
    if (u < 1) u = 1;
    if (u > 32767L) u = 32767L;
    return u;
}

void snd_init(void) { enabled = 1; clock_ms = 0; busy_until = 0; busy_prio = 0; }
int snd_enabled(void) { return enabled; }

void snd_off(void)
{
    static u8 blk[8];
    blk[0] = 11; blk[1] = 0; blk[2] = 0; blk[3] = 0; blk[4] = 0; blk[5] = 0; blk[6] = 0; blk[7] = 0;
    snd_ipc(blk, 7);
    busy_until = 0; busy_prio = 0;
}

void snd_enable(int on)
{
    enabled = on ? 1 : 0;
    if (!enabled) snd_off();
}

void snd_tick(long ms)
{
    clock_ms += ms;
    if (clock_ms >= busy_until) busy_prio = 0;
}

/* Bloque MT.IPCOM de BEEP (17 bytes): orden, n de parametros (10), "bits por parametro" en 32 bits
   (2 bits cada uno, LSB primero: 10 = 8 bits, 00 = 4 bits -> $00000AAA), 10 bytes de parametros, byte de respuesta (0). */
void snd_send(const struct snd_params *p, int prio)
{
    static u8 blk[20];
    long dur_ms;
    if (!enabled) return;
    if (clock_ms < busy_until && prio < busy_prio) return;       /* no pisar un sonido mas importante */
    blk[0] = 10; blk[1] = 10;
    blk[2] = 0; blk[3] = 0; blk[4] = 0x0A; blk[5] = 0xAA;
    blk[6] = p->pitch1; blk[7] = p->pitch2;
    blk[8] = (u8)(p->grad_x >> 8); blk[9] = (u8)(p->grad_x & 255);
    blk[10] = (u8)(p->duration >> 8); blk[11] = (u8)(p->duration & 255);
    blk[12] = (u8)(p->grad_y & 15); blk[13] = (u8)(p->wrap & 15);
    blk[14] = (u8)(p->fuzz & 15); blk[15] = (u8)(p->rnd & 15);
    blk[16] = 0;
    snd_ipc(blk, 17);
    dur_ms = (long)p->duration * 72L / 1000L;
    busy_until = clock_ms + dur_ms;
    busy_prio = prio;
}

static void tone(long pitch, long units, int prio)
{
    struct snd_params p;
    p.pitch1 = (u8)pitch; p.pitch2 = 0; p.grad_x = 0; p.duration = (unsigned)units;
    p.grad_y = 0; p.wrap = 0; p.fuzz = 0; p.rnd = 0;
    snd_send(&p, prio);
}

/* Barrido de pitch_a (inicio) a pitch_b (final). Pitch menor = tono mas agudo.
   Tono que baja: pitch1=a, pitch2=b, paso positivo. Tono que sube: pitch1=b, pitch2=a, paso negativo (16-paso). */
static void sweep(long a, long b, long units, int noisy, int prio)
{
    struct snd_params p;
    long range, step, notes;
    if (a == b) { tone(a, units, prio); return; }
    range = a > b ? a - b : b - a;
    step = 1 + range / 16; if (step > 7) step = 7;
    notes = range / step + 1;
    p.grad_x = (unsigned)(units / notes); if (p.grad_x < 1) p.grad_x = 1;
    p.duration = (unsigned)units;
    if (b > a) { p.pitch1 = (u8)a; p.pitch2 = (u8)b; p.grad_y = (u8)step; }
    else       { p.pitch1 = (u8)b; p.pitch2 = (u8)a; p.grad_y = (u8)((16 - step) & 15); }
    p.wrap = 0;
    p.fuzz = noisy ? SND_NOISE_FUZZ : 0;
    p.rnd = noisy ? SND_NOISE_RND : 0;
    snd_send(&p, prio);
}

/* Ruido: muchas notas cortas aleatorias dentro de [lo, hi] */
static void noise(long lo, long hi, long units, int prio)
{
    struct snd_params p;
    p.pitch1 = (u8)lo; p.pitch2 = (u8)hi; p.grad_x = 20; p.duration = (unsigned)units;
    p.grad_y = 1; p.wrap = 0; p.fuzz = 15; p.rnd = 15;
    snd_send(&p, prio);
}

/* SOBEEP: por clic 4 + 3 + 5*(x-1) + 4 + 2 + 3 = 5x + 11 ciclos */
void snd_sobeep(int x, int y)
{
    long cyc = 5L * x + 11;
    tone(snd_pitch_from_cycles(cyc), snd_units_from(y ? y : 256, cyc), SND_P_UI);
}

/* SOBLIP: el periodo baja un paso por clic (DEC): ciclos = 7*T + 14, de T = x-1 a x-y */
void snd_soblip(int x, int y)
{
    long t0 = x - 1, t1 = x - y, c0, c1;
    if (t0 < 1) t0 = 1;
    if (t1 < 1) t1 = 1;
    c0 = 7 * t0 + 14; c1 = 7 * t1 + 14;
    sweep(snd_pitch_from_cycles(c0), snd_pitch_from_cycles(c1), snd_units_from(y, (c0 + c1) / 2), 0, SND_P_BLIP);
}

/* SOEXPL: T sube de 51 a 50+y (sin pasar de 256); ciclos ~ 9*T + 1010 (retardo aleatorio medio ~900) */
void snd_soexpl(int y)
{
    long t1 = 50 + y, c0, c1;
    if (t1 > 256) t1 = 256;
    c0 = 9 * 51L + 1010; c1 = 9 * t1 + 1010;
    sweep(snd_pitch_from_cycles(c0), snd_pitch_from_cycles(c1), snd_units_from(y, (c0 + c1) / 2), 1,
          y >= 200 ? SND_P_DEATH : SND_P_EXPL);
    dornds(y ? y : 256);                         /* DEY; BNE BEEPL4 */
}

/* SOHISS: ~965 ciclos por clic (retardo aleatorio medio). y = 0 significa 256 clics */
void snd_sohiss(int y)
{
    long n = y ? y : 256;
    long p = snd_pitch_from_cycles(965);
    noise(p - 3, p + 5, snd_units_from(n, 965), SND_P_BLIP);
    dornds(n);
}

/* SOBOMB: 25 clics con retardo de 7*(224..255) ciclos: ~1745 por clic */
void snd_sobomb(void)
{
    long p = snd_pitch_from_cycles(1745);
    noise(p - 3, p + 3, snd_units_from(25, 1745), SND_P_BOMB);
    dornds(25);
}

/* LASNOISE: 11 clics, T de 152 a 172 (+2 por clic), ciclos = 5*T + 21 */
void snd_lasnoise(void)
{
    long c0 = 5L * 152 + 21, c1 = 5L * 172 + 21;
    sweep(snd_pitch_from_cycles(c0), snd_pitch_from_cycles(c1), snd_units_from(11, (c0 + c1) / 2), 0, SND_P_LASER);
}

void snd_beep(void) { snd_sobeep(110, 30); }
void snd_boop(void) { snd_sobeep(255, 99); }
void snd_bell(void) { tone(SND_MIN_PITCH, 1389, SND_P_UI); }      /* ~1 kHz durante ~100 ms */
void snd_exno(void) { snd_soexpl(15); }
void snd_exno3(void) { snd_soexpl(40); }
void snd_exno2(void) { snd_soexpl(55); }
void snd_death(void) { snd_soexpl(210); }
