

// this file is included from "create_all.vs"

// this file provides:
// proc propertyName(prop)		map for property names --> unique name
// proc ropertyValueName(prop,value)	map prop+value --> unique name


if(verbose) log("run: properties.vs\n") then


var fn_propertyaliases      = sourceDir # "PropertyAliases.txt"		 // name of source file
var fn_propertyvaluealiases = sourceDir # "PropertyValueAliases.txt" // name of source file
var fn_properties_h			= destDir # "properties.h"			 // name of target file

/*
Properites:
Es gibt ca. 120 Properties.
Diese lassen sich nach Art ihrer Werte klassifizieren als:
- Numeric Property:		Value = Numerisch
- String Property:		Value = Text
- Misc. Property:		?
- Catalog Property:		Value = Enum aus Liste. (growing)
- Enumerated Property:	Value = Enum aus Liste. (final)
- Binary Property:		Value = yes|no

Für die meisten Properties gibt es mehrere Namen, idR. eine Kurz- und eine Langform.
Ob und welcher Name in Programmen verwendet wird, ist egal, der offizielle Kurzname ist vielleicht empfehlenswert. Die Namen müssen aber im Script bekannt sein, um die offiziellen Unicode-Dateien zu parsen. Deshalb wird eine Map (propertyNames) angelegt.

Name matching:
Loose matching should be applied to all property names: the case distinctions, whitespace, and '_' are ignored.

Property Values:
Die Werte selbst müssen bei Bedarf mittels separater Scripte ausgewertet werden.
Für die ca. 20 Enumerated Properties wird die Headerdatei "enum_properties.h" erzeugt.

Da Property-Werte keine Unicode-global eindeutigen Bezeichner haben, sollte man auf diese nur mit Enum-Prefix zugreifen.
*/


var normalizedPropertyNames = {} // any real name --> normalized name for indexing
var propertyNames = {}			 // normalizedName --> real name for enum
var propertyPrefixes = {}		 // normalizedName --> prefix for real value enum names
var propertyValueNames = {}		 // normalizedNames (property + value) --> real name for enum


proc nameForEnum(original_name)	// convert real name --> name for enum
{
	var real_name = upperstr(original_name[1]) # original_name[2 to]
	replace real_name, "_", ""
	return real_name
}

proc normalizedName(prop) // convert real name --> normalized name used for indexing map
{
	replace prop, " ",""
	replace prop, "_",""
	replace prop, "-",""
	return lowerstr(prop)
}

proc normalizedPropertyName(prop) // real property name --> normalized name used for indexing map
{
	// get normalized property name for use as index in map

	if exists globals.normalizedPropertyNames.@prop
		return globals.normalizedPropertyNames.@prop
	then

	var norm_prop = normalizedName(prop)

	if exists globals.normalizedPropertyNames.@norm_prop
		var globals.normalizedPropertyNames.@prop = globals.normalizedPropertyNames.@norm_prop
		return globals.normalizedPropertyNames.@prop
	then

	log "property not found in normalizedPropertyNames[]: ",prop,"\n"; errors++
	var globals.normalizedPropertyNames.@prop = norm_prop
	return norm_prop
}

proc propertyPrefix(prop)
{
	// get short property name for use as prefix

	var norm_prop = normalizedPropertyName(prop)

	if !exists globals.propertyPrefixes.@norm_prop
		log "property not found in propertyNames[]: ",prop,"\n"; errors++
		var globals.propertyPrefixes.@norm_prop = nameForEnum(prop)
	then

	return globals.propertyPrefixes.@norm_prop
}

proc propertyName(prop)
{
	// get real name of property for use as enum:

	var norm_prop = normalizedPropertyName(prop)

	if !exists globals.propertyNames.@norm_prop
		log "property not found in propertyNames[]: ",prop,"\n"; errors++
		var globals.propertyNames.@norm_prop = nameForEnum(prop)
	then

	return globals.propertyNames.@norm_prop
}

