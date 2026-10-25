; ql_text.asm - texto con papel opaco escrito directamente en la pantalla del QL (modo 8), sintaxis de as68 (XTC68).
; long ql_puts(u8 *dst, const u8 *s, const unsigned short *font, const unsigned long *fm, long pp, long dd, long maxn)
;   (todo como palabras largas, como ql_trap3)
; Dibuja celdas de 8x8 una a continuacion de otra, mientras el caracter de s sea imprimible (32..127; el 0 final y cualquier otro valor
; paran) y no se hayan dibujado maxn; devuelve cuantos dibujo (el llamador avanza su cursor y su cadena). Equivale al camino opaco de
; draw_glyph (text.c): para cada una de las 8 lineas del glifo, linea = pp ^ (fm[b] & dd), escrita de una vez como palabra larga
; (4 bytes = 8 puntos) en dst + 128 * linea; la celda siguiente esta 4 bytes mas alla. font = 96 glifos x 8 palabras, cada una el byte de la
; fuente multiplicado por 4 (desplazamiento en bytes dentro de fm, que son 256 palabras largas: mascara de puntos de cada byte);
; pp = color del papel en los 4 bytes; dd = tinta XOR papel.
; Registros: a0 = destino, a1 = cadena, a2 = fuente, a3 = tabla fm, a4 = glifo, d2 = pp, d3 = dd, d4 = maxn, d5 = dibujados, d0/d1 = trabajo.
	.globl	_ql_puts
	.text
_ql_puts:
	movem.l	d2-d5/a2-a4,-(sp)
	move.l	32(sp),a0
	move.l	36(sp),a1
	move.l	40(sp),a2
	move.l	44(sp),a3
	move.l	48(sp),d2
	move.l	52(sp),d3
	move.l	56(sp),d4
	moveq	#0,d5
pt_loop:
	cmp.l	d4,d5
	bcc	pt_done
	moveq	#0,d0
	move.b	(a1),d0
	sub.w	#32,d0
	cmp.w	#95,d0
	bhi	pt_done
	lsl.w	#4,d0
	lea	0(a2,d0.w),a4
	move.w	0(a4),d0
	move.l	0(a3,d0.w),d1
	and.l	d3,d1
	eor.l	d2,d1
	move.l	d1,0(a0)
	move.w	2(a4),d0
	move.l	0(a3,d0.w),d1
	and.l	d3,d1
	eor.l	d2,d1
	move.l	d1,128(a0)
	move.w	4(a4),d0
	move.l	0(a3,d0.w),d1
	and.l	d3,d1
	eor.l	d2,d1
	move.l	d1,256(a0)
	move.w	6(a4),d0
	move.l	0(a3,d0.w),d1
	and.l	d3,d1
	eor.l	d2,d1
	move.l	d1,384(a0)
	move.w	8(a4),d0
	move.l	0(a3,d0.w),d1
	and.l	d3,d1
	eor.l	d2,d1
	move.l	d1,512(a0)
	move.w	10(a4),d0
	move.l	0(a3,d0.w),d1
	and.l	d3,d1
	eor.l	d2,d1
	move.l	d1,640(a0)
	move.w	12(a4),d0
	move.l	0(a3,d0.w),d1
	and.l	d3,d1
	eor.l	d2,d1
	move.l	d1,768(a0)
	move.w	14(a4),d0
	move.l	0(a3,d0.w),d1
	and.l	d3,d1
	eor.l	d2,d1
	move.l	d1,896(a0)
	addq.l	#1,a1
	addq.l	#4,a0
	addq.l	#1,d5
	bra	pt_loop
pt_done:
	move.l	d5,d0
	movem.l	(sp)+,d2-d5/a2-a4
	rts
