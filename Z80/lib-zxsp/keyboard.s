
; ============================================================
;
; ZX Spectrum Keyboard Routines for Text Input
; handle multiple keys down
; handle debounce
; handle auto repeat
;
; use freely, though noting me somewhere would be nice.
; (c) copyright 2006 - 2017 Kio (Günter Woigk)
;
; ============================================================

; this file defines:
;
; irpt_scan_keyboard:: (alias zxsp_keyboard) the keyboard scanning routine
; flush_inkey::  clear buffered keys
; get_inkey::    get key: return 0 if no key available
; wait_inkey::   get key: wait until key available
; wait_newkey::  get key: flush buffered keys and wait until key available
;
; keyboard tables
; misc. control key codes returned by the above calls.



.area SLOW_CDATA 	; may be located in slow ram, uncompressed
; --------------------------------------------------------
; Keyboard Tables

keycode_illegal  	equ	0		; illegal codes
keycode_cshift		equ	keycode_illegal	; single shift key
keycode_sshift		equ	keycode_illegal	; single shift key

keycode_edit::		equ	1
keycode_capslock::	equ	2
keycode_video::		equ	3
keycode_rvideo::	equ	4
keycode_left::		equ	5
keycode_down::		equ	6
keycode_up::		equ	7
keycode_right::		equ	8
keycode_graphics::	equ	9
keycode_delete::	equ	10

keycode_cshift_space::	equ	11	; caps shift + space        = Break
keycode_cshift_sshift::	equ	12	; caps shift + symbol shift = Extended Mode

keycode_return::	equ	13
poundsterling::		equ	96

; Unshifted Keys:

	defb	0			; => Übersetzung 0 -> 0
keys:	defb	keycode_cshift
	defm	"zxcv", "asdfg"
	defm	"qwert", "12345"
	defm	"09876", "poiuy"
	defb	keycode_return
	defm	"lkjh", " "
	defb	keycode_sshift
	defm	"mnb"

; Key + Symbol Shift:

skeys:	defb	keycode_cshift
	defb	':', poundsterling, '?', '/'
	defm	"~|\{}", "qwe<>", "!@#$%"
	defm	"_)('&"
	defm	'";I]['
	defb	keycode_return
	defm	"=+-^", " "
	defb	keycode_sshift
	defm	".,*"

; Key + Caps Shift:

ckeys:	defb	keycode_cshift
	defm	"ZXCV", "ASDFG", "QWERT"
	defb	keycode_edit, keycode_capslock, keycode_video, keycode_rvideo, keycode_left
	defb	keycode_delete, keycode_graphics, keycode_right, keycode_up, keycode_down
	defm	"POIUY"
	defb	keycode_return
	defm	"LKJH"
	defb	keycode_cshift_space
	defb	keycode_cshift_sshift
	defm	"MNB"

keys_end:



.area SLOW_DATA ; may be in slow ram
; --------------------------------------------------------
; Bis zu drei gleichzeitig gedrückte Tasten werden sinnvoll behandelt:
;
; - keine Taste					Trivial
;
; - eine Shifttaste				wird nicht gepostet
; - eine normale Taste				wird gepostet
;
; - beide Shifttasten				=> normale Taste "Symbolshift" + Caps-Shiftflag
; - eine normale Taste + eine Shifttaste	=> normale Taste + entsprechendes Shiftflag
; - zwei normale Tasten				Ann.: Überlappung von Tastenaschlägen bei schnellem Tippen: 
;						Die die bisher gedrückte Taste wird als gelöst betrachtet
;						und die neue gepostet
; - zwei normale Tasten + eine Shifttaste	dito, nur mit zus. Shiftflag
; - eine normale Taste + beide Shifttasten	dito, "Symbolshift" wird als normale Taste behandelt


; Puffer für bis zu 2 Zeichen zur Übergabe der Tastenscancodes an die Applikation:

key_inkey::	defs	2	; nokey (0) oder Scancode [1..40] + sshift ($40) + cshift ($80)

