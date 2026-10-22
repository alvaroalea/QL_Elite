; ql_ll9.asm - las partes calientes de LL9 (ship3d.c) en ensamblador del 68000 (as68): producto de la matriz (LL51), caras visibles (LL86..LL88) y
; proyeccion de los vertices (LL48..LL50). Bit a bit como las versiones en C (ship_ll9_faces_c y ship_ll9_verts_c); tools/test_ll9asm.py las
; ejecuta (con tools/m68k_mini.py) contra el C con miles de entradas al azar.
;
; OJO: los `int` de qcc son de 32 BITS (c68 por defecto, `.data4`): el ensamblador solo lee por simbolo variables de tipo u8 o puntero; los int
; (view_cy, view_yscale, clip_h) llegan como argumentos long.
; Convenciones: variables de C como simbolos externos (_zx15, _zx12, _zxx16, _zxx18, _zk3 = XX2, _xx3, _inwk, ...). Tablas de logaritmos
; intercaladas en _ql_lltab (src/ql_lltab.asm, 4 bytes por entrada: log, logL, alogh, 0): a5 = tabla; el indice es i*4 (el modo indexado solo
; tiene un desplazamiento de 8 bits). Las subrutinas trabajan en registros:
;   fmltu: d0 = a, d1 = Q (bytes, palabra baja limpia) -> d0 = resultado (palabra limpia); destruye d1-d4.
;   ll38 : d0 = A (signo en el bit 7), d1 = Q, d2 = R, d3 = S -> d0 = resultado, d3 = S nuevo, C = acarreo (em_carry); destruye solo d0 y d3.
;   ll28n: d0 = a (< Q), d1 = Q -> d0 = 256 * a / Q; destruye d1-d4.
;   ll61 : d0 = a, d1 = Q (U = 0 al entrar) -> d0 = R, d5 = U; destruye d1-d4, d6.
; No se escriben de vuelta los temporales de C (zQ, zR, zS, zT, zU, zx12/zx15 al salir, em_carry): nadie los lee despues de LL9.
	.globl	_ql_ll51
	.globl	_ql_ll9_faces
	.globl	_ql_ll9_verts
	.text

; ---------------------------------------------------------------------------------------------------------- subrutinas
fmltu:
	tst.b	d0
	beq	fm0
	tst.b	d1
	beq	fm0
	lsl.w	#2,d0
	lsl.w	#2,d1
	move.b	1(a5,d0.w),d2		; logL[a]
	add.b	1(a5,d1.w),d2		; + logL[Q]: C = acarreo de la parte baja
	move.b	0(a5,d0.w),d3		; log[a]
	move.b	0(a5,d1.w),d4		; log[Q]
	addx.b	d4,d3			; hi = log[a] + log[Q] + acarreo; C = hi >= 256
	bcc	fm0
	moveq	#0,d4
	move.b	d3,d4
	lsl.w	#2,d4
	moveq	#0,d0
	move.b	2(a5,d4.w),d0		; alogh[hi]
	rts
fm0:
	moveq	#0,d0
	rts

ll38:
	eor.b	d3,d0
	bmi	l38d			; signos distintos
	move.b	d1,d0
	add.b	d2,d0			; Q + R, C = acarreo
	rts
l38d:
	move.b	d2,d0
	sub.b	d1,d0			; R - Q
	bcs	l38n
	rts				; sin prestamo: R - Q, C = 0
l38n:
	eori.b	#$80,d3
	move.b	d1,d0
	sub.b	d2,d0			; Q - R, C = 0
	rts

ll28n:
	tst.b	d0
	beq	l28z
	lsl.w	#2,d0
	lsl.w	#2,d1
	move.b	1(a5,d0.w),d2		; logL[a]
	sub.b	1(a5,d1.w),d2		; - logL[Q]: X = C = prestamo
	move.b	0(a5,d0.w),d3
	move.b	0(a5,d1.w),d4
	subx.b	d4,d3			; log[a] - log[Q] - prestamo
	bcc	l28ff			; sin prestamo: cociente >= 1 -> 255
	moveq	#0,d4
	move.b	d3,d4
	lsl.w	#2,d4
	moveq	#0,d0
	move.b	2(a5,d4.w),d0
	rts
l28ff:
	move.w	#255,d0
	rts
l28z:
	moveq	#0,d0
	rts

