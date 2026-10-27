; ql_ipc.asm - call to MT.IPCOM (trap #1, D0=$11), as68 syntax (XTC68).
; long ql_ipc(char *blk)  -> D1.B of the reply (0..255) or negative error code
; c68 convention: arguments on the stack; the result goes in D0.
	.globl	_ql_ipc
	.text
_ql_ipc:
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	48(sp),a3
	moveq	#$11,d0
	trap	#1
	tst.l	d0
	bne	ipcfin
	moveq	#0,d0
	move.b	d1,d0
ipcfin:
	movem.l	(sp)+,d2-d7/a2-a6
	rts

; ql_trap3.asm (same unit) - generic call to a QDOS trap #3 (channel I/O), as68 syntax.
; long ql_trap3(long d0, long d1, long d2, long d3, long a0, long a1)
; Loads the registers, executes trap #3 and returns D0 (0 = ok, negative = QDOS error, e.g. -1 = ERR.NC).
; The arguments are after 11 saved registers (44 bytes) + return address: first at 48(sp).
	.globl	_ql_trap3
_ql_trap3:
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	48(sp),d0
	move.l	52(sp),d1
	move.l	56(sp),d2
	move.l	60(sp),d3
	move.l	64(sp),a0
	move.l	68(sp),a1
	trap	#3
	movem.l	(sp)+,d2-d7/a2-a6
	rts

; ql_trap1 (same unit) - generic call to a QDOS trap #1 (job manager), as68 syntax.
; long ql_trap1(long d0, long d1, long d2, long d3)
; Loads D0..D3, executes trap #1 and returns D0. Used with D0=5 (MT.FRJOB), D1=-1 (this job), D3=0 to
; end the program without going through the c68 runtime's exit (which writes "Press a key to exit" and waits for a key).
	.globl	_ql_trap1
_ql_trap1:
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	48(sp),d0
	move.l	52(sp),d1
	move.l	56(sp),d2
	move.l	60(sp),d3
	trap	#1
	movem.l	(sp)+,d2-d7/a2-a6
	rts

; 50 Hz clock (same unit): a task linked to the QDOS frame interrupt (MT.LPOLL, trap #1, D0=$1C) adds 1 to
; tk_count on every frame (50 per second; 60 on an American QL). void ql_tick_on(void) links it, void ql_tick_off(void) removes it
; (MT.RPOLL, D0=$1D; it must be done before ending: the task's code disappears with the job) and long ql_ticks(void)
; returns the count. Link block: +0 link (used by QDOS), +4 address of the routine.
	.globl	_ql_tick_on
_ql_tick_on:
	movem.l	d2-d7/a2-a6,-(sp)
	lea	tk_link,a0
	lea	tk_poll,a1
	move.l	a1,4(a0)
	moveq	#$1C,d0
	trap	#1
	movem.l	(sp)+,d2-d7/a2-a6
	rts

	.globl	_ql_tick_off
_ql_tick_off:
	movem.l	d2-d7/a2-a6,-(sp)
	lea	tk_link,a0
	moveq	#$1D,d0
	trap	#1
	movem.l	(sp)+,d2-d7/a2-a6
	rts

	.globl	_ql_ticks
_ql_ticks:
	move.l	tk_count,d0
	rts

tk_poll:
	move.l	a0,-(sp)
	lea	tk_count,a0
	addq.l	#1,(a0)
	move.l	(sp)+,a0
	rts

	.data
tk_link:
	.long	0
	.long	0
tk_count:
	.long	0