; Eine Zelle für die beim letzten Scannen erkannte Taste:
;
; Shifttasten werden als Flags im Tastencode gespeichert und für die gesamte Dauer der Taste 
; (Autorepeats) so angewandt, wie sie zum Zeitpunkt des Drückens der Taste aktiv waren.

key_oldkey	defs	1	; 0 = no Key; sonst: Keycode [1..40] + Shift-Flags
nokey		equ	0	; Zelle leer
cshift_scancode	equ	1	; bei range [1..40]
sshift_scancode	equ	37	; bei range [1..40]
sshift_mask	equ	$40	; Symbol Shift war zum Drückzeitpunkt aktiv
cshift_mask	equ	$80	; Caps Shift war zum Drückzeitpunkt aktiv
sshift_bit	equ	6
cshift_bit	equ	7

; Timer for debounce and auto repeat:
;
; startdelay + guardtime .. startdelay	=> Entprellschutz
; startdelay  .. 0			=> Wartezeit bis zum ersten Autorepeat
; repeatdelay .. 0			=> Wartezeit bis zu weiteren Autorepeats

key_timer	defs	1	; Countdown für Guardtime und bis Autorepeat 
key_guardtime	equ	2	; 2/50 sec.  Entprellschutzzeit
key_startdelay	equ	15	; 15/50 sec. Delay bis zum ersten Autorepeat
key_repeatdelay	equ	4	; 4/50 sec.  Delay bis zu weiteren Repeats



;.area GSINIT
; --------------------------------------------------------
; Initialization: No Action
; Note: Data area is assumed to be cleared with 0 after start-up.

; 	nop



.area SLOW_CODE ; may be in slow ram
; --------------------------------------------------------
; purge inkey buffer and wait until no key is pressed
;
; in:  --
; out: --
; mod: af

flush_inkey::
	ld	a,(key_oldkey)		; _zuerst_ key_oldkey lesen
	push	hl
	ld	hl,0
	ld	(key_inkey),hl		; _dann_ key_inkey löschen
	pop	hl
	and	a
	ret	z
	halt
	jr	flush_inkey



.area SLOW_CODE ; may be in slow ram
; --------------------------------------------------------
; Flush Input und warte auf eine neue Taste
;
; Der Tastaturpuffer wird geleert und es wird gewartet,
; bis eine evtl. noch gedrückte Taste gelöst wird.
; Erst danach wird auf eine neue Taste gewartet.
;
; Diese Routine sollte benutzt werden, wenn das Programm 
; überraschende Eingaben erwartet, z.B. nach "Scroll?"
;
; in:	--
; out: a: Zeichen aus Tastaturpuffer 
; mod:	af

wait_newkey::
	call	flush_inkey
	;jr	wait_inkey

; --------------------------------------------------------
; Lese Zeichen aus Tastaturpuffer:
; Wartet bis Zeichen verfügbar.
; Ansonsten wie "get_inkey".
;
; in:  --
; out: a: Zeichen aus Tastaturpuffer 
; mod: af

wait_inkey::
	call	get_inkey
	ret	nz		; -> ret nz & a!=0
	halt
	jr	wait_inkey



.area CODE
; --------------------------------------------------------
; Lese Zeichen aus Tastaturpuffer:
;
; Es wird entweder 0 (nokey), ein druckbares Zeichen oder ein Controlcode zurückgeliefert.
; Controlcodes sind wie oben definiert.
; Druckbare Zeichen außer "poundsterling" sind alle 7-Bit-Ascii.
;
; in:  --
; out: a: Zeichen aus Tastaturpuffer oder 0 (nokey)
;      f: z-Flag zeigt an, ob a=0 (nokey) oder a=Zeichencode
; mod: af

get_inkey::
	push	hl

; hole Tastencode
	ld	hl,(key_inkey)
	ld	a,l
	ld	l,h
	ld	h,0
	ld	(key_inkey),hl

; Konvertiere Tastencode in Zeichencode:
	ld	hl,keys -1
	cp	$41		; unshifted?
	jr	c,1$		; ja
	sub	24		; sshift_mask - 40
	cp	$81-24		; mit symbol shift?
	jr	c,1$		; ja
	sub	24		; sonst mit caps shift
	
