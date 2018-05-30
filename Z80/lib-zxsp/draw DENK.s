
#local

.area DATA

plotpos: db 0	; lo = x
	 db 0	; hi = y


#code CODE


offscrwidth::	equ	256
offscrheight::	equ	128
offscrsize::	equ	offscrwidth*offscrheight/8	; 4k
		.align 	32
offscrbuf::	ds	offscrsize, $ff

	; 408cc / full scanline + contention  codesize = 48 bytes
	; 78336cc / full screen + contention  codesize = 9216 bytes 
	; 52224cc / 2/3 screen  + contention  codesize = 6144 bytes

	; scanline = 224cc = 128cc contended + 96cc fast
	; screen writes (pushes) come in a block:
	; first to last push = 44+37+15 = 96cc
	; --> contention will delay pushes in ~first 3 lines 
	;     then pushes are all in border!
	;     then 1 scanline (32 bytes) is pushed in exactly 2 scanlines time.
	; top border = 64 scan lines uncontended 
	; --> 64*224/408 = 35.1 scanlines copied
	;     last scanline is copied after (128-35)*2 = 186 screen lines  ((128 lines screen))
	; --> if scrbuf is copied very low in screen (at most 5 scanlines from bottom)
	;     then full 256x128 pixel screen is copied without flicker if started after halt
	;     else screen copy must start shortly after scanline 64
	; after testing: 6 bottom lines is not possible even if pre-popping the first 16 bytes
	;     though 6 lines are ok on a ZXSP 128k! (228cc/scanline)

	; row = %bbrrrsss  -->  hl = %010bbsss.rrr00000	

	.macro copy32bytes %Q, %ZROW	; ZROW = row in specci screen
	 ld  sp, %Q				;10
	 pop af \ ex af,af'			;14
	 pop bc \ pop de \ pop hl \ exx		;34
	 pop af \ pop bc \ pop de \ pop hl	;40
	 ld sp, 0x4000 + (%ZROW&0xC0) << 5 + (%ZROW&0x38) << 2 + (%ZROW&0x07) << 8 + 16 ;10
	 push hl \ push de \ push bc \ push af	;44
	 exx \ push hl \ push de \ push bc	;37
	 ex af,af' \ push af			;15	= 204cc

	 ld  sp, %Q + 16
	 pop af \ ex af,af'
	 pop bc \ pop de \ pop hl \ exx
	 pop af \ pop bc \ pop de \ pop hl
	 ld sp, 0x4000 + (%ZROW&0xC0) << 5 + (%ZROW&0x38) << 2 + (%ZROW&0x07) << 8 + 32
	 push hl \ push de \ push bc \ push af
	 exx \ push hl \ push de \ push bc
	 ex af,af' \ push af			;	= 408cc / scanline
	.endm					;	= 78336cc / full screen
	
zxtop  = 58
zxrow  = zxtop
zxrows = 128	
qaddr  = offscrbuf

	di
	call	print_msg
	db 	ctl_setattr, black+white_paper, ctl_cls, 0
	ld	hl,offscrtest
	ld	(intv),hl	

	 ld  sp, qaddr
	 pop af \ ex af,af'
	 pop bc \ pop de \ pop hl \ exx	
	 pop af \ pop bc \ pop de \ pop hl
	 ld sp, 0x4000 + (zxrow&0xC0) << 5 + (zxrow&0x38) << 2 + (zxrow&0x07) << 8 + 16 + 2

	ei
	halt
	rst 0
	
offscrtest::
	; first row:
	;ld  sp, %Q
	;pop af \ ex af,af'
	;pop bc \ pop de \ pop hl \ exx	
	;pop af \ pop bc \ pop de \ pop hl
	 push hl \ push de \ push bc \ push af
	 exx \ push hl \ push de \ push bc
	 ex af,af' \ push af

	 ld  sp, qaddr + 16
	 pop af \ ex af,af'
	 pop bc \ pop de \ pop hl \ exx
	 pop af \ pop bc \ pop de \ pop hl
	 ld sp, 0x4000 + (zxrow&0xC0) << 5 + (zxrow&0x38) << 2 + (zxrow&0x07) << 8 + 32
	 push hl \ push de \ push bc \ push af
	 exx \ push hl \ push de \ push bc
	 ex af,af' \ push af			;	= 408cc / scanline

	.rept zxrows-1
	 copy32bytes qaddr, zxrow
qaddr = qaddr + 32
zxrow = zxrow + 1	
	.endm
	
fullstop:
	halt
	ds	1
	

sintab::
angle = 0	
	.rept 256
	 db	sin(angle,256,95)
angle = angle+1
	.endm
	








; ___________________________________	
; draw line to row,col DE
; draws first and last point

draw_line_to::
	push	bc
	push	de
	push	hl
	call	draw_line_to_fast
	pop	hl
	pop	de
	pop	bc
	ret
	
