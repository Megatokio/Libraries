
; --------------------------------------------------------
;		P R I N T - V D U 
; --------------------------------------------------------

; Betrachtungen zu HL + bitnr. C  vs.  HL + bitmaske C
;
; Die Text-VDU speichert die aktuelle Printposition in HL und C, 
; wobei HL auf das Pixelbyte im Bildschirm zeigt und C die Maske
; für das exakte Pixel ist. 
;
; Alternativ dazu wäre es möglich, die exakte Pixelposition in C
; als Bitnummer zu speichern. Das geht grundsätzlich schneller:
; zum setzen eines Bits kann 'set n,(hl)' benutzt werden, was 
; schneller ist als 'ld a,(hl)' + 'or c' + 'ld (hl),a' und
; zusätzlich das A-Register unbenutzt lässt. 
; Leider kennt die Z80 keine Bitbefehle mit berechneter Bitnummer -
; die Bitnummer ist immer im Opcode hart kodiert. Deshalb muss 
; der Bit-Set-Opcode gepatcht werden und deshalb muss die Routine 
; in's Ram und der Aufruf und der Patch des Opcodes wird immer umständlicher.
;
; Deshalb ist letztendlich - zumindest für Rom-Code - die Version
; mit der Bitmaske in C günstiger, und jetzt hier auch so implementiert.

; p8_reset				--
; p8_get_ctl_args		A	-- A
; p8_push_state				-- 
; p8_pop_state				-- 
;
; print_char			A	--
; print_msg			inline	--	
; print_text_de			DE	-- DE++
; print_text_dea		DE A	-- DE++
; print_uint			HL	--
; print_int			HL	--


; print_yes_no			cy	--
; print_no				-- nc
; print_yes				-- cy
; print_uint8			A	--
; print_hex4			HL	--
; print_hex2			A	--
; print_hex1			A	--
; print_bin16			HL	--
; print_bin8			A	--
; print_bin4			A	--
; p8_locate			B C	--
; print_getloc				-- B C
; p8_setattr			A	--
; scroll_screen				--
; scroll_screen_up			--
; show_scroll_and_wait_for_key		--
; print_find_char_wrap		HL BC	-- DE
; print_find_word_wrap		HL BC	-- DE
; p8_calc_print_width_hl	HL	-- HL BC
; print_add_print_width		HL BC	-- HL BC
; p8_calc_print_width_char	A	-- A
; calc_row_b_col_c_from_hlc	HL C	-- B C
; calc_hlc_from_row_b_col_c	B C	-- HL C



; -------------------------------
; Data

.area GSDATA
;scratch:		defs	10
print_data:		equ	$
print_hl:		defw	$4000
print_c:		defb	$80
print_attr:		defb	black + white_paper
print_flags:		defb	0
print_scrollcount::	defb	0
print_pending_ctl:	defw	0
print_data_size:	equ	$ - print_data
print_stack:		defs	print_data_size * 2	; up to 2 nested pushes

; print_flags:
pbit_narrow::		equ	0 ; bit in print_flags: narrow spacing: omit first spacing column
pbit_inverse::		equ	1 ; bit in print_flags: print inverse:  set pixels for 0-bits in glyph
pbit_bold::		equ	2 ; bit in print_flags: print inverse:  set pixels for 0-bits in glyph
;pbit_log::		equ	3 ; bit in print_flags: log all printing (print_logptr)++
pbit_pending_ctl::	equ	4 ; bit in print_flags: control code pending, awaiting argument byte

; Attributes:
#include "zxsp_defines.s"


; Control Codes:
; note: must match pc_jumpblock!

ctl_eot::	equ	0	; end of text
ctl_home::	equ	1	; locate 0,0
ctl_cls::	equ	2	; clear screen with print_attr, set border, home cursor
ctl_locate::	equ	3	; set cursor to row, col	 (req. 2 argument characters)
ctl_setcol::	equ	4	; set cursor to col		 (req. 1 argument character)
ctl_setrow::	equ	5	; set cursor to row		 (req. 1 argument character)
ctl_setattr::	equ	6	; set paper, pen, bright + flashing (req. 1 argument character)
ctl_setpen::	equ	7	; set pen			 (req. 1 argument character)
ctl_setpaper::	equ	8	; set paper			 (req. 1 argument character)
ctl_clrcols::	equ	9	; clear N pixel columns, don't move cursor (req. 1 argument character)
ctl_tab::	equ	10	; set cursor to next tab position (n*4*8 pixel)
;		equ	11,12
ctl_newline::	equ	13	; set cursor to start of next line (may scroll)
ctl_inverse::	equ	14;15	; +0/+1: reset/set bright attr
ctl_narrow::	equ	16;17	; +0/+1: reset/set bright attr
ctl_bold::	equ	18;19	; +0/+1: reset/set bold attr
ctl_flash::	equ	20;21	; +0/+1: reset/set flash attr
ctl_bright::	equ	22;23	; +0/+1: reset/set bright attr
;	 	equ	24,25

