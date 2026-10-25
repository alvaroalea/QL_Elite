; ql_ellrows.asm - actualiza el contorno de una elipse FILA A FILA: en cada linea de pantalla se borra (XOR) lo que habia de la elipse vieja y se
; dibuja lo de la nueva, una a continuacion de la otra, de modo que la elipse nunca desaparece del todo (solo la linea en curso esta a medias)
; y las filas que no cambian no se tocan. El contorno de cada fila sale de una tabla de tramos (gfx_ell_spans en gfx.c: 2 bytes por fila a dy lineas
; del centro, lo y hi: el contorno de la fila es cx + lo .. cx + hi y su reflejo; con lo = 0 un unico tramo cx - hi .. cx + hi), derivada de la tabla de
; semianchos de la elipse (half[dy]: fila dy < ry de half[dy + 1] + 1 a half[dy] si half[dy + 1] < half[dy], si no el punto half[dy]; fila ry de 0 a half[ry]). Es IDENTICO punto a punto al contorno de punto
; medio de gfx_ellipse_xor (comprobado en make test_ellrows). Sintaxis de as68; tools/test_ellrowsasm.py la ejecuta con tools/m68k_mini.py.
;
; void ql_ellrows(long *blk)    bloque de palabras largas (desplazamiento en bytes; solo se usan las palabras bajas de las coordenadas):
;   0 y (primera fila)  4 yfin (ultima fila, ya recortadas a la vista)
;   8 ocx  12 ocy  16 ory (-1 = no habia elipse vieja)  20 otab (tabla vieja)
;   24 ncx  28 ncy  32 nry (-1 = no hay elipse nueva)   36 ntab (tabla nueva)
;   40 GB (mascara del byte par de un punto lleno)  44 RB (del impar)  48 pantalla
; Registros: a2 = pantalla, a3 = tabla vieja, a4 = tabla nueva, a5 = inicio de la fila, a6 = bloque, d5/d6 = mascaras GB/RB.
	.globl	_ql_ellrows
	.text
_ql_ellrows:
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	48(sp),a6
	move.l	48(a6),a2
	move.l	40(a6),d5
	move.l	44(a6),d6
	move.l	20(a6),a3
	move.l	36(a6),a4
rowlp:
	move.w	2(a6),d0		; y
	move.w	d0,d1
	lsl.w	#7,d1
	lea	0(a2,d1.w),a5		; inicio de la fila y
	move.w	14(a6),d1		; elipse vieja: ocy, ory, tabla
	move.w	18(a6),d2
	move.l	a3,a0
	bsr	getspan
	move.w	d2,d4			; vieja: hi en d4, lo en d7
	move.w	d3,d7
	move.w	2(a6),d0
	move.w	30(a6),d1		; elipse nueva: ncy, nry, tabla
	move.w	34(a6),d2
	move.l	a4,a0
	bsr	getspan			; nueva: hi en d2, lo en d3
	tst.w	d4
	bpl	have
	tst.w	d2
	bmi	nextrow			; ninguna de las dos toca esta fila
have:
	cmp.w	d4,d2
	bne	draw
	cmp.w	d7,d3
	bne	draw
	move.w	10(a6),d0
	cmp.w	26(a6),d0
	beq	nextrow			; mismo tramo y mismo centro: nada que cambiar
draw:
	move.w	d2,-(sp)		; la nueva se guarda mientras se borra la vieja
	move.w	d3,-(sp)
	tst.w	d4
	bmi	dnew
	move.w	d4,d2
	move.w	d7,d3
	move.w	10(a6),d1
	bsr	spans
dnew:
	move.w	(sp)+,d3
	move.w	(sp)+,d2
	tst.w	d2
	bmi	nextrow
	move.w	26(a6),d1
	bsr	spans
nextrow:
	move.w	2(a6),d0
	addq.w	#1,d0
	move.w	d0,2(a6)
	cmp.w	6(a6),d0
	ble	rowlp
	movem.l	(sp)+,d2-d7/a2-a6
	rts

; getspan: d0 = y, d1 = cy, d2 = ry (-1 = sin elipse), a0 = tabla de tramos (2 bytes por fila: lo, hi)  ->  d2 = hi (-1 si la elipse no toca
; la fila), d3 = lo; destruye d1
getspan:
	move.w	d0,d3
	sub.w	d1,d3
	bpl	g1
	neg.w	d3			; dy = |y - cy|
