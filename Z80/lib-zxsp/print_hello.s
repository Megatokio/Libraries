

print_hello::
; set print channel to Screen:
	ld	a,2
	call	$1601

; print a message:
	ld	hl,2$
1$:	ld	a,(hl++)
	and	a
	ret	z
	rst	2
	jr	1$

2$:	dm	13, "Hello World! ", 0



