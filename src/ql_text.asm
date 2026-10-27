; ql_text.asm - text with opaque paper written directly to the QL screen (mode 8), as68 syntax (XTC68).
; long ql_puts(u8 *dst, const u8 *s, const unsigned short *font, const unsigned long *fm, long pp, long dd, long maxn)
;   (all as long words, like ql_trap3)
; Draws 8x8 cells one after another, while the character of s is printable (32..127; the final 0 and any other value
; stop it) and maxn have not been drawn; returns how many it drew (the caller advances its cursor and its string). Equivalent to the opaque path of
; draw_glyph (text.c): for each of the 8 lines of the glyph, line = pp ^ (fm[b] & dd), written at once as a long word
; (4 bytes = 8 dots) at dst + 128 * line; the next cell is 4 bytes further on. font = 96 glyphs x 8 words, each the font
; byte multiplied by 4 (byte offset inside fm, which is 256 long words: dot mask of each byte);
; pp = paper colour in the 4 bytes; dd = ink XOR paper.
; Registers: a0 = destination, a1 = string, a2 = font, a3 = fm table, a4 = glyph, d2 = pp, d3 = dd, d4 = maxn, d5 = drawn, d0/d1 = work.
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
