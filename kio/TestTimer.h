// Copyright (c) 2015 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
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
  #define TT		  TestTimer tt;
  #define TTest(T, S) tt.test(T, S)
#else
  #define TT
  #define TTest(T, S)
#endif