ctl_clreol::	equ	26	; clear to end of line, don't move cursor
;		equ	27,28,29,30,31



.area CODE

; __________________________________________
; Reset Text VDU
;
; reset flags, discard pending ctl code waiting for argument.
; does not clear screen, home cursor or set color attributes.
;
; in:	--
; out:	--
; mod:	af

p8_reset::
	xor	a
	ld	(print_flags),a		; switch off special modes, kill pending ctl arg
	ld	(print_scrollcount),a	; "scroll?" count
	ret

; __________________________________________
; Ask, whether a ctl code requires arguments
;
; in:	A = ctl code
; out:	A = no. of arguments.
; mod:	AF

p8_get_ctl_args::
	cp	ctl_tab
	jr	nc,1$		; ctl >= tab: 0 arguments
	cp	ctl_locate
	jr	c,1$		; ctl < locate: eot, home, cls: 0 args
	ld	a,2
	ret	z		; locate: 2 args	
	dec	a		; setcol, setrow, setattr,setpen,setpaper: 1 arg
	ret
1$:	xor	a
	ret

; __________________________________________
; save text vdu state 
; note: the "stack" has 2 slots for nested calls
;
; in:	--
; out:	--
; mod:	f 

p8_push_state::
	call	save_registers
p8_push_state_quick::
	ld	hl, print_data + 2 * print_data_size - 1	; Q
	ld	de, print_data + 3 * print_data_size - 1	; Z
	ld	bc, print_data_size * 2				; cnt
	lddr
	ret

; __________________________________________
; restore text vdu state 
;
; in:	--
; out:	--
; mod:	af

p8_pop_state::
	call	save_registers
p8_pop_state_quick::
	ld	hl, print_data + print_data_size	; Q
	ld	de, print_data				; Z
	ld	bc, print_data_size * 2			; cnt
	ldir
	ret

; __________________________________________
; Helper for print_char and print_text:
; preserve bc, hl, iy
; load and update print_hl and print_c
; af and de may be used as return values
;
; in  : AF B- DE -- --
; pres: -- BC -- HL IY
; out:  AF -- DE -- --

access_hlc:
	ex	iy,(sp)
	push	hl
	push	bc

	ld	hl,print_c
	ld	c,(hl)
	ld	hl,(print_hl)

	call	jp_iy

	ld	(print_hl),hl
	ld	hl,print_c
	ld	(hl),c

	pop	bc	
	pop	hl
	pop	iy
	ret

; __________________________________________
; print single character
;
; in:	a = char
; mod:	af

print_char::
	push	de
	call	print_char_fast
	pop	de
	ret

; __________________________________________
; print 0-delimited inline text 
;
; in:	pc++ = text
; mod:	af

print_msg::
	ex	hl,(sp)
	ex	hl,de
	call	print_text_de
	ex	hl,de
	ex	hl,(sp)
	ret
	
; in:	pc++ = text
; mod:	af, hl

print_msg_fast::
	ex	hl,de		; pres. de
	pop	de
	call	print_text_de
	ex	hl,de
	jp	hl
	
; __________________________________________
; print text 
;
; in:	de -> 0-terminated text
; out:	de++ -> behind $00
; mod:	af, de

print_text_de::
	call	access_hlc

1$:	ld	a,(de++)
	and	a
	ret	z
	push	de
	call	up_print_char_a_at_hl_bit_c
	pop	de
	jr	1$

; __________________________________________
; print text 
;
; in:	de -> text
;	a  =  text length
; out:  de++, a=0
; mod:	af, de

print_text_dea::
	call	access_hlc
	inc	a
	jr	2$

1$:	push	af
	ld	a,(de++)
	push	de
	call	up_print_char_a_at_hl_bit_c
	pop	de
	pop	af
2$:	dec	a
	jr	nz,1$
	ret

; __________________________________________
; print unsigned decimal number 
; print hl with auto-sized field width
;
; in:	hl = number
; out:	--
; mod:	af

print_uint::
	call	save_registers	
print_uint_fast::
	ld	de,scratch
	call	utoa
pd2:	ld	de,scratch
	jr	print_text_de


