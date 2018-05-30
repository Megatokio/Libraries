





; --------------------------------------------------
.area DATA
ula_out_byte:	defs	1


; --------------------------------------------------
; Setze Border-Farbe
; 
; in:	a = new color
; out:	a = current border color [0..7]
; mod:	af

#code CODE
scr_set_border::
	and	7
	ld	(ula_out_byte),a
	out	($FE),a
	ret

; --------------------------------------------------
; Setze Border-Farbe erneut auf den alten Wert
; 
; in:	--
; out:	a = current border color [0..7]
; mod:	af

scr_restore_border::
	ld	a,(ula_out_byte)
	out	($FE),a
	ret


; ------------------------------------------------------------
; UP: Berechne Attribut-Addresse HL zu Pixel (Byte) Adresse HL
;
; in:	HL = Pixel-Adresse	[$4000..$57FF]	= %010bbzzz.rrrccccc
; out:	HL = Attribut-Adresse   [$5800..$5AFF]	= %010110bb.rrrccccc
; mod:	HL, AF

calc_attr_hl_for_pixel_hl:
	ld	a,h
	rrca
	rrca
	rrca
	and	$03
	or	$58
	ld	h,a
	ret



; ----------------------------------------
; advance HL vertically to next pixel row
;
; screen layout:
;
; %010ccaaa.bbbzzzzz
;
; zzzzz	= character column
; aaa   = row inside character row
; bbb   = character row inside screen block
; cc    = screen block
;
; in:	HL 
; out:	HL
; mod:	AF, HL

calc_hl_for_next_pixel_row:
	inc	h		; aaa += 1
	ld	a,h
	and	7
	ret	nz		; advance pixel row inside a character row

; aaa++ overflowed to cc
; => add the overflow to bbb

; this is also a good entry after incrementing H 8 times for a character row
; to adjust HL to the next pixel row, the first row of the next character row.

adjust_hl_for_next_row:
	ld	a,l
	add	32
	ld	l,a		; bbb++
	
; now undo the overflow to cc
; except if there was also an overflow from bbb++
; in which case the overflow to cc is fine (though it came from bbb++, not aaa++)	
	ret	c		; bbb++ did overflow => ok => exit
	
; undo the overflow to cc:
	ld	a,h
	sub	8
	ld	h,a		; cc--
	ret



; ----------------------------------------
; Advance DE vertically to next pixel row.
; the same for DE:

calc_de_for_next_pixel_row:
	inc	d
	ld	a,d
	and	7
	ret	nz
adjust_de_for_next_row:
	ld	a,e
	add	32
	ld	e,a
	ret	c
	ld	a,d
	sub	8
	ld	d,a
	ret



; ----------------------------------------
; advance hl vertically to next character row
;
; column and pixel row inside character row 
; are preserved.
;
; in:	HL 
; out:	HL
; mod:	AF, HL

calc_hl_down_8_pixel_rows:
	ld	a,l
	add	32
	ld	l,a		; bbb++	
	ret	nc

; overflow to cc:
	ld	a,h
	add	8
	ld	h,a		; cc++
	ret



; ------------------------------------------------------------
; Clear screen
; Löscht Pixel mit 0x00 und Attribute mit dem Wert aus A.
; Setzt Border Color entsprechend der Paper Color.
; Um den Eindruck eines instantanen Löschens ohne pixel/attr out-of-sync-Blitzer
; zu erzeugen, werden die Attribute zunächst zusätzlich mit pen=paper color gelöscht.
;
; in:	A = new attribute
; out:	--
; mod:	AF, IY

clear_screen_with_attr:
	call	save_registers

	push	af		; attr retten

	and	$F8		
	ld	e,a		; e = attribut ohne pen bits
	rra
	rra
	rra
	;and	7		; a = paper bits moved to pen position
	;out	($fe),a		; set border
	call	scr_set_border
	or	e		; a = attr mit pen == paper

	ld	hl,$5800	; start of attr
	ld	bc,24*256+32	; 24 rows, 32 cols
	call	clear_attributes_quick
	
	pop	af		; attr
	ld	hl,$4000
	ld	bc,24*256 + 32
	jr	clear_cbox_with_attr_quick
	


