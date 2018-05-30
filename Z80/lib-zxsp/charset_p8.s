
; ============================================================
;
; proportional font for 8 scan lines per text row.
; the top pixel row of all characters is empty.
; most characters have an empty bottom pixel row as well.
; characters with descender have pixels in the bottom pixel row.
;
; created for my zxsp proportional print engine.
; use freely, though noting me somewhere would be nice.
; (c) copyright 2006 - 2017 Kio (Günter Woigk)
;
; ============================================================



; ------------------------------------------------------------
; Calculate address of character image (glyph)
;
; The character code is not checked.
; Bit 7 of the character code is ignored.
;
; in:	A = char [27..127]
; out:	HL -> glyph
; mod:	AF, HL

.area CODE
p8_calc_character_address::
	add	a,a			;4 	*2
	ld	hl,p8_charset_ptr	;10 	hl -> glyph pointer table
	add	a,l			;4
	ld	l,a			;4
	jr	nc,$+3			;7 |12
	inc	h			;4
	ld	a,(hl++)		;13
	ld	h,(hl)			;7
	ld	l,a			;4 	hl -> glyph
	ret				;27 = 84.5cc


; ------------------------------------------------------------
; Calculate width of character image (glyph)
;
; The character code is not checked.
; Bit 7 of the character code is ignored.
;
; in:	A = char [27..127]
; out:	A = width
;	HL -> address of character glyph
; mod:	AF, HL

.area CODE
p8_calc_character_width::
	add	a,a			;4 	*2
	ld	hl,p8_charset_ptr+2	;10 	hl -> glyph pointer table +2
	add	a,l			;4
	ld	l,a			;4
	jr	nc,$+3			;7 |12
	inc	h			;4	hl -> next char address
	ld	a,(hl)			;7	a = next char address .lo
	dec 	hl 			;6
	dec 	hl			;6	hl -> this char address
	sub	(hl)			;7	a = char width
	ret				;27 = 86.5cc


; ------------------------------------------------------------
; Character set 
; first byte of each glyph is marked with bit 0 set 

.area CONST
charcode_icursor:: equ 128+27
c27:		defb	%11111110  |  1	

charcode_left_arrow:: equ 128+28
c28:		defb	%00010000  |  1		
		defb	%00111000
		defb	%01111100
		defb	%11111110
		defb	%00111000
		defb	%00111000

charcode_down_arrow:: equ 128+29
c29:		defb	%00010000  |  1		
		defb    %00110000
		defb    %01111100
		defb    %11111100
		defb    %01111100
		defb    %00110000
		defb    %00010000

charcode_up_arrow:: equ 128+30
c30:		defb	%00010000  |  1		
		defb    %00011000
		defb    %01111100
		defb    %01111110
		defb    %01111100
		defb    %00011000
		defb    %00010000

charcode_right_arrow:: equ 128+31
c31:		defb	%00111000  |  1		
		defb	%00111000
		defb	%11111110
		defb	%01111100
		defb	%00111000
		defb	%00010000

c32:		defb	%00000000  |  1		; space
		defb	%00000000

c33:		defb    %01011110  |  1		; !

c34:		defb    %00000110  |  1		; "
		defb    %00000000
		defb    %00000110

c35:		defb    %00100100  |  1		; #
		defb    %01111110
		defb    %00100100
		defb    %00100100
		defb    %01111110
		defb    %00100100

c36:		defb    %01011100  |  1		; $
		defb    %01010100
		defb    %11111110
		defb    %01010100
		defb    %01110100

c37:		defb    %01000110  |  1		; %
		defb    %00100110
		defb    %00010000
		defb    %00001000
		defb    %01100100
		defb    %01100010

c38:		defb    %00100000  |  1		; &
		defb    %01010100
		defb    %01001010
		defb    %01010100
		defb    %00100000
		defb    %01010000

c39:		defb    %00000110  |  1		; '

c40:		defb    %00111100  |  1		; (
		defb    %01000010

c41:		defb    %01000010  |  1		; )
		defb    %00111100