g1:
	cmp.w	d2,d3
	bgt	gnone			; dy > ry
	add.w	d3,d3
	moveq	#0,d2
	move.b	1(a0,d3.w),d2		; hi
	moveq	#0,d1
	move.b	0(a0,d3.w),d1		; lo
	move.w	d1,d3
	rts
gnone:
	moveq	#-1,d2
	rts

; spans: d1 = cx (con signo), d2 = hi, d3 = lo (>= 0), a5 = fila: XOR de los tramos [cx - hi, cx - lo] y [cx + lo, cx + hi] (uno solo
; [cx - hi, cx + hi] si lo = 0); destruye d0-d4, d7, a0
spans:
	tst.w	d3
	bne	s1
	move.w	d1,d4
	sub.w	d2,d4			; x0 = cx - hi
	add.w	d2,d1			; x1 = cx + hi
	move.w	d1,d2
	move.w	d4,d1
	bra	run
s1:
	cmp.w	d2,d3
	bne	s2
	move.w	d1,d4			; lo = hi: un punto a cada lado (cx - hi y cx + hi)
	sub.w	d2,d4
	bmi	s2
	add.w	d2,d1
	cmp.w	#255,d1
	bgt	s2b
	move.w	d1,d2
	move.w	d4,d1
	bsr	r_pt
	move.w	d2,d1
	bra	r_pt
s2b:
	sub.w	d2,d1			; (se deshace la suma: fuera de la vista, camino general)
s2:
	move.w	d1,d0
	add.w	d3,d0			; x0 del tramo derecho = cx + lo
	move.w	d1,d4
	add.w	d2,d4			; x1 = cx + hi
	move.w	d0,-(sp)
	move.w	d4,-(sp)
	move.w	d1,d0
	sub.w	d3,d0			; x1 del tramo izquierdo = cx - lo
	sub.w	d2,d1			; x0 = cx - hi
	move.w	d0,d2
	bsr	run
	move.w	(sp)+,d2
	move.w	(sp)+,d1
				; sigue en run

; run: d1 = x0, d2 = x1 (con signo): recorta a 0..255 y dibuja
run:
	tst.w	d2
	bmi	rdone
	cmp.w	#255,d1
	bgt	rdone
	tst.w	d1
	bpl	r1
	moveq	#0,d1
r1:
	cmp.w	#255,d2
	ble	r2
	move.w	#255,d2
r2:
	cmp.w	d1,d2
	bne	r_hrun
				; un solo punto: sigue en r_pt
; r_pt: XOR de un punto (d1 = x, 0..255) en la fila a5; destruye d0, d1, d7, a0
r_pt:
	move.w	d1,d0
	lsr.w	#2,d0
	add.w	d0,d0
	lea	0(a5,d0.w),a0
	and.w	#3,d1
	add.w	d1,d1
	move.b	#$C0,d0
	lsr.b	d1,d0
	move.b	d5,d7
	and.b	d0,d7
	eor.b	d7,(a0)
	move.b	d6,d7
	and.b	d0,d7
	eor.b	d7,1(a0)
rdone:
	rts

; r_hrun: XOR de los puntos d1..d2 (0 <= d1 < d2 <= 255) de la fila a5; destruye d0-d4, d7, a0
r_hrun:
	move.w	d1,d3
	lsr.w	#2,d3			; palabra del primer punto
	move.w	d2,d4
	lsr.w	#2,d4			; palabra del ultimo
	move.w	d3,d0
	add.w	d0,d0
	lea	0(a5,d0.w),a0
	move.w	d1,d0
	and.w	#3,d0
	add.w	d0,d0
	move.b	#$FF,d7
	lsr.b	d0,d7			; mascara izquierda
	move.w	d2,d0
	and.w	#3,d0
	add.w	d0,d0
	neg.w	d0
	add.w	#6,d0
	move.b	#$FF,d1
	lsl.b	d0,d1			; mascara derecha
	cmp.w	d3,d4
	bne	h_multi
	and.b	d1,d7
	move.b	d5,d0
	and.b	d7,d0
	eor.b	d0,(a0)
	move.b	d6,d0
	and.b	d7,d0
	eor.b	d0,1(a0)
	rts
