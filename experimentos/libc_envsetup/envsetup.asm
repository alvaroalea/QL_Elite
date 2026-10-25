; envsetup.asm - modulo _envsetup de libc.a (runtime c68 4.24f) CORREGIDO, sintaxis as68.
; Parte de envsetup_orig.asm (transcripcion fiel, comprobada byte a byte); solo cambia e_set.
;
; Fallo del original: los punteros de Toolkit II en SV+$AC/$B0/$B4 son 0 sin Toolkit II y e_set lee la
; cadena QDOS de la direccion 0 de la ROM. Su primera palabra hace de longitud: 3 en la ROM JS (basura
; inofensiva), $4C02 en Minerva: qlstr_to_c copia 19458 bytes en un bufer de 50 de la pila y el programa
; se cuelga antes de main. Ademas _setcd arma "nombre=valor" en otro bufer de 50 bytes, asi que un valor
; de mas de 40 caracteres tambien lo desborda (nombre de hasta 8 + '=' + valor + 0).
; Correccion en e_set: puntero no nulo, par y dentro de la RAM (entre las variables del sistema y SV_RAMT),
; longitud de 1 a 40, y el bufer se pone a cero antes de __getcd (si la variable no cabe en 50 bytes
; __getcd no escribe nada y se leia basura de la pila).
;
; void _envsetup(char *env)   (lo llama Cinit antes de main con el entorno que pasa el trabajo padre)
;  1) si env apunta a $4AFC, la lista de entorno es *(env+2); si no, busca en la tabla de nombres de
;     SuperBASIC un procedimiento de codigo maquina (tipo 8) llamado *ENV* (extension ENV_BIN) y toma la
;     lista de la palabra larga que sigue a su primera instruccion.
;  2) putenv("") y putenv de cada cadena de la lista (terminada en cadena vacia).
;  3) PROG_USE, DATA_USE y SPL_USE desde los punteros de Toolkit II en SV+$AC, $B0 y $B4 (cadenas QDOS),
;     si no estan ya en el entorno.

	.text
envname:
	.ascii	"*ENV*"
	.data1	0
	.globl	__envsetup
__envsetup:
	link	a5,#-28
	movem.l	d3/a2-a4/a6,-(a7)
	move.l	8(a5),d3
	move.l	d3,a0
	cmp.w	#19196,(a0)		; $4AFC
	bne	e_bas
	lea	2,a0
	add.l	d3,a0
	move.l	(a0),d3
	bra.w	e_put
e_bas:
	moveq	#0,d3
	jsr	__super
	move.l	__sys_var,a0
	move.l	16(a0),d0		; SV_BASIC
	add.l	#104,d0			; + $68: variables de BASIC
	move.l	d0,a4
	move.l	24(a4),d0		; BV_NTBAS
	add.l	a4,d0
	move.l	d0,-24(a5)
	move.l	28(a4),d0		; BV_NTP
	add.l	a4,d0
	move.l	d0,a2
	move.l	32(a4),d0		; BV_NLBAS
	add.l	a4,d0
	move.l	d0,-16(a5)
	moveq	#8,d0
	sub.l	d0,a2
	bra	e_cmp
e_loop:
	cmp.b	#8,(a2)
	bne	e_next
	tst.b	1(a2)
	bne	e_next
	move.w	2(a2),d0
	ext.l	d0
	add.l	-16(a5),d0
	move.l	d0,-20(a5)
	pea	5
	move.l	-20(a5),a0
	addq.w	#1,a0
	pea	(a0)
	pea	envname
	jsr	_strncmp
	tst.l	d0
	lea	12(a7),a7
	bne	e_next
	move.l	4(a2),a0
	addq.w	#2,a0
	move.l	(a0),d3
	bra	e_user
e_next:
	subq.w	#8,a2
e_cmp:
	cmp.l	-24(a5),a2
	bcc	e_loop
e_user:
	jsr	__user
e_put:
	pea	envname+5		; ""
	jsr	__Putenv
	tst.l	d3
	beq	e_tk2
	move.l	d3,a6
	bra	e_ptst
e_plp:
	move.l	a6,-(a7)
	jsr	__Putenv
	move.l	a6,(a7)
	jsr	__StrLen
	addq.l	#1,d0
	add.l	d0,a6
	addq.w	#4,a7
e_ptst:
	tst.b	(a6)
	bne	e_plp
e_tk2:
	move.l	__sys_var,a0
	lea	172(a0),a3		; SV+$AC
	addq.w	#4,a7
	move.l	a3,d0
	beq	e_n1
	move.l	(a3)+,d0
	bra	e_s1
e_n1:
	move.l	#nulstr,d0
e_s1:
	move.l	d0,-(a7)
	pea	__prog_use
	bsr	e_set
	addq.w	#8,a7
	move.l	a3,d0
	beq	e_n2
	move.l	(a3)+,d0
	bra	e_s2
e_n2:
	move.l	#nulstr,d0
e_s2:
	move.l	d0,-(a7)
	pea	__data_use
	bsr	e_set
	addq.w	#8,a7
	move.l	a3,d0
	beq	e_n3
	move.l	(a3),d0
	bra	e_s3
e_n3:
	move.l	#nulstr,d0
e_s3:
	move.l	d0,-(a7)
	pea	__spl_use
	bsr	e_set
	movem.l	-48(a5),d3/a2-a4/a6
	unlk	a5
	rts

; e_set(char *nombre, char *cadena_qdos): si nombre no esta en el entorno, nombre=cadena
e_set:
	link	a5,#-50
	move.l	a2,-(a7)
	lea	-50(a5),a2
	clr.b	(a2)			; CORREGIDO: bufer vacio si __getcd no escribe
	pea	50
	move.l	a2,-(a7)
	move.l	8(a5),-(a7)
	jsr	__getcd
	tst.b	(a2)
	bne	e_sfin
	move.l	12(a5),d0		; CORREGIDO: validar el puntero de Toolkit II
	beq	e_sfin			; nulo: no hay Toolkit II
	btst	#0,d0
	bne	e_sfin			; impar
	move.l	__sys_var,a0
	cmp.l	a0,d0
	bls	e_sfin			; por debajo de las variables del sistema (ROM, pantalla)
	cmp.l	32(a0),d0		; SV_RAMT
	bcc	e_sfin			; por encima de la RAM
	move.l	d0,a0
	move.w	(a0),d0			; longitud de la cadena QDOS
	ble	e_sfin			; 0 o negativa
	cmp.w	#40,d0
	bgt	e_sfin			; no cabe en los bufer de 50 bytes de aqui y de __setcd
	move.l	12(a5),-(a7)
	move.l	a2,-(a7)
	jsr	_qlstr_to_c
	addq.w	#8,a7
	move.l	a2,-(a7)
	move.l	8(a5),-(a7)
	jsr	__setcd
e_sfin:
	move.l	-54(a5),a2
	unlk	a5
	rts

	.data
nulstr:
	.data2	0
