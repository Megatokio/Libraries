
; **************************************************************************
;  Kempston Mouse Interface
;  (c) 2017 by Günter Woigk



kmouse_port_x		equ	$fbdf  ; %----.-0-1.--0-.----
kmouse_port_y		equ	$ffdf  ; %----.-1-1.--0-.----
kmouse_port_buttons	equ	$fadf  ; %----.---0.--0-.----
; Buttons: [255 = None], [254 = Left], [253 = Right], [252 = Both]


.area SLOW_DATA
	
	.align 4
pointer:   	;mask1      pointer1   m2   p2
 	DEFB    %00111111, %00000000, $ff, $00
	DEFB    %00011111, %01000000, $ff, $00
	DEFB    %00001111, %01100000, $ff, $00
	DEFB    %00000111, %01110000, $ff, $00
	DEFB    %00000011, %01111000, $ff, $00
	DEFB    %00000001, %01111100, $ff, $00
	DEFB    %00000000, %01111110, $ff, $00
	DEFB    %00000000, %01101000, $ff, $00
	DEFB    %00000011, %01001000, $ff, $00
	DEFB    %00110001, %00000100, $ff, $00
	DEFB    %11110001, %00000100, $ff, $00
	DEFB    %11111000, %00000010, $ff, $00
	DEFB    %11111000, %00000010, $ff, $00
	DEFB    %11111100, %00000000, $ff, $00

sizeof_pointer:	equ	$ - pointer
		defs	sizeof_pointer * 7	; space for 7 shifted variants

pointer_rows:	equ	sizeof_pointer / 4
scrbuf:		defs    pointer_rows * 2

scrbuf_xy:	defw    0	; position of saved background	hi=row=Y, lo=col=X
port_xy::	defw    0	; old X and Y values read from port
mouse_guard:	defb	0	; debounce
mouse_hidden:	defb	1
mouse_flags:	defb	0
bg_valid:	equ	0	; bit in mouse_flags

.area GSDATA

mouse_xy::	defw    0	; (new) position of mouse pointer
mouse_x::	equ	mouse_xy
mouse_y::	equ	mouse_xy + 1
mouse_buttons::	defb	0


;max_x	equ	253	; 2 less than max. because the hot spot of the pointer image is at 1/1 
;max_y	equ	189	; and we don't implement moving the hot spot to screen location 0/0
;TODO	 		; because of yet another case. for symmetry also limit bottom/right

	

#code GSINIT

; create 7 shifted variants of the mouse pointer:
	ld	hl, pointer
	ld	de, pointer + sizeof_pointer
	ld	b,  7 * pointer_rows * 2

	and	a		; preset cy=0 for pointer
	ex	af,af'
	scf			; preset cy=1 for mask

1$:	ld	a,(hl++)	; mask
	rra
	ld	(de++),a	; mask
	ex	af,af'
	
	ld	a,(hl++)	; pointer
	rra
	ld	(de++),a	; pointer
	ex	af,af'		

	djnz	1$
	
	

#code SLOW_CODE	; may be in slow ram


zxsp_kempston_mouse::

; handle buttons:
;
; TODO: debounce, timestamp, 2 click buffer
;
	ld	hl, mouse_guard
	ld	a, (hl)
	and	a
	jr	z, 1$		; no guard time -> read buttons
	dec	(hl)		; else decrement guard time
	jr	2$		; and don't read buttons
	
1$:	ld  	bc, kmouse_port_buttons
	in  	a, (bc)
	xor  	255
	ld	hl, mouse_buttons
	cp	a, (hl)
	jr	z, 2$		; no change
	ld	(hl), a		; update buttons
	ld	a, 2
	ld	(mouse_guard), a
	;jr	2$

; mouse hidden? -> exit
2$:	ld	a,(mouse_hidden)
	and	a
	ret	nz			

; read mouse
      	call    read_mouse	; Read mouse port and update mouse_xy if the pointer has moved
      	ret	nc		; no movement

; restore background at scrbuf_xy
; save background at mouse_xy
; draw pointer
move_pointer:
      	call    restore_bg	; at scrbuf_pos