h_multi:
	move.b	d5,d0
	and.b	d7,d0
	eor.b	d0,(a0)
	move.b	d6,d0
	and.b	d7,d0
	eor.b	d0,1(a0)
	addq.l	#2,a0
	move.w	d4,d0
	sub.w	d3,d0
	subq.w	#1,d0
	beq	h_last
	subq.w	#1,d0
h_mid:
	eor.b	d5,(a0)+
	eor.b	d6,(a0)+
	dbra	d0,h_mid
h_last:
	move.b	d5,d0
	and.b	d1,d0
	eor.b	d0,(a0)
	move.b	d6,d0
	and.b	d1,d0
	eor.b	d0,1(a0)
	rts

; ---------------------------------------------------------------------------------------------------------------------------------
; void ql_ellrows4(long *blk)    modo 4, a los 512 puntos fisicos: el mismo bloque que ql_ellrows, pero ocx y ncx ya son x fisicas (2 x del
; juego), las tablas de tramos son de palabras de 16 bits (lo, hi: 4 bytes por fila, gfx_ell_spans en modo 4), los tramos se recortan a
; 0..511 y GB/RB valen $FF o 0. Es ql_ellrows con 8 puntos por palabra.
	.globl	_ql_ellrows4
_ql_ellrows4:
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	48(sp),a6
	move.l	48(a6),a2
	move.l	40(a6),d5
	move.l	44(a6),d6
	move.l	20(a6),a3
	move.l	36(a6),a4
rowlp4:
	move.w	2(a6),d0		; y
	move.w	d0,d1
	lsl.w	#7,d1
	lea	0(a2,d1.w),a5		; inicio de la fila y
	move.w	14(a6),d1		; elipse vieja: ocy, ory, tabla
	move.w	18(a6),d2
	move.l	a3,a0
	bsr	getspan4
	move.w	d2,d4			; vieja: hi en d4, lo en d7
	move.w	d3,d7
	move.w	2(a6),d0
	move.w	30(a6),d1		; elipse nueva: ncy, nry, tabla
	move.w	34(a6),d2
	move.l	a4,a0
	bsr	getspan4			; nueva: hi en d2, lo en d3
	tst.w	d4
	bpl	have4
	tst.w	d2
	bmi	nextrow4			; ninguna de las dos toca esta fila
have4:
	cmp.w	d4,d2
	bne	draw4
	cmp.w	d7,d3
	bne	draw4
	move.w	10(a6),d0
	cmp.w	26(a6),d0
	beq	nextrow4			; mismo tramo y mismo centro: nada que cambiar
draw4:
	move.w	d2,-(sp)		; la nueva se guarda mientras se borra la vieja
	move.w	d3,-(sp)
	tst.w	d4
	bmi	dnew4
	move.w	d4,d2
	move.w	d7,d3
	move.w	10(a6),d1
	bsr	spans4
dnew4:
	move.w	(sp)+,d3
	move.w	(sp)+,d2
	tst.w	d2
	bmi	nextrow4
	move.w	26(a6),d1
	bsr	spans4
nextrow4:
	move.w	2(a6),d0
	addq.w	#1,d0
	move.w	d0,2(a6)
	cmp.w	6(a6),d0
	ble	rowlp4
	movem.l	(sp)+,d2-d7/a2-a6
	rts

; getspan4: d0 = y, d1 = cy, d2 = ry (-1 = sin elipse), a0 = tabla de tramos (2 bytes por fila: lo, hi)  ->  d2 = hi (-1 si la elipse no toca
; la fila), d3 = lo; destruye d1
getspan4:
	move.w	d0,d3
	sub.w	d1,d3
	bpl	g14
	neg.w	d3			; dy = |y - cy|
g14:
	cmp.w	d2,d3
	bgt	gnone4			; dy > ry
	lsl.w	#2,d3
	move.w	2(a0,d3.w),d2		; hi
	move.w	0(a0,d3.w),d1		; lo
	move.w	d1,d3
	rts
gnone4:
	moveq	#-1,d2
	rts

