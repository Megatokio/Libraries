
; --------------------------------------------------------
;		S C R E E N - U T I L I T I E S
; --------------------------------------------------------

; scr_set_border		A		-- A

; clear_32_bytes		IY DE E		--  DE++
; copy_32_bytes			IY HL DE BC	--  HL++ DE++ BC++
; calc_ix_for_clear_c		C		-- IX
; calc_ix_for_copy_c		C		-- IX

; clear_bc			HL BC		-- HL++
; clear_bc_a			HL BC A		-- HL++
; clear_bc_e			HL BC E		-- HL++

; clear_pixels			HL B C		--
; clear_attributes		HL B C A	--
; clear_attributes_with_e_quick	HL B C E	--
; clear_screen_with_attr	A		--
; clear_cbox_with_attr		HL B C A	--

; copy_bc			HL DE BC	-- HL++ DE++ BC=0
; copy_pixels_to_buffer		HL DE B C	-- 
; copy_buffer_to_pixels		HL DE B C	--
; copy_attributes_to_buffer	HL DE B C	--
; copy_buffer_to_attributes	HL DE B C	--

; scroll_cbox_up		HL B C		--

; calc_hl_for_next_pixel_row	HL		-- HL
; calc_de_for_next_pixel_row	DE		-- DE
; adjust_hl_for_next_row	HL		-- HL
; adjust_de_for_next_row	DE		-- DE
; calc_hl_down_8_pixel_rows	HL		-- HL

; calc_attr_hl_for_pixel_hl	HL		-- HL
; calc_pixel_hl_from_attr_hl	HL		-- HL
; calc_attr_hl_for_row_b_col_c	B C		-- HL
; calc_pixel_hl_for_row_b_col_c	B C		-- HL
; calc_row_b_col_c_for_pixel_hl	HL		-- BC
; calc_row_b_col_c_for_attr_hl	HL		-- BC


; --------------------------------------------------
#code CODE





; ------------------------------------------------
; Lösche BC Bytes mit $00.		
;
; in:	hl	dest
;	bc	count
; out:	hl++
; mod:	af, bc, de, hl

clear_bc:
	sub	a		
	;jr	clear_bc_a



; ------------------------------------------------
; Lösche BC Bytes.
;
; in:	hl	dest
;	bc	count
;	a	filler
; out:	hl++
; mod:	af, bc, de, hl

clear_bc_a:
	ld	e,a		; e = filler

clear_bc_e:
	inc	b		; b = outer loop counter
	ld	d,4		; d = 4
	ld	a,c		; a = inner loop counter; bits 0&1 = remainder
	jr	cbc4
	
cbc3	ld	(hl),e
	inc	hl
	ld	(hl),e
	inc	hl
	ld	(hl),e
	inc	hl
	ld	(hl),e
	inc	hl

cbc4	sub	d		; 4
	jr	nc,cbc3
	
	dec	b
	djnz	cbc3

; a = remainder

cbcx	rrca
	jr	nc,cbc5
	ld	(hl),e
	inc	hl

cbc5	rrca
	ret	nc
	ld	(hl),e
	inc	hl
	ld	(hl),e
	inc	hl
	ret



; ------------------------------------------------
; slightly faster ldir.
; Rejecting BC=0
; Lohnt sich erst ab ca. 26 Bytes.
; kopiert bc Bytes von hl++ nach de++.
;
; Overhead für 0 Bytes:	15+18+71+21		= 131
;	      15 Bytes: 15+18+71+21 -15*5	=  56
;	      30 Bytes: 15+18+71+21 -30*5	= -19
;
; in:	hl, de, bc
; out:	hl++, de++, bc=0
; mod:	af, bc, de, hl, iy

copy_bc:
	sub	a			;  4
	cp	b			;  4
	jr	c,cpbc1			; 12 / 7
	;jr	copy_c



