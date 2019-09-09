#pragma once
/*	Copyright  (c)	Günter Woigk 2015 - 2019
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


class TestTimer
{
public:
	double _start;

public:
	TestTimer();

	void test(double maxdelay, cstr sourceposition);
		// mod. 2018:
		// TestTimer::test(maxdelay, "... <program location> ... <printformat> <timeunit>");
		// should be replaced with:
		// TestTimer::test(maxdelay, <program location>");

	void start();
};


#if XLOG
	#define TT			TestTimer tt;
	#define TTest(T,S)	tt.test(T,S)
#else
	#define TT
	#define TTest(T,S)
#endif