show_pointer:
      	call    save_bg		; at mouse_pos
      	;jp	draw_pointer	; at mouse_pos
draw_pointer::
	ld  	bc,(mouse_xy)
	call	calc_hl_from_row_b_col_c
	push	hl		; stack: -> screen byte

; get shifted variant of pointer:
	ld	a,c
	and	7
	; multiply with 14 * 4 = sizeof_pointer = *7 *8
	ld	e,a
	add	a
	add	a
	add	a	; *8
	sub	e	; *7
	add	a	; *14
	add	a	; *28
	ld	l,a
	ld	h,0
	add	hl,hl	; *56 = *14*4	
	ld	de,pointer
	add	hl,de		; hl -> pointer
	pop	de		; de -> screen

	; a = min(14,192-b)
	ld	a,192
	sub	b
	cp	14
	jr	c,$+4
	ld	a,14
	ld	b,a		; num rows

	; if c≥248 -> only one column
	ld	a,c
	cp	248
	jr	nc,2$

1$:	ld	a,(de)		; screen byte 1
	and	(hl)   \ inc l	; mask byte 1
	or	(hl)   \ inc l	; pointer byte 1
	ld	(de),a \ inc e
	ld	a,(de)		; screen byte 2
	and	(hl)   \ inc l	; mask byte 2
	or	(hl)   \ inc hl	; pointer byte 2
	ld	(de),a \ dec e

	inc	d
	ld	a,d
	and	7
	call	z, adjust_de_for_next_row
	djnz	1$
	ret
	
2$:	ld	a,(de)		; screen byte 1
	and	(hl)   \ inc l	; mask byte 1
	or	(hl)   \ inc l	; pointer byte 1
	ld	(de),a

	inc 	l		; skip mask byte 2
	inc 	hl		; skip pointer byte 2

	inc	d
	ld	a,d
	and	7
	call	z, adjust_de_for_next_row
	djnz	2$
	ret
	

; ___________________________________________
; show mouse
; decrements a mouse hidden counter
; mouse is only shown if mouse hidden counter reaches 0

show_mouse::
	ld	hl,mouse_hidden
	ld	a,(hl)
	and	a
	ret	z		; mouse is already visible
	dec	(hl)		; else increase visibility
	call	save_bg		; at new_pos
	jp	draw_pointer	; at new_pos


; ___________________________________________
; save background at new pointer position:

save_bg:
	ld	hl,mouse_flags
	set	bg_valid,(hl)
	
	ld	bc,(mouse_xy)
	ld	(scrbuf_xy),bc
	call	calc_hl_from_row_b_col_c ; hl -> screen
	ld  	de,scrbuf		 ; de -> srcbuf

	; a = min(14,192-b)
	ld	a,192
	sub	b
	cp	14
	jr	c,$+4
	ld	a,14
	ld	b,a		; num rows

	; if c≥248 -> only one column
	ld	a,c
	cp	248
	jr	nc,2$

	ld	c,99		; wg. ldi
1$:	ldi
	ld  	a,(hl) \ dec hl
	ld  	(de++),a

	;call    adjust_hl_for_next_row
	inc	h
	ld	a,h
	and	7
	call	z,adjust_hl_for_next_row ; advance pixel row inside a character row

	djnz    1$
	ret

2$:	ld  	a,(hl)
	ld  	(de++),a
	call	calc_hl_for_next_pixel_row	
	djnz    1$
	ret


; ___________________________________________
; hide mouse
; increments a mouse hidden counter
; so it can be hidden recursively

hide_mouse::
	ld	hl, mouse_hidden
	inc	(hl)
	ld	a,(hl)	
	dec	a
	ret	nz	; was hidden
	;jr	restore_bg
	
; ___________________________________________
; restore background at old pointer position:

restore_bg:
	ld	hl,mouse_flags
	bit	bg_valid,(hl)	; is stored background valid?
	ret	z		; no -> exit
	set	bg_valid,(hl)	; stored bg becomes invalid
	
	ld  	bc,(scrbuf_xy)
	call	calc_hl_from_row_b_col_c
	ex	hl,de		; de -> screen
	ld  	hl,scrbuf	; hl -> scrbuf

	; a = min(14,192-b)
	ld	a,192
	sub	b
	cp	14
	jr	c,$+4
	ld	a,14
	ld	b,a		; num rows

	; if c≥248 -> only one column
	ld	a,c
	cp	248
	jr	nc,2$

	ld	c,99		; wg. ldi
1$:	ldi
	ld  	a,(hl++)
	ld  	(de),a \ dec de	

	;call	calc_de_for_next_pixel_row
	inc	d
	ld	a,d
	and	7
	call	z,adjust_de_for_next_row ; advance pixel row inside a character row
	
	djnz    1$
	ret

2$:	ld  	a,(hl++)
	ld  	(de),a
	call	calc_de_for_next_pixel_row	
	djnz    1$
	ret



; ------------------------------------------------------------
; UP: Berechne Byteadresse HL 
; zu Row B [0..191] und Col C [0..255]
;
; in:	B  = row [0..191]
;	C  = col [0..255]
; out:	HL = Byteadresse [$4000..[$5800
; mod:	AF, HL
; pres: BC, DE

calc_hl_from_row_b_col_c:

;	b   = %bbrrrsss			BB=block, rrr=text row inside block, sss=subrow inside text row
;	c   = %ccccciii			ccccc = character column, iii = bit inside character
;	hl := %010bbsss.rrrccccc	

	ld	a,b
	and	7
	or	$40
	ld	h,a		; h = %01000sss 

	ld	a,b
	rrca
	rrca
	rrca			; a = %000bbrrr	
	and	$18		; a = %000bb000	
	or	h
	ld	h,a		; h = %010bbsss = fertig
	
	ld	a,b		; a = %bbrrrsss
	rlca
	rlca
	and	$e0		; a = %rrr00000
	ld	l,a		; l = %rrr00000
	
	ld	a,c		; a = %ccccciii
	rrca
	rrca
	rrca			; a = %iiiccccc
	and	$1F		; a = %000ccccc
	or	l
	ld	l,a		; l = %rrrccccc = fertig
	
	ret


; Read the kempston mouse port and update mouse_xy if the pointer has moved
; ret c  -> movement
; ret nc -> no movement
read_mouse:

; read X/Y ports and calc deltas:

	ld	hl,(port_xy)		; HL = old XY port

	ld  	bc,kmouse_port_y
	in	d,(bc)			; D = new Y
	ld	a,h
	sub	d
	ld	h,a			; H = delta Y

	ld  	bc,kmouse_port_x
	in	a,(bc)			
	ld	e,a			; E = new X
	sub	l
	ld	l,a			; L = delta X

	or	h
	ret	z			; no movement -> ret nc

	ex	hl,de			; DE = delta XY
	ld	(port_xy),hl		; update old XY


; update mouse_xy:
; DE = delta XY
; HL = old pointer XY

	ld	hl,(mouse_xy)	; HL = old pointer XY
	ld	a,l
	add	e			
	ld	l,a		; L = old X + delta X = new X

	bit	7,e
	jr	nz,neg_dx
; pos dx:	
	jr	nc,dx_ok	; no ovfl
	ld	l,255		; limit to max
	jr	dx_ok

neg_dx:	jr	c,dx_ok		; no ovfl
	ld	l,0		; limit to min
	;jr	dx_ok

dx_ok:	ld	a,h
	add	d
	ld	h,a		; H = old Y + delta Y = new Y

	bit	7,d
	jr	nz,neg_dy
; pos dy:	
	cp	192	; note: no test for uint8 ovfl: dy≥64 will wrap, TODO: actually happens! FIXME!
	jr	c,dy_ok
	ld	h,191
	jr	dy_ok

neg_dy:	jr	c,dy_ok		; no ovfl
	ld	h,0		; limit to min
	;jr	dy_ok
	
dy_ok:	ld	(mouse_xy),hl
	scf			; flag: movement
	ret			;69 bytes

