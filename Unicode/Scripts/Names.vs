#!/usr/local/bin/vipsi

var Sources = { "UnicodeData.txt" }
var Targets = { "Names.h", "CodePointRanges.h", "CodePointRanges_UCS4.h" }

var r = include( env._[to rfind(env._,"/")] # "Tools.vh" ); if r end r then
ParseArguments()
QuickExitTest()
include "/usr/local/lib/vipsi/TEXT.vl"



/* =======================================================================
   UnicodeData.txt einlesen

	Datei einlesen, 
	in Name[] und Code[] splitten,
	Namen aufbereiten und
	NamedCodePoints.h rausschreiben
*/

var Array = file (fn_Sources # "UnicodeData.txt")
var Code={}
var Name={}



// Split Lines
// Store Code -> Code[] und Name -> Name[]
// Wenn im Namen der eigene Character Code auftaucht, ersetze ihn durch "$$$$"
// Splitte Name in Worte
// Zähle Bytes für dekomprimierten CharacterName[] array

var NameArrayZSize = 0
 
split Array	
var i=0
do
	while ++i < count Array
	TEXT.SpinWheel()
	
	var z=Array[i]
	var s1=find(z,";")
	var s2=find(z,";",s1+1)
	Code[i] = z[to s1-1]				// HexStr
	Name[i] = z[s1+1 to s2-1]
	replace Name[i], "_", " "			
	NameArrayZSize += count Name[i] +1

	replace Name[i], "-", "- "
	split   Name[i], " "

	var j = find(Name[i],"")					// <-- Zwei Codes, 0F0A und 0FD0 haben "- " im Namen
	if j										//	   das führt durch die Ersetzungen zu "-  "
		if verbose put "$",Code[i]," Wort ",j-1," = \"",Name[i,j-1],"\" " then
		Name[i,j-1] #= " "						//	   das Leerzeichen muss erhalten bleiben
		del Name[i,j]
	then
	
	var j = find(Name[i], Code[i])				// <-- in manchen Namen kommt der eigene Code Point vor
	if j										//	   so komprimierts sich besser
		Name[i,j] = "$$$$" 
		if verbose put"." then 
	then
loop
put " "




/* ===========================================================
	Code[] behandeln:
=========================================================== */
	
	
proc Find(a,e,n) { do i=int((a+e)/2); n<eval("$"#Code[i])?e:a = i while a<e-1 loop return a }
var c4 = Find(1,count Code,$FFFF)
SetGlobalConstant("U_CODEPOINTS_UCS2",	c4,				"count of ucs2 code points defined in UnicodeData.txt")
SetGlobalConstant("U_CODEPOINTS_UCS4", count Code-c4,	"count of ucs4 code points defined in UnicodeData.txt")


// Collapse Code[]:

if Code[1] != "0000" log "CODE" end 1 then	

var ZA={}
var i=1
do
	while i <= count Code
	var ai=i, av = eval("$"#Code[i])
	
	do
		while ++i <= count Code
		while eval("$"#Code[i]) == av + i-ai
	loop
	ZA ##= {{ av,i-ai }}
loop

if ZA[count ZA,1] < ai  ZA ##= {{ av,i-ai }}  then

proc Find(a,e,n) { do i=int((a+e)/2); n<ZA[i,1]?e:a = i while a<e-1 loop return a }
var CRangesUCS2 = Find(1,count ZA,$FFFF)
var CRangesUCS4 = count ZA - CRangesUCS2
SetGlobalConstant("U_CODEPOINTRANGES_UCS2", CRangesUCS2, "sizeof contiguous ucs2 code point ranges table")
SetGlobalConstant("U_CODEPOINTRANGES_UCS4", CRangesUCS4, "sizeof contiguous ucs4 code point ranges table")


// Schreibe Code[] -> file "CodePointRanges.h"

var F=5
openout#F,fn_Includes # "CodePointRanges.h"
put #F,"// ranges of ucs2 code points defined in UnicodeData.txt"
WriteTable( F, 1, CRangesUCS2, proc(i){return{Hex4Str(ZA[i,1]),string(ZA[i,2])}} )

close#F
openout#F,fn_Includes # "CodePointRanges_UCS4.h"
put #F,"// ranges of ucs4 code points defined in UnicodeData.txt"
WriteTable( F, CRangesUCS2+1, CRangesUCS2+CRangesUCS4, proc(i){return{Hex6Str(ZA[i,1]),string(ZA[i,2])}} )
close#F




/* ============================================================
	Name[] behandeln
============================================================ */

// .bin rausschreiben  ((Scratch))

openout #F, fn_Targets#"Names.bin"
var i=0
do
	while ++i <= count Name
	var j=0
	do
		while ++j <= count Name[i]
		write #F, Name[i,j]
		if j<count Name[i] && !find(" -",rightstr(Name[i,j],1)) write #F, " " then
	loop
	write #F, charstr(0)
loop
close #F

// .bin wieder einlesen 
//		und komprimieren

var Name = file(fn_Targets#"Names.bin")
SetGlobalConstant( "U_NAMEARRAY_SIZE",		 count Name, "sizeof uncompressed Name array" )
freeze Name 
SetGlobalConstant( "U_NAMEARRAY_COMPRESSED", count Name, "sizeof compressed Name array" )

// .compressed.bin rausschreiben	((Statistics))

file(fn_Targets#"Names.ice") = Name

// .h rausschreiben

openout #F, fn_Targets#"Names.h"
put #F,	"\n// Compressed Character Names",
		"\n// Decompresses to a character array char[]",
		"\n// containing the null-terminated name strings of all characters",
		"\n// matching in sequence the codepoints from UnicodeData.txt",
		"\n// as enumerated in CodePointRanges.h plus CodePointRanges_UCS4.h"

put #F, "\n{\n"

var i=0
do
	while ++i < count Name
	put #F, charcode(Name[i]), ","
	if i%32==0 put #F, nl then
loop
	put #F, charcode(Name[i]), nl, "};", nl
close #F





// =================================================

put " ... ok", nl
end 0











