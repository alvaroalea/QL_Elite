; ql_ell.asm - elipse de punto medio y tramo horizontal en XOR escritos directamente en la pantalla del QL (modo 8), sintaxis de as68.
; Equivalen a gfx_ellipse_xor y gfx_hline_xor de gfx.c (que prepara los datos). Cada palabra son 4 puntos: byte par = verde (bit alto de
; cada pareja de bits), byte impar = rojo (alto) y azul (bajo); un punto se enciende con la mascara $C0, $30, $0C o $03.
; Sin recorte de y/x por parte de ql_hline (el llamante deja x en 0..255 e y en 0..255); ql_ellipse recorta cada punto al rectangulo.

; ---------------------------------------------------------------------------------------------------------------------------------
; void ql_ellipse(long *blk)    bloque de palabras largas (desplazamiento en bytes):
;   0 a8 (8 a^2)  4 b8 (8 b^2)  8 GB  12 RB  16 clx0  20 cly0  24 clx1  28 cly1  32 dx  36 dy  40 -  44 err  48 scr  52 2*ry
;   56 x0  60 x1  64 y0  68 y1        (dx, dy, err y x0..y1 se modifican)
; GB/RB: mascaras del byte par/impar para un punto lleno (GB = $AA si hay verde, RB = $AA si hay rojo y $55 si hay azul).
; Registros: d1 = x0, d2 = x1, d3 = y0, d4 = y1, a0 = bloque; el punto a dibujar va en d5 (x) y d6 (y) a la subrutina e_plot.
	.globl	_ql_ellipse
	.text
_ql_ellipse:
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	48(sp),a0
	move.l	56(a0),d1
	move.l	60(a0),d2
	move.l	64(a0),d3
	move.l	68(a0),d4
e_loop:
	move.l	d2,d5
	move.l	d3,d6
	bsr	e_plot
	cmp.l	d2,d1
	beq	e_p2
	move.l	d1,d5
	move.l	d3,d6
	bsr	e_plot
e_p2:
	cmp.l	d4,d3
	beq	e_p3
	move.l	d1,d5
	move.l	d4,d6
	bsr	e_plot
	cmp.l	d2,d1
	beq	e_p3
	move.l	d2,d5
	move.l	d4,d6
	bsr	e_plot
e_p3:
	move.l	44(a0),d0
	add.l	d0,d0
	cmp.l	36(a0),d0
	bgt	e_ny
	addq.l	#1,d3
	subq.l	#1,d4
	move.l	36(a0),d5
	add.l	0(a0),d5
	move.l	d5,36(a0)
	add.l	d5,44(a0)
e_ny:
	cmp.l	32(a0),d0
	bge	e_sx
	move.l	44(a0),d5
	add.l	d5,d5
	cmp.l	36(a0),d5
	ble	e_nx
e_sx:
	addq.l	#1,d1
	subq.l	#1,d2
	move.l	32(a0),d5
	add.l	4(a0),d5
	move.l	d5,32(a0)
	add.l	d5,44(a0)
e_nx:
	cmp.l	d2,d1
	ble	e_loop
; cola de las elipses estrechas: si y0 - y1 < 2 ry, filas hasta y0 - y1 = 2 ry (en x0 - 1 y x1 + 1)
	move.l	d3,d5
	sub.l	d4,d5
	cmp.l	52(a0),d5
	bge	e_end
e_tail:
	move.l	d1,d5
	subq.l	#1,d5
	move.l	d3,d6
	bsr	e_plot
	move.l	d2,d0
	addq.l	#1,d0
	move.l	d1,d5
	subq.l	#1,d5
	cmp.l	d5,d0
	beq	e_t2
	move.l	d0,d5
	move.l	d3,d6
	bsr	e_plot
e_t2:
	cmp.l	d4,d3
	beq	e_t4
	move.l	d1,d5
	subq.l	#1,d5
	move.l	d4,d6
	bsr	e_plot
	move.l	d2,d0
	addq.l	#1,d0
	move.l	d1,d5
	subq.l	#1,d5
	cmp.l	d5,d0
	beq	e_t4
	move.l	d0,d5
	move.l	d4,d6
	bsr	e_plot
e_t4:
	addq.l	#1,d3
	subq.l	#1,d4
	move.l	d3,d5
	sub.l	d4,d5
	cmp.l	52(a0),d5
	ble	e_tail
e_end:
	movem.l	(sp)+,d2-d7/a2-a6
	rts

