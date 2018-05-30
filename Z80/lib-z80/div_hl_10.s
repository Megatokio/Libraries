; __________________________________
; divide by 10
;
; in:	HL
; out:	HL = quotient
;     	A  = remainder
; pres: BC, DE

div_hl_10::
	push	bc
	ld 	bc,$0D0A
	xor 	a
	add 	hl,hl
	rla
	add 	hl,hl
	rla
	add 	hl,hl
	rla	
7$:	add 	hl,hl
	rla	
	cp 	c
	jr 	c,4$
	sub 	c
	inc 	l
4$:	djnz 	7$
	pop	bc
	ret
