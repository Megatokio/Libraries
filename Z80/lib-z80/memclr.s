

; this is extracted from lib-Z80-Vss/core-memory.s


; _________________________________________________
; clear memory
; 0-safe
;
; memclr  ( hl=memptr de=size, a=filler -- hl=hl de=0 bc=bc a=a )
; memclr0 ( hl=memptr de=size -- hl=hl de=0 bc=bc a=0 )

memclr0::
	xor	a

memclr::
	add	hl,de		; hl = memptr + size

	srl	d
	rr	e		; de = size/2
	jr	nc,1$
	ld	(--hl),a	; if size&1

1$:	srl	d
	inc	d
	rr	e		; de = size/4  ((d+1))
	jr	nc,2$
	ld	(--hl),a	; if size&2
	ld	(--hl),a

2$:	jr	z,4$		; skip inner loop if e=0

3$:	ld	(--hl),a	; 16.5cc/byte
	ld	(--hl),a
	ld	(--hl),a
	ld	(--hl),a
	dec	e
	jp	nz,3$
4$:	dec	d
	ret	z
	jr	3$



