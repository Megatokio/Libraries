#!/usr/local/bin/vipsi

var Target = "Names"
var r = include( env._[to rfind(env._,"/")] # "Tools.vh" ); if r end r then
ParseArguments()

include "/usr/local/lib/vipsi/TEXT.vl"



/* =======================================================================
   UnicodeData.txt einlesen

	Datei einlesen, 
	in Name[] und Code[] splitten,
	Namen aufbereiten und
	NamedCodePoints.h rausschreiben
*/

put " [Read] "

var Array = file (fn_UCD # "UnicodeData.txt")
var Code={}
var Name={}

var n = 7
var rep_codes = { «"#$%&'()», ":;<=", "*+,", ">?@", "./", "^", "_" }[to n]
var rep_count = { {},{},{},{},{},{},{},{} }[to n]

var CharsCode	= "!"
var CharsIdf	= " -ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
var CharsXToken = "[\\]"
var CharsRep	= "" var i=0 do while ++i<=n CharsRep #= rep_codes[i] loop
var CharsNToken = charstr(0) # CharsIdf # CharsCode # CharsRep # CharsXToken
var CharsReqSep	= CharsIdf[3 to]
var CharsNReqSep= CharsIdf[to 2]

var CharsToken  = ""
var i=0 do while ++i <= 255  if !find(CharsNToken,charstr(i)) CharsToken #= charstr(i) then  loop
if count CharsToken+count CharsNToken!=256 log "bubu" end 1 then

SetGlobalConstant( "U_NAMETOKENS_CHARS_CODE",	 "'"#CharsCode#"'",				 "code for code point replacement char" )
SetGlobalConstant( "U_NAMETOKENS_CHARS_IDF",	 convert(CharsIdf to quoted),	 "codes for identifiers" )
SetGlobalConstant( "U_NAMETOKENS_CHARS_REQSEP",  convert(CharsReqSep to quoted), "idf codes which require a separator after last char" )
SetGlobalConstant( "U_NAMETOKENS_CHARS_NREQSEP", convert(CharsNReqSep to quoted),"idf codes which require no separator after last char" )
SetGlobalConstant( "U_NAMETOKENS_CHARS_XTOKEN",  convert(CharsXToken to quoted), "codes which start a 2-byte token" )
//SetGlobalConstant( "U_NAMETOKENS_CHARS_TOKEN",	 convert(CharsToken to quoted),  "1-byte token codes" )	TODO: das gibt UTF-8 %-]
SetGlobalConstant( "U_NAMETOKENS_CHARS_NTOKEN",	 convert(CharsNToken to quoted), "codes which are no 1-byte token" )
SetGlobalConstant( "U_NAMETOKENS_CHARS_REP",	 string(rep_codes),				 "array of copy-from-prv-lines codes" )
SetGlobalConstant( "U_NAMETOKENS_CHARS_REPS",	 count(rep_codes),				 "sizeof of copy-from-prv-lines codes array" )


// Split Lines
// Store Code -> Code[] und Name -> Name[]
// Wenn im Namen der eigene Character Code auftaucht, ersetze ihn durch "!"
// Ersetze störende Zeichen in Name
// Splitte Name in Worte
// Zähle Bytes für decomprimierten CharacterName[] array

var NameArrayZSize = 0
 
split Array	
var i=0
do
	TEXT.SpinWheel()
	while ++i < count Array
	var z=Array[i]
	var s1=find(z,";")
	var s2=find(z,";",s1+1)
	Code[i] = z[to s1-1]				// HexStr
	Name[i] = upperstr(z[s1+1 to s2-1])
	replace Name[i], "<", ""
	replace Name[i], ">", ""
	replace Name[i], "_", " "			NameArrayZSize += count Name[i] +1
	replace Name[i], "-", "- "
	split   Name[i], " "

	var j = find(Name[i],"")					// <-- Zwei Codes, 0F0A und 0FD0 haben "- " im Namen
	if j										//	   das führt durch die Ersetzungen zu "-  "
		if verbose put "$",Code[i]," Wort ",j-1," = \"",Name[i,j-1],"\" " then
		Name[i,j-1] #= " "						//	   das Leerzeichen muss erhalten bleiben
		del Name[i,j]							//	   " " wurde bisher nicht verwenden, jetzt wirds..
	then
	
	var j = find(Name[i], Code[i])				// <-- in manchen Namen kommt der eigene Code Point vor
	if j										//	   so komprimierts sich besser
		Name[i,j] = CharsCode 
		if verbose put"." then 
	then
loop
put " "


proc Find(a,e,n) { do i=int((a+e)/2); n<eval("$"#Code[i])?e:a = i while a<e-1 loop return a }
var c4 = Find(1,count Code,$FFFF)
SetGlobalConstant("U_CODEPOINTS_UCS2",	c4,				"count of defined ucs2 code points in UnicodeData.txt")
SetGlobalConstant("U_CODEPOINTS_UCS4", count Code-c4,	"count of defined ucs4 code points in UnicodeData.txt")
SetGlobalConstant("U_NAMEARRAY_SIZE",  NameArrayZSize,  "sizeof uncompressed Name array" )


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
SetGlobalConstant("U_CODEPOINTRANGES_UCS2", CRangesUCS2, "count of ranges of defined ucs2 code points in UnicodeData.txt")
SetGlobalConstant("U_CODEPOINTRANGES_UCS4", CRangesUCS4, "count of ranges of defined ucs4 code points in UnicodeData.txt")

// Schreibe Code[] -> file "NamedCodePoints.h"

var F=5
openout#F,fn_Includes # "CodePointRanges.h"
WriteTable( F, 1, CRangesUCS2, proc(i){return{Hex4Str(ZA[i,1]),string(ZA[i,2])}} )
close#F
openout#F,fn_Includes # "CodePointRanges_UCS4.h"
WriteTable( F, CRangesUCS2+1, CRangesUCS2+CRangesUCS4, proc(i){return{Hex6Str(ZA[i,1]),string(ZA[i,2])}} )
close#F




/* ============================================================
	Komprimiere Wiederholungen der davorstehenden Zeilen
	Wiederholt werden ganze Worte/Wortsequenzen aus den letzten 1 .. 5 Zeilen
	Dadurch ist Reihenfolge der Dekomprimierung Token/Wiederholungen egal.

	Ergebnisse:
	Zeile -1:   { 3833, 7005, 3658, 2609, 715, 235, 74, 29 }
	Zeile -2:   { 1749, 191, 39, 32, 4 }
	Zeile -3:   { 539, 77, 11, 9 }
	Zeile -4:   { 537, 30, 5, 8 }
	Zeile -5:   { 259, 30, 8, 2 }
	Zeile -6:   { 255, 19, 12 }

	Einfache Repetitionen (Spalte 1) sind nicht so wichtig.
	Es stehen bis zu 20 Codes für Repetitionen zur Verfügung:
	
		" # $ % & ' ( ) * + , . / : ; < = > ? @
=>	
		" # $ % & ' ( )		Zeile -1, gleiche Wortposition, 1 .. 8 Worte
		: ; < =				Zeile -2, gleiche Wortposition, 1 .. 4 Worte
		* + ,				Zeile -3, gleiche Wortposition, 1 .. 3 Worte
		> ? @				Zeile -4, gleiche Wortposition, 1 .. 3 Worte
		./					Zeile -5, gleiche Wortposition, 1 .. 2 Worte
		
	Da Wiederholungen in Konkurrenz mit Tokenisierungen stehen,
	sollte nach der Tokenisierung nochmal geprüft werden, 
	wie effektiv die einzelnen Tok/Rep-Codes waren.
*/

if verbose put nl then
put " [Repetitionen] "
var i=count Name +1				// i = Schleife über alle Namen
do
	while --i >= 1
	TEXT.SpinWheel()
	
	z = Name[i]

	var ii = min(i+count rep_codes+1,count Name+1)		// ii = Schleife über die 3 nachfolgenden Namen
	do
		while --ii > i
		var zz = Name[ii]
		var n = min(count z, count zz)	// n = gemeinsame Namenslänge

		var j=n+1						// j = Schleife über alle Worte eines Namens
		do
			while --j >= 1
			if zz[j]!=z[j] next then	// anderes Wort

			var m = count rep_codes[ii-i]	// gleiches Wort =>
			var jj=j+1					// j = Schleife über weitere Worte
			do							
				zz[j] = ""				// Wort löschen
				while --j >= jj-m		// max. 4…6 Worte supported
				while j >= 1
				while zz[j]==z[j]
			loop
			j++ 
			
			zz[j] = rep_codes[ii-i][jj-j]		// rep control einfügen
			rep_count[ii-i][jj-j]++			// statistics
			Name[ii] = zz					// geänderten Namen zurückschreiben
		loop
	loop
loop
put " "

if verbose
	put "\nCopy controls:"
	var i=0 do  while ++i <= count rep_codes
		put "\n  Zeile -",i,":   ", rep_count[i]
	loop
	put nl
then

// da bestimmte rep-Sequenzen sehr häufig vorkommen,
// fassen wir rep-Sequenzen zu einem Wort zusammen, 
// so dass sie tokenisierbar werden.
// außerdem entfernen wir hier die gelöschten Worte

var i=0
do
	while ++i <= count Name
	var j = count Name[i] +1
	do
		while --j > 1		
		if count Name[i,j] > 1 next then
		if count Name[i,j] == 1 && !find(CharsRep,Name[i,j]) next then
		
		do
			while --j >= 1
			until count Name[i,j] > 1
			until count Name[i,j] == 1 && !find(CharsRep,Name[i,j]) 
			Name[i,j] #= Name[i,j+1]
			del Name[i,j+1]
		loop
	loop
loop



/* ==========================================================================
	Zähle und gewichte Worthäufigkeiten

	Bei Abschluss der Namen und der Token mit char(0):

	Für 1-char-Token:
		Gewicht = Einsparpotenzial
				= Cnt * Len(Token+D-1) - Len(Token+1)
	
		wobei D = 1, wenn das Token nicht auf "-" endet und ein Leerzeichen folgt

	Für 2-char-Token:
		Gewicht = Einsparpotenzial
				= Cnt * Len(Token+D-2) - Len(Token+1)
*/

var Token = {}					// { @"WORT"=Bewertung }		Bewertung für 1-Byte-Codes
var XToken = {}					// Ditto, Bewertung aber für 2-Byte-Codes

put " [NameTokens] "
var i=0
do		
	while ++i <= count Name		// i: Schleife über alle Namen
	TEXT.SpinWheel()

	z = Name[i]

	var j=0
	do
		while ++j <= count z	// j: Schleife über alle Worte im Namen
		var w = z[j]

		if count w == 0 log "?!?" end 1 then
		
		var d = j==count z || !find(CharsIdf,z[j+1,1])/*note: erkennung von token hier noch nicht möglich!*/ || !find(CharsReqSep,rightstr(w,1))	
		if exists Token.@w  Token.@w += count w -d; XToken.@w += count w -d -1; 
		else Token ##= { @w=-1-d } XToken ##= { @w=-2-d } then
	loop

loop
put " "

rsort Token
Token = Token[to count CharsToken] 

rsort XToken
var i=0
do
	while ++i <= count Token
	var w = name Token[i]
	del XToken.@w
loop

var Z={}
var i=0 do while ++i <= count CharsToken
	var c = charcode(CharsToken[i])
	Z[c] = Token[i]
	rename Z[c] = name Token[i]
loop
del Token rename Z="Token"
if count Token!=255 log "grrr" end 1 then


//var z[255-n]
//Token = Token[to ' '-1] ## z ## Token[' ' to]

Token ##= XToken[to 255 * count CharsXToken]
//if count name Token[']'] log "(GKzu)" end 1 then

if verbose
	var i=0 do while ++i <= count CharsToken
		if count Token[i]==1
			if	 find(CharsRep,Token[i]) put " \n  Token $",hexstr(i,2)," = \"", Token[i], "\" (Rep. Ctl.) " 
			elif find(CharsIdf,Token[i]) put " \n  Token $",hexstr(i,2)," = \"", Token[i], "\" (Idf) "
			else						 put " \n  Token $",hexstr(i,2)," = char($", hexstr(charcode(Token[i]),2), ") "
			then
		then
	loop
then



// Schreibe Token[] -> file "NameTokens.h"

proc HX(N)		// UP
{
	var T="", i=0 do while ++i <= count N
		T #= find(CharsIdf,N[i]) ? "'"#N[i]#"', " : "0x"#hexstr(charcode(N[i]),2)#","
	loop return T
}

var NameTokensSize = 0

openout#F,fn_Includes # "NameTokens.h"
put#F,"{"
var i=0
do
	while ++i <= count Token
	if i%255==1 put #F,"\n\n0," NameTokensSize += 1 then		// for char(0) at start of each subtable
	var tok = name Token[i]
	put #F,nl, HX(tok)  NameTokensSize += count tok
	put #F, i<count Token?"0x0,":"0x0" NameTokensSize += 1
loop
put #F,"\n};\n"
close #F





// Tokenisiere Namen

put " [Tokenize] "

var i=0					// Alle Token-Zähler löschen
do
	while ++i <= count Token
	Token[i] = - (count name Token[i] +1)
loop

var i=0
do
	TEXT.SpinWheel()
	while ++i <= count Name
	var j=0
	do
		while ++j <= count Name[i]
		var w = Name[i,j]
		if !count w log "%!%!" end 1 then
		if exists Token.@w 
			if name Token.@w!=w log " NAMEHASHERROR: \"",w,"\" == \"",name Token.@w,"\" " next then
			var t = index(Token.@w)
			if t <= 255
				if find(CharsNToken,charstr(t)) log "scheiBe:'",charstr(t),"':\"",w,"\"==\"",name Token.@w,"\"" end 1 then
				Name[i,j] = charstr(t)
				Token[t] += count name Token[t] -1 +(j<count Name[i] && find(CharsReqSep,rightstr(name Token[t],1)) && find(CharsIdf,Name[i,j+1,1]))
			else
				Token[t] += count name Token[t] -2 +(j<count Name[i] && find(CharsReqSep,rightstr(name Token[t],1)) && find(CharsIdf,Name[i,j+1,1]))
				t-=255 t--
				Name[i,j] = CharsXToken[1+t/255] # charstr(1+t%255)
			then
		then
	loop
loop
put " "


// Schreibe Namen[] -> file "Namen.h"
// Die Namen werden Lückenlos aneinandergehängt
// und mit 0 abgeschlossen

var NameArrayQSize = 0

put " [Write File] "
openout#F,fn_Target
put #F,"{"
var i=0
do								// Schleife über alle Namen
	while ++i < count Name
	TEXT.SpinWheel()
	put #F,nl
	var j=0
	do									// Schleife über alle Worte
		while ++j <= count Name[i]
		var z = Name[i,j]
		if count z==0 log "?" next then
		put #F, HX(z) NameArrayQSize += count z 
		if j<count Name[i] && find(CharsReqSep,rightstr(z,1)) && find(CharsIdf,Name[i,j+1,1])
			put #F,"' ', " NameArrayQSize += 1
		then
	loop
	put #F,  i<count Name?"0x0,":"0x0"  NameArrayQSize += 1
loop
put " "

put #F,"\n};\n"
close#F

SetGlobalConstant( "U_NAMETOKENS_SIZE",		 NameTokensSize, "sizeof NameTokens array" )
SetGlobalConstant( "U_NAMEARRAY_COMPRESSED", NameArrayQSize, "sizeof compressed Name array" )



if verbose
	put "\nErsparnisse durch Token:"
	var i=0
	var j=0
	do
		while j++ <= count CharsXToken
		var n=0 do while ++i <= j*255 n+= Token[i] loop i--
		if j==1 put "\n  1-Byte-Token:    ", n
		else	put "\n  2-Byte-Token[",j-1,"]: ", n	then
	loop
	if i!=count Token log "boobää!" then

	var i=0, f=0
	do
		while ++i<=count Token
		if Token[i]>0 next then
		if find(CharsNToken,charstr(i)) next then
		if !f log "\nVerluste für Token: " f=1 else log ", " then  log i,"=",Token[i]
	loop

	put "\nDateigrößen:"
	put "\n  Ranges UCS2: ", CRangesUCS2, "*4 = ", CRangesUCS2*4
	put "\n  Ranges UCS4: ", CRangesUCS4, "*6 = ", CRangesUCS4*6
	put "\n  NameToken:   ", NameTokensSize
	put "\n  Names:       ", NameArrayQSize, " -> ", NameArrayZSize, " Bytes"
then



// =================================================

put " ... ok", nl
end 0