; __________________________________________
; print signed decimal number 
; print hl with auto-sized field width
;
; in:	hl = number
; out:	--
; mod:	af

print_int::
	call	save_registers
print_int_fast::
	ld	de,scratch
	call	itoa
	jr	pd2


; __________________________________________
; Calc print width for character A
; TODO: bold printing (not yet implemented): probably +1 pixel
; A must be a printable character
;
; in:	A = character to print
; out:	A = print width
; mod:	AF, HL

p8_calc_print_width_char::

	call	p8_calc_character_width
	
	inc	a			; for character spacing	
	ld	hl,print_flags
	bit	pbit_narrow,(hl)	; narrow => 1 pixel spacing
	ret	nz	
	inc	a			; normal => 2 pixel spacing
	ret


; ------------------------------------------------------------
; Locate Textcursor at Row B, Column C
; Setup print_hl and print_c
;
; if row B >= 24, then the screen is scrolled 
; until row fits in screen
;
; in:	B = row [0..255]
;	C = col [0..255]
; out:	B = row [0..23]  (adjusted)
; mod:	af, b

p8_locate::
	ld	a,b
	cp	24
	jp	c,print_set_row_b_and_col_c
	call	scroll_screen
	dec	b
	jr	p8_locate

; ------------------------------------------------------------
; Locate Textcursor at Row A
; Setup print_hl and print_c
;
; if row A >= 24, then the screen is scrolled 
; until row fits in screen
;
; in:	A = row [0..23]
; out:	--
; mod:	af,iy

print_setrow::
	cp	24
	jr	c,pc_l4
	push	af
	call	scroll_screen
	pop	af
	dec	a
	jr	print_setrow

pc_l4:	push	bc
	ex	af,af'
	call	print_get_row_b_and_col_c
	ex	af,af'
	ld	b,a
	call	print_set_row_b_and_col_c
	pop	bc
	ret



; ------------------------------------------------------------
; Locate Textcursor at Column A
; Setup print_hl and print_c
;
; in:	A = col [0..255]
; out:	--
; mod:	af

print_setcol::
	push	bc
	ex	af,af'
	call	print_get_row_b_and_col_c
	ex	af,af'
	ld	c,a
	call	print_set_row_b_and_col_c
	pop	bc
	ret
	


; ------------------------------------------------------------
; Set color attributes for subsequent text
;
; in:	A = Attr
; out:	--
; mod:	--

p8_setattr::
	ld	(print_attr),a
	ret
	
	


; ------------------------------------------------------------
; Scroll screen up 
;
; scrolls screen up by 1 character row
; scrolls 23 rows and clears bottom row with print_attr
; scrolls pixels and attributes 
; printing position etc. are not updated
;
; in:	--
; out:	--
; mod:	af

scroll_screen::				; was: with question "SCROLL?"
scroll_screen_up::
	call	save_registers
	ld	hl,$4000		; screen start
	ld	bc,24*256+32		; 24 rows, 32 columns
	jp	scroll_cbox_up_quick






; ------------------------------------------------------------
; Determine text fitting within a given maximum size.
; Text break at word boundary
; Returned printable text may be 0 bytes long! 
;
; Note: if the entire 64k of memory did not contain any code <= ' ' or '-' 
;	then this routine would loop forever. But luckily this routine itself contains them. :-)
; 
; in:	HL -> text to print, terminated with 0
;	BC = max. print width
; out:	HL -> first not fitting character (0/ctl_code/' '/behind '-')
; mod:	AF, BC, DE, HL

print_find_word_wrap::

	push	hl
	call	print_find_char_wrap	; hl -> behind last fitting char
	pop	de			; de -> text start

	ld	a,(hl)
	cp	33
	ret	c			; return hl -> ' ' or ctl code or 0
	
pctw1:	dec	hl			; step back
	ld	a,(hl)
	cp	' '
	jr	z,pctw3			; return hl -> ' '

	cp	'-'
	jr	nz,pctw1
	inc	hl			; return hl -> behind '-'

; Word break found. Check for negative size:
; (if hl stepped back beyond start of text)

pctw3:	ld	a,h		; h muss ≥ d sein
	sub	d
	jr	nz,pctw4

; hi-bytes sind identisch	
	ld	a,l		; l muss ≥ e sein
	sub	e
	;ret	z		; hl = de 
	
pctw4:	ret	nc		; hl >= de
	ex	hl,de		; hl < de => hl := de
	ret