1$:	add	l		; hl += a
	ld	l,a
	jr	nc,2$
	inc	h
2$:	ld	a,(hl)		; hole Zeichencode

	pop	hl
	and	a		; -> ret z / ret nz
	ret	


; ------------------------------------------------
; query input available 
; out:	f: z -> avail = 0, nz -> avail ≥ 1
; mod: 	af

get_avail_in::
	ld	a,(key_inkey)
	and	a
	ret	



.area SLOW_CODE ; may be in slow ram
; ------------------------------------------------
; Interruptroutine: Scan Keyboard
; in:  --
; out: --
; mod: af, bc, de, hl

irpt_scan_keyboard::

; Prüfe, ob die Guardtime für die zuletzt erkannte Taste noch läuft:
; In dieser Zeit, kann die (existierende!) Taste nicht gelöscht werden
; und logischerweise auch noch keine Autorepeats erzeugen.
; Das gilt für eine normale Taste und max. ein Shiftbit im Tastencode.
; Nur wenn key_oldkey nur aus einem Shiftbit ohne eine normale Taste besteht,
; kann das andere Shiftbit (-> Extended Mode) oder eine normale Taste dazu kommen,
; wobei dann ein Tastenevent nach key_inkey geschrieben werden muss.

	ld	hl,key_timer		; hl -> key_timer
	ld	a,key_startdelay
	cp	(hl)
	jr	nc,1$			; timer ≤ startdelay => guardtime abgelaufen 

; Guardtime ist noch aktiv:
; => prüfe, ob sie nur für ein Shiftbit gilt:
	ld	a,(key_oldkey)		; zugehörige Taste
	ld	d,a			; d = Shiftflag (Ann.: alte Taste = einzelne Shifttaste)
	and	$3f			; w/o Shiftflags
	jr	z,2$			; nokey -> Guardtime nur für einzelne Shifttaste

; Guardtime noch aktiv oder key_inkey nicht frei.
; => Tastatur nicht scannen, weil Änderungen nicht gespeichert werden können.
;    Timer runterzählen aber nicht bis 0. (Autorepeat verzögern)

0$:	;ld	hl,key_timer
	dec	(hl)			; key_timer--
	ret	nz			
	inc	(hl)			; delay autorepeat
	ret

; Guardtime abgelaufen ((1$)) oder
; Guardtime gilt nur für ein einzelnes Shiftbit ((2$))
; => Prüfe, ob key_inkey frei ist:
;    Solange der Tastenpuffer nicht frei ist, ist das Scannen der Tastatur nutzlos,
;    da keine neue Taste oder Autorepeat gespeichert werden. => Exit.

1$:	ld	d,0			; d = neue Tasten = nokey;    2$: d = Shiftbit
2$:	ld	a,(key_inkey+1)
	and	a
	jr	nz,0$			; Tastenpuffer ist belegt! => Exit

; Guardtime für alte Taste ist nicht mehr aktiv oder gilt nur für ein enzelnes Shiftbit:
; => Eine neue Taste kann auch gespeichert werden. 
; => Scan Keyboard.

	;ld	d,d			; d = neue Taste + Shiftflags
	ld	e,0			; e = keyboard row number [0..7]
	ld	bc,$fefe		; bc = IO address incl. key row sub address in B
	jr	3$

; Schleife über alle Tastenzeilen:
4$:	inc	e			; e = row number ++
	rlc	b			; b = next IO key row sub address
	jr	nc,11$			; fertig

3$:	in	a,(c)			; a = new key bits
	cpl				; pressed keys := '1'
	and	$1f			; mask 5 valid keys
	jr	z,4$			; no pressed key(s) found 
		
; Tastenzeile enthält gedrückte Tasten:
; f = nc, a = key code, bc=IO address, d = current keys, e = row number, hl = --
	ld	h,a			; h = key bits
	ld	a,e			; row number
	add	a
	add	a
	add	e			; *5
	ld	l,a			; l = current key code

