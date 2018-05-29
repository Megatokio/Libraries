#!/usr/local/bin/vipsi


var Sources = { "SpecialCasing.txt" }
var	Targets = { "SpecialCasing.h" }

var r = include( env._[to rfind(env._,"/")] # "Tools.vh" ); if r end r then
ParseArguments()
QuickExitTest()


// ==================================================================


	var SpecialCasing = ReadFile( fn_Sources#"SpecialCasing.txt", 5 )
		sort SpecialCasing

	var SpecialCasingNames =  
		{ "Code_Point", "Lowercase_Mapping", "Titlecase_Mapping","Uppercase_Mapping", "Condition_List" }


var F = 5;
openout #F, fn_Targets # "SpecialCasing.h"

put #F, «
//	map for special lowercase, uppercase or titlecase mapping
//	generated on   »,datestr(now),« UTC+02.00 (CET/CEST)
//	          by   "»,fn_Script,«" 
//	          from SpecialCasing.txt

//	          see  http://www.unicode.org/Public/»,Unicode_Version,«/ucd/SpecialCasing.txt
//	               http://www.unicode.org/Public/»,Unicode_Version,«/ucd/UCD.html#Case_Mappings

//	struct UCS2_SC { UCS2Char code, lc[3], tc[3], uc[3]; cstr condition };
»

proc C(D)	// converter: "ABCD ABCD"	-> "{ 0xABCD,0xABCD,0x0000 }
{
	if count D	D #= " 0000 0000"; replace D," ",",0x"; return "{0x" # D[to 18] # "}"
	else return "{     0,     0,     0}" then
}

WriteTable( F, 1, count SpecialCasing, proc(i){ var z=SpecialCasing[i] return{ "0x"#z[1], C(z[2]), C(z[3]), C(z[4]), z[5]==""?"NULL":«"»#z[5]#«"» } } )
close #F

SetGlobalConstant( "U_SC_START", "0x"#SpecialCasing[1,1], "first code point which needs special handling for full casing" )
SetGlobalConstant( "U_SC_END",	 "0x"#hexstr(EvalHex(SpecialCasing[count SpecialCasing,1])+1,4), "last (excl.) code point for special casing" )






// ==================================================================

put " ... ok.", nl
end 0



