; ------------------------------------------------------------
; Determine text fitting within a given maximum size.
; Text break at character boundary.
; Returned printable text may be 0 bytes long! 
; 
; in:	HL -> text to print, terminated with 0
;	BC = max. print width
; out:	HL -> behind last fitting character
; mod:	AF, BC, DE, HL

print_find_char_wrap::
	
	ld	a,b
	cpl
	ld	b,a
	ld	a,c
	cpl
	ld	c,a			; bc := -1 -bc

	jr	print_add_print_width



; ------------------------------------------------------------
; Calc print width for text string (hl)++
; calculates actual print width with current settings for 
; narrow spacing and bold (TODO, bold NIMP)
;
; Print width is calculated up to the terminating 0-byte
; or up to the first encountered control code.
;
; on return HL points behind the last counted character,
; that is, to the terminating 0-byte or a control code.
;
; if print_add_print_width is called with a negative start value in BC
; then print_add_print_width also exits when incrementing BC reaches 0.
; then HL points to the character which caused the overflow and the
; calculated print width is too long by the print width of this character.
;
; in:	HL -> text to print, terminated with 0
; out:	BC = print width
;	HL -> behind last counted character
; mod:	AF, BC, HL

p8_calc_print_width_hl:	
	ld	bc,0		; counter
	
print_add_print_width::
pcpw:	ld	a,(hl)
	cp	32
	ret	c		; control code => return with hl -> aborting code
	inc	hl

	push	hl
	call	p8_calc_print_width_char	; a -- a
	pop	hl
	
	add	c
	ld	c,a
	jr	nc,pcpw
	inc	b
	jr	nz,pcpw

; BC ≥ 0: return pointer to not fitting character.
	dec	hl
	ret



	
; ------------------------------------------------------------
; ------------------------------------------------------------
;	Internal Routines
; ------------------------------------------------------------
; ------------------------------------------------------------



; __________________________________________
; print single character
;
; in:	a = char
; mod:	af, de

print_char_fast::
	call	access_hlc
	;jp	up_print_char_a_at_hl_bit_c
	
	

; ------------------------------------------------------------
; Print char a at address hl using bit c/8
;
; in:  HL,C = Printposition
;      A    = Char
; out: HL,C = Printposition, aktualisiert
; mod: AF, BC, DE, HL

up_print_char_a_at_hl_bit_c:
	ex	hl,de			; de -> screen byte
	ld	hl,print_flags		; log printing ?

	bit	pbit_pending_ctl,(hl)	; hl = print_flags
	jr	z,pc_ab
	res	pbit_pending_ctl,(hl)
	ld	hl,(print_pending_ctl)
	jp	(hl)

pc_ab:	cp	' '
	jr	c,pc_d			; control

; calc glyph address
	add	a,a			; *2
	ld	hl,p8_charset_ptr	; hl -> glyph pointer table
	add	a,l
	ld	l,a
	jr	nc,$+3
	inc	h
	ld	a,(hl)
	inc	hl
	ld	h,(hl)
	ld	l,a			; hl -> glyph, de preserved.
	ex	hl,de			; de -> glyph, hl->screen byte
	
; print char de at address hl using mask c
; increments hl and rotates c 

up_print_char_de_at_hl_bit_c:

	ld	a,(print_flags)
	bit	pbit_narrow,a
	call	z,print_add_spacing

	ld	a,(de)			; 1st glyph col w/o own start-marker
	and	$FE			; remove bit 0 = start-of-glyph marker

pc_ba:	push	hl
	ld	b,a
	call	calc_attr_hl_for_pixel_hl	; set attribute
	ld	a,(print_attr)
	ld	(hl),a
	ld	a,b
	pop	hl

pc_b:	ld	b,a
	rrc	a	
	jr	c,print_add_spacing	; bit 0 set => next char	
	jr	z,pc_z			; Byte B muss != 0 sein, sonst kein Abbruch!

; -------------------------------------------
; Paint Pixel Column HLC with Glyph Byte B:
; Byte B muss != 0 sein sonst kein Abbruch...

pc_c:	ld	a,h			; make pixel row sub address valid
	or	7			; make valid  
	ld	h,a			; pixel werden von unten nach oben geplottet

; %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
; Laufzeit für rst print_msg( ctl_home,"The Quick Brown Fox Jumped Over The Lazy Dogs.",$00 )
;	48k:	68144 T
;	16k:	69107 T		<- Laufzeitdifferenz alleine durch Stack im contended Ram!
;
; note: 'jr' ist bei allen Sprüngen günstiger als 'jp'
; Das unterste Pixel wird nicht in der Schleife, sondern vorab separat getestet.
; Das erspart 'dec h' (vorab) & 'inc h' (in der Schleife) und der Test lohnt sich meist,
; weil das unterste Pixel nur bei Unterlängen, also selten gesetzt ist.

	;inc	h
	;dec	h
	sla	b		; teste unterstes Pixel. das ist meist nicht gesetzt, 
	jr	c,pc_i		; außer bei Unterlängen. Der Extratest lohnt sich!