c42:		defb    %00010000  |  1		; *
		defb    %01010100
		defb    %00111000
		defb    %01010100
		defb    %00010000

c43:		defb    %00010000  |  1		; +
		defb    %00010000
		defb    %01111100
		defb    %00010000
		defb    %00010000

c44:		defb    %10000000  |  1		; ,
		defb    %01100000

c45:		defb    %00010000  |  1		; -
		defb    %00010000
		defb    %00010000
		defb    %00010000
		defb    %00010000

c46:		defb    %01100000  |  1		; .
		defb    %01100000

c47:		defb    %01000000  |  1		; /
		defb    %00100000
		defb    %00010000
		defb    %00001000
		defb    %00000100

c48:		defb    %00111100  |  1		; 0
		defb    %01100010
		defb    %01011010
		defb    %01000110
		defb    %00111100

c49:		defb    %01000100  |  1		; 1
		defb    %01000010
		defb    %01111110
		defb    %01000000
		defb    %01000000

c50:		defb    %01100100  |  1		; 2
		defb    %01010010
		defb    %01010010
		defb    %01010010
		defb    %01001100

c51:		defb    %00100100  |  1		; 3
		defb    %01000010
		defb    %01001010
		defb    %01001010
		defb    %00110100

c52:		defb    %00110000  |  1		; 4
		defb    %00101000
		defb    %00100100
		defb    %01111110
		defb    %00100000

c53:		defb    %00101110  |  1		; 5
		defb    %01001010
		defb    %01001010
		defb    %01001010
		defb    %00110010

c54:		defb    %00111100  |  1		; 6
		defb    %01001010
		defb    %01001010
		defb    %01001010
		defb    %00110000

c55:		defb    %00000010  |  1		; 7
		defb    %00000010
		defb    %01110010
		defb    %00001010
		defb    %00000110

c56:		defb    %00110100  |  1		; 8
		defb    %01001010
		defb    %01001010
		defb    %01001010
		defb    %00110100

c57:		defb    %00001100  |  1		; 9
		defb    %01010010
		defb    %01010010
		defb    %01010010
		defb    %00111100

c58:		defb    %01001000  |  1		; :

c59:		defb    %10000000  |  1		; ;
		defb    %01100100

c60:		defb    %00010000  |  1		; <
		defb    %00101000
		defb    %01000100

c61:		defb    %00101000  |  1		; =
		defb    %00101000
		defb    %00101000
		defb    %00101000

c62:		defb    %01000100  |  1		; >
		defb    %00101000
		defb    %00010000

c63:		defb    %00000100  |  1		; ?
		defb    %00000010
		defb    %01010010
		defb    %00001010
		defb    %00000100
	
c64:		defb    %00111100  |  1		; @
		defb    %01000010
		defb    %01011010
		defb    %01010110
		defb    %01011010
		defb    %00011100

c65:		defb    %01111100  |  1		; A
		defb    %00010010
		defb    %00010010
		defb    %00010010
		defb    %01111100

c66:		defb    %01111110  |  1		; B
		defb    %01001010
		defb    %01001010
		defb    %01001010
		defb    %00110100

c67:		defb    %00111100  |  1		; C
		defb    %01000010
		defb    %01000010
		defb    %01000010
		defb    %00100100

c68:		defb    %01111110  |  1		; D
		defb    %01000010
		defb    %01000010
		defb    %01000010
		defb    %00111100

c69:		defb    %01111110  |  1		; E
		defb    %01001010
		defb    %01001010
		defb    %01001010
		defb    %01000010

c70:		defb    %01111110  |  1		; F
		defb    %00001010
		defb    %00001010
		defb    %00001010
		defb    %00000010

c71:		defb    %00111100  |  1		; G
		defb    %01000010
		defb    %01010010
		defb    %01010010
		defb    %00110100

c72:		defb    %01111110  |  1		; H
		defb    %00001000
		defb    %00001000
		defb    %00001000
		defb    %01111110

c73:		defb    %01000010  |  1		; I
		defb    %01111110
		defb    %01000010

c74:		defb    %00110000  |  1		; J
		defb    %01000000
		defb    %01000000
		defb    %00111110

