#!/usr/local/bin/vipsi

var Sources = { "Blocks.txt" }
var Targets = { "Blocks.h" }
var r = include( env._[to rfind(env._,"/")] # "Tools.vh" ); if r end r then
ParseArguments()
QuickExitTest()


// ==================================================================


var BlockNames = ReadFile( fn_Sources#"Blocks.txt", 2 )


// {"1234..2345","Name"} -> {"1234","2345","Name"}
var i=0		
do									
	while ++i <= count BlockNames
	if islist BlockNames[i] 
		BlockNames[i] = split(BlockNames[i,1],"..") ## BlockNames[i,2 to]		
	then
loop
BlockNames = EvalHexColumns( BlockNames, 1, 2 )


// Lücken mit Leerblocks schließen
BlockNames ##= {{BlockNames[count BlockNames,2]+1,$ffffff,"No_Block"}}	
var i = count BlockNames, j = i
do
	while --i
	if istext BlockNames[i] next then	// skip comment: j doesn't change!
	if BlockNames[i,2]+1 != BlockNames[j,1] 	
		BlockNames = BlockNames[to i] ## {{BlockNames[i,2]+1,BlockNames[j,1]-1,"No_Block"}} ## BlockNames[i+1 to]
	then
	j = i
loop


proc Find(a,e,n) { do i=int((a+e)/2); n<BlockNames[i,1]?e:a = i while a<e-1 loop return a }
var n4 = count BlockNames
var n2 = Find(1,n4+1,$ffff) 
var n1 = Find(1,n2+1,$ff)	

SetGlobalConstant( "U_BLOCKS_UCS1", n1, "size of ucs1->block subtable" ) 
SetGlobalConstant( "U_BLOCKS_UCS2", n2, "size of ucs2->block subtable" )
SetGlobalConstant( "U_BLOCKS_UCS4", n4, "size of ucs4->block table" ) 






var F = 5;
openout #F, fn_Targets#"Blocks.h"
put #F, "\n// map code point ranges -> block property"
put #F, "\n// (similar to script, but much coarser and therefore not exact for per-code-point look up!)"
WriteTable( F, 1, n4, proc(i){return{Hex6Str(BlockNames[i,1]),"U_blk_"#lowerstr(replace(replace(BlockNames[i,3],"-","_")," ","_"))}} )
close #F





// ==================================================================

put " ... ok.", nl
end 0


