pc_a:	dec	h		; nächste Pixelzeile
	sla	b		; nächstes Pixel
	jr	nc,pc_a		; Pixel ist 0 => next Pixel
pc_i:	jr	z,pc_h		; Rest ist 0 => set pixel & fertig

pc_g:	ld	a,(hl)		; Rest ist nicht 0 => set Pixel und weiter
	or	c
	ld	(hl),a

pc_a2:	dec	h		; next pixel row
	sla	b		; next pixel
	jr	nc,pc_a		; Pixel ist 0 => next Pixel
	jr	nz,pc_g		; Rest ist nicht 0 => set Pixel und weiter
	
pc_h:	ld	a,(hl)		; Rest ist 0 => set pixel & fertig
	or	c
	ld	(hl),a

; ---- End of Paint 1 Pixel Column ----

pc_z:	rrc	c			; next col
	inc	de			; next glyph col
	ld	a,(de)
	jr	nc,pc_b			; cy flag von: 'rrc c'

; next screen byte	
	inc	l
	jr	nz,pc_ba

; next line && next block	
	call	pc_e
	ld	a,(de)
	jr	pc_ba


print_add_spacing:		; add character spacing to hl/c
	rrc	c
	ret	nc
	inc	l
	ret	nz
	
; next line && next block
pc_e:	ld	a,h
	add	8		; next block
	ld	h,a
	cp	$58
	ret	c
; end of screen
	ld	hl,$50E0
	jp	scroll_screen


; control code

pc_d:	ex	hl,de		; hl -> screen byte
pc_f:	and	a,31		; a := code [0 ... 31]
	add	a,a		; a := code*2
	ex	hl,de
	ld	hl,pc_jumpblock
	add	l
	ld	l,a
	jr	nc,$+3
	inc	h
	ld	a,(hl)
	inc	hl
	ld	h,(hl)
	ld	l,a
	jp	(hl)
	
; Jumpblock für Controlcodes [0 .. 31]	
; in:  Printposition in dec (nicht hlc)
; out: Printposition in hlc
pc_jumpblock:
	defw	pc_eot			; $00 End of text
	defw	pc_home			; $01 Locate 0,0
	defw	pc_cls			; $02 Clear screen & home cursor
	defw	pc_locate		; $03 Arguments: row,col
	defw	pc_setcol		; $04 Argument:  col [0..255]
	defw	pc_setrow		; $05 Argument:  row [0..23]  ((größere Werte scrollen Screen))
	defw	pc_setattr		; $06 Argument:  pen_ink+paper_ink+bright+flashing
	defw	pc_setpen		; $07 Argument:  pen color black ... white
	defw	pc_setpaper		; $08 Argument:  paper color black_paper ... white_paper
	defw	pc_clrcols		; $09 Argument:  pixel columns [0..255]  (0=256)
	defw	pc_tab			; $0A
	defw	pc_11,pc_12
	defw	pc_newline		; $0D
	defw	pc_inverse_off,pc_inverse_on
	defw	pc_narrow_off, pc_narrow_on
	defw	pc_bold_off,   pc_bold_on
	defw	pc_flash_off,  pc_flash_on
	defw	pc_bright_off, pc_bright_on
	defw	pc_24,pc_25
	defw	pc_clr2eol
	defw	pc_27,pc_28,pc_29,pc_30,pc_31


; unknown ctrl codes:
pc_11:
pc_12:
pc_24:
pc_25:
pc_27:
pc_28:
pc_29:
pc_30:
pc_31:
	;di		; debug
	;halt
	;jr	pc_eot

; end of text: char(0) 
; sollte eigentlich nicht vorkommen
pc_eot:
	ex	hl,de		
	ret

pc_flash_off:
	ld	hl,print_attr
	res	7,(hl)
	ex	hl,de
	ret
	
pc_flash_on:
	ld	hl,print_attr
	set	7,(hl)
	ex	hl,de
	ret

pc_bright_off:
	ld	hl,print_attr
	res	6,(hl)
	ex	hl,de
	ret

pc_bright_on:
	ld	hl,print_attr
	set	6,(hl)
	ex	hl,de
	ret