; e_plot: XOR de un punto (d5 = x, d6 = y) si esta dentro del rectangulo de recorte; destruye d0, d7, a1
e_plot:
	cmp.l	16(a0),d5
	blt	e_out
	cmp.l	24(a0),d5
	bgt	e_out
	cmp.l	20(a0),d6
	blt	e_out
	cmp.l	28(a0),d6
	bgt	e_out
	move.l	d6,d7
	lsl.l	#7,d7
	move.l	d5,d0
	lsr.l	#2,d0
	add.l	d0,d0
	add.l	d0,d7
	move.l	48(a0),a1
	adda.l	d7,a1
	move.l	d5,d7
	and.w	#3,d7
	add.w	d7,d7
	move.b	#$C0,d0
	lsr.b	d7,d0
	move.b	11(a0),d7
	and.b	d0,d7
	eor.b	d7,(a1)
	move.b	15(a0),d7
	and.b	d0,d7
	eor.b	d7,1(a1)
e_out:
	rts

; ---------------------------------------------------------------------------------------------------------------------------------
; Subrutinas de color y de lineas rectas (en registros; las usan ql_hline, ql_vline y ql_frame)
; s_col:   d3 = color 0..7  ->  d5 = mascara del byte par (verde), d6 = mascara del byte impar (rojo/azul) de un punto lleno
; s_hline: d1 = x0, d2 = x1 (x0 <= x1, 0..255), d0 = y (0..255), d5/d6 = mascaras, a2 = pantalla; destruye d0-d4, d7, a0
; s_vline: d1 = x (0..255), d2 = y0, d3 = y1 (y0 <= y1, 0..255), d5/d6 = mascaras, a2 = pantalla; destruye d0-d4, d7, a0
s_col:
	moveq	#0,d5
	moveq	#0,d6
	btst	#2,d3
	beq	c_ng
	move.b	#$AA,d5
c_ng:
	btst	#1,d3
	beq	c_nr
	move.b	#$AA,d6
c_nr:
	btst	#0,d3
	beq	c_nb
	or.b	#$55,d6
c_nb:
	rts

s_hline:
	move.l	a2,a0
	lsl.l	#7,d0
	adda.l	d0,a0
	move.l	d1,d3
	lsr.l	#2,d3
	move.l	d2,d4
	lsr.l	#2,d4
	move.l	d3,d0
	add.l	d0,d0
	adda.l	d0,a0
	move.l	d1,d0
	and.w	#3,d0
	add.w	d0,d0
	move.b	#$FF,d7
	lsr.b	d0,d7
	move.l	d2,d0
	and.w	#3,d0
	add.w	d0,d0
	neg.w	d0
	add.w	#6,d0
	move.b	#$FF,d1
	lsl.b	d0,d1
	cmp.l	d3,d4
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
	move.l	d4,d0
	sub.l	d3,d0
	subq.l	#1,d0
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

s_vline:
	move.l	d1,d0
	and.w	#3,d0
	add.w	d0,d0
	move.b	#$C0,d7
	lsr.b	d0,d7
	move.b	d5,d0
	and.b	d7,d0
	move.b	d6,d4
	and.b	d7,d4
	move.l	a2,a0
	move.l	d2,d7
	lsl.l	#7,d7
	adda.l	d7,a0
	move.l	d1,d7
	lsr.l	#2,d7
	add.l	d7,d7
	adda.l	d7,a0
	sub.l	d2,d3
v_lp:
	eor.b	d0,(a0)
	eor.b	d4,1(a0)
	lea	128(a0),a0
	dbra	d3,v_lp
	rts

; ---------------------------------------------------------------------------------------------------------------------------------
; void ql_hline(long x0, long x1, long y, long c, long scr)    x0 <= x1, 0..255; y 0..255; XOR de los puntos x0..x1 de la fila y
	.globl	_ql_hline
_ql_hline:
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	48(sp),d1
	move.l	52(sp),d2
	move.l	56(sp),d0
	move.l	60(sp),d3
	move.l	64(sp),a2
	bsr	s_col
	bsr	s_hline
	movem.l	(sp)+,d2-d7/a2-a6
	rts

; void ql_vline(long x, long y0, long y1, long c, long scr)    y0 <= y1, 0..255; x 0..255; XOR de los puntos (x, y0..y1)
	.globl	_ql_vline
_ql_vline:
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	48(sp),d1
	move.l	52(sp),d2
	move.l	60(sp),d3
	move.l	64(sp),a2
	bsr	s_col
	move.l	56(sp),d3
	bsr	s_vline
	movem.l	(sp)+,d2-d7/a2-a6
	rts

; ---------------------------------------------------------------------------------------------------------------------------------
; void ql_frame(long *blk)    contorno de rectangulo en XOR, cada punto una vez, recortado; bloque de palabras largas:
;   0 x0  4 y0  8 x1  12 y1 (aristas)   16 clx0  20 cly0  24 clx1  28 cly1 (recorte)   32 GB  36 RB  40 pantalla
; Arriba y abajo: filas y0 e y1 (x0..x1 recortado); izquierda y derecha: columnas x0 y x1 entre y0 + 1 e y1 - 1 (recortado); la
; fila de abajo solo si y1 != y0 y la columna derecha solo si x1 != x0.
	.globl	_ql_frame
