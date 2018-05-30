; ___________________________
; Save registers
; and prepare stack with routine address
; to restore the registers after return
;
; All registers except IY are passed through on entry.
; BC, DE, HL and IY are restored after exit.
; All other registers are preserved on exit.


.area CODE

; ___________________________
; in  : AF BC DE HL --
; pres: -- BC DE HL IY
; rval: AF -- -- -- 

#if required(save_registers) || required(save_bcdehl)
save_registers::
save_bcdehl::
	ex	iy,(sp)
	push	hl			
	push	de
	push	bc
	call	jp_iy
	pop	bc
	pop	de
	pop	hl
	pop	iy	
	ret
#endif

; ___________________________
; in  : AF BC DE HL --
; pres: AF BC DE HL IY
; rval: -- -- -- -- 

#if required(save_afbcdehl)
save_afbcdehl::
	ex	iy,(sp)
	push	hl			
	push	de
	push	bc
	push	af
	call	jp_iy
	pop	af
	pop	bc
	pop	de
	pop	hl
	pop	iy	
	ret
#endif

; ___________________________
; in  : AF BC DE HL --
; pres: -- BC DE -- IY
; rval: AF -- -- HL 

; pass    AF, BC, DE and HL
; restore BC, DE and IY
; return value in AF and HL

#if required(save_bcde)
save_bcde::
	ex	iy,(sp)
	push	de
	push	bc
	call	jp_iy
	pop	bc
	pop	de
	pop	iy	
	ret
#endif

; ___________________________
; in  : AF BC DE xx --
; pres: -- BC DE -- IY
; rval: AF -- -- HL 

#if required(save_bcde_no_hl)	
save_bcde_no_hl::
	pop	hl
	push	de
	push	bc
	call	jp_hl
	pop	bc
	pop	de
	ret
#endif

; ___________________________
; in  : AF BC DE HL --
; pres: -- BC -- HL IY
; rval: AF -- DE -- 

#if required(save_bchl)
save_bchl::
	ex	iy,(sp)
	push	hl
	push	bc
	call	jp_iy
	pop	bc
	pop	hl
	pop	iy	
	ret
#endif

; ___________________________
; in  : AF BC DE HL --
; pres: -- -- DE HL IY
; rval: AF BC -- --

#if required(save_dehl)
save_dehl::
	ex	iy,(sp)
	push	hl			
	push	de
	call	jp_iy
	pop	de
	pop	hl
	pop	iy	
	ret
#endif


#if required(jp_iy)
jp_iy::	jp	iy
#endif
