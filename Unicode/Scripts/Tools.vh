

put     "Script:  ", env._
put nl, "Sources: ", replace(replace(replace(string Sources,"{ ","")," }",""),", ",",\n         ")
put nl, "Targets: ", replace(replace(replace(string Targets,"{ ","")," }",""),", ",",\n         ")

	var verbose			= 0 
	var force			= 0

	var fn_Script		= fullpath(env._)
	var fn_Library		= fn_Script[ to rfind(fn_Script,"/Scripts/") ]		// path to Unicode Library base dir
		fn_Script		= fn_Script[ count fn_Library +9 to ]				// filename of script

	cd  fn_Library
	var d = dir( "Unicode ?.?.?/" )
	var i = count d; do while i; until d[i].flags[1]=="d"; i-=1; loop
	if i==0 log nl, " Unicode-Ordner nicht gefunden. aborted. ",nl; end 1; then
	
	var Unicode_Version	= replace( d[i].fname[9 to],"/","" )	
	var fn_Sources		= d[i].fname # "UCD/";				// partial path to unicode character database

	var fn_Includes		= "Includes/"						// partial path to Includes subdirectory
	var fn_Scripts		= "Scripts/"						// partial path to Scripts subdirectory
	var fn_Headers		= "Headers/"						// partial path to general Headers subdirectory
	var fn_Targets		= fn_Includes						// Targets directory is the Includes directory


	var GlobalConstants = {}
	
	proc SetGlobalConstant(Name,Value,*)
	{
		if count locals>2 rename locals[3]="Comment" else var Comment="" then 
		Comment = fn_Script # (": "#Comment)

		if verbose put nl,"  ",Name, " = ", Value then

		var fn_data   = fn_Scripts#"GlobalConstants.data" 
		var fn_header = fn_Includes#"GlobalConstants.h"

		if GlobalConstants=={} 
			try GlobalConstants = eval(file fn_data) else try del GlobalConstants.@Name then then
		then
		if exists GlobalConstants.@Name
			if !force && GlobalConstants.@Name=={Value,Comment} return then	// no need to save files
			GlobalConstants.@Name = {Value,Comment}
		else
			GlobalConstants ##= { @Name = {Value,Comment} }
		then

		var H="", i=0
		do
			while ++i <= count GlobalConstants
			var n = name(GlobalConstants[i]) n #= spacestr(23-count n) # " "
			var v = GlobalConstants[i,1]	 v=string(v) v #= spacestr(16-count v)
			var c = GlobalConstants[i,2]	 c = "/* " # c # " */" 
			H #= "#define " # n # v # c # nl
		loop 
		file fn_header = convert( H to utf8 )
		file fn_data   = replace( convert( string(GlobalConstants) to utf8 ), " }, ", " },\n" )
	} 



proc ParseArguments()
{
	var i=0
	do
		while istext globals[++i] && globals[i,1]=="-"
		if globals[i]=="-v" verbose=1 next then
		if globals[i]=="-f" force=1   next then
		if globals[i]=="-h" || globals[i]=="--help" 
			 put "\n  options: -v = verbose, -f = force\n"	end 0
		else log "\n unrecognized argument. aborted. \n"	end 1  then
	loop
}