_ql_frame:
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	48(sp),a3
	move.l	32(a3),d5
	move.l	36(a3),d6
	move.l	40(a3),a2
	move.l	4(a3),d0
	bsr	f_row
	move.l	12(a3),d0
	cmp.l	4(a3),d0
	beq	f_nb
	bsr	f_row
f_nb:
	move.l	(a3),d1
	bsr	f_col
	move.l	8(a3),d1
	cmp.l	(a3),d1
	beq	f_done
	bsr	f_col
f_done:
	movem.l	(sp)+,d2-d7/a2-a6
	rts

; f_row: d0 = y; pinta x0..x1 recortado si la fila esta dentro del recorte
f_row:
	cmp.l	20(a3),d0
	blt	f_rx
	cmp.l	28(a3),d0
	bgt	f_rx
	move.l	(a3),d1
	cmp.l	16(a3),d1
	bge	f_ra
	move.l	16(a3),d1
f_ra:
	move.l	8(a3),d2
	cmp.l	24(a3),d2
	ble	f_rb
	move.l	24(a3),d2
f_rb:
	cmp.l	d1,d2
	blt	f_rx
	bsr	s_hline
f_rx:
	rts

; f_col: d1 = x; pinta y0 + 1 .. y1 - 1 recortado si la columna esta dentro del recorte
f_col:
	cmp.l	16(a3),d1
	blt	f_cx
	cmp.l	24(a3),d1
	bgt	f_cx
	move.l	4(a3),d2
	addq.l	#1,d2
	cmp.l	20(a3),d2
	bge	f_ca
	move.l	20(a3),d2
f_ca:
	move.l	12(a3),d3
	subq.l	#1,d3
	cmp.l	28(a3),d3
	ble	f_cb
	move.l	28(a3),d3
f_cb:
	cmp.l	d2,d3
	blt	f_cx
	bsr	s_vline
f_cx:
	rts

; ---------------------------------------------------------------------------------------------------------------------------------
; void ql_xpix(long x, long y, long c, long scr)    XOR de un punto (x, y), 0..255, color c (0..7)
; Solo usa d0-d3 y a0 (guarda d2-d3): sin movem de todos los registros, para llamarla muy a menudo.
	.globl	_ql_xpix
_ql_xpix:
	movem.l	d2-d3,-(sp)
	move.l	12(sp),d0
	move.l	16(sp),d1
	move.l	24(sp),a0
	lsl.l	#7,d1
	adda.l	d1,a0
	move.l	d0,d1
	lsr.l	#2,d1
	add.l	d1,d1
	adda.l	d1,a0
	and.w	#3,d0
	add.w	d0,d0
	move.b	#$C0,d1
	lsr.b	d0,d1
	move.l	20(sp),d0
	move.b	d1,d2
	and.b	#$AA,d2
	move.b	d1,d3
	and.b	#$55,d3
	btst	#2,d0
	beq	x_ng
	eor.b	d2,(a0)
x_ng:
	moveq	#0,d1
	btst	#1,d0
	beq	x_nr
	move.b	d2,d1
x_nr:
	btst	#0,d0
	beq	x_nb
	or.b	d3,d1
x_nb:
	eor.b	d1,1(a0)
	movem.l	(sp)+,d2-d3
	rts

; ---------------------------------------------------------------------------------------------------------------------------------
; void ql_clearcells(long col, long row, long ncols, long nrows, long scr)    pone a negro (0) un rectangulo de celdas de texto (8 x 8
; puntos = 4 bytes de ancho): ncols columnas desde col y nrows filas desde row; escribe palabras largas (las celdas son multiplo de 4)
	.globl	_ql_clearcells
_ql_clearcells:
	movem.l	d2-d4,-(sp)
	move.l	20(sp),d0
	lsl.l	#8,d0
	lsl.l	#2,d0
	move.l	16(sp),d1
	lsl.l	#2,d1
	add.l	d1,d0
	move.l	32(sp),a0
	adda.l	d0,a0
	move.l	24(sp),d1
	beq	k_end
	move.l	28(sp),d2
	lsl.l	#3,d2
	beq	k_end
	move.l	d1,d3
	lsl.l	#2,d3
	neg.l	d3
	add.l	#128,d3
	subq.w	#1,d1
	subq.w	#1,d2
	moveq	#0,d0
k_row:
	move.w	d1,d4
k_col:
	move.l	d0,(a0)+
	dbra	d4,k_col
	adda.l	d3,a0
	dbra	d2,k_row
k_end:
	movem.l	(sp)+,d2-d4
	rts