; Schleife über Tasten (Bits):	
; a = --, f = nc, bc = io address, d = current keys, e = row number, h = key bits, l = current key code
5$:	inc	l			; next key code
	;and	a			; clear c-flag
	rr	h			; key bits
	jr	nc,5$			; key not pressed -> try next bit

; Gedrückte Taste gefunden:
; a = --, bc = io address, d = current keys, e = row number, h = key bits, l = new key code
; auf Shifttaste prüfen und ggf. in Shiftflag umwandeln:
	ld	a,l			; new key code
	cp	cshift_scancode		; caps shift key?
	jr	nz,7$			; no	
	set	cshift_bit,d		; d += cshift
	jr	6$				
7$:	cp	sshift_scancode		; symbol shift key?
	jr	nz,8$			; no -> normale Taste in l
	set	sshift_bit,d		; d += sshift

; Neue Taste war eine Shifttaste.
; Auf beide Shifttasten prüfen und ggf. durch legalen Code ersetzen:
6$:	ld	a,d
	cp	$c0			; jetzt beide Shifttasten?
	jr	c,9$			; no -> ok -> loop

; Jetzt sind beide Shifttasten gedrückt.
; -> Current key in d durch legalen Code ersetzen 
;    auf 2 Shifttasten + normale Taste testen
	res	sshift_bit,d		; d = alte normale Taste (if any) + Caps Shift Bit
	ld	l,sshift_scancode	; l = neue normale Taste "Symbol Shift"
	;jr	8$			

; normale Taste l gefunden:
8$:	ld	a,d			; a = new key
	and	$3f			; w/o shift flag
	jr	z,10$			; es gibt noch keine andere 'normale Taste' 

; 2 normale Tasten (d und l) gefunden:
; Ann.: Durch schnelles Tippen können kurzzeitig zwei Tasten gedrückt sein.
; -> (1) Taste d ist alt und l ist neu => d vergessen, l triggern
;    (2) Taste l ist alt und d ist neu => l vergessen, d triggern
;    (3) beide Tasten sind neu         => Ghostkey (oder abusive usage)
;
; (1), (2) Die Guardtime verhindert, dass diese Stellung beim nächsten Interrupt sofort
; erneut umschaltet, und so mit 50 Hz Tastenevents für diese Tasten erzeugt würden.
; Die Guardtime für diese neue Taste sollte ausreichen, dass die alte Taste tatsächlich 
; hochgeht und nach Ablauf der Guardtime tatsächlich nicht mehr unten ist. 
; Wenn aber jemand zwei Tasten gedrückt hält, kommt es jeweils nach Ablauf der Guardtime 
; zu einem alternierenden Triggern dieser beiden Tasten.
;
; (3) Schnelles Tippen bei konstant gedrückter Shifttaste kann zu 3 Tasten plus Ghostkey führen.
; Wenn Tasten d und l beide neu sind ist eine davon wahrscheinlich einen Ghostkey
; und es gibt eine alte Taste und es ist eine Shifttaste gedrückt. (sonst: abusive usage => egal)
; Da nicht entschieden werden kann, welche der beiden neuen Tasten nur ein Ghostkey ist,
; bleibt nichts weiter übrig, als zu warten, dass die alte Taste hochgeht und der 
; Ghostkey somit verschwindet. => keine neue Taste und kein Autorepeat posten, Warten => Exit

	ld	a,(key_oldkey)		; alte Taste
	and	$3f			; ohne Shiftbits
	cp	l
	jr	z,9$			; Taste l ist die alte und geht hoch -> d bleibt d -> loop

	xor	d
	and	$3f
	ret	nz			; Taste d und l sind neu => Ghostkey!
	
; Taste d ist die alte und geht hoch
	ld	a,d			; a = ältere Taste incl. Shiftbits
	and	$3f			; a = ältere Taste 

; 1. normale Taste gefunden:
10$:	xor	d			; a = Shiftbits
	or	l			; a = neue Taste incl. Shiftbits
	ld	d,a			; d = neue Taste incl. Shiftbits: fertig
	;jr	9$			; und weiter nach Tasten suchen

