// Copyright (c) 2014 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#undef NDEBUG
#define SAFETY	 2
#define LOGLEVEL 1
#include "Templates/sort.h"
#include "main.h"
#include "unix/FD.h"


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

static uint /*error*/ test_buffer_is_sorted()
{
	if (sum(bu) != cnt) return 1;
	for (uint i = 1; i < NELEM(bu); i++)
		if (bu[i - 1] > bu[i]) return 1;
	return 0;
}

static uint /*error*/ test_buffer_is_rsorted()
{
	if (sum(bu) != cnt) return 1;
	for (uint i = 1; i < NELEM(bu); i++)
		if (bu[i - 1] < bu[i]) return 1;
	return 0;
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

static uint /*error*/ rcompare_buffers()
{
	if (sum(bu) != cnt) return 1;
	if (sum(bu2) != cnt2) return 1;
	int a = 0, b = NELEM(bu);
	while (b)
		if (bu[a++] != bu2[--b]) return 1;
	return 0;
}


void test_sort_template(uint& num_tests, uint& num_errors)
{
	uint error;

	fill_buffer();
	copy_buffer();
	sort(bu, bu + NELEM(bu));
	swap_buffers();
	rsort(bu, bu + NELEM(bu));
	error = rcompare_buffers();
	num_tests++;
	num_errors += error;
	if (error) logline("test #89 failed: " __FILE__);

	fill_buffer();
	sort(bu, bu + NELEM(bu));
	error = test_buffer_is_sorted();
	num_tests++;
	num_errors += error;
	if (error) logline("test #43 failed: " __FILE__);
	rsort(bu, bu + NELEM(bu));
	error = test_buffer_is_rsorted();
	num_tests++;
	num_errors += error;
	if (error) logline("test #47 failed: " __FILE__);

	fill_buffer();
	rsort(bu, bu + NELEM(bu));
	error = test_buffer_is_rsorted();
	num_tests++;
	num_errors += error;
	if (error) logline("test #54 failed: " __FILE__);
	sort(bu, bu + NELEM(bu));
	error = test_buffer_is_sorted();
	num_tests++;
	num_errors += error;
	if (error) logline("test #58 failed: " __FILE__);
}


void test_sort_template_with_fu(uint& num_tests, uint& num_errors)
{
	uint error;

	fill_buffer();
	copy_buffer();
	sort(bu, bu + NELEM(bu), gt);
	swap_buffers();
	sort(bu, bu + NELEM(bu), lt);
	error = rcompare_buffers();
	num_tests++;
	num_errors += error;
	if (error) logline("test #128 failed: " __FILE__);

	fill_buffer();
	sort(bu, bu + NELEM(bu), gt);
	error = test_buffer_is_sorted();
	num_tests++;
	num_errors += error;
	if (error) logline("test #134 failed: " __FILE__);
	sort(bu, bu + NELEM(bu), lt);
	error = test_buffer_is_rsorted();
	num_tests++;
	num_errors += error;
	if (error) logline("test #138 failed: " __FILE__);

	fill_buffer();
	sort(bu, bu + NELEM(bu), lt);
	error = test_buffer_is_rsorted();
	num_tests++;
	num_errors += error;
	if (error) logline("test #144 failed: " __FILE__);
	sort(bu, bu + NELEM(bu), gt);
	error = test_buffer_is_sorted();
	num_tests++;
	num_errors += error;
	if (error) logline("test #148 failed: " __FILE__);
}

void test_sort_int(uint& num_tests, uint& num_errors)
{
	uint error;

	fill_buffer();
	copy_buffer();
	sort(bu, bu + NELEM(bu));
	swap_buffers();
	rsort(bu, bu + NELEM(bu));
	error = rcompare_buffers();
	num_tests++;
	num_errors += error;
	if (error) logline("test #180 failed: " __FILE__);

	fill_buffer();
	sort(bu, bu + NELEM(bu));
	error = test_buffer_is_sorted();
	num_tests++;
	num_errors += error;
	if (error) logline("test #186 failed: " __FILE__);
	rsort(bu, bu + NELEM(bu));
	error = test_buffer_is_rsorted();
	num_tests++;
	num_errors += error;
	if (error) logline("test #190 failed: " __FILE__);

	fill_buffer();
	rsort(bu, bu + NELEM(bu));
	error = test_buffer_is_rsorted();
	num_tests++;
	num_errors += error;
	if (error) logline("test #194 failed: " __FILE__);
	sort(bu, bu + NELEM(bu));
	error = test_buffer_is_sorted();
	num_tests++;
	num_errors += error;
	if (error) logline("test #200 failed: " __FILE__);
}

void test_sort_int_with_fu(uint& num_tests, uint& num_errors)
{
	uint error;

	fill_buffer();
	copy_buffer();
	sort(bu, bu + NELEM(bu), gt);
	swap_buffers();
	sort(bu, bu + NELEM(bu), lt);
	error = rcompare_buffers();
	num_tests++;
	num_errors += error;
	if (error) logline("test #214 failed: " __FILE__);

	fill_buffer();
	sort(bu, bu + NELEM(bu), gt);
	error = test_buffer_is_sorted();
	num_tests++;
	num_errors += error;
	if (error) logline("test #220 failed: " __FILE__);
	sort(bu, bu + NELEM(bu), lt);
	error = test_buffer_is_rsorted();
	num_tests++;
	num_errors += error;
	if (error) logline("test #224 failed: " __FILE__);

	fill_buffer();
	sort(bu, bu + NELEM(bu), lt);
	error = test_buffer_is_rsorted();
	num_tests++;
	num_errors += error;
	if (error) logline("test #230 failed: " __FILE__);
	sort(bu, bu + NELEM(bu), gt);
	error = test_buffer_is_sorted();
	num_tests++;
	num_errors += error;
	if (error) logline("test #234 failed: " __FILE__);
}

void test_speed()
{
	uint32 sz  = 100000;
	char*  bu1 = new char[sz];
	for (char* p = bu1; p < bu1 + sz; p++) { *p = char(random()); }
	char* bu2 = new char[sz];
	memcpy(bu2, bu1, sz);

	double t0 = now();
	sort(bu2, bu2 + sz);
	double t1 = now();
	sort(bu1, bu1 + sz);
	double t2 = now();

	assert(memcmp(bu1, bu2, sz) == 0);

	logline("sort_int(100000)  = %g", t1 - t0);
	logline("sort<int>(100000) = %g", t2 - t1);

	delete[] bu1;
	delete[] bu2;
}

#include "Templates/Array.h"
static void test_sort_ptr_to_object(uint& /*num_tests*/, uint& /*num_errors*/)
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
	assert(array == array2);
	while (array.count()) delete array.pop();
}

void test_sort(uint& num_tests, uint& num_errors)
{
	logIn("test sort.h");

	try
	{
		sort(bu, bu);
	}
	catch (std::exception&)
	{
		logline("test #265 failed: " __FILE__);
		num_errors++;
	}
	num_tests++;

	test_sort_template(num_tests, num_errors);
	test_sort_template_with_fu(num_tests, num_errors);

	test_sort_int(num_tests, num_errors);
	test_sort_int_with_fu(num_tests, num_errors);

	test_sort_ptr_to_object(num_tests, num_errors);

	test_speed();
}
