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

; ql_trap3.asm (misma unidad) - llamada generica a un trap #3 de QDOS (E/S de canal), sintaxis as68.
; long ql_trap3(long d0, long d1, long d2, long d3, long a0, long a1)
; Carga los registros, ejecuta trap #3 y devuelve D0 (0 = ok, negativo = error QDOS, p. ej. -1 = ERR.NC).
; Los argumentos estan tras 11 registros guardados (44 bytes) + direccion de retorno: primero en 48(sp).
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

; ql_trap1 (misma unidad) - llamada generica a un trap #1 de QDOS (gestor de trabajos), sintaxis as68.
; long ql_trap1(long d0, long d1, long d2, long d3)
; Carga D0..D3, ejecuta trap #1 y devuelve D0. Se usa con D0=5 (MT.FRJOB), D1=-1 (este trabajo), D3=0 para
; terminar el programa sin pasar por la salida del runtime de c68 (que escribe "Press a key to exit" y espera una tecla).
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
