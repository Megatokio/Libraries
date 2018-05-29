#!/usr/local/bin/vipsi

var Sources = { "UnicodeData.txt" }
var Targets = { "CanonicalCombiningClass.h", "CanonicalCombiningClass_UCS4.h" }

var r = include( env._[to rfind(env._,"/")] # "Tools.vh" ); if r end r then
ParseArguments()
QuickExitTest()


// =======================================================================

	var UnicodeData = ReadFile( fn_Sources#"UnicodeData.txt", 15 )
	var UnicodeDataNames = { "Code Point","Name","General Category","Canonical Combining Class","Bidi Class",
		"Decomposition Type And Mapping","Decimal Digit Value","Digit Value","Numeric Value","Bidi Mirrored",
		"Old Name","ISO Comment","Simple Uppercase Mapping","Simple Lowercase Mapping","Simple Titlecase Mapping" }


	UnicodeData = EvalHexColumn( UnicodeData, 1 )		// Code_Point

	if UnicodeData[512,1] != $01FF	log nl,"die ersten 512 Code Points nicht dicht besetzt!?!",nl; end 1; then

	var i = count UnicodeData do while UnicodeData[i--,1]!=$10000 loop
	var end_ucs2 = i						if verbose put nl,"  UCS-2:   ", end_ucs2	then
	var end_ucs4 = count UnicodeData		if verbose put nl,"  UCS-4:   ", end_ucs4-end_ucs2	then


// =======================================================================
// Canonical Combining Class


// collapse collections:
	proc Collapse ( QA, d, dflt_val )
	{		
		var Leer[count QA[1]]	// {0,0} oder {0,0,0}
		var i=1; do while ++i <= count Leer Leer[i] = dflt_val loop
		var qi=1, zi=1			
		var ZA[zi] = QA[qi] 
		do
			while ++qi <= count QA
			if QA[qi-1,1]+d!=QA[qi,1] &&				// Nummerierungs-Lücke?
			   ZA[zi,2 to]!=Leer[2 to]					// Letzter Code hatte non-defaults
				Leer[1]=QA[qi-1,1]+d ZA ##= {Leer} zi++	// => Leereintrag mit defaults
			then			
			if QA[qi,2 to]!=ZA[zi,2 to]					// Werte ändern sich
				ZA ##= { QA[qi] } zi++					// => Start eines neuen Bereiches 
			then			
		loop
		if ZA[zi,2 to]!=Leer[2 to]						// Letzter Code hatte non-defaults
			Leer[1]=QA[qi-1,1]+d ZA ##= {Leer} zi++		// => Leereintrag mit defaults
		then
		return ZA
	}



// =======================================================================
// Canonical Combining Class

	var ccc2={}, ccc4={}
	var i=0
	do while ++i <= end_ucs2; ccc2 ##= {{UnicodeData[i,1],UnicodeData[i,4]}}; loop i--	
	do while ++i <= end_ucs4; ccc4 ##= {{UnicodeData[i,1],UnicodeData[i,4]}}; loop		

	ccc2 = Collapse(ccc2,1,"0")
	ccc4 = Collapse(ccc4,1,"0")


SetGlobalConstant( "U_CCC_UCS2",	   count ccc2,									"size of Canonical Combining Class Table ccc2" ) 
SetGlobalConstant( "U_CCC_UCS2_START", Hex4Str(ccc2[1,2]!="0"?ccc2[1,1]:ccc2[2,1]), "first Code Point of Interest in ccc2 table" ) 
SetGlobalConstant( "U_CCC_UCS2_END",   Hex4Str(ccc2[count ccc2,1]),                 "last (excl.) Code Point of Interest in ccc2 table" ) 

SetGlobalConstant( "U_CCC_UCS4",	   count ccc4,									"size of Canonical Combining Class Table ccc4" ) 
SetGlobalConstant( "U_CCC_UCS4_START", Hex6Str(ccc4[1,2]!="0"?ccc4[1,1]:ccc4[2,1]), "first Code Point of Interest in ccc4 table" ) 
SetGlobalConstant( "U_CCC_UCS4_END",   Hex6Str(ccc4[count ccc4,1]),					"last (excl.) Code Point of Interest in ccc4 table" ) 

SetGlobalConstant( "U_ccc_defaultvalue", "U_ccc_0",		  "Canonical Combining Class default Property Value" ) 



var Header = «
/*	generated on   »#datestr(now)#« UTC+02.00 (CET/CEST)
	          by   "»#fn_Script#«" 
	          from UnicodeData.txt

	          see  http://www.unicode.org/Public/»#Unicode_Version#«/ucd/UnicodeData.txt
	               http://www.unicode.org/Public/»#Unicode_Version#«/ucd/UCD.html#Canonical_Combining_Class

	map Code Point ranges -> Canonical Combining Class (ccc, enumerated Property)
*/»


var F=5
openout #F, fn_Includes # "CanonicalCombiningClass.h"
put #F, Header
WriteTable( F,1,count ccc2,proc(i){return{Hex4Str(ccc2[i,1]),"U_ccc_"#ccc2[i,2]}} )
close #F

openout #F, fn_Includes # "CanonicalCombiningClassUCS4.h"
put #F, Header
WriteTable( F,1,count ccc4,proc(i){return{Hex6Str(ccc4[i,1]),"U_ccc_"#ccc4[i,2]}} )
close #F









// =================================================

put " ... ok", nl
end 0











