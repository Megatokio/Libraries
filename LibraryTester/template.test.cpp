// Copyright (c) 2020 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#undef NDEBUG
#define SAFETY	 2
#define LOGLEVEL 1
#include "kio/kio.h"
#include "main.h"

void test_whatever(uint& num_tests, uint& num_errors)
{
	logIn("test whatever");

	TRY const char abcd[5] = "abcd";
	assert(peek4X(abcd) == 'abcd');
	assert(peek4Z(abcd) == 'dcba');
	END
}
