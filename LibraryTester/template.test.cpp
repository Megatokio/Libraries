/*	Copyright  (c)	Günter Woigk 2020 - 2021
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


#undef NDEBUG
#define SAFETY 2
#define LOGLEVEL 1
#include "kio/kio.h"
#include "main.h"

void test_whatever (uint& num_tests, uint& num_errors)
{
	logIn("test whatever");

	TRY
		char const abcd[5]="abcd";
		assert( peek4X(abcd) == 'abcd' );
		assert( peek4Z(abcd) == 'dcba' );
	END
}































