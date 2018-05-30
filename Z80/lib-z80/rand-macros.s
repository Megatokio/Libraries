

#if 0
http://www.retroprogramming.com/2017/07/xorshift-pseudorandom-numbers-in-z80.html
Xorshift is a simple, fast pseudorandom number generator developed by George Marsaglia. The generator 
combines three xorshift operations where a number is exclusive-ored with a shifted copy of itself.
#endif

; ____________________________________________
; predictable pseudo random number in range [1 .. $ffff]
; self-modifying
;
; out:	hl
; mod:	a

.macro rand_ffff
myseed = $+1
	ld 	hl,12345	; seed must not be 0
	ld 	a,h
	rra
	ld 	a,l
	rra
	xor 	h
	ld 	h,a
	ld 	a,l
	rra
	ld 	a,h
	rra
	xor 	l
	ld 	l,a
	xor 	h
	ld 	h,a
	ld 	(myseed),hl
.endm


; ____________________________________________
; not so predictable pseudo random number in range [0 .. $ffff]
; adds entropy from r register
; register r.bit7 should be set
; self-modifying
;
; out:	hl
; mod:	a

.macro rand16
myseed = $+1
	ld 	hl,12345	; seed should not be 0. Register R.bit7 should be set.
	ld	a,r
	xor 	a,h
	rra
	ld 	a,l
	rra
	xor 	h
	ld 	h,a
	ld 	a,l
	rra
	ld 	a,h
	rra
	xor 	l
	ld 	l,a
	xor 	h
	ld 	h,a
	ld 	(myseed),hl
.endm