; spans4: d1 = cx (con signo), d2 = hi, d3 = lo (>= 0), a5 = fila: XOR de los tramos [cx - hi, cx - lo] y [cx + lo, cx + hi] (uno solo
; [cx - hi, cx + hi] si lo = 0); destruye d0-d4, d7, a0
spans4:
	tst.w	d3
	bne	s14
	move.w	d1,d4
	sub.w	d2,d4			; x0 = cx - hi
	add.w	d2,d1			; x1 = cx + hi
	move.w	d1,d2
	move.w	d4,d1
	bra	run4
s14:
	cmp.w	d2,d3
	bne	s24
	move.w	d1,d4			; lo = hi: un punto a cada lado (cx - hi y cx + hi)
	sub.w	d2,d4
	bmi	s24
	add.w	d2,d1
	cmp.w	#511,d1
	bgt	s2b4
	move.w	d1,d2
	move.w	d4,d1
	bsr	r_pt4
	move.w	d2,d1
	bra	r_pt4
s2b4:
	sub.w	d2,d1			; (se deshace la suma: fuera de la vista, camino general)
s24:
	move.w	d1,d0
	add.w	d3,d0			; x0 del tramo derecho = cx + lo
	move.w	d1,d4
	add.w	d2,d4			; x1 = cx + hi
	move.w	d0,-(sp)
	move.w	d4,-(sp)
	move.w	d1,d0
	sub.w	d3,d0			; x1 del tramo izquierdo = cx - lo
	sub.w	d2,d1			; x0 = cx - hi
	move.w	d0,d2
	bsr	run4
	move.w	(sp)+,d2
	move.w	(sp)+,d1
				; sigue en run4

; run4: d1 = x0, d2 = x1 (con signo): recorta a 0..255 y dibuja
run4:
	tst.w	d2
	bmi	rdone4
	cmp.w	#511,d1
	bgt	rdone4
	tst.w	d1
	bpl	r14
	moveq	#0,d1
r14:
	cmp.w	#511,d2
	ble	r24
	move.w	#511,d2
r24:
	cmp.w	d1,d2
	bne	r_hrun4
				; un solo punto: sigue en r_pt4
; r_pt4: XOR de un punto (d1 = x, 0..255) en la fila a5; destruye d0, d1, d7, a0
r_pt4:
	move.w	d1,d0
	lsr.w	#3,d0
	add.w	d0,d0
	lea	0(a5,d0.w),a0
	and.w	#7,d1
	move.b	#$80,d0
	lsr.b	d1,d0
	move.b	d5,d7
	and.b	d0,d7
	eor.b	d7,(a0)
	move.b	d6,d7
	and.b	d0,d7
	eor.b	d7,1(a0)
rdone4:
	rts

; r_hrun4: XOR de los puntos d1..d2 (0 <= d1 < d2 <= 255) de la fila a5; destruye d0-d4, d7, a0
r_hrun4:
	move.w	d1,d3
	lsr.w	#3,d3			; palabra del primer punto
	move.w	d2,d4
	lsr.w	#3,d4			; palabra del ultimo
	move.w	d3,d0
	add.w	d0,d0
	lea	0(a5,d0.w),a0
	move.w	d1,d0
	and.w	#7,d0
	move.b	#$FF,d7
	lsr.b	d0,d7			; mascara izquierda
	move.w	d2,d0
	and.w	#7,d0
	neg.w	d0
	add.w	#7,d0
	move.b	#$FF,d1
	lsl.b	d0,d1			; mascara derecha
	cmp.w	d3,d4
	bne	h_multi4
	and.b	d1,d7
	move.b	d5,d0
	and.b	d7,d0
	eor.b	d0,(a0)
	move.b	d6,d0
	and.b	d7,d0
	eor.b	d0,1(a0)
	rts
h_multi4:
	move.b	d5,d0
	and.b	d7,d0
	eor.b	d0,(a0)
	move.b	d6,d0
	and.b	d7,d0
	eor.b	d0,1(a0)
	addq.l	#2,a0
	move.w	d4,d0
	sub.w	d3,d0
	subq.w	#1,d0
	beq	h_last4
	subq.w	#1,d0
h_mid4:
	eor.b	d5,(a0)+
	eor.b	d6,(a0)+
	dbra	d0,h_mid4
h_last4:
	move.b	d5,d0
	and.b	d1,d0
	eor.b	d0,(a0)
	move.b	d6,d0
	and.b	d1,d0
	eor.b	d0,1(a0)
	rts