pc_inverse_off:	
	ld	hl,print_flags
	res	pbit_inverse,(hl)
	ex	hl,de
	ret
	
pc_inverse_on:
	ld	hl,print_flags
	set	pbit_inverse,(hl)
	ex	hl,de
	ret

pc_narrow_off:
	ld	hl,print_flags
	res	pbit_narrow,(hl)
	ex	hl,de
	ret
	
pc_narrow_on:
	ld	hl,print_flags
	set	pbit_narrow,(hl)
	ex	hl,de
	ret

pc_bold_off:
	ld	hl,print_flags
	res	pbit_bold,(hl)
	ex	hl,de
	ret

pc_bold_on:
	ld	hl,print_flags
	set	pbit_bold,(hl)
	ex	hl,de
	ret

; cls, set border, home cursor, reset scroll count
pc_cls:				
	ld	a,(print_attr)
	call	clear_screen_with_attr	; clear screen with current attr
	;jr	pc_home

; home cursor, reset scroll count
pc_home:
	sub	a		; reset "Scroll?" message count down
	ld	(print_scrollcount),a
	ld	c,$80
	ld	hl,$4000	
	ret

; tab => alle 4 Byte (etwa 6 Buchstaben)
; TODO: Attribute setzen?
pc_tab:
	ld	a,3		; mask
	jr	pc_nl

pc_newline:
	ld	a,$1f		; mask
pc_nl:	ld	c,$80
	ex	hl,de		; hl := screenbyte addr
	or	l
	inc	a		
	ld	l,a
	ret	nz		; kein Übertrag über Blockgrenze (Zeilengrenze egal!)
	jp	pc_e		; nächster Block, evtl. Screenende => scrollen
	
pc_setpen:			; Argument: pen color black ... white
	ld	hl,pc_setpen_resume
	jr	pc_argx

pc_setpaper:			; Argument: paper color black ... white
	ld	hl,pc_setpaper_resume
	jr	pc_argx

pc_locate:			; Arguments: row [-128.. [0..23] ..127], col [0..255]
	ld	hl,pc_locate_resume
	jr	pc_argx

pc_locate_resume:	
	call	pc_setrow_resume
	ex	hl,de
	;jr	pc_setcol
	
pc_setcol:			; Argument: col [0..255]
	ld	hl,pc_setcol_resume
	jr	pc_argx

pc_setrow:			; Argument: row [0..23] bzw. [-128 .. +127] may scroll
	ld	hl,pc_setrow_resume
	jr	pc_argx

pc_clrcols:			; Argument: columns [0..255]  (0=256)
	ld	hl,pc_clrcols_resume
	jr	pc_argx

pc_clr2eol:
	ld	b,0		; b = cols = 0 = 256; will be truncated
	ex	hl,de		; hl -> pixel
	jp	print_clear_cols
	
pc_setattr:			; Argument: attribute %FBPapPen (Flash, Bright, Paper, Pen)
	ld	hl,pc_setattr_resume
	;jr	pc_argx
		
pc_argx:ld	(print_pending_ctl),hl
	ld	hl,print_flags
	set	pbit_pending_ctl,(hl)
	ex	hl,de
	ret

pc_clrcols_resume:
	ld	b,a		; b = cols
	ex	hl,de		; hl -> pixel
	jr	print_clear_cols
	
pc_setpen_resume:
	and	a,$07		; force legal
	ld	b,a		; b := new val.
	ld	a,$F8		; other bits mask
	jr	pc_sp_r

pc_setpaper_resume:
	rlca
	rlca
	rlca
	and	a,$38		; force legal
	ld	b,a		; b := new val.
	ld	a,$C7		; other bits mask
pc_sp_r:ld	hl,print_attr
	and	a,(hl)		; a := other bits
	or	b		; a := new attr
	ld	(hl),a		; store new attr
	ex	hl,de
	ret

pc_setattr_resume:
	ld	(print_attr),a	; store new attr
	ex	hl,de
	ret

pc_setcol_resume:
	ex	hl,de		; => wieder hlc
	push	af		; new col [0..255]
	call	calc_row_b_col_c_from_hlc
	pop	af
	ld	c,a		; new col
	jp	calc_hlc_from_row_b_col_c

pc_setrow_resume:
	ex	hl,de
	cp	a,24		; force legal (TODO: evtl. scroll up/down)
	jr	c,pc_sr1	; [0 .. a .. 23]
	add	a		; mi -> cy
	ld	a,23
	jr	nc,$+3		; a>23  =>  a:=23
	sub	a		; a<0   =>  a:= 0

