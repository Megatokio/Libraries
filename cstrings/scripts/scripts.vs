

// this file is included from "create_all.vs"


if(verbose) log("run: scripts.vs\n") then

if !exists propertyValueName
	log "properties.vs must be included first\n"
then

if !exists unicodeData
	log "unicode_data.vs must be included first\n"
then



var fn_scripts  	  = sourceDir # "Scripts.txt"	// name of source file
var fn_ucs2_scripts_h = destDir # "ucs2_scripts.h"	// name of target file
var fn_ucs4_scripts_h = destDir # "ucs4_scripts.h"	// name of target file

var UCS2_NUM_SCRIPTS  = "UCS2_SCRIPTS_CNT"			// name for global constant
var UCS4_NUM_SCRIPTS  = "UCS4_SCRIPTS_CNT"			// name for global constant


proc readScripts()
{
	// read source file, extract and append data to unicodeData[] items

	vlog("readScripts\n")

	var data = file fn_scripts
	convert data from utf8
	data = split(data,nl)

	// preset entries:
	var ScriptUnknown = propertyValueName("script","unknown")
	var i=0
	do
		while ++i<=count
		unicodeData unicodeData[i] ##= {Script=ScriptUnknown}
	loop

	// assign scripts from Scripts.txt:
	var di=0
	var ui=1
	do
		while ++di <= count data
		var line = split(data[di],"#")[1]
		replace line," ",""
		if line=="" next then

		// examples:
		// «0000..001F    ; Common # Cc  [32] <control-0000>..<control-001F>»  or
		// «0020          ; Common # Zs       SPACE»

		line = split(line,";")
		var script = propertyValueName("script",line[2])
		var range = split(line[1],"..")
		var a = eval("$"#range[1])
		var e = count range >= 2 ? eval("$"#range[2]) : a  // incl.

		if a<unicodeData[ui].Code
			ui = 1
		then
		do
			while unicodeData[ui].Code < a
			ui++
		loop
		do
			while unicodeData[ui].Code <= e
			unicodeData[ui++] ##= {Script = script}
		loop
	loop
}

proc createScripts()
{
	vlog("createScripts\n")

	globals.scriptSystems = {}

	var i = 1
	do
		while i <= count unicodeData
		var start = unicodeData[i].Code
		var script = unicodeData[i].Script
		do
			while ++i <= count unicodeData
			while unicodeData[i].Script == script
		loop
		var ende = i <= count unicodeData ? unicodeData[i].Code : $110000
		scriptSystems ##= {{ Start = start, Ende = ende, Count = ende-start, Script = script }}
	loop
}

proc writeScripts()
{
	vlog("writeScripts\n")

	put "writing: ",fn_ucs2_scripts_h,nl

	proc Replace(text,*)
	{
		var i = 1;
		do
			while ++i < count locals
			replace text, "{"#(i-1)#"}", string(locals[i])
		loop
		return text
	}

	var count_ucs2 = 0, longest_used_ucs2=0, used_ucs2=0
	var count_ucs4 = 0, longest_used_ucs4=0, used_ucs4=0
	var last_end = 0
	var ScriptUnknown = propertyValueName("script","unknown")

	var text_used = "ADD_USED(0x{1},{2},{3})\n"

	proc writeUcs2(item)
	{
		put#F, Replace(text_used, hexstr(item.Start,4), item.Count, item.Script)
		count_ucs2++
		used_ucs2++
		last_end = item.Ende
		longest_used_ucs2 = max(longest_used_ucs2, item.Count)
	}
	proc writeUcs4(item)
	{
		put#F, Replace(text_used, hexstr(item.Start,6), item.Count, item.Script)
		count_ucs4++
		used_ucs4++
		last_end = item.Ende
		longest_used_ucs4 = max(longest_used_ucs4, item.Count)
	}

	var fileinfo = «
// map code point ranges -> Script property
// created »#datestr(now)#« by script »#scriptName#«
//
// note: code ranges include unassigned character codes to reduce map size
//       if in doubt you must also check that GeneralCategory != GcUnassigned
//
// Macros define code ranges:
//
// ADD_USED ( blk_start, blk_size, script_category )
// ADD_NONE ( blk_start, blk_size, script_unknown )
\n\n»

	var F
	openout#F, fn_ucs2_scripts_h
	put #F, fileinfo
	var i = 1
	do
		until scriptSystems[i].Start >= $10000
		writeUcs2(scriptSystems[i++])
	loop
	close#F

	if (last_end != $10000) log "oops#1..",nl end 1 then

	vlog("writing: ",fn_ucs4_scripts_h,nl)
	var F
	openout#F, fn_ucs4_scripts_h
	put #F, fileinfo
	//var i = i
	do
		while i <= count scriptSystems
		writeUcs4(scriptSystems[i++])
	loop
	close#F

	if (last_end != $110000) log "oops#2..",nl end 1 then

	addGlobalConstant(UCS2_NUM_SCRIPTS, count_ucs2, "table ucs2_scripts[]: number of entries")
	addGlobalConstant(UCS4_NUM_SCRIPTS, count_ucs4, "table ucs4_scripts[]: number of entries")

	vlog("count_ucs2 = ",count_ucs2, nl)
	vlog("longest_used_ucs2 = ",longest_used_ucs2,nl)
	vlog("count_ucs4 = ",count_ucs4, nl)
	vlog("longest_used_ucs4 = ",longest_used_ucs4,nl)
}

// run it:
var scriptSystems = {}
readScripts()
createScripts()
writeScripts()





