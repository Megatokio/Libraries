/*	Copyright  (c)	Günter Woigk 2014 - 2019
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY, NOT EVEN THE
	IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE
	AND IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY DAMAGES
	ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/


#include <iostream>
#include "kio/kio.h"
#include "Templates/RCPtr.h"
#include "Templates/RCObject.h"


extern void test_tempmem(uint& num_tests, uint& num_errors);
extern void test_cstrings(uint& num_tests, uint& num_errors);
extern void test_sort(uint& num_tests, uint& num_errors);
extern void test_Array(uint& num_tests, uint& num_errors);
extern void test_hashmap(uint& num_tests, uint& num_errors);
extern void test_StrArray(uint& num_tests, uint& num_errors);
extern void test_RCArray(uint& num_tests, uint& num_errors);
extern void TestStringClass(uint& num_tests, uint& num_errors);


int main(int, const char**)
{
	uint num_tests=0;
	uint num_errors=0;

	test_tempmem(num_tests, num_errors);
	test_cstrings(num_tests, num_errors);
	test_Array(num_tests, num_errors);
	test_StrArray(num_tests, num_errors);
	test_RCArray(num_tests, num_errors);
	test_hashmap(num_tests, num_errors);
	test_sort(num_tests, num_errors);
	TestStringClass(num_tests, num_errors);

	std::cout << "\n" << num_tests << " tests run, " << num_errors << " errors.\n";

	return 0;
}