; ------------------------------------------------
; kopiert C Bytes von hl++ nach de++.
; Rejecting C = 0
; B wird erhalten.
; Lohnt sich erst ab ca. 23 Bytes.
;
; in:	hl, de, c
; out:	hl++, de++, c=0
; mod:	af, c, de, hl, iy

copy_c	ld	a,32			;  7
	cp	c			;  4
	jr	c,cpbc3			; 12 / 7
	
cpbc5	pop	iy			; 14	eigene Return-Adresse		
	push	hl			; 11	retten			
	sub	c			;  4	32-C				
	add	a			;  4	2* (32-C)			

	ld	hl,copy_32_bytes	; 11			
	add	l			;  4
	ld	l,a			;  4

;#if hi(copy_32_bytes) != hi(copy_0_bytes)
	jr	nc,$+3			; 12 / 7
	inc	h			;  0 / 4
;#endif	
	ex	hl,(sp)			; 19
	ret				; 27 (incl. call)

cpbc1	ld	iy,cpbc2		; 14
cpbc2	cp	b			;  4
	jp	c,copy_32_bytes		; 10 / 18 - 32*5
	ld	a,32			;  7
cpbc3	ld	iy,cpbc4		; 14
cpbc4	cp	c			;  4
	jp	c,copy_32_bytes		; 10 / 18 - 32*5
	jr	cpbc5			; 12



#if 0

; ------------------------------------------
; save screen pixels to buffer
;
; in:	hl -> 1st pixel byte
;	b  = rows  (characters)
;	c  = cols
; out:	hl = handle
; mod:	af,iy

scr_save_pixels:
	push	bc
	push	de

; allocate buffer
	push	hl			; sp: -> source
	push	bc			; sp: b=rows and c=cols
	call	mult_bc			; hl = b x c
	add	hl,hl			; characters are 8 pixel rows each
	inc	hl			; +4 for source address, rows and cols
	add	hl,hl			; hl = size := hl*8
	add	hl,hl
	ld	bc,hl			; bc = size
	call	mem_heap_alloc		; hl -> handle, de -> dest; bc=size
	pop	bc			; b=rows, c=cols
	ex	hl,(sp)			; sp: handle; hl->source; de->dest; bc=rows/cols

; store 1st pixel address, rows and cols in buffer:
	call	up_store_hlbc_to_de

; setup copy routine
	push	ix			
	call	calc_ix_for_copy_c	; setup IX from C
	ld	iy,pcp1ret		; ix = return address from ldi-routine

; copy B character rows:
pcp0	push	bc			; B = row count down, C = cols	
	ld	bc,9*256		; B: 8 rows; +1 because c will underflow 1x 
					; C: 0 => will 1x overflow and decr b
	ld	a,l			; save l to restore hl
	
; copy 8 pixel rows:
pcp1	jp	(ix)			; copy C bytes
pcp1ret	ld	l,a			; restore hl
	inc	h			; hl+=256 => skip over remainder of row
	djnz	pcp1

	call	adjust_hl_for_next_row	; adjust hl for next character row	
	pop	bc
	djnz	pcp0			; B runterzählen

	jr	pop_ixhldebc



; ------------------------------------------
; copy attributes to buffer
;
; in:	hl -> 1st attr byte
;	b  = rows  (characters)
;	c  = cols
; out:	handle
; mod:	af

scr_save_attributes:
	push	bc
	push	de

; allocate buffer:
	push	hl			; sp: -> source
	push	bc			; sp: b=rows and c=cols
	call	mult_bc			; hl = b x c
	inc	hl			; add 2 bytes to store rows and cols
	inc	hl
	inc	hl			; add 2 bytes to store 1st attr byte address
	inc	hl
	ld	bc,hl			; bc = size
	call	mem_heap_alloc		; hl -> handle, de -> dest; bc=size
	pop	bc			; b=rows, c=cols
	ex	hl,(sp)			; sp: handle; hl->source; de->dest; bc=rows/cols

; store source, rows and cols in buffer
	call	up_store_hlbc_to_de	; store hlbc via de++

