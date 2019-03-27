/*	Copyright  (c)	Günter Woigk 2014 - 2019
					mailto:kio@little-bat.de

	This file is free software

 	This program is distributed in the hope that it will be useful,
 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	• Redistributions of source code must retain the above copyright notice,
	  this list of conditions and the following disclaimer.
	• Redistributions in binary form must reproduce the above copyright notice,
	  this list of conditions and the following disclaimer in the documentation
	  and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
	OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
	WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
	OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
	ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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



