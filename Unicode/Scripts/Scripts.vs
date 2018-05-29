#!/usr/local/bin/vipsi

var Sources = { "Scripts.txt" }
var Targets = { "Scripts.h", "Scripts_UCS4.h" }

var r = include( env._[to rfind(env._,"/")] # "Tools.vh" ); if r end r then
ParseArguments()
QuickExitTest()


// ==================================================================


var Array = ReadFile( fn_Sources#"Scripts.txt", 2 )


// {"1234..2345","Name"} -> {"1234","2345","Name"}
// {"1234","Name"}		 -> {"1234","1234","Name"}
// Außerdem "Common" bzw. dessen Alias "Zyyy", was Default ist, rauswerfen
var i=0		
do									
	while ++i <= count Array
	if Array[i,2]=="Common" || Array[i,2]=="Zyyy" del Array[i--] next then
	var z = split(Array[i,1],"..") 
	Array[i] = { z[1], z[count z], Array[i,2] }
loop
Array = EvalHexColumns( Array, 1, 2 )
sort Array


// Sequenzen kombinieren:
var i=0
do
	while ++i < count Array
	do
		while i<count Array && Array[i,2]+1 == Array[i+1,1] 
		Array[i,2] = Array[i+1,2]
		del Array[i+1]
	loop
loop


// Lücken mit Leerblocks schließen
if Array[1,1]!=0 Array = {{0,Array[1,1]-1,"Common"}} ## Array then
if Array[count Array,3]!="Common" Array ##= {{Array[count Array,2]+1,$ffffff,"Common"}} then

var i = count Array
do
	while --i
	if Array[i,2]+1 != Array[i+1,1] 	
		if Array[i,2]+1 > Array[i+1,1] 	
			log nl,"  Script Block Überschneidung: ",Array[i,1],"  "
		else
			Array = Array[to i] ## {{Array[i,2]+1,Array[i+1,1]-1,"Common"}} ## Array[i+1 to]
		then
	then
loop



proc Find(a,e,n) { do i=int((a+e)/2); n<Array[i,1]?e:a = i while a<e-1 loop return a }
var n4 = count Array
var n2 = Find(1,n4+1,$ffff) 
var n1 = Find(1,n2+1,$ff)	

if Array[n2+1,1]!=$10000 log "\n  UCS4-Tabelle beginnt nicht mit 0x10000 Aborted.\n" end 1 then

SetGlobalConstant( "U_SCRIPTS_UCS1", n1,   "size of ucs1->script subtable" ) 
SetGlobalConstant( "U_SCRIPTS_UCS2", n2,   "size of ucs2->script table" )
SetGlobalConstant( "U_SCRIPTS_UCS4", n4-n2,"size of ucs4->script table" ) 






var F = 5;
openout #F, fn_Targets#"Scripts.h"
put #F, "// map ucs2 code point ranges -> script property"
WriteTable( F, 1,n2, proc(i){return{Hex4Str(Array[i,1]),"U_sc_"#lowerstr(Array[i,3])}} )
close#F

openout #F, fn_Targets#"Scripts_UCS4.h"
put #F, "// map ucs4 code point ranges -> script property"
WriteTable( F, n2+1, n4, proc(i){return{Hex6Str(Array[i,1]),"U_sc_"#lowerstr(Array[i,3])}} )
close#F







// ==================================================================

put " ... ok.", nl
end 0


