proc propertyValueName(prop,value)
{
	// get real name of property value for use as enum:

	var norm_prop = normalizedPropertyName(prop)
	var norm_value = normalizedName(value)

	if !exists globals.propertyValueNames.@norm_prop.@norm_value
		log "property:value not found in propertyValueNames[]: ",prop, ":",value, "\n"; errors++
		var prefix = propertyPrefix(prop)
		var globals.propertyValueNames.@norm_prop.@norm_value = nameForEnum(prefix # "_" # value)
	then

	return globals.propertyValueNames.@norm_prop.@norm_value
}

proc readPropertyNames()
{
	// read file "PropertyAliases.txt" and create global map
	// zeilen: "cjkRSUnicode  ; kRSUnicode     ; Unicode_Radical_Stroke; URS"

	vlog("readPropertyNames\n")

	var globals.propertyNames = {}
	var data = split(convert(file fn_propertyaliases from utf8),nl)

	// remove empty lines and comments
	var i = count data
	do
		while i
		var zeile = split(data[i--],"#")[1]
		replace zeile," ",""
		if zeile=="" del data[i+1]; next; then
		data[i+1] = zeile
	loop

	// now: "ShortName;LongName;OtherName;YetAnotherName"
	// split lines, normalize names and add to map:
	var i=0
	do
		while ++i <= count data
		var aliases = split(data[i],";")

		var short_name = nameForEnum(aliases[1])
		var long_name  = nameForEnum(aliases[2])
		var norm_name  = normalizedName(long_name)

		var globals.propertyPrefixes.@norm_name = short_name
		var globals.propertyNames.@norm_name = long_name

		var j=0
		do
			while ++j <= count aliases
			var alias = aliases[j]
			var norm_alias = normalizedName(alias)
			var real_alias = nameForEnum(alias)

			var globals.normalizedPropertyNames.@alias = norm_name
			var globals.normalizedPropertyNames.@norm_alias = norm_name
			var globals.normalizedPropertyNames.@real_alias = norm_name
		loop
	loop
}

proc readPropertyValueNames()
{
	// read property value names from file PropertyValueAliases.txt
	// for each property create a subtable for name mapping in propertyValueNames
	// this creates an entry for all properties which have at least one value
	// in file PropertyNameAliases.txt, e.g. also for the binary properties

	vlog("readPropertyValueNames\n")

	var globals.propertyValueNames = {}	// list of lists
	var globals.propertyValueNames.block.none = "BlkNone"	// --> blocks.vs

	// Ccc has 241 property values which are not all enumerated in file PropertyNameAliases.txt:
	// init Ccc property value names with 241 names.
	// Names for values which actually have a real name will be overwritten later.
	if 0
		var prop = normalizedPropertyName("ccc")
		var pfx  = propertyPrefixes.@prop
		var i = 0
		do
			var globals.propertyValueNames.@prop.@string(i) = pfx # string(i)
			while ++i <= 240
		loop
	then

	// read property values and add them to their respective subtable:
	// example line: "property;shortValue;longValue;otherValue;yetAnotherValue"

	var data = split(convert(file fn_propertyvaluealiases from utf8),nl)
	var i=0
	do
		while ++i <= count data
		var zeile = split(data[i],"#")[1]	// one line in file
		replace zeile," ",""
		if zeile=="" next then				// empty
		var fields = split(zeile,";")		// split in fields

		var norm_prop = normalizedPropertyName(fields[1])
		var prefix = propertyPrefixes.@norm_prop
		var long_name = fields[fields[1]=="ccc" ? 4 : 3]
		 	if long_name[to 3] == "CCC" long_name = long_name[4 to] then
		var real_value = nameForEnum(prefix # "_" # long_name)

		var j=1
		do
			while ++j <= count fields
			var norm_value = normalizedName(fields[j])
			var globals.propertyValueNames.@norm_prop.@norm_value = real_value
		loop
	loop

}

proc writeEnumProperties()
{
	vlog("writeEnumProperties\n")

	//	enum PropertyName =
	//	{
	//		PropertyValueName, ...
	//	}

	var fileinfo = «
	// enums for boolean, enumerated and catalog properties
	// created »#datestr(now)#« by script »#scriptName#«\n\n»

	put "writing: ",fn_properties_h,nl
	var F
	openout#F, fn_properties_h
	put #F, fileinfo

	var i=0
	do
		while ++i <= count propertyValueNames
		var propertyname = name propertyValueNames[i]
			propertyname = propertyNames.@propertyname

		var values = propertyValueNames[i]
		var real_names = {}
		var j=0
		do
			while ++j <= count values
			var real_valuename = values[j]
			var real_names.@real_valuename = real_valuename
		loop

		if count real_names == 2 &&
		   real_names[1] == replace(real_names[2],"Yes","No")
		  	next
		  then

		put #F, "enum ", propertyname, nl
		put #F, "{", nl

		var j=0
		do
			while ++j <= count real_names
			put #F, "\t", real_names[j], ",\n"
		loop

		put #F, "};\n\n"

	loop
	close#F
}



readPropertyNames()
readPropertyValueNames()
writeEnumProperties()













