// Copyright (c) 2014 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "main.h"
#include "Templates/RCObject.h"
#include "Templates/RCPtr.h"
#include <iostream>


int main(int, const char**)
{
	uint num_tests	= 0;
	uint num_errors = 0;

	test_kio_util(num_tests, num_errors);
	test_tempmem(num_tests, num_errors);
	test_cstrings(num_tests, num_errors);
	test_relational_operators(num_tests, num_errors);
	test_Array(num_tests, num_errors);
	test_StrArray(num_tests, num_errors);
	test_RCArray(num_tests, num_errors);
	test_hashmap(num_tests, num_errors);
	test_sort(num_tests, num_errors);
	TestStringClass(num_tests, num_errors);
	test_z80_disass(num_tests, num_errors);
	test_z80_opcode_length(num_tests, num_errors);
	test_z80_major_opcode(num_tests, num_errors);
	test_z80_clock_cycles(num_tests, num_errors);

	std::cout << "\n" << num_tests << " tests run, " << num_errors << " errors.\n";

	return 0;
}
