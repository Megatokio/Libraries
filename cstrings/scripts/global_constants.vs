

// this file is included from "create_all.vs"


if(verbose) log("run: global_constants.vs\n") then


proc writeGlobalConstantsHeaderFile()
{
	if(verbose) log("run: global_constants.vs\n") then
	vlog("writeGlobalConstantsHeaderFile\n")

	var fname = destDir # "global_constants.h"
	put "writing: ",fname,nl

	var F
	openout#F,fname

	put #F,
	«// global constants for unicode support files in library "cstrings/"\n»,
	«// created »#datestr(now)#« by script »#scriptName#«\n\n»

	var i = 0
	do
		while ++i <= count globalConstants
		var s = "#define "
		s #= name(globalConstants[i]) # " "
		s #= spacestr(28-count s)
		s #= string(globalConstants[i,1])
		s #= spacestr(36-count s)
		s #= " // " # globalConstants[i,2]
		put #F, s,nl
	loop
	close#F
}

proc addGlobalConstant(Name,Value,Comment)
{
	vlog( "const ", Name, " = ", Value, nl )
	var globals.globalConstants.@Name = { Value, Comment }
}


var globalConstants = {}