pc_sr1:	push	af
	call	calc_row_b_col_c_from_hlc
	sub	a
	ld	(print_scrollcount),a		; reset scroll count
	pop	af
	ld	b,a
	jp	calc_hlc_from_row_b_col_c



; ------------------------------------------------------------
; Clear current row with current paper color
;
; Does not move the text cursor
;
; in:	--
; out:	--
; mod:	af

print_clear_current_row:
	call	save_registers
	ld	hl,(print_hl)
	jr	print_clear_row_hl_quick



; ------------------------------------------------------------
; Clear text row with current paper color
;
; in:	b = row
; out:	--
; mod:	af

print_clear_row:
	call	save_registers
	ld	c,0
	call	calc_hlc_from_row_b_col_c
	jr	pcr_hl



; ------------------------------------------------------------
; Clear text row with current paper color
;
; in:	hl -> current pixel byte
; out:	--
; mod:	af

print_clear_row_hl:
	call	save_registers

print_clear_row_hl_quick:	
	ld	a,h			; hl -> top row of pixels
	and	$f8
	ld	h,a

	ld	a,l			; hl -> column 0
	and	$e0
	ld	l,a
	
pcr_hl:	ld	bc, 1*256 + 32		; 1 row, 32 cells
	ld	a,(print_attr)
	jp	clear_cbox_with_attr_quick



; ------------------------------------------------------------
; Clear range of text row
; 
; Lösche B Pixelspalten ab hlc mit dem aktuellen print_attr.
; Löscht maximal bis Zeilenende
; Wenn B=0 wird B=256 angenommen.
;
; in:	hl -> current pixel byte
;	c  =  current pixel mask
;	b  =  width; 0=256
; out:	--
; mod:	af

print_clear_cols:
	call	save_registers

; validate width: must not extend beyond end of line
	ld	de,bc		; retten
	call	calc_row_b_col_c_from_hlc	
	ld	a,d		; width
	and	a		; width = 0 = 256 ?
	jr	z,pcc1		; ja => truncate
	add	c		; current col + width
	jr	nc,pcc2		; ok

; truncate b to end of line:
pcc1:	xor	a
	sub	c
	ld	d,a		; d = width (truncated)
pcc2:	ld	bc,de		; b = width; c = pixel mask (again)	

; print preceding columns:
; wir drucken auch die ersten vollen 8 pixelspalten mit up_clear_partial_cell,
; weil dadurch B, falls es 0 für 256 war, unter $100 erniedrigt wird.
; das erspart das Geteste und Verzweigen für B=0.
	;bit	7,c			; start at start of cell?
	;jr	nz,pcb1			; yes

	call	up_clear_partial_cell
	ld	a,b
	and	a
	ret	z			; 0 pixels remaining
	inc	l			; hl -> next pixel byte

; clear complete cells:
pcb1:	ld	d,b			; d = b: count retten
	ld	a,b
	rrca
	rrca
	rrca
	and	$1f
	ld	e,a			; cell count retten

	ld	c,a			; c = cells	
	ld	b,1			; b = rows
	ld	a,(print_attr)		; a = attr
	call	nz,clear_cbox_with_attr

	ld	a,l
	add	e
	ld	l,a			; hl += cells
	ld	c,$80			; c = pixel mask

; clear remaining pixels right:
	ld	a,d			; a = count
	and	7
	ret	z			; no trailing partial block

	ld	b,a
	;jr	up_clear_partial_cell
	


; ---------------------------------------
; clear partial cell with mask:
;
; in:	hl -> top pixel row
;	c = current pixel mask
;	b = count >= 1
; out:	c  updated
;	b  decremented by number of pixel columns cleared
;	hl not updated!
; mod:	af, bc, hl

up_clear_partial_cell:
	xor	a
pcb3:	jr	c,pcb2			; end of character cell reached
	or	c
	rrc	c
	djnz	pcb3

; a = mask

pcb2:	push	bc
	push	hl

	cpl
	ld	c,a		; mask

	ld	a,h		; hl -> top row of pixels
	and	$f8
	ld	h,a

	ld	b,8
pcb4:	ld	a,(hl)
	and	c
	ld	(hl),a
	inc	h
	djnz	pcb4
	dec	h		; hl back into cell, bottom row

	call	calc_attr_hl_for_pixel_hl
	ld	a,(print_attr)
	ld	(hl),a

	pop	hl
	pop	bc
	ret



; ------------------------------------------------------------
; Get Textcursor Row and Column
;
; in:	--
; out:	B = row [0..23]
;	C = col [0..255]
; mod:	af, bc