ll61:
	tst.b	d1
	beq	l61bad
	moveq	#0,d6
l61a:
	lsr.b	#1,d0
	addq.b	#1,d6
	cmp.b	d1,d0
	bcc	l61a			; a >= Q: seguir dividiendo a entre 2
	bsr	ll28n
	moveq	#0,d5
l61b:
	add.b	d0,d0			; t = a << 1: C = bit que sale
	addx.b	d5,d5			; U = (U << 1) | C
	tst.b	d5
	bmi	l61bad
	subq.b	#1,d6
	bne	l61b
	rts
l61bad:
	moveq	#50,d0
	moveq	#50,d5
	rts

; una fila de LL51: a0 = fila de la matriz (6 bytes), a1 = salida (2 bytes); el vector es XX15
l51row:
	moveq	#0,d0
	move.b	(a0),d0
	moveq	#0,d1
	move.b	_zx15,d1
	bsr	fmltu
	move.b	d0,d5			; T
	move.b	_zx15+1,d6
	move.b	1(a0),d0
	eor.b	d0,d6			; S = XX15+1 EOR XX16+1
	moveq	#0,d0
	move.b	2(a0),d0
	moveq	#0,d1
	move.b	_zx15+2,d1
	bsr	fmltu
	move.b	d0,d1			; Q = resultado
	move.b	d5,d2			; R = T
	move.b	_zx15+3,d0
	move.b	3(a0),d4
	eor.b	d4,d0
	move.b	d6,d3
	bsr	ll38
	move.b	d0,d5
	move.b	d3,d6
	moveq	#0,d0
	move.b	4(a0),d0
	moveq	#0,d1
	move.b	_zx15+4,d1
	bsr	fmltu
	move.b	d0,d1
	move.b	d5,d2
	move.b	_zx15+5,d0
	move.b	5(a0),d4
	eor.b	d4,d0
	move.b	d6,d3
	bsr	ll38
	move.b	d0,(a1)
	move.b	d3,1(a1)
	rts

; XX12 = XX15 (vector) por la matriz XX16
l51all:
	lea	_zxx16,a0
	lea	_zx12,a1
	bsr	l51row
	lea	_zxx16+6,a0
	lea	_zx12+2,a1
	bsr	l51row
	lea	_zxx16+12,a0
	lea	_zx12+4,a1
	bsr	l51row
	rts

; ---------------------------------------------------------------------------------------------------------- void ql_ll51(void)
_ql_ll51:
	movem.l	d2-d7/a2-a6,-(sp)
	lea	_ql_lltab,a5
	bsr	l51all
	movem.l	(sp)+,d2-d7/a2-a6
	rts

; ---------------------------------------------------------------------------------------------------------- void ql_ll9_faces(const u8 *v)
; LL86..LL88: para cada cara (4 bytes en v) escribe en XX2 su visibilidad (0 = oculta; si no, el producto con la matriz de la nave)
_ql_ll9_faces:
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	48(sp),a6		; v: datos de las caras
	lea	_ql_lltab,a5
	lea	_zk3,a2			; XX2
	moveq	#0,d7			; y
fl86:
	moveq	#0,d0
	move.b	d7,d0
	lea	0(a6,d0.w),a0
	move.b	(a0),d1
	move.b	d1,_zx12+1
	and.b	#$1F,d1
	cmp.b	_zxx4,d1
	bcc	fl87			; (v[y] & $1F) >= XX4: la cara cuenta
	lsr.w	#2,d0
	move.b	#$FF,0(a2,d0.w)		; XX2[y >> 2] = 255
	addq.b	#4,d7
	bra	fl88
fl87:
	move.b	(a0),d1
	add.b	d1,d1
	move.b	d1,_zx12+3
	add.b	d1,d1
	move.b	d1,_zx12+5
	move.b	1(a0),_zx12
	move.b	2(a0),_zx12+2
	move.b	3(a0),_zx12+4
	addq.b	#3,d7
	moveq	#0,d6
	move.b	_zxx17,d6		; x
	cmp.b	#4,d6
	bcs	fl92
	move.b	_zxx18,_zx15
	move.b	_zxx18+2,_zx15+1
	move.b	_zxx18+3,_zx15+2
	move.b	_zxx18+5,_zx15+3
	move.b	_zxx18+6,_zx15+4
	move.b	_zxx18+8,_zx15+5
	bra	fl89
