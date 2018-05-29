#!/usr/local/bin/vipsi

var Sources = { "EastAsianWidth.txt" }
var Targets = { "EastAsianWidth.h", "EastAsianWidth_UCS4.h" }

var r = include( env._[to rfind(env._,"/")] # "Tools.vh" ); if r end r then
ParseArguments()
QuickExitTest()
include "/usr/local/lib/vipsi/TEXT.vl"


// =======================================================================
// East Asian Width

	var Array = ReadFile( fn_Sources#"EastAsianWidth.txt", 2 )

//	{ CODE, WIDTH_PROPERTY }
//	{ START..END, WIDTH_PROPERTY }

// normalize -->  { START, END, WIDTH_PROPERTY }

	put " "
	var i=0
	do
		TEXT.SpinWheel()
		while ++i <= count Array
		var prop = lowerstr(Array[i,2])  if prop=="n" del Array[i--] next then		// "N" ist Default
		var dots = find(Array[i,1],"..")
		Array[i] = dots ? { eval("$"#Array[i,1,to dots-1]), eval("$"#Array[i,1,dots+2 to]), prop } 
						: { eval("$"#Array[i,1]),			eval("$"#Array[i,1]),			prop } 
	loop

	sort Array


// collapse collections:

	put " "
	var i=1
	do
		TEXT.SpinWheel()
		while ++i <= count Array
		if Array[i,3]==Array[i-1,3] && Array[i-1,2]+1 == Array[i,1]		// prop bleibt gleich und keine Lücke
			Array[i-1,2]=Array[i,2]
			del Array[i--]
		then			
	loop


// UCS1/2/4-Bereich

	proc Find(a,e,n) { do i=int((a+e)/2); n<Array[i,1]?e:a = i while a<e-1 loop return a }
	var n4 = count Array
	var n2 = Find(1,n4+1,$ffff) 
	var n1 = Find(1,n2+1,$ff)	


	SetGlobalConstant( "U_EA_UCS1",			n1,			"size of East Asian Width ucs1 subtable" ) 
	SetGlobalConstant( "U_EA_UCS2",			n2,			"size of East Asian Width ucs2 table" ) 
	SetGlobalConstant( "U_EA_UCS4",			n4-n2,		"size of East Asian Width ccc4 table" ) 
	SetGlobalConstant( "U_ea_defaultvalue", "U_ea_n",	"East Asian Width default Property Value" ) 

	SetGlobalConstant( "U_EA_UCS1_START",	Hex4Str(Array[1,1]),	"first non-dflt in East Asian Width ucs1 subtable" ) 
	SetGlobalConstant( "U_EA_UCS1_END",		Hex4Str(Array[n1,2]),	"last non-dflt in East Asian Width ucs1 subtable" ) 

	SetGlobalConstant( "U_EA_UCS2_START",	Hex4Str(Array[n1+1,1]),	"first non-dflt in East Asian Width ucs2 table" ) 
	SetGlobalConstant( "U_EA_UCS2_END",		Hex4Str(Array[n2,2]),	"last non-dflt in East Asian Width ucs2 table" ) 

	SetGlobalConstant( "U_EA_UCS4_START",	Hex6Str(Array[n2+1,1]),	"first non-dflt in East Asian Width ucs4 table" ) 
	SetGlobalConstant( "U_EA_UCS4_END",		Hex6Str(Array[n4,2]),	"last non-dflt in East Asian Width ucs4 table" ) 



var F=5
openout #F, fn_Includes # "EastAsianWidth.h"
put #F, "// East Asian Width"
WriteTable( F,1,n2,proc(i){return{Hex4Str(Array[i,1]),Hex4Str(Array[i,2]),"U_ea_"#Array[i,3]}} )
close #F

openout #F, fn_Includes # "EastAsianWidth_UCS4.h"
put #F, "// East Asian Width"
WriteTable( F,n2+1,n4,proc(i){return{Hex6Str(Array[i,1]),Hex6Str(Array[i,2]),"U_ea_"#Array[i,3]}} )
close #F










// =================================================

put " ... ok", nl
end 0