; setup copy routine
	push	ix
	call	calc_ix_for_copy_c	; setup IX from C
	ld	iy,pca1ret		; iy = return address from ldi-routine	
	ld	a,b			; a = rows
	
; copy A attr rows:
pca0	ld	bc,32			; bc = offset of attribute rows
	jp	(ix)			; copy C bytes
pca1ret	add	hl,bc			; skip over remainder of row
	dec	a
	jr	nz,pca0

pop_ixhldebc:
	pop	ix		; ix
	pop	hl		; handle
	pop	de		; de
	pop	bc		; bc
	ret

up_store_hlbc_to_de:
	ex	hl,de
	ld	(hl),e			; store source address
	inc	hl
	ld	(hl),d
	inc	hl
	ld	(hl),b			; store rows
	inc	hl
	ld	(hl),c			; store cols
	inc	hl
	ex	hl,de
	ret
	


; ------------------------------------------
; copy buffer back to screen pixels
; handle from heap
;
; in:	--	
; out:	--
; mod:	af

scr_restore_pixels:
	push	bc
	push	de
	push	hl
	push	ix
	push	iy
	call	mem_heap_pop_handle	; hl -> handle
	call	mem_get_address		; hl -> source data

; restore 1st pixel address, rows and cols in buffer:
	ld	e,(hl)			; 1st pixel address
	inc	hl
	ld	d,(hl)
	inc	hl
	ld	b,(hl)			; rows
	inc	hl
	ld	c,(hl)			; cols
	inc	hl			; bc=rows/cols; hl->buffer; de->1st pixel

; setup copy routine:
	call	calc_ix_for_copy_c	; setup IX from C
	ld	iy,pp1ret		; iy = return address from ldi-routine

; copy B character rows:
pp0	push	bc			; B = row count down, C = cols	
	ld	bc,9*256		; B: 8 rows; (+1 because c will underflow 1x)
					; C: will underflow 1x and decr b
	ld	a,e			; a: save e for restore de
	
; copy 8 pixel rows:
pp1	jp	(ix)			; copy one row
pp1ret	ld	e,a			; restore de
	inc	d			; next pixel row: de += 256
	djnz	pp1

	call	adjust_de_for_next_row	; adjust hl for next character row	
	pop	bc
	djnz	pp0			; next row

pop_iyixhldebc_dealloc:
	pop	iy
pop_ixhldebc_dealloc:
	pop	ix
	pop	hl
	pop	de
	pop	bc
	ret


; ------------------------------------------
; copy buffer back to screen attributes
; hl from heap
;
; in:	--
; out:	--
; mod:	af

scr_restore_attributes:
	push	bc
	push	de
	push	hl
	push	ix
	push	iy
	call	mem_heap_pop_handle	; hl -> handle
	call	mem_get_address		; hl -> source data

; restore 1st attr address, rows and cols in buffer:
	ld	e,(hl)			; 1st attr address
	inc	hl
	ld	d,(hl)
	inc	hl
	ld	b,(hl)			; rows
	inc	hl
	ld	c,(hl)			; cols
	inc	hl			; bc=rows/cols; hl->buffer; de->1st attr

; copy:
	call	calc_ix_for_copy_c	; setup IX from C
	ld	iy,ppa2ret		; iy = return address from ldi-routine
	ld	a,b			; a = rows

ppa3	ld	bc,32			; offset of attr rows
	jp	(ix)			; copy C bytes
ppa2ret	ex	hl,de
	add	hl,bc			; skip over remainder of row
	ex	hl,de
	dec	a
	jr	nz,ppa3			; B runterzählen, C bewahren!

	jr	pop_iyixhldebc_dealloc

#endif	




