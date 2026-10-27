#include "sound.h"

/* Apple II: CPU at 1.0205 MHz; each click flips the speaker, so frequency = 1.0205e6 / (2 * cycles_per_click).
   QL (IPC 8049, 22,917 steps per second; sQLux, which models the real protocol): half period = (pitch - 1 + 10.6) steps, i.e.
   frequency = 22917 / (2 * (pitch + 9.6)) Hz  =>  pitch = 0.022457 * cycles - 9.6.
   Duration and interval in steps of 1/22917 s = 43.64 us (not the 72 us of the manual): clicks * cycles / 1.0205e6 * 22917 = clicks * cycles * 0.022457. */
#define SND_NOISE_FUZZ 12   /* tune by ear on a real QL */
#define SND_NOISE_RND  12

static int enabled = 1;
void (*snd_dornd_hook)(void) = 0;

/* the DORNDs consumed by the sounds with random delay, only if sound is on (BIT DNOIZ; BMI SOUR) */
static void dornds(long n) { if (enabled && snd_dornd_hook) while (n--) snd_dornd_hook(); }
static long clock_ms = 0;
static long busy_until = 0;
static int busy_prio = 0;

long snd_pitch_from_cycles(long cycles)
{
    long n = cycles * 22457L + 500000L - 9600000L;
    long p;
    if (n < 0) return SND_MIN_PITCH;
    p = n / 1000000L;
    if (p < SND_MIN_PITCH) p = SND_MIN_PITCH;
    if (p > 255) p = 255;
    return p;
}

long snd_units_from(long clicks, long cycles)
{
    long u = clicks * cycles * 2246L / 100000L;
    if (u < 1) u = 1;
    if (u > 32767L) u = 32767L;
    return u;
}

void snd_init(void) { enabled = 1; clock_ms = 0; busy_until = 0; busy_prio = 0; }
int snd_enabled(void) { return enabled; }

