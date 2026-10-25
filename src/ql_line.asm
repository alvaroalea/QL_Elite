; ql_line.asm - linea de Bresenham en XOR escrita directamente en la pantalla del QL (modo 8), sintaxis de as68 (XTC68).
; void ql_line(long x1, long y1, long x2, long y2, long c, long scr)   (todo como palabras largas, como ql_trap3)
; Equivale a loin_bres_c (lines.c): ordena los extremos (x menor primero; con la misma x, y menor primero), un punto por cada x si
; dx >= dy y por cada y si no, error inicial = mitad, los dos extremos incluidos. Coordenadas 0..255 (la pantalla tiene 256 lineas:
; no hace falta recortar); c = color 0..7 (bit 2 verde, bit 1 rojo, bit 0 azul); scr = direccion de la pantalla.
; Cada palabra son 4 puntos: byte par = verde (bit alto de cada pareja), byte impar = rojo (alto) y azul (bajo). P selecciona la
; pareja de bits del punto actual: $C0, $30, $0C, $03. Registros: a0 = palabra actual, d4 = P, d5 = mascara del byte par,
; d6 = mascara del byte impar, d7 = error, d2/d3 = dx/dy y contador, d1 = copia del paso grande.
; Modo 4 (_gfx_m4 != 0): la linea va a los 512 puntos fisicos (x fisica = 2 x del juego): ver ql_line4 al final. Los colores de entrada son
; los del juego (0..7): negro, rojo (2) y verde (4) son ellos; todo lo demas, blanco.
	.globl	_ql_line
	.text
_ql_line:
	tst.b	_gfx_m4
	bne	_ql_line4
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

; ---------------------------------------------------------------------------------------------------------------------------------
; void ql_line4(long x1, long y1, long x2, long y2, long c, long scr)    modo 4, mismos argumentos que ql_line (x1, x2 del juego, 0..255)
; Los extremos se ordenan igual que en ql_line (con las x del juego) y luego x = 2 x: Bresenham sobre los 512 puntos (loin_bres_c con
; gfx_m4). Palabra = 8 puntos: byte par = verde, byte impar = rojo, bit 7 = el punto de la izquierda. Registros como en ql_line, con
; d4 = mascara del punto ($80, $40 ... $01), d5 = byte par de un punto lleno ($FF o 0) y d6 = byte impar.
	.globl	_ql_line4
_ql_line4:
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	48(sp),d1
	move.l	52(sp),d0
	move.l	56(sp),d2
	move.l	60(sp),d3
	cmp.l	d1,d2
	bhi	h_ordered
	bcs	h_xchg
	cmp.l	d0,d3
	bcc	h_ordered
h_xchg:
	exg	d1,d2
	exg	d0,d3
h_ordered:
	add.l	d1,d1
	add.l	d2,d2
	move.l	d0,d4
	lsl.l	#7,d4
	move.l	d1,d5
	lsr.l	#3,d5
	add.l	d5,d5
	add.l	d5,d4
	move.l	68(sp),a0
	adda.l	d4,a0
	move.l	d1,d5
	and.w	#7,d5
	move.b	#$80,d4
	lsr.b	d5,d4
	sub.l	d1,d2
	sub.l	d0,d3
	move.l	64(sp),d0
	moveq	#0,d5
	moveq	#0,d6
	tst.l	d0
	beq	h_cdone
	moveq	#-1,d5
	moveq	#-1,d6
	cmp.l	#2,d0
	bne	h_nored
	moveq	#0,d5
	bra	h_cdone
h_nored:
	cmp.l	#4,d0
	bne	h_cdone
	moveq	#0,d6
h_cdone:
	tst.l	d3
	bmi	h_goup
	cmp.l	d3,d2
	bcs	h_ymajd
	move.l	d2,d1
	move.l	d2,d7
	lsr.l	#1,d7
h_xd1:
	move.b	d5,d0
	and.b	d4,d0
	eor.b	d0,(a0)
	move.b	d6,d0
	and.b	d4,d0
	eor.b	d0,1(a0)
	sub.w	d3,d7
	bpl	h_xd2
	add.w	d1,d7
	lea	128(a0),a0
h_xd2:
	lsr.b	#1,d4
	bne	h_xd3
	move.b	#$80,d4
	addq.l	#2,a0
h_xd3:
	dbra	d2,h_xd1
	bra	h_done
h_ymajd:
	move.l	d3,d1
	move.l	d3,d7
	lsr.l	#1,d7
h_yd1:
	move.b	d5,d0
	and.b	d4,d0
	eor.b	d0,(a0)
	move.b	d6,d0
	and.b	d4,d0
	eor.b	d0,1(a0)
	sub.w	d2,d7
	bpl	h_yd2
	add.w	d1,d7
	lsr.b	#1,d4
	bne	h_yd2
	move.b	#$80,d4
	addq.l	#2,a0
h_yd2:
	lea	128(a0),a0
	dbra	d3,h_yd1
	bra	h_done
h_goup:
	neg.l	d3
	cmp.l	d3,d2
	bcs	h_ymaju
	move.l	d2,d1
	move.l	d2,d7
	lsr.l	#1,d7
h_xu1:
	move.b	d5,d0
	and.b	d4,d0
	eor.b	d0,(a0)
	move.b	d6,d0
	and.b	d4,d0
	eor.b	d0,1(a0)
	sub.w	d3,d7
	bpl	h_xu2
	add.w	d1,d7
	lea	-128(a0),a0
h_xu2:
	lsr.b	#1,d4
	bne	h_xu3
	move.b	#$80,d4
	addq.l	#2,a0
h_xu3:
	dbra	d2,h_xu1
	bra	h_done
h_ymaju:
	move.l	d3,d1
	move.l	d3,d7
	lsr.l	#1,d7
h_yu1:
	move.b	d5,d0
	and.b	d4,d0
	eor.b	d0,(a0)
	move.b	d6,d0
	and.b	d4,d0
	eor.b	d0,1(a0)
	sub.w	d2,d7
	bpl	h_yu2
	add.w	d1,d7
	lsr.b	#1,d4
	bne	h_yu2
	move.b	#$80,d4
	addq.l	#2,a0
h_yu2:
	lea	-128(a0),a0
	dbra	d3,h_yu1
h_done:
	movem.l	(sp)+,d2-d7/a2-a6
	rts