print_get_row_b_and_col_c:
	push	hl
	ld	hl,print_c
	ld	c,(hl)
	ld	hl,(print_hl)
	call	calc_row_b_col_c_from_hlc
	pop	hl
	ret
	
	
; ------------------------------------------------------------
; get current print position in row=B, col=C
;
; out:	B  = row [0..23]
;	C  = col [0..255]
; mod:	AF, BC, HL

print_getloc::
	ld	hl,(print_hl)
	ld	a,(print_c)
	ld	c,a
	;jp	calc_row_b_col_c_from_hlc
	

; ------------------------------------------------------------
; UP: Berechne Row B und Col C
; zu Byteadresse HL und Bitmaske C 
;
; in:	HL = Byteadresse [$4000..$57FF]
;	C  = Bitmaske
; out:	B  = row [0..23]
;	C  = col [0..255]
;	A  = B
; mod:	AF, BC

calc_row_b_col_c_from_hlc:
	
; konvertiere pixelmaske in c in spaltenzahl 0 (ganz links) ... 7 (ganz rechts)	
; Diese Routine braucht im Mittel 78.0 Taktzyklen.
; Eine Schleife mit rlca/inc r/jr nz bräuchte im Mittel etwa 100 Takte.

	ld	a,$0F		; Maske: ____XXXX  hier dabei? => col+4
	and	c
	ld	b,a		; b := 0 wenn a=0
	jr	z,$+4
	ld	b,4		; b := 4 wenn a!=0

	ld	a,$33		; Maske: __XX__XX  hier dabei? => col+2
	and	c
	jr	z,$+4
	set	1,b

	ld	a,$55		; Maske: _X_X_X_X  hier dabei? => col+1
	and	c
	jr	z,$+3
	inc	b		
	ld	c,b		; c = %00000bbb = col inside char [0..7]
	
; l = %rrrccccc		wobei rrr = row inside char  und ccccc = char col
; l rotieren, dass es so aussieht:
; l = %cccccrrr		damit stehen rrr und ccccc auf den positionen, 
;			die ihrer Wertigkeit entspricht

	ld	a,l		
	rlca
	rlca
	rlca			
	ld	b,a		; b = a = %cccccrrr

	and	$F8		; a = %ccccc000
	or	c		; a = %cccccbbb  
	ld	c,a		; c = pixelspalte = fertig

	ld	a,b		 
	and	7		
	ld	b,a		; b = %00000rrr

; h = %010bbzzz		wobei bb = 8-Textzeilen-Block  und zzz = pixelzeile innerhalb eines Zeichens
	
	ld	a,h	
	and	$18		; a = %000bb000		
	or	b		; a = %000bbrrr
	ld	b,a		; b = character row = fertig
	
	ret



; ------------------------------------------------------------
; Set Textcursor Row & Column
;
; in:	B = row [0..23]
;	C = col [0..255]
; out:	--
; mod:	af

print_set_row_b_and_col_c:
	push	hl
	push	bc
	call	calc_hlc_from_row_b_col_c
	ld	(print_hl),hl
	ld	hl,print_c
	ld	(hl),c
	pop	bc
	pop	hl
	ret
	
	

; ------------------------------------------------------------
; UP: Berechne Byteadresse HL und Bitmaske C 
; zu Row B [0..23] und Col C [0..255]
;
; in:	B  = row [0..23]
;	C  = col [0..255]
; out:	HL = Byteadresse [$4000..[$5800
;	C  = Bitmaske
;	A  = C
; mod:	AF, BC, HL

calc_hlc_from_row_b_col_c:

;	hl := %010bb000.rrrccccc	bb=block, rr=row inside block, ccccc=char col
;	c = %10000000 >> (c&7)

	ld	a,b
	and	$18		; a = %000bb000
	or	$40
	ld	h,a		; h = %010bb000 = fertig
	
	ld	a,b
	and	7
	ld	b,a		; b = %00000rrr
	
	ld	a,c
	and	$F8		; a = %ccccc000
	or	b		; a = %cccccrrr
	rrca
	rrca
	rrca			; a = %rrrccccc
	ld	l,a		; l = %rrrccccc = fertig
	
	ld	a,c		; a = pixel col
	and	7		; a = pixel col inside byte
	add	lo(pmasks)
	ld	c,a
	ld	b,hi(pmasks)
	ld	a,(bc)
	ld	c,a

	ret
	
	defs	hi($) != hi($+7) ? 256 - lo($) : 0
pmasks:	defb	$80, $40, $20, $10, $08, $04, $02, $01



		




