// Copyright (c) 2019 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#undef NDEBUG
#define SAFETY	 2
#define LOGLEVEL 1
#include "Templates/Array.h"
#include "Templates/RCObject.h"
#include "Templates/RCPtr.h"
#include "main.h"
#include "unix/FD.h"


static void test1(uint& num_tests, uint& num_errors)
{
	// eq() ne() gt() lt()

	TRY static const int n[] = {-2, -1, 0, +1, +2};
	for (uint i = 0; i < NELEM(n); i++)
		for (uint j = 0; j < NELEM(n); j++)
		{
			int a = n[i];
			int b = n[j];
			assert(eq(a, b) == (a == b));
			assert(ne(a, b) == (a != b));
			assert(lt(a, b) == (a < b));
			assert(gt(a, b) == (a > b));
			assert(le(a, b) == (a <= b));
			assert(ge(a, b) == (a >= b));
		}
	END

		TRY static const uint n[] = {0, +1, +2, ~0u};
	for (uint i = 0; i < NELEM(n); i++)
		for (uint j = 0; j < NELEM(n); j++)
		{
			uint a = n[i];
			uint b = n[j];
			assert(eq(a, b) == (a == b));
			assert(ne(a, b) == (a != b));
			assert(lt(a, b) == (a < b));
			assert(gt(a, b) == (a > b));
			assert(le(a, b) == (a <= b));
			assert(ge(a, b) == (a >= b));
		}
	END

		TRY static const signed char n[] = {-2, -1, 0, +1, +2};
	for (uint i = 0; i < NELEM(n); i++)
		for (uint j = 0; j < NELEM(n); j++)
		{
			signed char a = n[i];
			signed char b = n[j];
			assert(eq(a, b) == (a == b));
			assert(ne(a, b) == (a != b));
			assert(lt(a, b) == (a < b));
			assert(gt(a, b) == (a > b));
			assert(le(a, b) == (a <= b));
			assert(ge(a, b) == (a >= b));
		}
	END

		TRY static const uint64 n[] = {0, +1, +2, ~0u};
	for (uint i = 0; i < NELEM(n); i++)
		for (uint j = 0; j < NELEM(n); j++)
		{
			uint64 a = n[i];
			uint64 b = n[j];
			assert(eq(a, b) == (a == b));
			assert(ne(a, b) == (a != b));
			assert(lt(a, b) == (a < b));
			assert(gt(a, b) == (a > b));
			assert(le(a, b) == (a <= b));
			assert(ge(a, b) == (a >= b));
		}
	END

		TRY static const float32 n[] = {-3.3e-13f, 2.2e-3f, 2.2e+3f, 0.0f};
	for (uint i = 0; i < NELEM(n); i++)
		for (uint j = 0; j < NELEM(n); j++)
		{
			float32 a = n[i];
			float32 b = n[j];
			assert(eq(a, b) == (a == b));
			assert(ne(a, b) == (a != b));
			assert(lt(a, b) == (a < b));
			assert(gt(a, b) == (a > b));
			assert(le(a, b) == (a <= b));
			assert(ge(a, b) == (a >= b));
		}
	END

		TRY static const float64 n[] = {-3.3e-13, 2.2e-3, 2.2e+3, 0.0};
	for (uint i = 0; i < NELEM(n); i++)
		for (uint j = 0; j < NELEM(n); j++)
		{
			float64 a = n[i];
			float64 b = n[j];
			assert(eq(a, b) == (a == b));
			assert(ne(a, b) == (a != b));
			assert(lt(a, b) == (a < b));
			assert(gt(a, b) == (a > b));
			assert(le(a, b) == (a <= b));
			assert(ge(a, b) == (a >= b));
		}
	END

		TRY static const float128 n[] = {-3.3e-13l, 2.2e-3l, 2.2e+3l, 0.0l};
	for (uint i = 0; i < NELEM(n); i++)
		for (uint j = 0; j < NELEM(n); j++)
		{
			float128 a = n[i];
			float128 b = n[j];
			assert(eq(a, b) == (a == b));
			assert(ne(a, b) == (a != b));
			assert(lt(a, b) == (a < b));
			assert(gt(a, b) == (a > b));
			assert(le(a, b) == (a <= b));
			assert(ge(a, b) == (a >= b));
		}
	END

		// eq() ne() gt() lt() for str and cstr

		TRY cstr a = "1.1e33l",
				 b = "Anton", c = "anton", e = nullptr;
	assert(eq(b, "Anton"));
	assert(ne(b, c));
	assert(gt(b, a));
	assert(lt(a, b));
	assert(gt(b, ""));
	assert(lt("", b));
	assert(eq("", e));
	assert(ne(e, b));
	assert(gt(b, e));
	assert(lt(e, b));
	assert(!gt("", e));
	assert(!lt("", e));
	assert(!gt(e, ""));
	assert(!lt(e, ""));
	assert(!gt(e, e));
	assert(!lt(e, e));
	END

		TRY cstr a = "Anton",
				 b = "anton";
	str c		   = dupstr(b);
	assert(eq(b, c));
	assert(eq(c, b));
	assert(ne(a, c));
	assert(ne(c, a));
	assert(gt(c, a));
	assert(!gt(a, c));
	assert(lt(a, c));
	assert(!lt(c, a));
	assert(gt(c, ""));
	assert(lt("", c));
	END

		TRY class Foo
	{
	public:
		int n;
		Foo(int n) : n(n) {}
		bool operator==(const Foo& b) const { return n == b.n; }
		bool operator!=(const Foo& b) const { return n != b.n; }
		bool operator<(const Foo& b) const { return n < b.n; }
		bool operator>(const Foo& b) const { return n > b.n; }
		bool operator<=(const Foo& b) const { return n <= b.n; }
		bool operator>=(const Foo& b) const { return n >= b.n; }
	};

	static Foo n[] = {-2, -1, 0, 2, 3};

	for (uint i = 0; i < NELEM(n); i++)
		for (uint j = 0; j < NELEM(n); j++)
		{
			const Foo& a = n[i];
			Foo&	   b = n[j];
			assert((a == b) == (a.n == b.n));
			assert((a != b) == (a.n != b.n));
			assert((a < b) == (a.n < b.n));
			assert((a > b) == (a.n > b.n));
			assert((a <= b) == (a.n <= b.n));
			assert((a >= b) == (a.n >= b.n));
		}

	for (uint i = 0; i < NELEM(n); i++)
		for (uint j = 0; j < NELEM(n); j++)
		{
			const Foo& a = n[i];
			Foo&	   b = n[j];
			assert(eq(a, b) == (a.n == b.n));
			assert(ne(a, b) == (a.n != b.n));
			assert(lt(a, b) == (a.n < b.n));
			assert(gt(a, b) == (a.n > b.n));
			assert(le(a, b) == (a.n <= b.n));
			assert(ge(a, b) == (a.n >= b.n));
		}

	for (uint i = 0; i < NELEM(n); i++)
		for (uint j = 0; j < NELEM(n); j++)
		{
			const Foo* a = &n[i];
			Foo*	   b = &n[j];
			assert(eq(a, b) == (a->n == b->n));
			assert(ne(a, b) == (a->n != b->n));
			assert(lt(a, b) == (a->n < b->n));
			assert(gt(a, b) == (a->n > b->n));
			assert(le(a, b) == (a->n <= b->n));
			assert(ge(a, b) == (a->n >= b->n));
		}
	END

		TRY class Foo : public RCObject
	{
	public:
		int n;
		Foo(int n) : n(n) {}
		bool operator==(const Foo& b) const { return n == b.n; }
		bool operator!=(const Foo& b) const { return n != b.n; }
		bool operator<(const Foo& b) const { return n < b.n; }
		bool operator>(const Foo& b) const { return n > b.n; }
		bool operator<=(const Foo& b) const { return n <= b.n; }
		bool operator>=(const Foo& b) const { return n >= b.n; }
	};

	RCPtr<Foo> p[] = {new Foo(-2), new Foo(-1), new Foo(0), new Foo(2), new Foo(3), new Foo(-2)};

	for (uint i = 0; i < NELEM(p); i++)
		for (uint j = 0; j < NELEM(p); j++)
		{
			RCPtr<Foo> a = p[i];
			RCPtr<Foo> b = p[j];
			assert((a == b) == (a.ptr() == b.ptr()));
			assert((a != b) == (a.ptr() != b.ptr()));
			assert((a < b) == (a.ptr() < b.ptr()));
			assert((a > b) == (a.ptr() > b.ptr()));
			assert((a <= b) == (a.ptr() <= b.ptr()));
			assert((a >= b) == (a.ptr() >= b.ptr()));
		}

	for (uint i = 0; i < NELEM(p); i++)
		for (uint j = 0; j < NELEM(p); j++)
		{
			RCPtr<Foo> a = p[i];
			RCPtr<Foo> b = p[j];
			assert((*a == *b) == (a->n == b->n));
			assert((*a != *b) == (a->n != b->n));
			assert((*a < *b) == (a->n < b->n));
			assert((*a > *b) == (a->n > b->n));
			assert((*a <= *b) == (a->n <= b->n));
			assert((*a >= *b) == (a->n >= b->n));
		}

	for (uint i = 0; i < NELEM(p); i++)
		for (uint j = 0; j < NELEM(p); j++)
		{
			RCPtr<Foo> a = p[i];
			RCPtr<Foo> b = p[j];
			assert(eq(a, b) == (a->n == b->n));
			assert(ne(a, b) == (a->n != b->n));
			assert(lt(a, b) == (a->n < b->n));
			assert(gt(a, b) == (a->n > b->n));
			assert(le(a, b) == (a->n <= b->n));
			assert(ge(a, b) == (a->n >= b->n));
		}
	END
}

void test_relational_operators(uint& num_tests, uint& num_errors)
{
	logIn("test relational operators");
	test1(num_tests, num_errors);
	//test2(num_tests,num_errors);
	//test3(num_tests,num_errors);
	//test4(num_tests,num_errors);
}