fl92ov:					; desbordamiento: se reduce a la mitad XX18 y se repite con x = 1
	move.b	_zxx18,d0
	lsr.b	#1,d0
	move.b	d0,_zxx18
	move.b	_zxx18+6,d0
	lsr.b	#1,d0
	move.b	d0,_zxx18+6
	move.b	_zxx18+3,d0
	lsr.b	#1,d0
	move.b	d0,_zxx18+3
	moveq	#1,d6
fl92:
	move.b	_zx12,d0
	move.b	_zx12+2,d1
	move.b	_zx12+4,d2
	lsr.b	d6,d0
	lsr.b	d6,d1
	lsr.b	d6,d2
	move.b	d0,_zx15
	move.b	d1,_zx15+2
	move.b	_zx12+5,d3		; S
	move.b	_zxx18+6,d1		; Q
	move.b	_zxx18+8,d0		; A
	bsr	ll38			; R = d2
	bcs	fl92ov
	move.b	d0,_zx15+4
	move.b	d3,_zx15+5
	move.b	_zx15,d2
	move.b	_zx12+1,d3
	move.b	_zxx18,d1
	move.b	_zxx18+2,d0
	bsr	ll38
	bcs	fl92ov
	move.b	d0,_zx15
	move.b	d3,_zx15+1
	move.b	_zx15+2,d2
	move.b	_zx12+3,d3
	move.b	_zxx18+3,d1
	move.b	_zxx18+5,d0
	bsr	ll38
	bcs	fl92ov
	move.b	d0,_zx15+2
	move.b	d3,_zx15+3
fl89:
	moveq	#0,d0
	move.b	_zx15,d0
	moveq	#0,d1
	move.b	_zx12,d1
	bsr	fmltu
	move.b	d0,d5			; T
	move.b	_zx12+1,d6
	move.b	_zx15+1,d0
	eor.b	d0,d6			; S
	moveq	#0,d0
	move.b	_zx15+2,d0
	moveq	#0,d1
	move.b	_zx12+2,d1
	bsr	fmltu
	move.b	d0,d1			; Q
	move.b	d5,d2			; R = T
	move.b	_zx12+3,d0
	move.b	_zx15+3,d4
	eor.b	d4,d0
	move.b	d6,d3
	bsr	ll38
	move.b	d0,d5
	move.b	d3,d6
	moveq	#0,d0
	move.b	_zx15+4,d0
	moveq	#0,d1
	move.b	_zx12+4,d1
	bsr	fmltu
	move.b	d0,d1
	move.b	d5,d2
	move.b	_zx15+5,d0
	move.b	_zx12+5,d4
	eor.b	d4,d0
	move.b	d6,d3
	bsr	ll38
	tst.b	d3
	bmi	fl89s
	moveq	#0,d0			; S positivo: la cara no se ve (a = 0)
fl89s:
	moveq	#0,d1
	move.b	d7,d1
	lsr.w	#2,d1
	move.b	d0,0(a2,d1.w)		; XX2[y >> 2] = a
	addq.b	#1,d7
fl88:
	cmp.b	_zxx20,d7
	bcs	fl86			; y < XX20: otra cara
	movem.l	(sp)+,d2-d7/a2-a6
	rts

; ---------------------------------------------------------------------------------------------------------- void ql_ll9_verts(long view_cy, long view_yscale)
; LL48..LL50: para cada vertice (6 bytes desde XX0 + 20) con alguna cara visible, XX3 = su posicion en pantalla (x e y de 16 bits)
_ql_ll9_verts:
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	48(sp),a3		; a3 = view_cy (bits 0..7) + 256 si view_yscale: los int de qcc son de 32 bits, asi que llegan como argumentos long
	move.l	52(sp),d0
	beq	vys0
	adda.w	#256,a3
vys0:
	lea	_ql_lltab,a5
	move.l	_zxx0,a6
	lea	20(a6),a6		; vertices
	lea	_zk3,a2			; XX2
	lea	_xx3,a4
	clr.b	_zcnt
	moveq	#0,d7			; y
