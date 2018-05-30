
; ------------------------------------------------------------
; Calculate Screen Byte Address HL for Text Position ROW, COL
;
; in:	ROW [0..23] = -bcde-l
;	COL [0..31] = abcdehl
; out:	HL = Byteadresse [$4000..[$5800
; mod:	AF

;	row  = %000bbrrr	bb = block, rrr = text row inside block
;	col  = %000ccccc	ccccc = character column
;	addr = %010bb000.rrrccccc

.macro calc_scr_addr_for_row_col %ROW, %COL

	ld	a, %COL
	and	$1F
	ld	l,a		; l = %000ccccc

	ld	a, %ROW
	and	7		; a = %00000rrr
	rrca
	rrca
	rrca
	or	l
	ld	l,a		; l = %rrrccccc

	ld	a, %ROW
	and	$18		; a = %000bb000
	or	$40
	ld	h,a		; h = %010bb000 = fertig
.endm



; ------------------------------------------------------------
; Calculate Attribute Byte Address HL for Text Position ROW, COL
;
; in:	ROW [0..23] = abcdehl
;	COL [0..31] = -bcdeh-
; out:	HL = Byteadresse [$4000..[$5800
; mod:	AF

;	row  = %000bbrrr	bb = block, rrr = text row inside block
;	col  = %000ccccc	ccccc = character column
;	addr = %010110bb.rrrccccc

.macro calc_attr_addr_for_row_col %ROW, %COL
	ld	a, %ROW		; a = %000bbrrr
	ld	h, $58>>2	; h = %00010110;  $5800: Start of Attributes
	rlca			; a = %00bbrrr0; cy=0
	rla			; a = %0bbrrr00; cy=0
	rla			; a = %bbrrr000; cy=0
	rla \ rl h		; a = %brrr0000; h = %0010110b; cy = 0
	rla \ rl h		; a = %rrr00000; h = %010110bb = fertig
	or	%COL		; a = %rrrccccc
	ld	l,a		; l = %rrrccccc = fertig
.endm