void snd_off(void)
{
    static u8 blk[8];
    blk[0] = 11; blk[1] = 0; blk[2] = 0; blk[3] = 0; blk[4] = 0; blk[5] = 0;
    blk[6] = 1;                                  /* reply %01 = none (with 0 = %00, MT.IPCOM read 4 bits that the 8049 does not send) */
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

/* BEEP MT.IPCOM block (15 bytes), as the 8049 expects it (64 parameter bits; sQLux, ChibiAkumas example): command 10,
   8 parameter bytes, specifier $0000AAAA (2 bits per byte: %10 = 8 bits), pitch 1, pitch 2, 16-bit interval and duration
   LOW BYTE FIRST (the 8049 is little-endian; in big-endian a duration of $00E6 arrived as $E600: extremely long beeps),
   step/wrap byte (step in the high nibble), random/fuzzy byte (random in the high one) and the reply length: 1 = %01 =
   none. With 0 (%00 = 4 bits) MT.IPCOM read 4 bits that the 8049 does not send and the link went out of sync: after several
   BEEPs in a row the QL hung. */
void snd_send(const struct snd_params *p, int prio)
{
    static u8 blk[16];
    long dur_ms;
    if (!enabled) return;
    if (clock_ms < busy_until && prio < busy_prio) return;       /* do not overwrite a more important sound */
    blk[0] = 10; blk[1] = 8;
    blk[2] = 0; blk[3] = 0; blk[4] = 0xAA; blk[5] = 0xAA;
    blk[6] = p->pitch1; blk[7] = p->pitch2;
    blk[8] = (u8)(p->grad_x & 255); blk[9] = (u8)((p->grad_x >> 8) & 0x7F);
    blk[10] = (u8)(p->duration & 255); blk[11] = (u8)((p->duration >> 8) & 0x7F);
    blk[12] = (u8)(((p->grad_y & 15) << 4) | (p->wrap & 15));
    blk[13] = (u8)(((p->rnd & 15) << 4) | (p->fuzz & 15));
    blk[14] = 1;
    snd_ipc(blk, 15);
    dur_ms = (long)p->duration * 4364L / 100000L;                /* 43.64 us per step */
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

/* Sweep from pitch_a (start) to pitch_b (end). Lower pitch = higher tone.
   Falling tone: pitch1=a, pitch2=b, positive step. Rising tone: pitch1=b, pitch2=a, negative step (16-step). */
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

/* Noise: many short random notes within [lo, hi] */
static void noise(long lo, long hi, long units, int prio)
{
    struct snd_params p;
    p.pitch1 = (u8)lo; p.pitch2 = (u8)hi; p.grad_x = 20; p.duration = (unsigned)units;
    p.grad_y = 1; p.wrap = 0; p.fuzz = 15; p.rnd = 15;
    snd_send(&p, prio);
}

/* SOBEEP: per click 4 + 3 + 5*(x-1) + 4 + 2 + 3 = 5x + 11 cycles */
void snd_sobeep(int x, int y)
{
    long cyc = 5L * x + 11;
    tone(snd_pitch_from_cycles(cyc), snd_units_from(y ? y : 256, cyc), SND_P_UI);
}

/* SOBLIP: the period drops one step per click (DEC): cycles = 7*T + 14, from T = x-1 to x-y */
void snd_soblip(int x, int y)
{
    long t0 = x - 1, t1 = x - y, c0, c1;
    if (t0 < 1) t0 = 1;
    if (t1 < 1) t1 = 1;
    c0 = 7 * t0 + 14; c1 = 7 * t1 + 14;
    sweep(snd_pitch_from_cycles(c0), snd_pitch_from_cycles(c1), snd_units_from(y, (c0 + c1) / 2), 0, SND_P_BLIP);
}

/* SOEXPL: T rises from 51 to 50+y (not past 256); cycles ~ 9*T + 1010 (average random delay ~900) */
void snd_soexpl(int y)
{
    long t1 = 50 + y, c0, c1;
    if (t1 > 256) t1 = 256;
    c0 = 9 * 51L + 1010; c1 = 9 * t1 + 1010;
    sweep(snd_pitch_from_cycles(c0), snd_pitch_from_cycles(c1), snd_units_from(y, (c0 + c1) / 2), 1,
          y >= 200 ? SND_P_DEATH : SND_P_EXPL);
    dornds(y ? y : 256);                         /* DEY; BNE BEEPL4 */
}

/* SOHISS: ~965 cycles per click (average random delay). y = 0 means 256 clicks */
void snd_sohiss(int y)
{
    long n = y ? y : 256;
    long p = snd_pitch_from_cycles(965);
    noise(p - 3, p + 5, snd_units_from(n, 965), SND_P_BLIP);
    dornds(n);
}

/* SOBOMB: 25 clicks with a delay of 7*(224..255) cycles: ~1745 per click */
void snd_sobomb(void)
{
    long p = snd_pitch_from_cycles(1745);
    noise(p - 3, p + 3, snd_units_from(25, 1745), SND_P_BOMB);
    dornds(25);
}

/* LASNOISE: 11 clicks, T from 152 to 172 (+2 per click), cycles = 5*T + 21 */
void snd_lasnoise(void)
{
    long c0 = 5L * 152 + 21, c1 = 5L * 172 + 21;
    sweep(snd_pitch_from_cycles(c0), snd_pitch_from_cycles(c1), snd_units_from(11, (c0 + c1) / 2), 0, SND_P_LASER);
}

void snd_beep(void) { snd_sobeep(110, 30); }
void snd_boop(void) { snd_sobeep(255, 99); }
void snd_bell(void) { tone(SND_MIN_PITCH, 2292, SND_P_UI); }      /* ~1 kHz for ~100 ms (2292 steps of 43.64 us) */
void snd_exno(void) { snd_soexpl(15); }
void snd_exno3(void) { snd_soexpl(40); }
void snd_exno2(void) { snd_soexpl(55); }
void snd_death(void) { snd_soexpl(210); }
