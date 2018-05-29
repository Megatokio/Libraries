#!/usr/local/bin/vipsi

var Sources = { "UnicodeData.txt" }
var Targets = { "NumericValue.h", "NumericValue_UCS4.h" }

var r = include( env._[to rfind(env._,"/")] # "Tools.vh" ); if r end r then
ParseArguments()
QuickExitTest()


// =======================================================================

	var UnicodeData = ReadFile( fn_Sources#"UnicodeData.txt", 15 )
	var UnicodeDataNames = { "Code Point","Name","General Category","Canonical Combining Class","Bidi Class",
		"Decomposition Type And Mapping","Decimal Digit Value","Digit Value","Numeric Value","Bidi Mirrored",
		"Old Name","ISO Comment","Simple Uppercase Mapping","Simple Lowercase Mapping","Simple Titlecase Mapping" }


	var N = {"nd","nl","no"}	// "Decimal Number", "Letter Number", "Other Number"
	var Number = {}
	var i=0
	do
		while ++i <= count UnicodeData
		var gc = lowerstr(UnicodeData[i,3])
		if !find(N,gc) del UnicodeData[i--] next then		// non-number
		
	//	put nl, UnicodeData[i,1], ": ", gc, ": 7/8/9 = ", UnicodeData[i,7], ",", UnicodeData[i,8], ",", UnicodeData[i,9]

		Number ##= {{ Code=eval("$"#UnicodeData[i,1]), GC=gc, Count=1, /* UnicodeData[i,8], */ Value=UnicodeData[i,9] }}
						// Note: Digit-Value ist für manchen Number-Others angegeben.
						// Das sind z.B. hochgestellte Ziffern, Zahlen im Kreis oder Ziffernfolgen ohne Null.
						// Eine Benutzung als Ziffer erscheint mir suspekt. 
						// => Die zus. Info durch Digit-Value ignoriere ich erst mal.

		if gc=="nd"														// Nd (Number-Digit) hat immer alle 3 gesetzt
			if UnicodeData[i,7]=="" log "buh1" end 1 then
			if UnicodeData[i,8]!=UnicodeData[i,7] log "buh1.1" end 1 then
			if UnicodeData[i,9]!=UnicodeData[i,7] log "buh1.2" end 1 then
		
		elif gc=="nl"														// Nl (Number-Letter) hat immer nur Numeric-Value gesetzt
			if UnicodeData[i,7]!="" log "buh2" end 1 then
			if UnicodeData[i,8]!="" log "buh3" end 1 then
			if UnicodeData[i,9]=="" log " no value " then	// 1x Ohne!

		elif gc=="no"														// No hat manchmal Digit-Value, aber immer Num-Val gesetzt
			if UnicodeData[i,7]!="" log "buh4" end 1 then
			if UnicodeData[i,8]!="" && UnicodeData[i,8]!=UnicodeData[i,9] log "buh5" end 1 then
			if UnicodeData[i,9]=="" log " no value " then	// 1x Ohne!
		then
	loop
	
	del UnicodeData

	if verbose
		put nl, "Numerische Zeichen: ", count Number
	then
	

	//  [0..9]
	//	[1..9] x [10 .. 10000]
	//	[1..9] / [1..9]
	//
	// N = B x 10^E / D		=		%BBBB.BBBB.EEEE.DDDD
	//				Range	=>	N =	N(RangeStartCodePoint) + %0000.0001.0000.0000 * (MyCodePoint-RangeStartCodePoint)
	//
	// Value auswerten und in B, E und D splitten

	var i=0
	do
		while ++i <= count Number
		var z = Number[i].Value
		if find(z,"/")
			Number[i].Value = { B=eval(z[to find(z,"/")-1]), E=0, D=eval(z[find(z,"/")+1 to]) }
		elif z==""
			Number[i].Value = { B=0, E=0, D=0/*NaN*/ }
		else
			z = { B=eval z, E=0, D=1 }
			if z.B do until z.B%10 z.B/=10 z.E++ loop then
			Number[i].Value = z
		then
	loop

	// Ranges erkennen und zusammenziehen
	
	var i=0
	do
		while ++i <= count Number		
		
		var vi = Number[i].Value
		if vi.D!=1 next then
		
		var j=i+1
		var ci = Number[i].Code
		var cj = ci +1
		do
			while j <= count Number
			while Number[j].Code == cj
			while Number[j].GC == Number[i].GC

			var vj = Number[j].Value
			while vi.D==vj.D
			
			if vj.E == vi.E && vj.B - vi.B == cj - ci
				Number[i].Count += 1
			//	put "<E,B>"
				del Number[j] cj++
				next
			elif vj.E == vi.E-1 && vj.B - vi.B*10 == cj - ci && Number[i].Count==1
				vi.E--; vi.B*=10 
				Number[i].Value = vi
				Number[i].Count += 1
			//	put "<E-1,B*10>"
				del Number[j] cj++
				next
			elif vj.E == vi.E+1 && vj.B*10 - vi.B == cj - ci
				Number[i].Count += 1
			//	put "<E+1,B/10>"
				del Number[j] cj++
				next
			else
				exit
			then
		loop
	loop

	if verbose
		put nl, "Numerische Blocks: ", count Number
	then


	proc Find(a,e,n) { do i=int((a+e)/2); n<Number[i].Code?e:a = i while a<e-1 loop return a }
	var RangesUCS1 = Find(1,count Number,$FF)
	var RangesUCS2 = Find(1,count Number,$FFFF)
	var RangesUCS4 = count Number - RangesUCS2
	SetGlobalConstant("U_NUMERIC_UCS1", RangesUCS1, "sizeof ucs1 numeric values subtable")
	SetGlobalConstant("U_NUMERIC_UCS2", RangesUCS2, "sizeof ucs2 numeric values table")
	SetGlobalConstant("U_NUMERIC_UCS4", RangesUCS4, "sizeof ucs4 numeric values table")


// =================================================


var F=5
openout#F, fn_Targets # "NumericValue.h"
put#F,	"\n// Code Point UCS2 -> Numeric Value",
		"\n// struct { UCS2Char StartCode, uchar GeneralCategory, uchar Count, signed short CodedValue };",
		"\n// CodedValue = %BBBB.BBBB.EEEE.DDDD",
		"\n// -> Value = BBBBBBBB * 10eEEEE / DDDD",
		"\n// CodedValue == 0 -> NaN (kommt 2x vor)",
		"\n// Count>1 -> Value[i] = (BBBBBBBB+i) * 10eEEEE / DDDD"

WriteTable( F, 1, RangesUCS2, proc(i){var z=Number[i] return{Hex4Str(z.Code),"U_gc_"#z.GC,string(z.Count),
										 "0x"#hexstr($100+z.Value.B,2)#hexstr(z.Value.E)#hexstr(z.Value.D)}} )
close#F

openout#F, fn_Targets # "NumericValue_UCS4.h"
put#F,	"// Code Point UCS4 -> Numeric Value"

WriteTable( F, RangesUCS2+1, count Number, proc(i){var z=Number[i] return{Hex6Str(z.Code),"U_gc_"#z.GC,string(z.Count),
													"0x"#hexstr($100+z.Value.B,2)#hexstr(z.Value.E)#hexstr(z.Value.D)}} )
close#F





// =================================================

put " ... ok", nl
end 0