c75:		defb    %01111110  |  1		; K
		defb    %00011000
		defb    %00011000
		defb    %00100100
		defb    %01000010

c76:		defb    %01111110  |  1		; L
		defb    %01000000
		defb    %01000000
		defb    %01000000

c77:		defb    %01111110  |  1		; M
		defb    %00000100
		defb    %00001000
		defb    %00001000
		defb    %00000100
		defb    %01111110

c78:		defb    %01111110  |  1		; N
		defb    %00000100
		defb    %00001000
		defb    %00010000
		defb    %00100000
		defb    %01111110

c79:		defb    %00111100  |  1		; O
		defb    %01000010
		defb    %01000010
		defb    %01000010
		defb    %01000010
		defb    %00111100

c80:		defb    %01111110  |  1		; P
		defb    %00010010
		defb    %00010010
		defb    %00010010
		defb    %00001100

c81:		defb    %00111100  |  1		; Q
		defb    %01000010
		defb    %01010010
		defb    %01100010
		defb    %01000010
		defb    %00111100

c82:		defb    %01111110  |  1		; R
		defb    %00010010
		defb    %00010010
		defb    %00110010
		defb    %01001100

c83:		defb    %00100100  |  1		; S
		defb    %01001010
		defb    %01001010
		defb    %01001010
		defb    %00110000

c84:		defb    %00000010  |  1		; T
		defb    %00000010
		defb    %01111110
		defb    %00000010
		defb    %00000010

c85:		defb    %00111110  |  1		; U
		defb    %01000000
		defb    %01000000
		defb    %01000000
		defb    %00111110

c86:		defb    %00011110  |  1		; V
		defb    %00100000
		defb    %01000000
		defb    %00100000
		defb    %00011110

c87:		defb    %00111110  |  1		; W
		defb    %01000000
		defb    %00100000
		defb    %00100000
		defb    %01000000
		defb    %00111110

c88:		defb    %01000010  |  1		; X
		defb    %00100100
		defb    %00011000
		defb    %00011000
		defb    %00100100
		defb    %01000010

c89:		defb    %00000110  |  1		; Y
		defb    %00001000
		defb    %01110000
		defb    %00001000
		defb    %00000110

c90:		defb    %01000010  |  1		; Z
		defb    %01100010
		defb    %01010010
		defb    %01001010
		defb    %01000110
		defb    %01000010

c91:		defb    %01111110  |  1		; [
		defb    %01000010
		defb    %01000010

c92:		defb    %00000100  |  1		; \ 
		defb    %00001000
		defb    %00010000
		defb    %00100000
		defb    %01000000

c93:		defb    %01000010  |  1		; ]
		defb    %01000010
		defb    %01111110

c94:		defb    %00001000  |  1		; ^ (Pfeil hoch)
		defb    %00000100
		defb    %01111110
		defb    %00000100
		defb    %00001000

c95:		defb    %10000000  |  1		; _
		defb    %10000000
		defb    %10000000
		defb    %10000000

c96:		defb    %01001000  |  1		; £
		defb    %01111100
		defb    %01001010
		defb    %01000010
		defb    %01000100

c97:		defb    %00100000  |  1		; a
		defb    %01010100
		defb    %01010100
		defb    %01010100
		defb    %01111000

c98:		defb    %01111110  |  1		; b
		defb    %01001000
		defb    %01001000
		defb    %01001000
		defb    %00110000

c99:		defb    %00111000  |  1		; c
		defb    %01000100
		defb    %01000100
		defb    %01000100

c100:		defb    %00110000  |  1		; d
		defb    %01001000
		defb    %01001000
		defb    %01001000
		defb    %01111110

c101:		defb    %00111000  |  1		; e
		defb    %01010100
		defb    %01010100
		defb    %01010100
		defb    %01001000

c102:		defb    %01111100  |  1		; f
		defb    %00001010
		defb    %00000010

c103:		defb    %00011000  |  1		; g
		defb    %10100100
		defb    %10100100
		defb    %10100100
		defb    %01111100

