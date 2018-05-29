#!/usr/local/bin/vipsi

var Sources = { "UnicodeData.txt" }
var Targets = { "SimpleUppercase_UCS4.h", "SimpleTitlecase.h", "SimpleLowercase_Even.h", "SimpleUppercase_Even.h",
				"SimpleLowercase_Odd.h", "SimpleUppercase_Odd.h", "SimpleLowercase_UCS4.h" }

var r = include( env._[to rfind(env._,"/")] # "Tools.vh" ); if r end r then
ParseArguments()
QuickExitTest()


// =======================================================================
// Simple case conversion


	var UnicodeData = ReadFile( fn_Sources#"UnicodeData.txt", 15 )
		UnicodeData = EvalHexColumns( UnicodeData, 1,13,14,15 )			// Code_Point, UC, LC, TC

	var UnicodeDataNames = { "Code Point","Name","General Category","Canonical Combining Class","Bidi Class",
		"Decomposition Type And Mapping","Decimal Digit Value","Digit Value","Numeric Value","Bidi Mirrored",
		"Old Name","ISO Comment","Simple Uppercase Mapping","Simple Lowercase Mapping","Simple Titlecase Mapping" }

	if UnicodeData[512,1] != $01FF	log nl,"die ersten 512 Code Points nicht dicht besetzt!?!",nl; end 1; then




// =======================================================================
// create 'collections':

	var	A_lc = {}	// ucs4 range, lc 
	var	A_uc = {}	// ucs4 range, uc 
	var a_tc = {}	// ucs2 range, tc!=uc 
	var	a_ou = {}	// ucs2 range, odd  uc 
	var	a_ol = {}	// ucs2 range, odd  lc 
	var	a_eu = {}	// ucs2 range, even uc 
	var	a_el = {}	// ucs2 range, even lc 


	var i=0, c,uc,lc,tc
	do
		while ++i <= count UnicodeData
		c = UnicodeData[i,1]
		uc = UnicodeData[i,13]	
		lc = UnicodeData[i,14]	
		tc = UnicodeData[i,15]	

		if tc!=uc a_tc	##= {{ c,tc }}  then

		if uc uc -= c then
		if lc lc -= c then	

		if c<=$ffff  
			if c&1	a_ou ##= {{c,uc}} a_ol ##= {{c,lc}} 
			else	a_eu ##= {{c,uc}} a_el ##= {{c,lc}}  
			then
		else  
			A_uc ##= {{ c,uc }}  
			A_lc ##= {{ c,lc }}  
		then
	loop
	
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
	
	if a_tc[count a_tc,1]>$ffff log nl,"  ",hexstr(a_tc[count a_tc,1]),": titlecase != uppercase: we need a UCS4 map." end 1 then
	
	A_uc = Collapse(A_uc,1,0)	if verbose put nl,"  UCS4 UC     map size = ", count A_uc			then
	A_lc = Collapse(A_lc,1,0)	if verbose put nl,"  UCS4 LC     map size = ", count A_lc			then
								if verbose put nl,"  UCS2 TC!=UC map size = ", count a_tc			then
	a_ou = Collapse(a_ou,2,0)	if verbose put nl,"  UCS2 odd UC map size = ", count a_ou, " + "	then
	a_ol = Collapse(a_ol,2,0)	if verbose put nl,"  UCS2 odd LC map size = ", count a_ol, " + "	then
	a_eu = Collapse(a_eu,2,0)	if verbose put nl,"  UCS2 evenUC map size = ", count a_eu, " + "	then
	a_el = Collapse(a_el,2,0)	if verbose put nl,"  UCS2 evenLC map size = ", count a_el, " = ", 
																			(count a_ou+count a_ol+count a_eu+count a_el)*4, " Bytes" then


SetGlobalConstant( "U_SUC_UCS4",			count A_uc,								"sizeof simple uppercase table for UCS4" )
SetGlobalConstant( "U_SUC_UCS4_START",		Hex6Str(A_uc[1,2]?A_uc[1,1]:A_uc[2,1]),	"first item of interest in simple uc table for UCS4" )
SetGlobalConstant( "U_SUC_UCS4_END",		Hex6Str(A_uc[count A_uc,1]),			"last (excl.) item of interest in simple uc table for UCS4" )

SetGlobalConstant( "U_SLC_UCS4",			count A_lc,								"sizeof simple lowercase table for UCS4" )
SetGlobalConstant( "U_SLC_UCS4_START",		Hex6Str(A_lc[1,2]?A_lc[1,1]:A_lc[2,1]),	"first item of interest in simple lc table for UCS4" )
SetGlobalConstant( "U_SLC_UCS4_END",		Hex6Str(A_lc[count A_lc,1]),			"last (excl.) item of interest in simple lc table for UCS4" )

SetGlobalConstant( "U_SUC_EVEN_UCS2",		count a_eu,								"sizeof simple uppercase table for even UCS2 code points" )
SetGlobalConstant( "U_SUC_EVEN_UCS2_START", Hex4Str(a_eu[1,2]?a_eu[1,1]:a_eu[2,1]),	"first item of interest in simple uc table for even UCS2" )
SetGlobalConstant( "U_SUC_EVEN_UCS2_END",   Hex4Str(a_eu[count a_eu,1]),			"last (excl.) item of interest in simple uc table for even UCS2" )

SetGlobalConstant( "U_SLC_EVEN_UCS2",		count a_el,								"sizeof simple lowercase table for even UCS2 code points" )
SetGlobalConstant( "U_SLC_EVEN_UCS2_START", Hex4Str(a_el[1,2]?a_el[1,1]:a_el[2,1]),	"first item of interest in simple lc table for even UCS2" )
SetGlobalConstant( "U_SLC_EVEN_UCS2_END",   Hex4Str(a_el[count a_el,1]),			"last (excl.) item of interest in simple lc table for even UCS2" )

SetGlobalConstant( "U_SUC_ODD_UCS2",		count a_ou,								"sizeof simple uppercase table for odd UCS2 code points" )
SetGlobalConstant( "U_SUC_ODD_UCS2_START",	Hex4Str(a_ou[1,2]?a_ou[1,1]:a_ou[2,1]),	"first item of interest in simple uc table for odd UCS2" )
SetGlobalConstant( "U_SUC_ODD_UCS2_END",	Hex4Str(a_ou[count a_ou,1]),			"last (excl.) item of interest in simple uc table for odd UCS2" )

SetGlobalConstant( "U_SLC_ODD_UCS2",		count a_ol,								"sizeof simple lowercase table for odd UCS2 code points" )
SetGlobalConstant( "U_SLC_ODD_UCS2_START",	Hex4Str(a_ol[1,2]?a_ol[1,1]:a_ol[2,1]),	"first item of interest in simple lc table for odd UCS2" )
SetGlobalConstant( "U_SLC_ODD_UCS2_END",	Hex4Str(a_ol[count a_ol,1]),			"last (excl.) item of interest in simple lc table for odd UCS2" )

SetGlobalConstant( "U_STC_UCS2",			count a_tc,								"sizeof simple titlecase exception table for UCS2" )
SetGlobalConstant( "U_STC_UCS2_START",		Hex4Str(a_tc[1,1]),						"first item of interest in simple tc exception table for UCS2" )
SetGlobalConstant( "U_STC_UCS2_END",		Hex4Str(a_tc[count a_tc,1]),			"last (excl.) item of interest in simple tc exception table for UCS2" )

var f = count a_tc==12 && a_tc[1,1]==$01C4 && a_tc[12,1]==$01F3
SetGlobalConstant( "U_STC_UCS2_TWELVECASES", f, "simple titlecase: only 12 exceptions for UCS2" )	if !f log nl,"  Warning: no longer 12 TC exceptions!  " then


var Header = «
/*	generated on   »#datestr(now)#« UTC+02.00 (CET/CEST)
	          by   "»#fn_Script#«" 
	          from UnicodeData.txt

	          see  http://www.unicode.org/Public/»#Unicode_Version#«/ucd/UnicodeData.txt
	               http://www.unicode.org/Public/»#Unicode_Version#«/ucd/UCD.html#Case_Mappings
*/»

	
var F = 5
openout #F, fn_Includes # "SimpleUppercase_UCS4.h"
put #F, Header,"\n/*  Uppercase Mapping, UCS4\n*/"
WriteTable( F, 1, count A_uc, proc(i){return{Hex6Str(A_uc[i,1]),A_uc[i,2]}} )
close#F

openout #F, fn_Includes # "SimpleLowercase_UCS4.h"
put #F, Header,"\n/*  Lowercase Mapping, UCS4\n*/"
WriteTable( F, 1, count A_lc, proc(i){return{Hex6Str(A_lc[i,1]),A_lc[i,2]}} )
close#F

openout #F, fn_Includes # "SimpleUppercase_Odd.h"
put #F, Header,"\n/*  Uppercase Mapping, UCS2, Odd Code Points\n*/"
WriteTable( F, 1, count a_ou, proc(i){return{Hex4Str(a_ou[i,1]),a_ou[i,2]}} )
close#F

openout #F, fn_Includes # "SimpleLowercase_Odd.h"
put #F, Header,"\n/*  Lowercase Mapping, UCS2, Odd Code Points\n*/"
WriteTable( F, 1, count a_ol, proc(i){return{Hex4Str(a_ol[i,1]),a_ol[i,2]}} )
close#F

openout #F, fn_Includes # "SimpleUppercase_Even.h"
put #F, Header,"\n/*  Uppercase Mapping, UCS2, Even Code Points\n*/"
WriteTable( F, 1, count a_eu, proc(i){return{Hex4Str(a_eu[i,1]),a_eu[i,2]}} )
close#F

openout #F, fn_Includes # "SimpleLowercase_Even.h"
put #F, Header,"\n/*  Lowercase Mapping, UCS2, Even Code Points\n*/"
WriteTable( F, 1, count a_el, proc(i){return{Hex4Str(a_el[i,1]),a_el[i,2]}} )
close#F

openout #F, fn_Includes # "SimpleTitlecase.h"
put #F, Header,"\n/*  Titlecase Mapping, UCS2, only Code Points with different Uppercase Mapping\n*/"
WriteTable( F, 1, count a_tc, proc(i){return{Hex4Str(a_tc[i,1]),Hex4Str(a_tc[i,2])}} )
close#F





// =================================================

put " ... ok", nl
end 0











