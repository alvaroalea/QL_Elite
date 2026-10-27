; ql_ellrows.asm - updates the outline of an ellipse ROW BY ROW: on each screen line what was there of the old ellipse is erased (XOR) and that of the new one
; is drawn, one right after the other, so the ellipse never disappears completely (only the current line is half done)
; and the rows that do not change are not touched. The outline of each row comes from a span table (gfx_ell_spans in gfx.c: 2 bytes per row at dy lines
; from the centre, lo and hi: the row's outline is cx + lo .. cx + hi and its reflection; with lo = 0 a single span cx - hi .. cx + hi), derived from the
; ellipse half-width table (half[dy]: row dy < ry from half[dy + 1] + 1 to half[dy] if half[dy + 1] < half[dy], otherwise the point half[dy]; row ry from 0 to half[ry]). It is IDENTICAL point by point to the midpoint
; outline of gfx_ellipse_xor (checked in make test_ellrows). as68 syntax; tools/test_ellrowsasm.py runs it with tools/m68k_mini.py.
;
; void ql_ellrows(long *blk)    block of long words (offset in bytes; only the low words of the coordinates are used):
;   0 y (first row)  4 yfin (last row, already clipped to the view)
;   8 ocx  12 ocy  16 ory (-1 = there was no old ellipse)  20 otab (old table)
;   24 ncx  28 ncy  32 nry (-1 = there is no new ellipse)   36 ntab (new table)
;   40 GB (even byte mask of a full dot)  44 RB (of the odd one)  48 screen
; Registers: a2 = screen, a3 = old table, a4 = new table, a5 = start of the row, a6 = block, d5/d6 = GB/RB masks.
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
	lea	0(a2,d1.w),a5		; start of row y
	move.w	14(a6),d1		; old ellipse: ocy, ory, table
	move.w	18(a6),d2
	move.l	a3,a0
	bsr	getspan
	move.w	d2,d4			; old: hi in d4, lo in d7
	move.w	d3,d7
	move.w	2(a6),d0
	move.w	30(a6),d1		; new ellipse: ncy, nry, table
	move.w	34(a6),d2
	move.l	a4,a0
	bsr	getspan			; new: hi in d2, lo in d3
	tst.w	d4
	bpl	have
	tst.w	d2
	bmi	nextrow			; neither touches this row
have:
	cmp.w	d4,d2
	bne	draw
	cmp.w	d7,d3
	bne	draw
	move.w	10(a6),d0
	cmp.w	26(a6),d0
	beq	nextrow			; same span and same centre: nothing to change
draw:
	move.w	d2,-(sp)		; the new one is saved while the old one is erased
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

; getspan: d0 = y, d1 = cy, d2 = ry (-1 = no ellipse), a0 = span table (2 bytes per row: lo, hi)  ->  d2 = hi (-1 if the ellipse does not touch
; the row), d3 = lo; destroys d1
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

; spans: d1 = cx (signed), d2 = hi, d3 = lo (>= 0), a5 = row: XOR of the spans [cx - hi, cx - lo] and [cx + lo, cx + hi] (a single one
; [cx - hi, cx + hi] if lo = 0); destroys d0-d4, d7, a0
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
	move.w	d1,d4			; lo = hi: one dot on each side (cx - hi and cx + hi)
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
	sub.w	d2,d1			; (the sum is undone: outside the view, general path)
s2:
	move.w	d1,d0
	add.w	d3,d0			; x0 of the right span = cx + lo
	move.w	d1,d4
	add.w	d2,d4			; x1 = cx + hi
	move.w	d0,-(sp)
	move.w	d4,-(sp)
	move.w	d1,d0
	sub.w	d3,d0			; x1 of the left span = cx - lo
	sub.w	d2,d1			; x0 = cx - hi
	move.w	d0,d2
	bsr	run
	move.w	(sp)+,d2
	move.w	(sp)+,d1
				; continues in run

; run: d1 = x0, d2 = x1 (signed): clips to 0..255 and draws
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
				; a single dot: continues in r_pt
; r_pt: XOR of a dot (d1 = x, 0..255) in row a5; destroys d0, d1, d7, a0
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

; r_hrun: XOR of the dots d1..d2 (0 <= d1 < d2 <= 255) of row a5; destroys d0-d4, d7, a0
r_hrun:
	move.w	d1,d3
	lsr.w	#2,d3			; word of the first dot
	move.w	d2,d4
	lsr.w	#2,d4			; word of the last
	move.w	d3,d0
	add.w	d0,d0
	lea	0(a5,d0.w),a0
	move.w	d1,d0
	and.w	#3,d0
	add.w	d0,d0
	move.b	#$FF,d7
	lsr.b	d0,d7			; left mask
	move.w	d2,d0
	and.w	#3,d0
	add.w	d0,d0
	neg.w	d0
	add.w	#6,d0
	move.b	#$FF,d1
	lsl.b	d0,d1			; right mask
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
; void ql_ellrows4(long *blk)    mode 4, at the 512 physical dots: the same block as ql_ellrows, but ocx and ncx are already physical x (2 x the
; game's), the span tables are 16-bit words (lo, hi: 4 bytes per row, gfx_ell_spans in mode 4), the spans are clipped to
; 0..511 and GB/RB are $FF or 0. It is ql_ellrows with 8 dots per word.
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
	lea	0(a2,d1.w),a5		; start of row y
	move.w	14(a6),d1		; old ellipse: ocy, ory, table
	move.w	18(a6),d2
	move.l	a3,a0
	bsr	getspan4
	move.w	d2,d4			; old: hi in d4, lo in d7
	move.w	d3,d7
	move.w	2(a6),d0
	move.w	30(a6),d1		; new ellipse: ncy, nry, table
	move.w	34(a6),d2
	move.l	a4,a0
	bsr	getspan4			; new: hi in d2, lo in d3
	tst.w	d4
	bpl	have4
	tst.w	d2
	bmi	nextrow4			; neither touches this row
have4:
	cmp.w	d4,d2
	bne	draw4
	cmp.w	d7,d3
	bne	draw4
	move.w	10(a6),d0
	cmp.w	26(a6),d0
	beq	nextrow4			; same span and same centre: nothing to change
draw4:
	move.w	d2,-(sp)		; the new one is saved while the old one is erased
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

; getspan4: d0 = y, d1 = cy, d2 = ry (-1 = no ellipse), a0 = span table (2 bytes per row: lo, hi)  ->  d2 = hi (-1 if the ellipse does not touch
; the row), d3 = lo; destroys d1
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

; spans4: d1 = cx (signed), d2 = hi, d3 = lo (>= 0), a5 = row: XOR of the spans [cx - hi, cx - lo] and [cx + lo, cx + hi] (a single one
; [cx - hi, cx + hi] if lo = 0); destroys d0-d4, d7, a0
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
	move.w	d1,d4			; lo = hi: one dot on each side (cx - hi and cx + hi)
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
	sub.w	d2,d1			; (the sum is undone: outside the view, general path)
s24:
	move.w	d1,d0
	add.w	d3,d0			; x0 of the right span = cx + lo
	move.w	d1,d4
	add.w	d2,d4			; x1 = cx + hi
	move.w	d0,-(sp)
	move.w	d4,-(sp)
	move.w	d1,d0
	sub.w	d3,d0			; x1 of the left span = cx - lo
	sub.w	d2,d1			; x0 = cx - hi
	move.w	d0,d2
	bsr	run4
	move.w	(sp)+,d2
	move.w	(sp)+,d1
				; continues in run4

; run4: d1 = x0, d2 = x1 (signed): clips to 0..255 and draws
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
				; a single dot: continues in r_pt4
; r_pt4: XOR of a dot (d1 = x, 0..255) in row a5; destroys d0, d1, d7, a0
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

; r_hrun4: XOR of the dots d1..d2 (0 <= d1 < d2 <= 255) of row a5; destroys d0-d4, d7, a0
r_hrun4:
	move.w	d1,d3
	lsr.w	#3,d3			; word of the first dot
	move.w	d2,d4
	lsr.w	#3,d4			; word of the last
	move.w	d3,d0
	add.w	d0,d0
	lea	0(a5,d0.w),a0
	move.w	d1,d0
	and.w	#7,d0
	move.b	#$FF,d7
	lsr.b	d0,d7			; left mask
	move.w	d2,d0
	and.w	#7,d0
	neg.w	d0
	add.w	#7,d0
	move.b	#$FF,d1
	lsl.b	d0,d1			; right mask
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
