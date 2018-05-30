
; ------------------------------------------------------------
; Display "scroll?" and wait for key
;
; in:	--
; out:	--
; mod:	af, iy

show_scroll_and_wait_for_key::
	call	save_registers
			
	ld	bc,1*256 + 6		; rows = 1; cols = 6
	ld	hl,$5000+8*32 -6	; HL -> screen
	call	scr_save_pixels
	call	clear_pixels
	call	calc_attr_hl_for_pixel_hl
	call	scr_save_attributes
	call	clear_attributes
	call	print_push_state_quick

	call	print_reset
	call	print_msg
	defb	ctl_locate, 23, 256-6*8
	defb	ctl_setattr, white + red_paper
	defm	"scroll?",$00
	call	wait_newkey

	call	print_pop_state_quick
	call	scr_restore_attributes
	jp	scr_restore_pixels
		
		
scroll_screen::
	ld	a,(print_scrollcount)
	dec	a
	jp	p,ss1

	call	show_scroll_and_wait_for_key
	ld	a,24 -1	

ss1:	ld	(print_scrollcount),a
	jp	scroll_screen_up


; ------------------------------------------------------------
; print "yes" or "no" depending on cy-flag:
;
; cy -> print "yes"
; nc -> print "no"
;
; in:	cy-flag
; out:	cy-flag 
; mod:	--

print_yes_no::
	jr	c,print_yes
	;jr	nc,print_no

	

; ------------------------------------------------------------
; print "no" & ret nc
;
; in:	--
; out:	f: cy = 0
; mod:	f

print_no::
	call	print_msg
	defm	"no",0
	and	a		; enforce nc
	ret



; ------------------------------------------------------------
; print "yes" & ret c
;
; in:	--
; out:	f:  cy = 1
; mod:	f

print_yes::
	call	print_msg
	defm	"yes",0
	scf			; enforce cy
	ret


; ---------------------------------------------------------
; Speed Test
;
; set both breakpoints and examine T cycle count 
; in zxsp or other emulator.
;
; 2006-09-17: ~69000 T cycles
; 2017-07-22: ~71000 T cycles
; this will vary with every change made to the print routine.

#if 0
print_speed_test::
	halt
	di
	call	print_msg		; <- breakpoint here
	defb	ctl_home
	defm	"The Quick Brown Fox Jumped Over The Lazy Dogs.",0
	ei				; <- breakpoint here
	halt
	halt
	jp	main_menu
#endif


; ------------------------------------------------------------
; print unsigned decimal number 
; print a with auto-sized field width
;
; in:	a = number
; out:	--
; mod:	af

print_uint8::
	call	save_registers
print_uint8_fast::
	ld	l,a
	ld	h,0
	jr	print_uint_fast



; ------------------------------------------------------------
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
	call	decstr		; max. 5 char		TODO
pd2:	ex	hl,de
	ld	(hl),0
	ld	hl,scratch
	jr	print_text_hl

		
; ------------------------------------------------------------
; print signed decimal number 
; print hl with auto-sized field width
;
; in:	hl = number
; out:	--
; mod:	af

print_int::
	call	save_registers
	ld	de,scratch
	call	vzdecstr		; max. 6 char		TODO
	jr	pd2

		
; ------------------------------------------------------------
; print word as 4 hex chars
;
; in:	hl = number
; out:	--
; mod:	af

print_hex4::
	call	save_registers
	ld	de,scratch
	call	hexstr4			; TODO
	jr	pd2



; ------------------------------------------------------------
; print byte as 2 hex chars
;
; in:	a = number
; out:	--
; mod:	af

print_hex2::
	call	save_registers	
	ld	de,scratch
	call	hexstr2	
	jr	pd2



; ------------------------------------------------------------
; print hex char
;
; in:	a = number (lower nibble only)
; out:	--
; mod:	af

print_hex1::
	and	$0f
	add	'0'
	cp	'9'+1
	jr	c,print_char
	add	7
	jr	print_char



; ------------------------------------------------------------
; print word as 16 binary chars
;
; in:	hl = number
; out:	--
; mod:	af

print_bin16::
	push	hl
	ld	a,h
	call	print_bin8		; TODO
	pop	hl
	ld	a,l
	;jr	print_bin8   



; ------------------------------------------------------------
; print byte as 8 binary chars
;
; in:	a = number
; out:	--
; mod:	af

print_bin8::
	call	save_registers	
	ld	de,scratch
	call	binstr8			; TODO
	jr	pd2



; ------------------------------------------------------------
; print nibble as 4 binary chars
;
; in:	a = number (lower nibble only)
; out:	--
; mod:	af

print_bin4::
	call	save_registers	
	ld	de,scratch
	call	binstr4			; TODO
	jr	pd2

	





