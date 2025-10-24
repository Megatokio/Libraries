// Copyright (c) 2014 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#define loglevel 1
#include "Templates/sort.h"
#include "Templates/Array.h"
#include "doctest/doctest/doctest.h"


static int bu[1000];
static int cnt;
static int bu2[1000];
static int cnt2;


static void fill_buffer()
{
	cnt = 0;
	for (uint i = 0; i < NELEM(bu); i++) cnt += bu[i] = int(random());
}

static int sum(int bu[])
{
	int cnt = 0;
	for (uint i = 0; i < NELEM(::bu); i++) cnt += bu[i];
	return cnt;
}

static bool buffer_is_sorted()
{
	if (sum(bu) != cnt) return false;
	for (uint i = 1; i < NELEM(bu); i++)
		if (bu[i - 1] > bu[i]) return false;
	return true;
}

static bool buffer_is_rsorted()
{
	if (sum(bu) != cnt) return false;
	for (uint i = 1; i < NELEM(bu); i++)
		if (bu[i - 1] < bu[i]) return false;
	return true;
}

static void copy_buffer()
{
	memcpy(bu2, bu, sizeof(bu));
	cnt2 = cnt;
}

static void swap_buffers()
{
	int z[NELEM(bu)];
	memcpy(z, bu, sizeof(bu));
	memcpy(bu, bu2, sizeof(bu));
	memcpy(bu2, z, sizeof(bu));
	kio::swap(cnt2, cnt);
}

static bool rcompare_buffers()
{
	//if (sum(bu) != cnt) return false;
	//if (sum(bu2) != cnt2) return false;
	for (int a = 0, b = NELEM(bu); b;)
		if (bu[a++] != bu2[--b]) return false;
	return true;
}

TEST_CASE("sort")
{
	SUBCASE("") { logline("●●● %s:", __FILE__); }

	SUBCASE("sort_empty_buffer")
	{
		CHECK_NOTHROW(sort(bu, bu)); //
	}

	SUBCASE("sort_template")
	{
		fill_buffer();
		copy_buffer();
		sort(bu, bu + NELEM(bu));
		CHECK_UNARY(buffer_is_sorted());
		swap_buffers();
		rsort(bu, bu + NELEM(bu));
		CHECK_UNARY(buffer_is_rsorted());
		CHECK_UNARY(rcompare_buffers());

		fill_buffer();
		sort(bu, bu + NELEM(bu));
		CHECK_UNARY(buffer_is_sorted());
		rsort(bu, bu + NELEM(bu));
		CHECK(buffer_is_rsorted());

		fill_buffer();
		rsort(bu, bu + NELEM(bu));
		CHECK(buffer_is_rsorted());
		sort(bu, bu + NELEM(bu));
		CHECK(buffer_is_sorted());
	}


	SUBCASE("sort_template_with_fu")
	{
		fill_buffer();
		copy_buffer();
		sort(bu, bu + NELEM(bu), gt);
		swap_buffers();
		sort(bu, bu + NELEM(bu), lt);
		CHECK_UNARY(rcompare_buffers());

		fill_buffer();
		sort(bu, bu + NELEM(bu), gt);
		CHECK_UNARY(buffer_is_sorted());
		sort(bu, bu + NELEM(bu), lt);
		CHECK_UNARY(buffer_is_rsorted());

		fill_buffer();
		sort(bu, bu + NELEM(bu), lt);
		CHECK_UNARY(buffer_is_rsorted());
		sort(bu, bu + NELEM(bu), gt);
		CHECK_UNARY(buffer_is_sorted());
	}

	SUBCASE("sort_int")
	{
		fill_buffer();
		copy_buffer();
		sort(bu, bu + NELEM(bu));
		swap_buffers();
		rsort(bu, bu + NELEM(bu));
		CHECK_UNARY(rcompare_buffers());

		fill_buffer();
		sort(bu, bu + NELEM(bu));
		CHECK_UNARY(buffer_is_sorted());
		rsort(bu, bu + NELEM(bu));
		CHECK_UNARY(buffer_is_rsorted());

		fill_buffer();
		rsort(bu, bu + NELEM(bu));
		CHECK_UNARY(buffer_is_rsorted());
		sort(bu, bu + NELEM(bu));
		CHECK_UNARY(buffer_is_sorted());
	}

	SUBCASE("sort_int_with_fu")
	{
		fill_buffer();
		copy_buffer();
		sort(bu, bu + NELEM(bu), gt);
		swap_buffers();
		sort(bu, bu + NELEM(bu), lt);
		CHECK_UNARY(rcompare_buffers());

		fill_buffer();
		sort(bu, bu + NELEM(bu), gt);
		CHECK_UNARY(buffer_is_sorted());
		sort(bu, bu + NELEM(bu), lt);
		CHECK_UNARY(buffer_is_rsorted());

		fill_buffer();
		sort(bu, bu + NELEM(bu), lt);
		CHECK_UNARY(buffer_is_rsorted());
		sort(bu, bu + NELEM(bu), gt);
		CHECK_UNARY(buffer_is_sorted());
	}

	SUBCASE("sort_speed_test")
	{
		uint32 sz  = 100000;
		char*  bu1 = new char[sz];
		char*  bu2 = new char[sz];
		for (char* p = bu1; p < bu1 + sz; p++) { *p = char(random()); }
		memcpy(bu2, bu1, sz);

		double t0 = now();
		sort(bu2, bu2 + sz);
		sort(bu1, bu1 + sz);
		double t2 = now();

		CHECK_UNARY(memcmp(bu1, bu2, sz) == 0);

		logline("sort<char>(100000) = %g", (t2 - t0) / 2);

		delete[] bu1;
		delete[] bu2;
	}

	SUBCASE("sort_ptr_to_object")
	{
		struct Foo
		{
			int a, b;
			Foo(int a, int b) : a(a), b(b) {}
			bool operator>(const Foo& q) const { return a != q.a ? a > q.a : b > q.b; }
			bool operator!=(const Foo& q) const { return a != q.a || b != q.b; }
		};

		Array<Foo*> array(0u, 100 * 100);
		for (int a = 100; a--;)
			for (int b = 100; b--;) array.append(new Foo(a, b));
		Array<Foo*> array2(array);
		array.shuffle();
		array.sort();
		array.revert();
		CHECK(array == array2);
		while (array.count()) delete array.pop();
	}
}

/*

















*/