; ------------------------------------------------------------
; clear B rows á C character cells
;
; Löscht 8x pixel mit $00 + 1x attr je Feld mit print_attr
;
; in:	a = attribute
;	b = rows
;	c = columns
;	hl -> 1. screen byte
; out:	--
; mod:	AF, IY

clear_cbox_with_attr:
	call	save_registers
	
clear_cbox_with_attr_quick:
	ld	e,a				; attr
	call	clear_pixels
	call	calc_attr_hl_for_pixel_hl
	jp	clear_attributes_with_e_quick



; ------------------------------------------------------------
; Lösche die Attribute ab HL für B x C Character Cells
;
; in:	a = attribute byte
;	hl -> 1. attribute byte
;	b = rows
;	c = columns
; out:	--
; mod:	AF, IY

clear_attributes:
	call	save_registers

clear_attributes_quick:
	ld	e,a		; E = fill byte 

clear_attributes_with_e_quick:
	push	ix
	call	calc_ix_for_clear_c
	ld	iy,claret	; return address from jp(ix)
	ld	a,l		; A = L for restore & increment

; loop over B rows:
cb84:	jp	(ix)
claret:	add	32		
	ld	l,a		; hl++
	jr	nc,$+3
	inc	h
	
	djnz	cb84

	pop	ix
	ret



; ------------------------------------------------------------
; Lösche die Pixel ab HL für B x C Character Cells 
;
; in:	hl -> 1. screen byte
;	b = rows
;	c = columns
; out:	--
; mod:	AF, IY

clear_pixels:
	call	save_registers
	ld	a,h
	and	$F8		; %010bbzzz -> %010bb000
	ld	h,a		; force legal: hl -> top row of character cell

clear_pixels_quick:
	ld	e,0		; E = fill byte = 0
	push	ix
	call	calc_ix_for_clear_c
	ld	iy,clrret	; return address from jp(ix)
	ld	c,b		; C = rows = outer counter

; loop over B character rows:
cb80:	ld	b,8		; b = counter = 8 pixel rows
	ld	a,l		; L retten

; loop over 8 pixel rows:
cb81:	jp	(ix)		; clear pixel row (hl)++
clrret:	ld	l,a		; L erneuern
	inc	h		; next pixel row
	djnz	cb81		; next row inside character row

	call	adjust_hl_for_next_row	; hl++
	dec	c
	jr	nz,cb80		; next character row

	pop	ix
	ret



; ------------------------------------------------------------
; Scroll screen up: B rows, C cols
;
; scrolls C columns up by 1 character row
; scrolls B-1 rows and clears bottom row with print_attr
; scrolls pixels and attributes 
; printing position etc. are not updated
; uses (but does not change) bc', de', hl'
;
; in:	hl -> current screen byte
;	b = rows
;	c = cols
; out:	--
; mod:	AF, IY
	
scroll_cbox_up:	
	call	save_registers

	ld	a,b			; Security:
	or	c			; trap 0-wide or 0-high blocks
	ret	z
	ret	m

	ld	a,h			; Force legal:
	and	$F8			; hl -> top pixel row of character row
	ld	h,a

scroll_cbox_up_quick:
	push	ix
	call	calc_ix_for_copy_c	; setup IX
	
; prepare copy pixels:	
	push	hl			; hl für copy attr
	ld	de,hl			; de -> upper row = dest
	call	calc_hl_down_8_pixel_rows	; hl -> lower row = source

; prepare copy attributes:
	exx				; attr set
	ex	hl,(sp)			; save hl' and get hl->screen_byte 
	push	de			; save de'
	push	bc			; save bc'
	call	calc_attr_hl_for_pixel_hl
	ex	hl,de			; de -> upper row = dest
	ld	hl,32	
	add	hl,de			; hl -> lower row = source		
	exx				; pixel set
	
	jr	psb3			; copy b-1 rows; maybe 0
	
; --- main loop: copy B-1 character rows: ---

; copy 8 pixel rows:
psb0:	ld	a,8			; 8 rows
	ld	iy,psb1ret

	push	bc			; B = row count down, C = cols	

psb1:	ld	bc,256			; offset of pixel rows inside character rows
	jp	(ix)			; copy C bytes