; Taste abgehandelt, nach weiteren Tasten suchen:
9$:	ld	a,h			; remaining key bits
	and	a			; Test for z and clear cy
	jr	z,4$			; keine Tasten mehr drin => nächste Zeile
	jr	5$			; weitere Tasten drin => weiter suchen
	
; Keyboardmatrix ist gescannt.
; d = new key = Nokey|Tastencode + Noshift|SShift|CShift
;     old key = Nokey|Tastencode + Noshift|SShift|CShift

11$:	ld	hl,key_oldkey		; hl -> key_oldkey
	ld	a,d			; new key
	and	$3f
	jr	nz,15$			; es ist eine normale Taste gedrückt

; Es ist keine Taste oder nur eine Shifttaste gedrückt:
; => kein neues Tastenevent oder Autorepeat 
;
; Guardtime noch gültig?
;	(old key == new key == Shifttaste)
;	timer--
; Guardtime abgelaufen?
;	new key == nokey?
;		 old key := new key = d = nokey
;		 timer egal
;	new key == shift key?
;		(es könnte jetzt auch die andere Shifttaste sein.)
;		old key == nokey?
;			old key := new key = d = shiftkey
;			timer := startdelay + guardtime
;		old key != nokey?
;			old key := new key = d = shiftkey
;			timer-- aber nicht auf 0

	xor	a,(hl)		; a = old key & set flags
	ld	(hl),d		; old key := new key
	inc	hl		; hl -> key_timer
	jr	nz,0$		; old key != nokey => old key == shiftkey => new key egal => timer-- & exit
	xor	d		; a = new key & set flags
	ret	z		; old key == nokey && new key == nokey => timer egal & exit

; old key == nokey && new key != nokey 
; => shiftkey goes down:
	ld	(hl),key_startdelay + key_guardtime
	ret

; Es ist eine normale Taste und evtl. eine Shifttaste in d gedrückt:
;
; neue Taste ohne Shiftbits != alte Taste ohne Shiftbits?
;	ignore whether old key is still down
;	neue Taste posten
;	old key := new key
;	timer := guardtime
; neue Taste ohne Shiftbits == alte Taste ohne Shiftbits?
;	old key untouched (ignore change of shift keys!)
;	key_inkey[0] leer?				
;		timer--
;		Timer abgelaufen?
;			Autorepeat posten
;			timer := repeatdelay
;	key_inkey[0] nicht leer?			
;		timer-- aber nicht bis 0		

15$:	ld	a,(hl)			; old key
	xor	d			; new key
	and	$3f			; w/o shiftbits
	jr	nz,13$			; Haupttaste änderte sich

; Haupttaste blieb gleich:
	inc	hl			; hl-> key_timer
	dec	(hl)			; timer--
	ret	nz			; repeatdelay noch nicht abgelaufen => fertig!
; Autorepeat, außer wenn inkey buffer nicht ganz leer:
	ld	a,(key_inkey)		; key_inkey[0]
	and	a
	jr	z,12$			; inkey buffer leer
	inc	(hl)			; noch eine Taste im inkey buffer => delay autorepeat!
	ret
; Autorepeat
12$:	ld	(hl),key_repeatdelay	; timer := repeat delay
	dec	hl			; hl -> key_oldkey
	ld	d,(hl)			; d = old key with with old shift flags 
	jr	14$			; post it & exit
	
; Haupttaste änderte sich => neue Haupttaste
13$:	ld	(hl),d			; old key := new key incl. new shift flags
	inc	hl			; hl -> key_timer
	ld	(hl),key_startdelay + key_guardtime
	;jr	14$			; post it & exit	

; post key in d:
14$:	ld	hl,key_inkey		; hl -> key_inkey[0]
	ld	a,(hl)
	and	a
	jr	z,$+3			; key_inkey[0] ist frei; sonst
	inc	hl			; hl -> key_inkey[1]
	ld	(hl),d			; post new key
	ret




	