; ___________________________________	
; draw line from row,col HL to row,col DE
; draws first and last point

draw_line::
	push	bc
	push	de
	push	hl
	call	draw_line_fast
	pop	hl
	pop	de
	pop	bc
	ret

; ___________________________________	
; move drawing position to row,col DE
move_to::
	ex	hl,de
move_to_hl::	
	ld	(plotpos),hl
	ret

; ___________________________________	
; set point at row,col DE
plot_point::
	ex	hl,de		; HL = new pos
plot_point_hl::
	ld	(plotpos),hl
	call	calc_hlc_from_row_h_col_l
	ld	a,(hl)
	or	c
	ld	(hl),a
	ret



; draw horizonal line:
; DE = point1 and HL=point2 and D=H
; we want to draw from HL to DE from left to right:
dl_dy0:	ld	a,e
	sub	l
	jr	nc,1$		; dx ≥ 0
	ex	hl,de		; swap DE and HL
	ld	a,e
	sub	l		; now dx ≥ 0
1$:	ld	e,a		; E = dx
	
	; setup loop:
	
	call	calc_hlc_from_row_h_col_l  ; screen byte address and pixel mask of start position
	
	inc	e		; E = dx+1	loop counter

	; plot pixel loop:

2$:	ld	a,(hl)
	or	c		; plot x,y
	ld	(hl),a

	dec	e
	ret	z
	
	rrc	c		; x++
	jr	nc, 2$
	inc	l
	jr	2$


; draw vertical line:
; HL = point1 and D = dy
dl_dx0:	
	; setup loop:
	
	call	calc_hlc_from_row_h_col_l  ; screen byte address and pixel mask of start position
	
	inc	d		; D = dx+1	loop counter
	
	; plot pixel loop:

2$:	ld	a,(hl)
	or	c		; plot x,y
	ld	(hl),a

	dec	d
	ret	z

	; step down:
	inc	h		; y++
	ld	a,h
	and	7
	jr	nz, 2$
	call	adjust_hl_for_next_row
	jr	2$
	

; draw line:
; from old plotpos to DE = new point
draw_line_to_fast::
	; get drawing position:
	ld	hl,(plotpos)	; HL = old pos
	;jr	draw_line_fast
	
; draw line:
; HL = point1, DE = point2
draw_line_fast::
	; update drawing position:
	ld	(plotpos),de	; DE = new pos

	; we want to draw from HL to DE
	; calc dy and order start/end so that dy is pos:
	ld	a,d
	sub	h
	jr	z, dl_dy0	; dy = 0 -> purely horizontal line
	jr	nc, 1$		; dy > 0

	ex	hl,de		; dy < 0 --> dy > 0
	ld	a,d
	sub	h		; now dy > 0
1$:	ld	d,a		; D = dy 
	
	; D = dy > 0
	
	; calc dx and branch for +dx or -dx routines
	ld	a,e
	sub	l
	jr	z, dl_dx0	; dx = 0 -> purely vertical line
	jr	nc, dl_ltr	; dx > 0 --> left to right

	; dx < 0 --> right to left
	ld	a,l
	sub	e
	ld	e,a		; D = dy, E = |dx|
	
	; branch for hor or vert routine:
	cp	d		; dx ≥ dy?
	jr	c, dl_v_rtl	; dy > |dx| -> vertical, dx<0

	; patch 'inc x' in pixel loop:
	ld	a, opcode(rlc c)
	ld	(dl1+1), a
	ld	a, opcode(dec l)
	ld	(dl6), a
	jr	dl_hor

	; dx > 0 --> left to right
dl_ltr:	ld	e,a		; D = dy, E = dx

	; hor or vert?
	cp	d		; dx ≥ dy?
	jr	c, dl_v_ltr	; dy > dx -> vertical, dx>0
	
	; patch 'inc x' in pixel loop:
	ld	a, opcode(rrc c)
	ld	(dl1+1), a
	ld	a, opcode(inc l)
	ld	(dl6), a
	;jr	dl_hor
	
; draw horizontally, x-- and y++
; in loop:
;   HL mask C = current pixel position
;   B = N (loop counter)
;   D = dy
;   E = dx
;   A/A' = z (step-aside counter)
; the screen byte is only actually read and written when hl changes

	; setup loop:
	
dl_hor:	call	calc_hlc_from_row_h_col_l  ; screen byte address and pixel mask of start position
	
	ld	b,e		; B = dx	loop counter
	inc	b		; B = dx+1

	ld	a,e
	srl	a		; A = z = dx/2	step-aside counter
	ex	af,af'
	ld	a,(hl)
	call	dl30
	ld	(hl),a
	ret

	; plot pixel loop:

	; step aside test:
dl5:	ex	af,af'		; A = z
	add	d		; z += dy
	cp	e		; z ≥ dx ?
	jr	nc,dl4		; yes -> step aside
dl2:	ex	af,af'		; A' = z

	; plot pixel
