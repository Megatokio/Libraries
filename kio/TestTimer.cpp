/*	Copyright  (c)	Günter Woigk 2015 - 2021
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
#include "TestTimer.h"
#include "cstrings/cstrings.h"


TestTimer::TestTimer()
{
	_start = now();
}


void TestTimer::start()
{
	_start = now();
}


void TestTimer::test(double maxdelay, cstr sourceposition)
{
	double end   = now();
	double delay = end-_start;
	_start = end;

	if(delay > maxdelay)
	{
		if (maxdelay >= 1.0) logline("TT: %s took %.3f sec", sourceposition, delay);
		else if (maxdelay >= 1e-3) logline("TT: %s took %.3f msec", sourceposition, delay*1e3);
		else logline("TT: %s took %.3f µsec", sourceposition, delay*1e6);
	}
}


