// Copyright (c) 2018 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#undef NDEBUG
#define SAFETY	 2
#define LOGLEVEL 1
#include "tempmem.h"
#include "hash/sdbm_hash.h"
#include "main.h"
#include "unix/FD.h"
#include <pthread.h>


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

static volatile uint num_tests;
static volatile uint num_errors;

void* do_test_tempmem(void*)
{
	TRY TempMemPool z;
	(void)tempstr(0);
	(void)tempstr(8);
	assert(size_t(tempstr(79)) % _MAX_ALIGNMENT != 0); // not required but expected
	assert(size_t(tempstr(79)) % _MAX_ALIGNMENT != 0); // not required but expected
	assert(size_t(tempmem(80)) % _MAX_ALIGNMENT == 0); // required
	(void)tempstr(12345);
	z.purge();
	(void)tempstr(8);
	END

		TempMemPool* outerpool = TempMemPool::getPool();
	TempMemPool		 tempmempool;
	TRY				 assert(TempMemPool::getPool() == &tempmempool);
	assert(TempMemPool::getXPool() == outerpool);
	END

		static const int N = 2000;

	TRY ptr list1[N];
	ptr		list2[N];
	uint	size[N];
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
		assert(memcmp(list1[i], list2[i], size[i]) == 0);
		delete[] list2[i];
	}
	END

		TRY for (uint i = 0; i < N; i++)
	{
		uint n = min(random(0x1fff), random(0x1fff));
		str	 a = tempmempool.allocStr(n);
		str	 b = tempmempool.allocMem(n);
		assert(a[n] == 0);
		assert(size_t(b) % _MAX_ALIGNMENT == 0);
	}
	END

		TRY tempmempool.purge();
	END

		TRY for (uint i = 0; i < N; i++)
	{
		uint n = min(random(0x1fff), random(0x1fff));
		str	 a = tempstr(n);
		str	 b = tempmem(n);
		str	 c = xtempstr(n);
		str	 d = xtempmem(n);
		assert(a[n] == 0);
		assert(size_t(b) % _MAX_ALIGNMENT == 0);
		assert(c[n] == 0);
		assert(size_t(d) % _MAX_ALIGNMENT == 0);
	}
	END

		TRY ptr list1[N];
	ptr			list2[N];
	uint		size[N];
	uint		hash[N];
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
		for (uint i = 0; i < N; i++) { assert(memcmp(list1[i], list2[i], size[i]) == 0); }
		alloc_some_bytes();
		assert(TempMemPool::getPool() == &z);
		TempMemPool::getPool()->purge();
		alloc_some_bytes();
	}
	alloc_some_bytes();
	for (uint i = 0; i < N; i++) { assert(sdbm_hash(list2[i], size[i]) == hash[i]); }
	END

		return nullptr;
}


void test_tempmem(uint& num_tests, uint& num_errors)
{
	logIn("test TempMemPool");

	::num_tests	 = 0;
	::num_errors = 0;

	double start = now();

	pthread_t threads[10];
	for (uint i = 0; i < NELEM(threads); i++) { pthread_create(&threads[i], nullptr, do_test_tempmem, nullptr); }
	do_test_tempmem(nullptr);
	for (uint i = NELEM(threads); i--;) { pthread_join(threads[i], nullptr); }

	logline("tempmem test time = %f sec.", now() - start);

	num_tests += ::num_tests;
	num_errors += ::num_errors;
}
