	.text
	.globl _start
	.globl _X_lmul
	.globl _X_ulmul
	.globl _X_ldiv
	.globl _plat_init
	.globl _plat_shutdown
	.globl _plat_flush_input
	.globl _plat_wait_key
	.globl _plat_present
	.globl _screen
_start:
	jsr _main
	moveq #1,d0
	clr.l d1
	trap #0
_X_lmul:
_X_ulmul:
	move.l (a7)+,a0
	move.l (a7)+,d0
	move.l (a7)+,d1
	muls.l d1,d0
	jmp (a0)
_X_ldiv:
	move.l (a7)+,a0
	move.l (a7)+,d0
	move.l (a7)+,d1
	divs.l d1,d0
	jmp (a0)
	.globl _X_uldiv
	.globl _X_lrem
	.globl _X_ulrem
_X_uldiv:
	move.l (a7)+,a0
	move.l (a7)+,d0
	move.l (a7)+,d1
	divu.l d1,d0
	jmp (a0)
_X_lrem:
	move.l (a7)+,a0
	move.l (a7)+,d0
	move.l (a7)+,d1
	move.l d2,-(a7)
	move.l d0,d2
	divs.l d1,d0
	muls.l d1,d0
	sub.l d0,d2
	move.l d2,d0
	move.l (a7)+,d2
	jmp (a0)
_X_ulrem:
	move.l (a7)+,a0
	move.l (a7)+,d0
	move.l (a7)+,d1
	move.l d2,-(a7)
	move.l d0,d2
	divu.l d1,d0
	muls.l d1,d0
	sub.l d0,d2
	move.l d2,d0
	move.l (a7)+,d2
	jmp (a0)
	.globl _fopen
	.globl _fread
	.globl _fwrite
	.globl _fclose
_fopen:
_fread:
_fwrite:
_fclose:
	moveq #0,d0
	rts
	.globl _X_asuldiv
_X_asuldiv:
	move.l (a7)+,a0
	move.l (a7)+,a1
	move.l (a7)+,d1
	move.l (a1),d0
	divu.l d1,d0
	move.l d0,(a1)
	jmp (a0)
	.globl _X_asldiv
_X_asldiv:
	move.l (a7)+,a0
	move.l (a7)+,a1
	move.l (a7)+,d1
	move.l (a1),d0
	divs.l d1,d0
	move.l d0,(a1)
	jmp (a0)
	.globl _X_asulmul
_X_asulmul:
	move.l (a7)+,a0
	move.l (a7)+,a1
	move.l (a7)+,d1
	move.l (a1),d0
	muls.l d1,d0
	move.l d0,(a1)
	jmp (a0)
	.globl _X_aslmul
_X_aslmul:
	move.l (a7)+,a0
	move.l (a7)+,a1
	move.l (a7)+,d1
	move.l (a1),d0
	muls.l d1,d0
	move.l d0,(a1)
	jmp (a0)
	.globl _X_asulrem
_X_asulrem:
	move.l (a7)+,a0
	move.l (a7)+,a1
	move.l (a7)+,d1
	move.l d2,-(a7)
	move.l (a1),d2
	move.l d2,d0
	divu.l d1,d0
	muls.l d1,d0
	sub.l d0,d2
	move.l d2,d0
	move.l d2,(a1)
	move.l (a7)+,d2
	jmp (a0)
	.globl _X_aslrem
_X_aslrem:
	move.l (a7)+,a0
	move.l (a7)+,a1
	move.l (a7)+,d1
	move.l d2,-(a7)
	move.l (a1),d2
	move.l d2,d0
	divs.l d1,d0
	muls.l d1,d0
	sub.l d0,d2
	move.l d2,d0
	move.l d2,(a1)
	move.l (a7)+,d2
	jmp (a0)
_plat_init:
_plat_shutdown:
_plat_flush_input:
_plat_wait_key:
	rts
_plat_present:
	moveq #5,d0
	lea fname,a1
	move.l a1,d1
	move.l #0x241,d2
	move.l #0644,d3
	trap #0
	move.l d0,d4
	moveq #4,d0
	move.l d4,d1
	move.l _screen,d2
	move.l #32768,d3
	trap #0
	moveq #6,d0
	move.l d4,d1
	trap #0
	rts
	.data
_screen: .long scrbuf
fname: .asciz "screen.bin"
	.bss
	.balign 4
scrbuf: .space 32768