vloop:
	move.b	d7,_zxx17
	moveq	#0,d0
	move.b	d7,d0
	lea	0(a6,d0.w),a0
	move.b	(a0),_zx15
	move.b	1(a0),_zx15+2
	move.b	2(a0),_zx15+4
	move.b	3(a0),d2
	move.b	d2,_zT
	and.b	#$1F,d2
	cmp.b	_zxx4,d2
	bcs	vnext			; (t & $1F) < XX4: el vertice no cuenta
	moveq	#0,d0
	move.b	4(a0),d0
	move.w	d0,d1
	and.w	#15,d1
	lsr.w	#4,d0
	tst.b	0(a2,d1.w)
	bne	v49
	tst.b	0(a2,d0.w)
	bne	v49
	moveq	#0,d0
	move.b	5(a0),d0
	move.w	d0,d1
	and.w	#15,d1
	lsr.w	#4,d0
	tst.b	0(a2,d1.w)
	bne	v49
	tst.b	0(a2,d0.w)
	bne	v49
	bra	vnext
v49:					; algun vertice con cara visible: se proyecta
	move.b	_zT,d0
	move.b	d0,_zx15+1
	add.b	d0,d0
	move.b	d0,_zx15+3
	add.b	d0,d0
	move.b	d0,_zx15+5
	bsr	l51all			; XX12 = vector por la matriz
	move.b	_inwk+2,_zx15+2
	move.b	_inwk+2,d0
	move.b	_zx12+1,d1
	eor.b	d1,d0
	bmi	v52
	move.b	_zx12,d0		; mismo signo: suma
	add.b	_inwk,d0
	move.b	d0,_zx15
	move.b	_inwk+1,d0
	moveq	#0,d1
	addx.b	d1,d0
	move.b	d0,_zx15+1
	bra	v53
v52:					; signos distintos: resta
	move.b	_inwk,d0
	sub.b	_zx12,d0
	move.b	d0,_zx15
	move.b	_inwk+1,d0
	moveq	#0,d1
	subx.b	d1,d0
	bcs	v52n			; (el MOVE borra C: se salta antes de guardar)
	move.b	d0,_zx15+1
	bra	v53
v52n:
	move.b	d0,_zx15+1
	eori.b	#$FF,_zx15+1		; negativo: se niega el valor de 16 bits y se cambia el signo
	eori.b	#$80,_zx15+2
	move.b	_zx15,d0
	neg.b	d0
	move.b	d0,_zx15
	bne	v53
	addq.b	#1,_zx15+1
v53:
	move.b	_inwk+5,_zx15+5
	move.b	_inwk+5,d0
	move.b	_zx12+3,d1
	eor.b	d1,d0
	bmi	v54
	move.b	_zx12+2,d0
	add.b	_inwk+3,d0
	move.b	d0,_zx15+3
	move.b	_inwk+4,d0
	moveq	#0,d1
	addx.b	d1,d0
	move.b	d0,_zx15+4
	bra	v55
v54:
	move.b	_inwk+3,d0
	sub.b	_zx12+2,d0
	move.b	d0,_zx15+3
	move.b	_inwk+4,d0
	moveq	#0,d1
	subx.b	d1,d0
	bcs	v54n
	move.b	d0,_zx15+4
	bra	v55
v54n:
	move.b	d0,_zx15+4
	eori.b	#$FF,_zx15+4
	eori.b	#$80,_zx15+5
	move.b	_zx15+3,d0
	neg.b	d0
	move.b	d0,_zx15+3
	bne	v55
	addq.b	#1,_zx15+4
v55:					; profundidad (d2 = U:T)
	tst.b	_zx12+5
	bmi	v56
	move.b	_zx12+4,d2
	add.b	_inwk+6,d2
	move.b	_inwk+7,d3
	moveq	#0,d4
	addx.b	d4,d3
	bra	v56j
v56:
	move.b	_inwk+6,d2
	sub.b	_zx12+4,d2
	move.b	_inwk+7,d3
	moveq	#0,d4
	subx.b	d4,d3
	bcs	v140
	tst.b	d3
	bne	v56j
	cmp.b	#4,d2
	bcc	v56j
v140:
	moveq	#0,d3
	moveq	#4,d2
v56j:
	lsl.w	#8,d3
	move.b	d2,d3			; d3 = Z de 16 bits (U:T)
	move.b	_zx15+1,d0
	lsl.w	#8,d0
	move.b	_zx15,d0		; d0 = X de 16 bits
	move.b	_zx15+4,d1
	lsl.w	#8,d1
	move.b	_zx15+3,d1		; d1 = Y de 16 bits
