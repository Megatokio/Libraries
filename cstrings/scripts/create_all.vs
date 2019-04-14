#!/usr/local/bin/vipsi

var verbose	= 0
var force	= 0
var errors  = 0

var fn_dest_dir    = "unicode/"

var scriptName	// name of this script
var rootDir 	// fqn: "cstrings/"
var scriptDir 	// fqn: "cstrings/scripts/"
var destDir 	// fqn: "cstrings/unicode/""
var sourceDir	// fqn: "Unicode-xxx/UCD/"


/*
	================== functions =====================
*/

proc vlog(*){} // dummy variant if verbose=0

proc parseArguments()
{
	var i=0
	do
		while istext globals[++i]  // stop at var globals.verbose
		if globals[i] == "-v"
			globals.vlog = proc(*){put join(locals,"")}
			verbose=1
			next
		then
		if globals[i] == "-f" force=1   next then
		if globals[i] == "-h" || globals[i]=="--help"
			 put "options: -v = verbose, -f = force\n"
			 end 0
		then
		log "unrecognized argument. aborted. \n";
		end 1
	loop
}

proc calculateFileAndDirectoryPaths()
{
	// calculate file and directory paths

	var splitPath = split(fullpath(env._), "/")

	rootDir 	= join(splitPath[to count splitPath -2],"/") # "/"
	scriptName 	= splitPath[count splitPath]
	scriptDir 	= rootDir # splitPath[count splitPath -1] # "/"
	if splitPath[2]=="usr" rootDir = fullpath(".") then
	destDir 	= rootDir # fn_dest_dir
		var d = dir( rootDir # "Unicode-??.?.?/" )
		var i = count d; do while i; until d[i].flags[1]=="d"; i-=1; loop
		if i==0 log "\nUnicode-Ordner nicht gefunden. aborted.\n"; end 1; then
	sourceDir	= rootDir # d[i].fname # "UCD/"

	if verbose
		put " script name = ",scriptName,nl
		put " script dir  = ",scriptDir,nl
		put " root dir    = ",rootDir,nl
		put " dest dir    = ",destDir,nl
		put " source dir  = ",sourceDir,nl
	then

	if !isdir destDir
		log "created headers directory\n"
		new dir destDir
	then
}



/*
	================ DO IT! =====================
*/


parseArguments()
calculateFileAndDirectoryPaths()

include scriptDir # "global_constants.vs"
include scriptDir # "properties.vs"
include scriptDir # "blocks.vs"
include scriptDir # "unicode_data.vs"
include scriptDir # "general_category.vs"
include scriptDir # "simple_casing.vs"
include scriptDir # "numeric_values.vs"
include scriptDir # "scripts.vs"

writeGlobalConstantsHeaderFile()

if errors
	log "... ", errors, " error", (errors==1?"":"s"), nl
	end 1
else
	put " ... ok.", nl
	end 0
then

