psb1ret:add	hl,bc			; skip over remainder of row
	ex	hl,de		
	add	hl,bc		
	ex	hl,de
	dec	a
	jr	nz,psb1

	pop	bc

	call	adjust_hl_for_next_row	; adjust de and hl for next character row
	call	adjust_de_for_next_row
	
; copy 1 attribute row:
	exx				; attr set
	ld	iy,psb2ret	
	ld	bc,32			; offset of attr rows
	jp	(ix)			; copy C bytes
psb2ret:add	hl,bc			; skip over remainder of row
	ex	hl,de		
	add	hl,bc		
	ex	hl,de
	exx				; pixel set

psb3:	djnz	psb0			; B runterzählen, C bewahren!

; --- end of main loop ---

	ex	hl,de			; hl -> pixels of the last character row
	;ld	c,c			; c = columns
	inc	b			; b = 1 row
	
	push	bc
	call	clear_pixels_quick
	pop	bc

	pop	bc			; restore bc'
	pop	de			; restore de'
	pop	hl			; restore hl'
	pop	ix			; restore ix

	exx				; hl -> attr of the last character row
	jp	clear_attributes_quick
	
	

; --------------------------------------------------
; Lösche 32 Bytes im Bildschirm
; (HL)++ = E
;
; Der zu löschende Bereich muss vollständig in einem 256-Byte-Block liegen,
; was beim Bildschirm immer der Fall ist. H wird nicht incrementiert,
; und wenn der zu löschende Bereich an einer 256-Byte-Grenze endet, muss der
; Aufrufer H selbst incrementieren. Dazu kann er sofort das Z-Flag prüfen.
; Beim Bildschirm-Löschen ist es aber sehr günstig, dass H nicht verändert wird,
; da man beim Weiterschalten innerhalb einer Zeichenzeile dann immer L restauriert 
; und H incrementiert.
;
; Die verwendete Methode ist vermutlich die zweitschnellste nach 
; Löschen mit push dd. Hier muss aber nicht der Interrupt (immer wieder)
; ausgeschaltet werden und es kann auch eine ungerade Anzahl von Bytes leichter
; gelöscht werden. 
; Das Löschen von beliebigen Speicherbereichen ist allerding hiermit nicht ideal, 
; weil das Splitten an den 256-Byte-Grenzen unverhältnismäßig aufwendig ist.
;
; in:	IY: Rücksprungadresse
;	HL -> Ziel
;	E = Füllbyte
; out:	L++
;	F: Z: L overflowed -> H++ needed.
; mod:	F, L

clear_32_bytes:
	.rept 32
	 ld	(hl),e		;7
	 inc	l		;4
	.endm			; note: inc l: der evtl. Übertrag muss vom Aufrufer behandelt werden.
clear_0_bytes:
	jp	(iy)



; --------------------------------------------------
; Kopiere 32 Bytes 
; (hl++) -> (de++) und bc--
;
; in:	iy: Rücksprungadresse
;	hl -> Quelle
;	de -> Ziel
;	bc = Zähler
; out:	hl++
;	de++
;	bc--
; mod:	f, bc, de, hl

copy_32_bytes:
	.rept 32
	 ldi			;16	ld (de++),(hl++) \ bc--
	.endm
copy_0_bytes:
jp_iy:	jp	(iy)



; -------------------------------------------------
; Berechne Einsprungsadresse in clear_32_bytes
;
; in:	c = bytes  [0..c..32]
; out:	ix -> Einsprungsadresse für jp (ix)
; mod:	af, ix

calc_ix_for_clear_c:
	ld	ix,clear_32_bytes  
	jr	calc_ix



; -------------------------------------------------
; Berechne Einsprungsadresse in copy_32_bytes
;
; in:	c = bytes  [0..c..32]
; out:	ix -> Einsprungsadresse für jp (ix)
; mod:	af, ix

calc_ix_for_copy_c:
	ld	ix,copy_32_bytes  

calc_ix:
	ld	a,64
	sub	c
	sub	c
	add	xl
	ld	xl,a
	ret	nc
	inc	xh
	ret



; ----------------------------------
; Save registers BC .. HL

#include "../lib-z80/save_registers.s"