c104:		defb    %01111110  |  1		; h
		defb    %00001000
		defb    %00001000
		defb    %00001000
		defb    %01110000

c105:		defb    %01001000  |  1		; i
		defb    %01111010
		defb    %01000000

c106:		defb    %01000000  |  1		; j
		defb    %10000000
		defb    %01111010
		defb    %00000000

c107:		defb    %01111110  |  1		; k
		defb    %00010000
		defb    %00101000
		defb    %01000100

c108:		defb    %00111110  |  1		; l
		defb    %01000000
		defb    %01000000

c109:		defb    %01111100  |  1		; m
		defb    %00000100
		defb    %00000100
		defb    %01111000
		defb    %00000100
		defb    %00000100
		defb    %01111000

c110:		defb    %01111100  |  1		; n
		defb    %00000100
		defb    %00000100
		defb    %00000100
		defb    %01111000

c111:		defb    %00111000  |  1		; o
		defb    %01000100
		defb    %01000100
		defb    %01000100
		defb    %00111000

c112:		defb    %11111100  |  1		; p
		defb    %00100100
		defb    %00100100
		defb    %00100100
		defb    %00011000

c113:		defb    %00011000  |  1		; q
		defb    %00100100
		defb    %00100100
		defb    %00100100
		defb    %11111100

c114:		defb    %01111000  |  1		; r
		defb    %00000100
		defb    %00000100
		defb    %00000100

c115:		defb    %01001000  |  1		; s
		defb    %01010100
		defb    %01010100
		defb    %01010100
		defb    %00100000

c116:		defb    %00000100  |  1		; t
		defb    %00111110
		defb    %01000100
		defb    %01000000

c117:		defb    %00111100  |  1		; u
		defb    %01000000
		defb    %01000000
		defb    %01000000
		defb    %00111100

c118:		defb    %00001100  |  1		; v
		defb    %00110000
		defb    %01000000
		defb    %00110000
		defb    %00001100

c119:		defb    %00111100  |  1		; w
		defb    %01000000
		defb    %00111000
		defb    %01000000
		defb    %00111100

c120:		defb    %01000100  |  1		; x
		defb    %00101000
		defb    %00010000
		defb    %00101000
		defb    %01000100

c121:		defb    %00011100  |  1		; y
		defb    %10100000
		defb    %10100000
		defb    %01111100

c122:		defb    %01000100  |  1		; z
		defb    %01100100
		defb    %01010100
		defb    %01001100
		defb    %01000100

c123:		defb    %00010000  |  1		; {
		defb    %01111100
		defb    %10000010
		defb    %10000010

c124:		defb    %01111110  |  1		; |

c125:		defb    %10000010  |  1		; }
		defb    %10000010
		defb    %01101100
		defb    %00010000

c126:		defb    %00000100  |  1		; ~
		defb    %00000010
		defb    %00000100
		defb    %00000010

c127:		defb    %00111000  |  1		; ©   (c) 
		defb    %01000100
		defb    %10010010
		defb    %10101010
		defb    %10101010
		defb    %10000010
		defb    %01000100
		defb    %00111000

c128:		defb    %00000000  |  1		; stopper for last char


; 96 Pointer: char(33) .. char(127) + stopper

		defw	c27,c28,c29,c30,c31
p8_charset_ptr:: equ	$ - 32*2
		defw	c32,c33,c34,c35,c36,c37,c38,c39
		defw	c40,c41,c42,c43,c44,c45,c46,c47,c48,c49
		defw	c50,c51,c52,c53,c54,c55,c56,c57,c58,c59
		defw	c60,c61,c62,c63,c64,c65,c66,c67,c68,c69
		defw	c70,c71,c72,c73,c74,c75,c76,c77,c78,c79
		defw	c80,c81,c82,c83,c84,c85,c86,c87,c88,c89
		defw	c90,c91,c92,c93,c94,c95,c96,c97,c98,c99
		defw	c100,c101,c102,c103,c104,c105,c106,c107,c108,c109
		defw	c110,c111,c112,c113,c114,c115,c116,c117,c118,c119
		defw	c120,c121,c122,c123,c124,c125,c126,c127,c128