; ------------------------------------------------------------
; UP: Berechne Pixel-Addresse HL zu Attribut-Adresse HL
;     Berechnet die Adresse des obersten Pixelbytes: hl & $0007 = 0
;
; in:	HL = Attribut-Adresse   [$5800..$5AFF]	= %010110bb.rrrccccc
; out:	HL = Pixel-Byte-Adresse [$4000..$57FF]	= %010bb000.rrrccccc
; mod:	HL, AF

calc_pixel_hl_for_attr_hl:
	ld	a,h
	and	$0f
	rlca
	rlca
	rlca
	ld	h,a
	ret



; ------------------------------------------------------------
; Berechne Attribut-Adresse für Zeile B Spalte C:
;
; in:	B = character row	[0..23]		= %000bbrrr
;	C = character column	[0..31]		= %000ccccc
; out:	HL -> attribute byte			= %010110bb.rrrccccc
; mod:	AF, HL

calc_attr_hl_for_row_b_col_c:
	ld	a,b		; a = %000bbrrr
	ld	h,$58>>2	; h = %00010110;  $5800: Start of Attributes
	rlca			; a = %00101100; cy=0
	rla			; a = %01011000; cy=0
	rla			; a = %bbrrr000; cy=0
	rla			; a = %brrr0000; cy = b
	rl	h		; h = %0010110b; cy = 0
	rla			; a = %rrr00000; cy = b
	rl	h		; h = %010110bb = fertig
	or	c		; a = %rrrccccc
	ld	l,a		; l = %rrrccccc = fertig
	ret



; ------------------------------------------------------------
; Berechne Pixel-Adresse für Zeile B Spalte C:
; Berechnet die Adresse des obersten Pixelbytes: hl & $0007 = 0
;
; in:	B = character row	[0..23]		= %000bbrrr
;	C = character column	[0..31]		= %000ccccc
; out:	HL -> pixel byte			= %010bb000.rrrccccc
; mod:	AF, HL

calc_pixel_hl_for_row_b_col_c:
	ld	a,b
	rrca
	rrca
	rrca	
	and	$E0		; a = %rrr00000
	or	c		; a = %rrrccccc
	ld	l,a		; l = %rrrccccc = fertig	
	ld	a,b		; a = %000bbrrr
	and	$18		; a = %000bb000
	or	$40		; a = %010bb000
	ld	h,a		; h = %010bb000 = fertig
	ret



; ------------------------------------------------------------
; Berechne Zeile B und Spalte C zu einer Pixeladresse HL
;
; in:	HL -> pixel byte			= %010bbzzz.rrrccccc
; out:	B = character row	[0..23]		= %000bbrrr
;	C = character column	[0..31]		= %000ccccc
; mod:	AF, BC

calc_row_b_col_c_for_pixel_hl:
	ld	a,h		; a = %010bbzzz
	and	$18		; a = %000bb000
	ld	b,a		; b = %000bb000

	ld	a,l		; a = %rrrccccc
	and	$1f		; a = %000ccccc
	ld	c,a		; c = %000ccccc = feritg

	xor	l		; a = %rrr00000
	rlca
	rlca
	rlca			; a = %00000rrr
	or	b		; a = %000bbrrr
	ld	b,a		; b = %000bbrrr = fertig
	ret



; ------------------------------------------------------------
; Berechne Zeile B und Spalte C zu einer Attributadresse HL
;
; in:	HL -> attribute byte			= %010110bb.rrrccccc
; out:	B = character row	[0..23]		= %000bbrrr
;	C = character column	[0..31]		= %000ccccc
; mod:	AF, BC

calc_row_b_col_c_for_attr_hl:
	ld	a,h		; a = %010110bb
	and	3		; a = %000000bb
	ld	b,a		; b = %000000bb
	
	ld	a,l		; a = %rrrccccc
	and	$1f		; a = %000ccccc
	ld	c,a		; c = %000ccccc = fertig

	xor	l		; a = %rrr00000
	or	b		; a = %rrr000bb
	rlca
	rlca
	rlca			; a = %000bbrrr
	ld	b,a		; b = %000bbrrr = fertig
	ret
	







