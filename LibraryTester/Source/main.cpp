// Copyright (c) 2014 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "Templates/Array.h"
#include "doctest/doctest/doctest.h"


doctest::String toString(Array<cstr> log)
{
	if (log.count() == 0) return "{<empty>}";
	cstr s = log[0];
	for (uint i = 1; i < log.count(); i++) { s = catstr(s, ",\n", log[i]); }
	return catstr("{\n", s, "}");
}
