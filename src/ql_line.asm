; ql_line.asm - linea de Bresenham en XOR escrita directamente en la pantalla del QL (modo 8), sintaxis de as68 (XTC68).
; void ql_line(long x1, long y1, long x2, long y2, long c, long scr)   (todo como palabras largas, como ql_trap3)
; Equivale a loin_bres_c (lines.c): ordena los extremos (x menor primero; con la misma x, y menor primero), un punto por cada x si
; dx >= dy y por cada y si no, error inicial = mitad, los dos extremos incluidos. Coordenadas 0..255 (la pantalla tiene 256 lineas:
; no hace falta recortar); c = color 0..7 (bit 2 verde, bit 1 rojo, bit 0 azul); scr = direccion de la pantalla.
; Cada palabra son 4 puntos: byte par = verde (bit alto de cada pareja), byte impar = rojo (alto) y azul (bajo). P selecciona la
; pareja de bits del punto actual: $C0, $30, $0C, $03. Registros: a0 = palabra actual, d4 = P, d5 = mascara del byte par,
; d6 = mascara del byte impar, d7 = error, d2/d3 = dx/dy y contador, d1 = copia del paso grande.
	.globl	_ql_line
	.text
_ql_line:
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	48(sp),d1
	move.l	52(sp),d0
	move.l	56(sp),d2
	move.l	60(sp),d3
	cmp.l	d1,d2
	bhi	ordered
	bcs	xchg
	cmp.l	d0,d3
	bcc	ordered
xchg:
	exg	d1,d2
	exg	d0,d3
ordered:
	move.l	d0,d4
	lsl.l	#7,d4
	move.l	d1,d5
	lsr.l	#2,d5
	add.l	d5,d5
	add.l	d5,d4
	move.l	68(sp),a0
	adda.l	d4,a0
	move.l	d1,d5
	and.w	#3,d5
	add.w	d5,d5
	move.b	#$C0,d4
	lsr.b	d5,d4
	sub.l	d1,d2
	sub.l	d0,d3
	move.l	64(sp),d0
	moveq	#0,d5
	moveq	#0,d6
	btst	#2,d0
	beq	nog
	move.b	#$AA,d5
nog:
	btst	#1,d0
	beq	nor
	move.b	#$AA,d6
nor:
	btst	#0,d0
	beq	nob
	or.b	#$55,d6
nob:
	tst.l	d3
	bmi	goup
	cmp.l	d3,d2
	bcs	ymajd
	move.l	d2,d1
	move.l	d2,d7
	lsr.l	#1,d7
xd1:
	move.b	d5,d0
	and.b	d4,d0
	eor.b	d0,(a0)
	move.b	d6,d0
	and.b	d4,d0
	eor.b	d0,1(a0)
	sub.w	d3,d7
	bpl	xd2
	add.w	d1,d7
	lea	128(a0),a0
xd2:
	lsr.b	#2,d4
	bne	xd3
	move.b	#$C0,d4
	addq.l	#2,a0
xd3:
	dbra	d2,xd1
	bra	done
ymajd:
	move.l	d3,d1
	move.l	d3,d7
	lsr.l	#1,d7
yd1:
	move.b	d5,d0
	and.b	d4,d0
	eor.b	d0,(a0)
	move.b	d6,d0
	and.b	d4,d0
	eor.b	d0,1(a0)
	sub.w	d2,d7
	bpl	yd2
	add.w	d1,d7
	lsr.b	#2,d4
	bne	yd2
	move.b	#$C0,d4
	addq.l	#2,a0
yd2:
	lea	128(a0),a0
	dbra	d3,yd1
	bra	done
goup:
	neg.l	d3
	cmp.l	d3,d2
	bcs	ymaju
	move.l	d2,d1
	move.l	d2,d7
	lsr.l	#1,d7
xu1:
	move.b	d5,d0
	and.b	d4,d0
	eor.b	d0,(a0)
	move.b	d6,d0
	and.b	d4,d0
	eor.b	d0,1(a0)
	sub.w	d3,d7
	bpl	xu2
	add.w	d1,d7
	lea	-128(a0),a0
xu2:
	lsr.b	#2,d4
	bne	xu3
	move.b	#$C0,d4
	addq.l	#2,a0
xu3:
	dbra	d2,xu1
	bra	done
ymaju:
	move.l	d3,d1
	move.l	d3,d7
	lsr.l	#1,d7
yu1:
	move.b	d5,d0
	and.b	d4,d0
	eor.b	d0,(a0)
	move.b	d6,d0
	and.b	d4,d0
	eor.b	d0,1(a0)
	sub.w	d2,d7
	bpl	yu2
	add.w	d1,d7
	lsr.b	#2,d4
	bne	yu2
	move.b	#$C0,d4
	addq.l	#2,a0
yu2:
	lea	-128(a0),a0
	dbra	d3,yu1
done:
	movem.l	(sp)+,d2-d7/a2-a6
	rts
