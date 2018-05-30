; __________________________________
; convert int or uint to text
;
; in:	HL = number
;	DE -> buffer
; out:	A  = 0
;	HL = 0
;	DE -> behind 0-delimiter
; pres: BC

; itoa ( hl:number de:buffer -- ) 
; utoa ( hl:number de:buffer -- ) 

itoa::	bit	7,h
	jr	z,utoa
	ld	a,'-'
	ld	(de++),a

	xor	a	; hl = -hl
	sub	a,l
	ld	l,a
	sbc	a,a	; nc => 0, cy => 0xff
	sub	a,h
	ld	h,a

utoa::	push	bc
	xor	a
	push	af	; end marker a=0, z

1$:	call	div_hl_10
	add	'0'
	push	af	; digit: a>0, nz
	ld	a,h
	or	l
	jr	nz,1$

2$:	pop	af
	ld	(de++),a
	jr	nz,2$
	pop	bc
	ret

#if !defined(div_hl_10)
#include "div_hl_10.s"
#endif