proc QuickExitTest()
{
	if force return then
	Targets = fn_Targets#Targets
	Sources = fn_Sources#Sources

	var md_target = 1e99
	var i=0 do while ++i<=count Targets
		if !exists file Targets[i] return then
		md_target = min( md_target, mdate Targets[i] )
	loop
	var md_sources = max( mdate(fn_Scripts#fn_Script), mdate(fn_Scripts#"Tools.vh") )
	var i=0 do while ++i<=count Sources
		md_sources = max( md_sources, mdate Sources[i] )
	loop

	if md_sources+30/*sec*/ < md_target 	
		put nl,"  ", count Targets==1 ? "Target ist" : "Targets sind", " up-to-date.", nl
		end 0
	then
}	
	

//	Read file, clean-up, split into lines, split into fields
//	assert num_fields fields per line
//
proc ReadFile( fn, num_fields )
{
	var array = file fn 
	convert array from utf8
	replace array,"\t"," ";  
	do while find(array,"  ") replace array, "  ", " "	loop
	replace array, "\n ",nl	

	var k = find(array,"#")												// ex. Kommentare?	
	var l = find(array,"\n#") || find(array,"\n\n") || array[1]==nl		// ex. Leerzeilen?

	split array;				// split into lines
	del array[count array]		// split erzeugt nach dem letzten nl immer noch eine Leerzeile

	var i=0
	do
		while ++i <= count array
		replace array[i], "; ", ";"	
		replace array[i], " ;", ";"	
		replace array[i], " #", "#"
	loop


	if k						// Entferne Kommentare
		var i=0
		do
			while ++i <= count array
			k = find(array[i],"#") 
			if k array[i] = array[i, to k-1] then
		loop
	then

	if l						// Entferne Leerzeilen
		var i=0
		do
			while ++i <= count array
			if array[i]=="" del array[i--] then
		loop
	then

	split array,";"				// split lines into fields

	var i = 0					// prüfe Anzahl Felder
	var m=0,p=0
	var a = split( spacestr(num_fields), " " )
	do
		while ++i <= count array
		if count array[i] == num_fields			next then
		if count array[i] >  num_fields p++ next then
		m++ array[i] ##= a[to num_fields-count array[i]]
	loop
	
	if p put nl, "  ",p," Zeilen hatten zuviele Felder" then
	if m put nl, "  ",m," Zeilen hatten zuwenig Felder" then

	return array
}


proc Hex4Str(N) { return "0x" # hexstr(N,4) }		// --> 0xABCD
proc Hex6Str(N) { return "0x" # hexstr(N,6) }		// --> 0xABCDEF


proc Hex4ArrayStr(A,Nelem)		// --> {0xABCD,0xCDEF,0x0000}
{
	if islist A 
		var i=1, Text = Hex4Str(A[1])	
		do while ++i<=count A; Text #= "," # Hex4Str(A[i]); loop; i--
	else
		var i=1, Text = Hex4Str(A)
	then
	Text #= spacestr(Nelem-i,",0x0000")
	return "{" # Text # "}"
}


//	evaluate field, single column or multiple columns in array
//	comment lines in arrays are skipped
//
proc EvalHex ( Text )
{
	if Text=="" return 0 then
	if find(Text," ") log " EvalHex(",Text,") " then

	try 
		return eval("$"#Text) 
	else
		log " EvalHex(",Text,") " 
		return 0
	then
}

proc EvalHexColumn( array, idx )
{
	var i = 0
	do
		while ++i <= count array
		if islist array[i]		// else skip over comment lines
			array[i,idx] = EvalHex( array[i,idx] ) 
		then
	loop
	return array
}

proc EvalHexColumns( array, idx, * )
{
	var n = count locals -1
	var j = 1
	do
		while ++j <= n 
		idx = locals[j]	
		var i = 0
		do
			while ++i <= count array
			if islist array[i]		// else skip over comment lines
				array[i,idx] = EvalHex( array[i,idx] )
			then
		loop
	loop
	return array
}


/*	Write Table to File
	F = Stream Id
	a = Start index (incl.)
	e = End index (incl.)
	P = Access proc;		e.g.: proc(i) { return MyArray[i]; }
	
	the resultung file should be parsable by C/C++ and VIP.
*/
proc WriteTable ( F, a, e, P, * )		// Stream, Start, EndIncl ArrayAccessProc(i), GKflag
{
	if count locals > 4 rename locals[5]="GK" else var GK=1 then	// geschweifte Klammern?
	var GKauf = "{"[to GK], GKzu="}"[to GK]
	
	var A[1], sz[1]
	var i = a-1
	do
		while ++i<=e
		A[i] = P(i) 
		var j=0
		do
			while ++j < count A[i]
			var n = count string A[i,j]
			if sz[j]<n sz[j]=n then
		loop
	loop

	put #F, "\n{"
	i=a-1
	do
		while ++i<=e
		var z = A[i]
		put#F, nl, GKauf
		j=0  
		do
			while ++j < count z  
			put #F, z[j], ", ", spacestr(sz[j]-count z[j])  
		loop 
		put#F, z[j], i<count A ? GKzu#"," : GKzu#"\n};\n"
	loop
}


















