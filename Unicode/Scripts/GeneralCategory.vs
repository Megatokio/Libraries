#!/usr/local/bin/vipsi

var Sources = { "UnicodeData.txt" }
var Targets = { "GeneralCategory.h" }

var r = include( env._[to rfind(env._,"/")] # "Tools.vh" ); if r end r then
ParseArguments()
QuickExitTest()


// =======================================================================
// General Category

	var UnicodeData = ReadFile( fn_Sources#"UnicodeData.txt", 15 )
		UnicodeData = EvalHexColumn( UnicodeData, 1 )

	var UnicodeDataNames = { "Code Point","Name","General Category","Canonical Combining Class","Bidi Class",
		"Decomposition Type And Mapping","Decimal Digit Value","Digit Value","Numeric Value","Bidi Mirrored",
		"Old Name","ISO Comment","Simple Uppercase Mapping","Simple Lowercase Mapping","Simple Titlecase Mapping" }


	if UnicodeData[512,1] != $01FF	log nl,"die ersten 512 Code Points nicht dicht besetzt!?!",nl; end 1; then

	proc Find(a,e,n) { do i=int((a+e)/2); n<UnicodeData[i,1]?e:a = i while a<e-1 loop return a }

	var end_ucs2 = Find(1,count UnicodeData,$ffff)	if verbose put nl,"  UCS-2:   ", end_ucs2	then
	var end_ucs4 = count UnicodeData				if verbose put nl,"  UCS-4:   ", end_ucs4-end_ucs2	then


// collapse collections:
	proc Collapse ( QA, dflt_val )
	{		
		var Leer[count QA[1]]	// {0,0} oder {0,0,0}
		var i=1; do while ++i <= count Leer Leer[i] = dflt_val loop
		var qi=1, zi=1			
		var ZA[zi] = QA[qi] 
		do
			while ++qi <= count QA
			if QA[qi-1,1]+1!=QA[qi,1] &&				// Nummerierungs-Lücke?
			   ZA[zi,2 to]!=Leer[2 to]					// Letzter Code hatte non-defaults
				Leer[1]=QA[qi-1,1]+1 ZA ##= {Leer} zi++	// => Leereintrag mit defaults
			then			
			if QA[qi,2 to]!=ZA[zi,2 to]					// Werte ändern sich
				ZA ##= { QA[qi] } zi++					// => Start eines neuen Bereiches 
			then			
		loop
		if ZA[zi,2 to]!=Leer[2 to]						// Letzter Code hatte non-defaults
			Leer[1]=QA[qi-1,1]+1; ZA ##= {Leer} zi++	// => Leereintrag mit defaults
		then
		return ZA
	}

	var TA = Collapse( { {1,8},{2,8},{3,8} }, 8 )	if(TA!={{1,8}})			log "1:",TA end 1 then
	var TA = Collapse( { {1,8},{2,8},{3,8} }, 9 )	if(TA!={{1,8},{4,9}})	log "2:",TA end 1 then
	var TA = Collapse( { {1,8},      {3,8} }, 9 )	if(TA!={{1,8},{2,9},{3,8},{4,9}})	log "3:",TA end 1 then
	var TA = Collapse( { {1,8},{2,9},{4,8} }, 9 )	if(TA!={{1,8},{2,9},{4,8},{5,9}})	log "4:",TA end 1 then
	var TA = Collapse( { {1,8},      {3,8} }, 8 )	if(TA!={{1,8}})			log "5:",TA end 1 then


	var gc2={}, gc4={}
	var i=0
	do while ++i <= end_ucs2; gc2 ##= {{UnicodeData[i,1],UnicodeData[i,3]}}; loop i--	
	do while ++i <= end_ucs4; gc4 ##= {{UnicodeData[i,1],UnicodeData[i,3]}}; loop		

	var i=1
	do while++i<count gc2 
		if gc2[i,1]+1==gc2[i+1,1]	// wenn keine Lücke folgt
		&& gc2[i,2]!=gc2[i-1,2]		// und die aktuelle Kategorie nun wechselt
			if   gc2[i,2]=="Ll"							// für Ll:
				var LL = gc2[i,1]&1 ? "LueLlo" : "LuoLle"
				if gc2[i+1,2]=="Lu" || gc2[i-1,2]==LL	// wenn Lu folgt oder passendes LloLue:LuoLle vorausging
					gc2[i,2] = LL		
				then
			elif gc2[i,2]=="Lu"							// für Lu:
				var LL = gc2[i,1]&1 ? "LuoLle" : "LueLlo"
				if gc2[i+1,2]=="Ll" || gc2[i-1,2]==LL	// wenn Ll folgt oder passendes LloLue:LuoLle vorausging
					gc2[i,2] = LL		
				then
			then
		then
	loop

	gc2 = Collapse(gc2,"Cn")
	gc4 = Collapse(gc4,"Cn")

proc Find2(a,e,n) { do i=int((a+e)/2); n<gc2[i,1]?e:a = i while a<e-1 loop return a }
proc Find4(a,e,n) { do i=int((a+e)/2); n<gc4[i,1]?e:a = i while a<e-1 loop return a }

// Unvollständig enumerierte Blocks auffüllen
	var i = Find2(1,count gc2, $3400)				// First CJK Ideograph Extension A
		if gc2[i+2,1]!=$4db5 log "murks$4db5" end 1 then
		del gc2[i+1]		// den eingeschobenen "unasigned"-Block entfernen
		del gc2[i+1]		// den End-Eintrag entfernen
	var i = Find2(1,count gc2, $4e00)				// First CJK Ideograph 
		if gc2[i+2,1]!=$9fbb log "murks$9fbb" end 1 then
		del gc2[i+1]		// den eingeschobenen "unasigned"-Block entfernen
		del gc2[i+1]		// den End-Eintrag entfernen
	var i = Find2(1,count gc2, $ac00)				// First Hangul Syllable
		if gc2[i+2,1]!=$d7a3 log "murks$ac00" end 1 then
		del gc2[i+1]		// den eingeschobenen "unasigned"-Block entfernen
		del gc2[i+1]		// den End-Eintrag entfernen
	var i = Find2(1,count gc2, $D800)				// First Non private use high surrogate
		if gc2[i+2,1]!=$db7f log "murks$D800" end 1 then
		del gc2[i+1]		// den eingeschobenen "unasigned"-Block entfernen
		del gc2[i+1]		// den End-Eintrag entfernen
	var i = Find2(1,count gc2, $Db80)				// First Private use high surrogate
		if gc2[i+2,1]!=$dbff log "murks$Db80" end 1 then
		del gc2[i+1]		// den eingeschobenen "unasigned"-Block entfernen
		del gc2[i+1]		// den End-Eintrag entfernen
	var i = Find2(1,count gc2, $dc00)				// First low surrogate
		if gc2[i+2,1]!=$dfff log "murks$dc00" end 1 then
		del gc2[i+1]		// den eingeschobenen "unasigned"-Block entfernen
		del gc2[i+1]		// den End-Eintrag entfernen
	var i = Find2(1,count gc2, $e000)				// First Private use
		if gc2[i+2,1]!=$f8ff log "murks$e000" end 1 then
		del gc2[i+1]		// den eingeschobenen "unasigned"-Block entfernen
		del gc2[i+1]		// den End-Eintrag entfernen
	
	var i = Find4(1,count gc4, $20000)				// First CJK ideographic extension B
		if gc4[i+2,1]!=$2a6d6 log "murks$20000" end 1 then
		del gc4[i+1]		// den eingeschobenen "unasigned"-Block entfernen
		del gc4[i+1]		// den End-Eintrag entfernen
	var i = Find4(1,count gc4, $f0000)				// First plane 15 private use
		if gc4[i+2,1]!=$ffffd log "murks$f0000" end 1 then
		del gc4[i+1]		// den eingeschobenen "unasigned"-Block entfernen
		del gc4[i+1]		// den End-Eintrag entfernen
	var i = Find4(1,count gc4, $100000)				// First plane 16 private use
		if gc4[i+2,1]!=$10fffd log "murks$100000" end 1 then
		del gc4[i+1]		// den eingeschobenen "unasigned"-Block entfernen
		del gc4[i+1]		// den End-Eintrag entfernen



var n1 = Find2(1,count gc2+1,$ff)	

SetGlobalConstant( "U_GC_UCS1",			n1,							"sizeof ucs1->general category subtable" ) 
SetGlobalConstant( "U_GC_UCS2",			count gc2,					"sizeof ucs2->general category table" )
SetGlobalConstant( "U_GC_UCS2_START",	Hex4Str(gc2[1,1]),			"first Code Point of Interest in gc table" ) 
SetGlobalConstant( "U_GC_UCS2_END",		Hex4Str(gc2[count gc2,1]),	"last (excl.) Code Point of Interest in gc table" ) 
SetGlobalConstant( "U_GC_UCS4",			count gc4,					"sizeof ucs4->general category table" ) 
SetGlobalConstant( "U_GC_UCS4_START",	Hex6Str(gc4[1,1]),			"first Code Point of Interest in ucs4 gc table" ) 
SetGlobalConstant( "U_GC_UCS4_END",		Hex6Str(gc4[count gc4,1]),	"last (excl.) Code Point of Interest in ucs4 gc table" ) 
SetGlobalConstant( "U_gc_defaultvalue",	"U_gc_cn",					"General Category default Property Value" ) 


var Header = «
/*	generated on   »#datestr(now)#« UTC+02.00 (CET/CEST)
	          by   "»#fn_Script#«" 
	          from UnicodeData.txt

	          see  http://www.unicode.org/Public/»#Unicode_Version#«/ucd/UnicodeData.txt
	               http://www.unicode.org/Public/»#Unicode_Version#«/ucd/UCD.html#General_Category_Values

	map Code Point ranges -> General Category property value (gc, Enumerated Property)
*/»


var F = 5
openout #F, fn_Includes # "GeneralCategory.h"
put #F, Header
put #F, «
#define U_gc_luello	-1		// Lu on even position, Ll on odd position
#define U_gc_luolle	-2		// Lu on odd position,  Lo on even position
»
WriteTable( F, 1, count gc2, proc(i){ return { Hex4Str(gc2[i,1]), "U_gc_"#lowerstr(gc2[i,2]) } } )
close #F

openout #F, fn_Includes # "GeneralCategory_UCS4.h"
put #F, Header
WriteTable( F, 1, count gc4, proc(i){ return { Hex6Str(gc4[i,1]), "U_gc_"#lowerstr(gc4[i,2]) } } )
close #F








// =================================================

put " ... ok", nl
end 0