dl30:	or	c		; plot x,y

	; loop end test
	dec	b		; loop end test
	ret	z

	; increment x
dl1:	rlc	c		; x++/x--	<--- patched for x++ / x--
	jp	nc, dl5
	ld	(hl),a
dl6:	dec	l		;		<--- patched for x++ / x--
	ld	a,(hl)
	jr	dl5

	; step down:
dl4:	sub	e		; z -= dx
	ex	af,af'		; A' = z

	ld	(hl),a
	inc	h		; y++
	ld	a,h
	and	7
	ld	a,(hl)
	jp	nz,dl30

	call	adjust_hl_for_next_row
	ld	a,(hl)
	jr	dl30



; draw vertical line, dx>0
; D = dy, E = |dx|, HL = point1
dl_v_ltr:	
	; patch 'inc x' in pixel loop:
	ld	a, opcode(rrc c)
	ld	(dv1+1), a
	ld	a, opcode(inc l)
	ld	(dv6), a
	jr	dl_ver


; draw vertical line, dx<0
; D = dy, E = |dx|, HL = point1
dl_v_rtl:
	; patch 'inc x' in pixel loop:
	ld	a, opcode(rlc c)
	ld	(dv1+1), a
	ld	a, opcode(dec l)
	ld	(dv6), a
	;jr	dl_ver


; draw vertically, x++/x-- and y++
; in loop:
;   HL mask C = current pixel position
;   B = N (loop counter)
;   D = dy
;   E = dx
;   A/A' = z (step-aside counter)

	; setup loop:
	
dl_ver:	call	calc_hlc_from_row_h_col_l  ; screen byte address and pixel mask of start position
	
	ld	b,d		; B = dy	loop counter
	inc	b		; B = dy+1

	ld	a,d
	srl	a		; A = z = dy/2	step-aside counter
	jr	dv2

	; plot pixel loop:
	
dv5:	ex	af,af'		; A = z
	add	e		; z += dx
	cp	d		; z ≥ dy ?
	jr	nc,dv4		; yes -> step aside
	
dv2:	ex	af,af'		; A' = z
dv3:	ld	a,(hl)
	or	c		; plot x,y
	ld	(hl),a

	dec	b		; loop end test
	ret	z

	; step down:
	inc	h		; y++
	ld	a,h
	and	7
	jp	nz,dv5
	call	adjust_hl_for_next_row
	jr	dv5

	; step aside:
dv4:	sub	d		; z -= dy
	ex	af,af'		; A' = z
dv1:	rlc	c		; x--	<--- patched for x++ / x--
	jp	nc, dv3
dv6:	dec	l		;	<--- patched for x++ / x--
	jr	dv3
	


; ------------------------------------------------------------
; Calculate byte address HL and bit mask C
; for row B [0..191] and col C [0..255]
;
; in:	B  = row [0..191]
;	C  = col [0..255]
; out:	HL = byte address [$4000..[$5800
;	C  = bit mask
; mod:	AF, HL
; pres: BC, DE

calc_hlc_from_row_h_col_l:
	ld	bc,hl

calc_hlc_from_row_b_col_c:

;	b = row = %bbrrrsss		bb=block, rrr=text row inside block, sss=subrow inside text row
;	c = col = %ccccciii		ccccc = character column, iii = bit inside character
;	hl := %010bbsss.rrrccccc	
;	c  := %80 >> iii

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
	
	ld	a,c		; a = pixel col
	and	7		; a = pixel col inside byte
	add	lo(pmasks)
	ld	c,a
	ld	b,hi(pmasks)
	ld	a,(bc)
	ld	c,a
	
	ret


.area DATA
	.align hi($) != hi($+7) ? 256 : 1
pmasks:	defb	$80, $40, $20, $10, $08, $04, $02, $01


#code CODE
	;ld	hl,$3305
	;ld	de,$33fb
	;call 	draw_line_fast
	;ld	de,$44fb
	;call	draw_line_to_fast
	;ld	de,$4405
	;call	draw_line_to_fast
	;ld	de,$3305
	;call	draw_line_to_fast

#if 1
test_draw::
	ld	hl,$0000	
	ld	de,$5b60
	
1$:	call	draw_line
	
	inc	e
	ld	a,5
	add	l
	ld	l,a
	jr	nc, 1$
	
	ld	h,a
	ld	l,255
	
2$:	call	draw_line
	
	inc	d
	ld	a,5
	add	h
	ld	h,a
	cp	192
	jr	c, 2$

	add	$40
	cpl
	ld	l,a
	ld	h,191

3$:	call	draw_line
	
	dec	e
	ld	a,-5
	add	l
	ld	l,a
	jr	c, 3$

	sub	$40
	ld	h,a
	ld	l,0
	
4$:	call	draw_line
	
	dec	d
	ld	a,-5
	add	h
	ld	h,a
	jr	c, 4$
	
	ret
#endif

	
#endlocal











