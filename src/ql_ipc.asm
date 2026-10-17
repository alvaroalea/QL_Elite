; ql_ipc.asm - llamada a MT.IPCOM (trap #1, D0=$11), sintaxis de as68 (XTC68).
; long ql_ipc(char *blk)  -> D1.B de la respuesta (0..255) o codigo de error negativo
; Convencion de c68: argumentos en la pila; el resultado va en D0.
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
