#pragma once
/*	Copyright  (c)	Günter Woigk 2019 - 2020
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

#include "kio/kio.h"

extern void test_tempmem(uint& num_tests, uint& num_errors);
extern void test_cstrings(uint& num_tests, uint& num_errors);
extern void test_sort(uint& num_tests, uint& num_errors);
extern void test_Array(uint& num_tests, uint& num_errors);
extern void test_hashmap(uint& num_tests, uint& num_errors);
extern void test_StrArray(uint& num_tests, uint& num_errors);
extern void test_RCArray(uint& num_tests, uint& num_errors);
extern void TestStringClass(uint& num_tests, uint& num_errors);
extern void test_relational_operators(uint& num_tests, uint& num_errors);
extern void test_kio_util (uint& num_tests, uint& num_errors);


#undef  assert
#define assert(X) do { if(!(X)){throw internal_error(__FILE__, __LINE__, "FAILED: " #X);} } while(0)
#undef IERR
#define IERR() throw internal_error(__FILE__, __LINE__,internalerror)

#define TRY num_tests++; try{
#define END }catch(std::exception& e){num_errors++; logline("%s",e.what());}
#define EXPECT(X) num_errors++; logline("%s line %i: FAILED: did not throw",__FILE__,__LINE__);}catch(X&){}\
  catch(std::exception&){num_errors++;logline("%s line %i: FAILED: wrong type of exception thrown",__FILE__,__LINE__);}



