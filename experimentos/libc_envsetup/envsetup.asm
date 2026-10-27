; envsetup.asm - FIXED _envsetup module of libc.a (c68 4.24f runtime), as68 syntax.
; Starts from envsetup_orig.asm (faithful transcription, checked byte by byte); only e_set changes.
;
; Original bug: the Toolkit II pointers at SV+$AC/$B0/$B4 are 0 without Toolkit II and e_set reads the
; QDOS string at ROM address 0. Its first word acts as the length: 3 on the JS ROM (harmless
; garbage), $4C02 on Minerva: qlstr_to_c copies 19458 bytes into a 50-byte stack buffer and the program
; hangs before main. Also _setcd builds "name=value" in another 50-byte buffer, so a value
; longer than 40 characters overflows it as well (name of up to 8 + '=' + value + 0).
; Fix in e_set: non-null, even pointer inside RAM (between the system variables and SV_RAMT),
; length from 1 to 40, and the buffer is zeroed before __getcd (if the variable does not fit in 50 bytes
; __getcd writes nothing and garbage from the stack was read).
;
; void _envsetup(char *env)   (called by Cinit before main with the environment passed by the parent job)
;  1) if env points to $4AFC, the environment list is *(env+2); otherwise, it looks in SuperBASIC's name table
;     for a machine-code procedure (type 8) called *ENV* (extension ENV_BIN) and takes the
;     list from the long word that follows its first instruction.
;  2) putenv("") and putenv of each string in the list (terminated by an empty string).
;  3) PROG_USE, DATA_USE and SPL_USE from the Toolkit II pointers at SV+$AC, $B0 and $B4 (QDOS strings),
;     if they are not already in the environment.

	.text
envname:
	.ascii	"*ENV*"
	.data1	0
	.globl	__envsetup
__envsetup:
	link	a5,#-28
	movem.l	d3/a2-a4/a6,-(a7)
	move.l	8(a5),d3
	move.l	d3,a0
	cmp.w	#19196,(a0)		; $4AFC
	bne	e_bas
	lea	2,a0
	add.l	d3,a0
	move.l	(a0),d3
	bra.w	e_put
e_bas:
	moveq	#0,d3
	jsr	__super
	move.l	__sys_var,a0
	move.l	16(a0),d0		; SV_BASIC
	add.l	#104,d0			; + $68: BASIC variables
	move.l	d0,a4
	move.l	24(a4),d0		; BV_NTBAS
	add.l	a4,d0
	move.l	d0,-24(a5)
	move.l	28(a4),d0		; BV_NTP
	add.l	a4,d0
	move.l	d0,a2
	move.l	32(a4),d0		; BV_NLBAS
	add.l	a4,d0
	move.l	d0,-16(a5)
	moveq	#8,d0
	sub.l	d0,a2
	bra	e_cmp
e_loop:
	cmp.b	#8,(a2)
	bne	e_next
	tst.b	1(a2)
	bne	e_next
	move.w	2(a2),d0
	ext.l	d0
	add.l	-16(a5),d0
	move.l	d0,-20(a5)
	pea	5
	move.l	-20(a5),a0
	addq.w	#1,a0
	pea	(a0)
	pea	envname
	jsr	_strncmp
	tst.l	d0
	lea	12(a7),a7
	bne	e_next
	move.l	4(a2),a0
	addq.w	#2,a0
	move.l	(a0),d3
	bra	e_user
e_next:
	subq.w	#8,a2
e_cmp:
	cmp.l	-24(a5),a2
	bcc	e_loop
e_user:
	jsr	__user
e_put:
	pea	envname+5		; ""
	jsr	__Putenv
	tst.l	d3
	beq	e_tk2
	move.l	d3,a6
	bra	e_ptst
e_plp:
	move.l	a6,-(a7)
	jsr	__Putenv
	move.l	a6,(a7)
	jsr	__StrLen
	addq.l	#1,d0
	add.l	d0,a6
	addq.w	#4,a7
e_ptst:
	tst.b	(a6)
	bne	e_plp
e_tk2:
	move.l	__sys_var,a0
	lea	172(a0),a3		; SV+$AC
	addq.w	#4,a7
	move.l	a3,d0
	beq	e_n1
	move.l	(a3)+,d0
	bra	e_s1
e_n1:
	move.l	#nulstr,d0
e_s1:
	move.l	d0,-(a7)
	pea	__prog_use
	bsr	e_set
	addq.w	#8,a7
	move.l	a3,d0
	beq	e_n2
	move.l	(a3)+,d0
	bra	e_s2
e_n2:
	move.l	#nulstr,d0
e_s2:
	move.l	d0,-(a7)
	pea	__data_use
	bsr	e_set
	addq.w	#8,a7
	move.l	a3,d0
	beq	e_n3
	move.l	(a3),d0
	bra	e_s3
e_n3:
	move.l	#nulstr,d0
e_s3:
	move.l	d0,-(a7)
	pea	__spl_use
	bsr	e_set
	movem.l	-48(a5),d3/a2-a4/a6
	unlk	a5
	rts

; e_set(char *name, char *qdos_string): if name is not in the environment, name=string
e_set:
	link	a5,#-50
	move.l	a2,-(a7)
	lea	-50(a5),a2
	clr.b	(a2)			; FIXED: empty buffer if __getcd writes nothing
	pea	50
	move.l	a2,-(a7)
	move.l	8(a5),-(a7)
	jsr	__getcd
	tst.b	(a2)
	bne	e_sfin
	move.l	12(a5),d0		; FIXED: validate the Toolkit II pointer
	beq	e_sfin			; null: there is no Toolkit II
	btst	#0,d0
	bne	e_sfin			; odd
	move.l	__sys_var,a0
	cmp.l	a0,d0
	bls	e_sfin			; below the system variables (ROM, screen)
	cmp.l	32(a0),d0		; SV_RAMT
	bcc	e_sfin			; above RAM
	move.l	d0,a0
	move.w	(a0),d0			; length of the QDOS string
	ble	e_sfin			; 0 or negative
	cmp.w	#40,d0
	bgt	e_sfin			; does not fit in the 50-byte buffers here and in __setcd
	move.l	12(a5),-(a7)
	move.l	a2,-(a7)
	jsr	_qlstr_to_c
	addq.w	#8,a7
	move.l	a2,-(a7)
	move.l	8(a5),-(a7)
	jsr	__setcd
e_sfin:
	move.l	-54(a5),a2
	unlk	a5
	rts

	.data
nulstr:
	.data2	0
