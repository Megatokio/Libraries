// Copyright (c) 2018 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "tempmem.h"
#include "doctest/doctest/doctest.h"
#include "hash/sdbm_hash.h"
#include <thread>


// 2019-11: use own random number generator because stdlib random() is BLOCKING when called from multiple threads!
// source: https://en.wikipedia.org/wiki/Permuted_congruential_generator

static uint64		state	   = 0x4d595df4d0f33173; // Or something seed-dependent
static const uint64 multiplier = 6364136223846793005u;
static const uint64 increment  = 1442695040888963407u; // Or an arbitrary odd constant

static inline uint32 rotr32(uint32 x, uint r) { return x >> r | x << (-r & 31); }

static uint32 pcg32()
{
	uint64 x	 = state;
	uint   count = uint(x >> 59); // 59 = 64 - 5

	state = x * multiplier + increment;
	x ^= x >> 18;						   // 18 = (64 - 27)/2
	return rotr32(uint32(x >> 27), count); // 27 = 32 - 5
}

static void pcg32_init(uint64 seed)
{
	state = seed + increment;
	pcg32();
}

ON_INIT([] { pcg32_init(uint64(now<time_t>() * 123456789)); });


static uint random(uint n) { return (uint32(n) * uint16(pcg32())) >> 16; }

static void alloc_some_bytes(uint n = 99);
static void alloc_some_bytes(uint n)
{
	for (uint i = 0; i < n; i++) (void)tempstr(random(999));
}


void test1()
{
	TempMemPool z;
	(void)tempstr(0);
	(void)tempstr(8);
	CHECK(size_t(tempstr(79)) % native_alignment != 0); // not required but expected
	CHECK(size_t(tempstr(79)) % native_alignment != 0); // not required but expected
	CHECK(size_t(tempmem(80)) % native_alignment == 0); // required
	(void)tempstr(12345);
	z.purge();
	(void)tempstr(8);
}

static constexpr int N = 2000;

void test3(TempMemPool& tempmempool)
{
	ptr	 list1[N];
	ptr	 list2[N];
	uint size[N];
	for (uint i = 0; i < N; i++)
	{
		uint n	 = min(random(0x1fff), random(0x1fff));
		list1[i] = tempmempool.alloc(n);
		list2[i] = new char[n];
		size[i]	 = n;
		while (n--) list1[i][n] = char(random(256));
		memcpy(list2[i], list1[i], size[i]);
	}
	for (uint i = 0; i < N; i++)
	{
		CHECK(memcmp(list1[i], list2[i], size[i]) == 0);
		delete[] list2[i];
	}
}

void test4(TempMemPool& tempmempool)
{
	for (uint i = 0; i < N; i++)
	{
		uint n = min(random(0x1fff), random(0x1fff));
		str	 a = tempmempool.allocStr(n);
		str	 b = tempmempool.allocMem(n);
		CHECK(a[n] == 0);
		CHECK(size_t(b) % native_alignment == 0);
	}
}

void test6()
{
	for (uint i = 0; i < N; i++)
	{
		uint n = min(random(0x1fff), random(0x1fff));
		str	 a = tempstr(n);
		str	 b = tempmem(n);
		str	 c = xtempstr(n);
		str	 d = xtempmem(n);
		CHECK(a[n] == 0);
		CHECK(size_t(b) % native_alignment == 0);
		CHECK(c[n] == 0);
		CHECK(size_t(d) % native_alignment == 0);
	}
}

void test7()
{
	ptr	 list1[N];
	ptr	 list2[N];
	uint size[N];
	uint hash[N];
	{
		TempMemPool z;
		for (uint i = 0; i < N; i++)
		{
			uint n	 = min(random(0x1fff), random(0x1fff));
			list1[i] = tempmem(n);
			list2[i] = xtempmem(n);
			size[i]	 = n;
			while (n--) list1[i][n] = char(random(256));
			hash[i] = sdbm_hash(list1[i], size[i]);
			memcpy(list2[i], list1[i], size[i]);
		}
		alloc_some_bytes();
		for (uint i = 0; i < N; i++) { CHECK(memcmp(list1[i], list2[i], size[i]) == 0); }
		alloc_some_bytes();
		CHECK(TempMemPool::getPool() == &z);
		TempMemPool::getPool()->purge();
		alloc_some_bytes();
	}
	alloc_some_bytes();
	for (uint i = 0; i < N; i++) { CHECK(sdbm_hash(list2[i], size[i]) == hash[i]); }
}

TEST_CASE("TempMemPool")
{
	SUBCASE("") { logline("●●● %s:", __FILE__); }

	auto all_tests = []() //
	{
		test1();

		TempMemPool* outerpool = TempMemPool::getPool();
		TempMemPool	 tempmempool;
		CHECK(TempMemPool::getPool() == &tempmempool);
		CHECK(TempMemPool::getXPool() == outerpool);

		test3(tempmempool);
		test4(tempmempool);
		tempmempool.purge();
		test6();
		test7();
	};

	SUBCASE("single thread") { all_tests(); }

#ifndef NO_THREADS
	SUBCASE("multi-threaded")
	{
		double		start = now();
		std::thread threads[10];
		for (uint i = 0; i < NELEM(threads); i++) { threads[i] = std::thread(all_tests); }
		all_tests();
		for (uint i = 0; i < NELEM(threads); i++) { threads[i].join(); }
		logline("tempmem: test time = %f sec.", now() - start);
	}
#endif
}


/*












































*/
