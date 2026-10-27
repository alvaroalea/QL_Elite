/* QL sound through the IPC (commands 10 = BEEP and 11 = BEEP OFF). C89.
   On the Apple II the sounds are loops that "click" the speaker and block the CPU; on the QL the IPC generates
   the tone by itself, so each effect is translated into BEEP parameters calculated with the original's timings.
   Warnings: (1) a new BEEP cancels the previous one; hence the priorities (see snd_tick);
   (2) the manual warns against using BEEP while a job is loaded/unloaded (it may hang the QL): silence during file loading;
   (3) very high tones make the IPC keyboard reading difficult: the minimum pitch is SND_MIN_PITCH. */
#ifndef SOUND_H
#define SOUND_H
#include "platform.h"

#define SND_MIN_PITCH 2

#define SND_P_UI    1      /* beep, boop, bell */
#define SND_P_LASER 2
#define SND_P_BLIP  3      /* ECM, hyperspace, hisses */
#define SND_P_BOMB  4
#define SND_P_EXPL  5
#define SND_P_DEATH 6

struct snd_params {
    u8 pitch1, pitch2;
    unsigned grad_x;       /* interval between notes (43.64 us steps of the 8049) */
    unsigned duration;     /* 1..32767 (0 would be "infinite") */
    u8 grad_y, wrap, fuzz, rnd;   /* 4 bits each */
};

/* Backend: sends an MT.IPCOM block of 'len' bytes. Returns 0 if all goes well. */
int snd_ipc(const u8 *blk, int len);

void snd_init(void);
extern void (*snd_dornd_hook)(void);     /* a game DORND (set by ships_hooks): the original's SOEXPL, SOHISS and SOBOMB call
                                            DORND once per click for the delay, and that changes the random number sequence */
void snd_enable(int on);           /* equivalent to the original's DNOIZ; when disabled it silences */
int  snd_enabled(void);
void snd_off(void);                /* BEEP without parameters */
void snd_tick(long ms);            /* the main loop reports the elapsed time */
void snd_send(const struct snd_params *p, int prio);

/* Effects of the original (same names and arguments as the Apple II routines) */
void snd_sobeep(int x, int y);     /* tone: period x, length y (clicks) */
void snd_soblip(int x, int y);     /* ECM (y=20) and hyperspace (y=90): the period falls = tone rises */
void snd_soexpl(int y);            /* explosion: y = 15, 40, 55 or 210 */
void snd_sohiss(int y);            /* hiss: y = 0 (256), 10, 50, 120 */
void snd_sobomb(void);
void snd_lasnoise(void);
void snd_beep(void);
void snd_boop(void);
void snd_bell(void);
void snd_exno(void);               /* laser hit on another ship */
void snd_exno3(void);              /* collision, canister or exploding missile */
void snd_exno2(void);              /* ship destroyed */
void snd_death(void);

/* Conversions (exposed for tests) */
long snd_pitch_from_cycles(long cycles);          /* 6502 cycles between clicks -> QL pitch */
long snd_units_from(long clicks, long cycles);    /* duration in QL units */
#endif