v57:					; mientras algun valor pase de 8 bits, los tres se dividen entre 2
	move.w	d0,d2
	or.w	d1,d2
	or.w	d3,d2
	and.w	#$FF00,d2
	beq	v60
	lsr.w	#1,d0
	lsr.w	#1,d1
	lsr.w	#1,d3
	bra	v57
v60:
	move.b	d1,_zx15+3
	move.b	d3,_zT			; Q = Z
	move.b	d0,_zx15
	moveq	#0,d0
	move.b	_zx15,d0
	moveq	#0,d1
	move.b	_zT,d1
	moveq	#0,d5
	cmp.b	d1,d0
	bcc	v61a			; X >= Z: LL61
	bsr	ll28n
	bra	v69
v61a:
	bsr	ll61
v69:					; d5:d0 = U:R, x de pantalla = 128 +- (U:R)
	moveq	#0,d3
	move.b	_zcnt,d3
	tst.b	_zx15+2
	bmi	v62
	move.b	d0,d1
	add.b	#$80,d1
	move.b	d1,0(a4,d3.w)
	move.b	d5,d1
	moveq	#0,d2
	addx.b	d2,d1
	move.b	d1,1(a4,d3.w)
	bra	v66
v62:
	move.b	#$80,d1
	sub.b	d0,d1
	move.b	d1,0(a4,d3.w)
	moveq	#0,d1
	subx.b	d5,d1
	move.b	d1,1(a4,d3.w)
v66:
	moveq	#0,d0
	move.b	_zx15+3,d0
	moveq	#0,d1
	move.b	_zT,d1
	moveq	#0,d5
	cmp.b	d1,d0
	bcc	v67a
	bsr	ll28n
	bra	v68
v67a:
	bsr	ll61
v68:
	move.l	a3,d1
	btst	#8,d1
	beq	v68n
	moveq	#0,d1			; vista nativa: el desplazamiento vertical x 6/5 (v + v / 5)
	move.b	d5,d1
	lsl.w	#8,d1
	move.b	d0,d1
	moveq	#0,d2
	move.w	d1,d2
	divu	#5,d2
	add.w	d2,d1
	move.b	d1,d0
	lsr.w	#8,d1
	move.b	d1,d5
v68n:
	moveq	#0,d3
	move.b	_zcnt,d3
	addq.w	#2,d3
	tst.b	_zx15+5
	bmi	v70
	move.l	a3,d1
	sub.b	d0,d1
	move.b	d1,0(a4,d3.w)
	moveq	#0,d1
	subx.b	d5,d1
	move.b	d1,1(a4,d3.w)
	bra	vnext
v70:
	move.l	a3,d1
	add.b	d0,d1
	move.b	d1,0(a4,d3.w)
	move.b	d5,d1
	moveq	#0,d2
	addx.b	d2,d1
	move.b	d1,1(a4,d3.w)
vnext:
	addq.b	#4,_zcnt
	move.b	_zxx17,d7
	add.b	#6,d7
	bcs	vdone			; se pasa de 255
	cmp.b	_zxx20,d7
	bcs	vloop			; y < XX20: otro vertice
vdone:
	movem.l	(sp)+,d2-d7/a2-a6
	rts

; ---------------------------------------------------------------------------------------------------------- long ql_ll9_edges(const u8 *v, long cont, long clip_h)
; LL75..LL81: recorre las aristas (4 bytes cada una desde v) con alguna cara visible; las que caen ENTERAS en la vista (los dos extremos con
; el byte alto de x e y a cero y la y baja <= clip_h - 1: la salida inmediata LL146 de CLIP2) se guardan en el montón de líneas y se dibujan
; aqui (la nueva y, si habia, la vieja que sustituye: ql_line en XOR). Si una arista necesita recorte, devuelve 1 con XX15 y XX12 cargados
; como en LL79 y la arista en curso en ql_edge_v: el C la recorta (ship_clip2 + ship_lsput) y vuelve a llamar con cont = 1 (sigue en LL78).
; Devuelve 0 al terminar. a6 = arista, a4 = XX3, a2 = XX2, a3 = montón de líneas (XX19), d5 = clip_h - 1.
	.globl	_ql_ll9_edges
