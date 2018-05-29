#!/usr/local/bin/vipsi

var Sources = { "PropertyAliases.txt", "PropertyValueAliases.txt" }
var Targets = { "Property_ShortName.h", "Property_LongName.h", "Property_Enum.h", "Property_Group.h", "PropertyGroup_Name.h", "PropertyGroup_Enum.h", 
				"PropertyValue_Enum.h", "PropertyValue_ShortName.h", "PropertyValue_LongName.h" }

var r = include( env._[to rfind(env._,"/")] # "Tools.vh" ); if r end r then
ParseArguments()
QuickExitTest()


proc Abort(n)	{ log "Aborted at ID ",n end 1 }
proc Trap(n,v)	{ if v Abort(n) then }


/* ==================================================================
		Unicode Properties.
*/

var Array = file( fn_Sources # "PropertyAliases.txt" )
	split Array 
	del Array[count Array]
	var i = find(Array,"")		Trap(1,i==0)
	Array = Array[i to]
	split Array,"; "


/*	{ "# ================================================" }
	{ "# Xxxyyyzzz Properties" }
	{ "# ================================================" }
	{ "kurz", "langname", "alias" }		->	Enum:	U_kurz, U_langname, U_alis		Name:	"kurz", "langname"
	{ "kurz", "langname", "" }				Enum:	U_kurz, U_langname				Name:	"kurz", "langname"		

	Kurz- und Langname können "_" enthalten.	-> Beim Langnamen durch " " ersetzen.
	Die Kurznamen wirken teils etwas willkürlich lc/uc. -> Für Enum auf lc normieren.
	Langnamen in Enum auch auf lc normieren.
	Manchmal sind Langname == Kurzname. -> Bei Enum beachten!
*/

 
//	Gruppen bestimmen und zu allen Properties hinzufügen	->  { "Group", "Kurzname", "Langname" [, "Alias"] }
//	Kommentare entfernen
 
var Groups = {}			//	{ "GroupName", … }
var Group  = ""			// current group name

var i=1
do
	if count Array[i] > 1						// normaler Property-Eintrag
		Array[i,1] = Array[i,1][to find(Array[i,1]#" "," ")-1]
		Array[i,2] = Array[i,2][to find(Array[i,2]#" "," ")-1]
		Array[i] = Group ## Array[i] 
		i++ next 
	then

	Trap( 21, Array[i,1]!="" )					// Leerzeile
	do while Array[i,1]=="" del Array[i] loop
	Trap( 22, Array[i,1][to 5]!="# ===" )		// # ====================
	del Array[i]

	Trap( 23, count Array[i]!=1 )				// Xxxyyyzzz Properties
	var z = Array[i,1]
	del Array[i]
	until find(z,"Total:")						// # Total: 88
	Trap( 24, !find(z,"Properties") )
	if verbose put "\n  ", z[3 to] then

	Trap( 25, Array[i,1][to 5]!="# ===" )		// # ====================
	del Array[i]

	Group = z[3 to rfind(z," ")-1] 
	Groups ##= { Group }
loop
do while i<=count Array del Array[i] loop



// Write Property Group Stuff:

	var F = 5;
	openout #F, fn_Includes # "PropertyGroup_Enum.h"
	put#F,"// property groups, grouped by data type."
	WriteTable( F, 1, count Groups, proc(i){ return{"U_"#lowerstr(Groups[i])#"_property"} }, 0 )
	close #F

	openout #F, fn_Includes # "PropertyGroup_Names.h"
	put#F,"// names for property groups. matches enum."
	WriteTable( F, 1, count Groups, proc(i){ return { «"» # Groups[i] # « Property"» } }, 0 )
	close #F

	SetGlobalConstant("U_PROPERTYGROUPS",count Groups)



// Write Property Stuff:

	openout #F, fn_Includes # "Property_ShortNames.h"
	put#F,"// character property short names. matches enum."
	WriteTable( F, 1, count Array, proc(i){ return{«"»#Array[i,2]#«"»} }, 0 )
	close #F

	openout #F, fn_Includes # "Property_LongNames.h"
	put#F,"// character property long names. matches enum."
	WriteTable( F, 1, count Array, proc(i){ return{«"»#replace(Array[i,3],"_"," ")#«"»} }, 0 )
	close #F

	openout #F, fn_Includes # "Property_Groups.h"
	put#F,"// map property -> property group."
	WriteTable( F, 1, count Array, proc(i){ return{"U_"#lowerstr(Array[i,1])#"_property"} }, 0 )
	close #F

	proc P(i)					// Enum
	{	
		var z = Array[i, 2 to]; convert z to lower
		if z[2]==z[1] del z[2] then
		z = "U_"#z
		return count z==1 ? { z[1] } : count z==2 ? { z[1], z[2]#" = "#z[1] } : { z[1], z[2]#" = "#z[1], z[3]#" = "#z[1] }
	}

	openout #F, fn_Includes # "Property_Enum.h"
	put#F,"// character properties."
	WriteTable( F, 1, count Array, P, 0 )
	close #F

	SetGlobalConstant( "U_PROPERTIES", count Array )



// ==================================================================


var Array = ReadFile( fn_Sources#"PropertyValueAliases.txt", 4 )


/*	{ "prop", "kurz", "langname", "alias" }		->	Enum:	U_prop_kurz, …langname, …alias		Name:	U_prop,	"kurz",		"langname"
	{ "prop", "kurz", "langname", "" }						U_prop_kurz, …langname				Name:	U_prop,	"kurz",		"langname"
	{ "prop", "n/a",  "langname", "alias" }					U_prop_langname, …alias				Name:	U_prop,	"prop123",	"langname"
	{ "prop", "n/a",  "langname", "" }						U_prop_langname						Name:	U_prop,	"prop123",	"langname"
	{ "ccc",  "123",  "kurz", "langname" }					U_ccc_123, _kurz, …langname	= 123	Name:	U_ccc,	"kurz",		"langname"
	((fehlender Eintrag für ccc))							U_ccc_123 = 123						Name:	U_ccc,	0,			0

	bei n/a sollte evtl. ein Default-Kurzname generiert werden, à la "prop"#index
	ccc belegt 0 … 255.	Bei Lücken in der Aufzählung von ccc im Quellfile werden Enums algorithmisch generiert 
						und Namen sollten vom C-Prg. algorithmisch generiert werden. 

	Für Enum werden Prop, Kurzname, Langname und Alias in lowercase normiert.
	Langname kann "-", " ", "." enthalten -> für Enum auf "_" normieren.
	Langname kann "_" enthalten -> für Name auf " " normieren.
	Kurzname kann "." enthalten -> für Enum auf "_" normieren.
	Manchmal ist Kurzname == Langname. -> bei Enum beachten.
*/


/*	Codes 0 .. 255 (hopefully) must be reserved for Property ccc (Canonical Combining Class)
	because this a) maps directly to numbers and b) doesn't explicitely list codes 10[ .. ]199
	so reordering is neccessary here...
*/
var ccc[256], i=0
do while ++i<=256 ccc[i]={"ccc" } loop				// { "ccc" }
i=0
do
	while ++i <= count Array
	var z = Array[i]
	if Array[i,1]=="ccc"
		var j = eval(Array[i,2])+1	// j = idx
		del Array[i,2]				// --> Zeile nur noch 3 Items
		ccc[j] = Array[i]			// nach ccc verschieben
		del Array[i--]
	then
loop
Array = ccc ## Array 


/*	jetzt:
	{ "ccc",  "kurz", "langname" }							U_ccc_123, _kurz, …langname	= 123	Name:	U_ccc,	"kurz",		"langname"
	{ "ccc" }												U_ccc_123 = 123						Name:	U_ccc,	0,			0
	{ "prop", "kurz", "langname", "alias" }		->	Enum:	U_prop_kurz, …langname, …alias		Name:	U_prop,	"kurz",		"langname"
	{ "prop", "kurz", "langname", "" }						U_prop_kurz, …langname				Name:	U_prop,	"kurz",		"langname"
	{ "prop", "n/a",  "langname", "alias" }					U_prop_langname, …alias				Name:	U_prop,	0,			"langname"
	{ "prop", "n/a",  "langname", "" }						U_prop_langname						Name:	U_prop,	0,			"langname"
*/


	var F = 5;
	openout #F, fn_Targets # "PropertyValue_ShortNames.h"
	put #F, "// Short names for enumerated and catalog properties."
	put #F, "\nchar const * const U_PropertyValue_ccc_ShortNames[U_ccc_propertyvalues] = {\n"
	var prop="ccc", i=0
	do
		while ++i <= count Array
		if Array[i,1]!=prop 
			put #F," };\n"
			prop  = Array[i,1]
			put #F, "\nchar const * const U_PropertyValue_"#lowerstr(prop)#"_ShortNames[U_"#lowerstr(prop)#"_propertyvalues] = {\n"
		then
		var z=Array[i] 
		put #F, exists z[2]&&z[2]!="n/a" ? «"»#z[2]#«"» : exists z[3]&&count z[3]<=4 ? «"»#z[3]#«"»:"0", ", "
	loop
	put #F," };\n"
	close #F

	openout #F, fn_Targets # "PropertyValue_LongNames.h"
	put #F, "// Long names for enumerated and catalog properties."
	put #F, "\nchar const * const U_PropertyValue_ccc_LongNames[U_ccc_propertyvalues] = {\n"
	var prop="ccc", i=0
	do
		while ++i <= count Array
		if Array[i,1]!=prop 
			put #F," };\n"
			prop  = Array[i,1]
			put #F, "\nchar const * const U_PropertyValue_"#lowerstr(prop)#"_LongNames[U_"#lowerstr(prop)#"_propertyvalues] = {\n"
		then
		var z=Array[i] 
		put #F, exists z[3] ? «"» # replace(z[3],"_"," ") # «"» : "0", ", "
	loop
	put #F," };\n"
	close #F


	openout #F, fn_Targets # "PropertyValue_Enum.h"

	put #F, "// Enumeration of Values for catalog and enumerated properties"
	put #F, "\n{"
	var prop="ccc", prop0=1

	i=0 do
		while ++i <= count Array

		if Array[i,1]!=prop 
			put #F,nl
			SetGlobalConstant("U_"#lowerstr(prop)#"_propertyvalues",i-prop0)
			prop  = Array[i,1]
			prop0 = i
		then

		var z = Array[i]
		var j=1 do while ++j <= count z
			replace z[j]," ","_"
			replace z[j],".","_"
			replace z[j],"-","_"
		loop
		convert z to lower

		if i<=256 
			z = { string(i-prop0) } ## z[2 to] 
			z = "U_ccc_" # z # (" = " # string(i-prop0))
		else
			if z[4]==""    del z[4] then		
			if z[3]==z[2]  del z[3] then
			if z[2]=="n/a" del z[2] then
			z = ("U_" # z[1] # "_") # z[2 to] # (" = " # string(i-prop0))
		then
		
		if count z put #F, nl then  
		do while count z  put #F, z[1], ",\t"  del z[1]loop
		
	loop

	SetGlobalConstant("U_"#lowerstr(prop)#"_propertyvalues",i-prop0)
	put #F,"\n};\n"
	close #F



	SetGlobalConstant("U_PROPERTYVALUES",count Array)



// ==================================================================

put " ... ok.", nl
end 0


