_ql_ll9_edges:
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	48(sp),a6
	move.l	52(sp),d0
	lea	_xx3,a4
	lea	_zk3,a2
	move.l	_zxx19,a3
	move.l	56(sp),d5		; clip_h (long: argumento)
	subq.w	#1,d5
	tst.l	d0
	bne	e78			; cont: la arista en curso ya esta hecha
e75:
	move.b	(a6),d0
	cmp.b	_zxx4,d0
	bcs	e78			; v[0] < XX4: la arista no cuenta
	moveq	#0,d0
	move.b	1(a6),d0
	move.w	d0,d1
	and.w	#15,d1
	lsr.w	#4,d0
	tst.b	0(a2,d1.w)
	bne	e79
	tst.b	0(a2,d0.w)
	beq	e78
e79:
	moveq	#0,d0
	move.b	2(a6),d0
	lea	0(a4,d0.w),a0		; extremo 1: x lo, x hi, y lo, y hi
	moveq	#0,d1
	move.b	3(a6),d1
	lea	0(a4,d1.w),a1		; extremo 2
	move.b	1(a0),d2
	or.b	3(a0),d2
	bne	eslow
	move.b	1(a1),d2
	or.b	3(a1),d2
	bne	eslow
	move.b	2(a0),d2
	cmp.b	d5,d2
	bhi	eslow			; y1 > clip_h - 1
	move.b	2(a1),d2
	cmp.b	d5,d2
	bhi	eslow
	; LSPUT: el montón se queda con la linea nueva y la vieja (si habia) se borra
	moveq	#0,d0
	move.b	_zlsnum,d0
	lea	0(a3,d0.w),a5		; montón + zlsnum
	moveq	#0,d6			; d6 = 0: habia linea vieja
	move.b	_zlsnum,d4
	cmp.b	_zlsnum2,d4
	bcs	elo
	moveq	#1,d6			; zlsnum >= zlsnum2: no habia
elo:
	move.b	(a5),d2			; vieja: x1, y1, x2, y2 (d2, d3, d4, d7)
	move.b	1(a5),d3
	move.b	2(a5),d4
	move.b	3(a5),d7
	move.b	(a0),(a5)		; nueva al montón
	move.b	2(a0),1(a5)
	move.b	(a1),2(a5)
	move.b	2(a1),3(a5)
	addq.b	#4,_zlsnum
	btst	#1,_ship_dbg_skip
	bne	e78			; DEBUG (SHIFT-D): sin dibujo
	move.l	_screen,-(sp)		; ql_line(x1, y1, x2, y2, c, scr): argumentos como palabras largas, el primero (x1) el ultimo en apilarse
	move.l	#7,-(sp)
	moveq	#0,d0
	move.b	3(a5),d0
	move.l	d0,-(sp)
	move.b	2(a5),d0
	move.l	d0,-(sp)
	move.b	1(a5),d0
	move.l	d0,-(sp)
	move.b	(a5),d0
	move.l	d0,-(sp)
	jsr	_ql_line
	lea	24(sp),sp
	tst.b	d6
	bne	e78			; sin linea vieja
	move.l	_screen,-(sp)
	move.l	#7,-(sp)
	moveq	#0,d0
	move.b	d7,d0
	move.l	d0,-(sp)
	move.b	d4,d0
	move.l	d0,-(sp)
	move.b	d3,d0
	move.l	d0,-(sp)
	move.b	d2,d0
	move.l	d0,-(sp)
	jsr	_ql_line
	lea	24(sp),sp
e78:
	move.b	_zlsnum,d0
	cmp.b	_zcnt,d0
	bcc	edone			; zlsnum >= zcnt: el montón esta lleno
	addq.l	#4,a6
	addq.b	#1,_zxx17
	move.b	_zxx17,d0
	cmp.b	_zxx20,d0
	bcs	e75
edone:
	moveq	#0,d0
	movem.l	(sp)+,d2-d7/a2-a6
	rts
eslow:					; arista que hay que recortar (en C): se cargan XX15 y XX12 como en LL79
	move.b	(a0),_zx15
	move.b	1(a0),_zx15+1
	move.b	2(a0),_zx15+2
	move.b	3(a0),_zx15+3
	move.b	(a1),_zx15+4
	move.b	2(a1),_zx12
	move.b	3(a1),_zx12+1
	move.b	1(a1),_zx15+5
	move.l	a6,_ql_edge_v
	moveq	#1,d0
	movem.l	(sp)+,d2-d7/a2-a6
	rts
