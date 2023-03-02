// Copyright (c) 2008 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "tempmem.h"
#include "kio/kio.h"

#if defined(NO_THREADS)
  #define PTHREADS 0
#elif defined(TEMPMEM_USE_PTHREADS)
  #define PTHREADS 1
#elif defined(TEMPMEM_USE_THREAD_LOCAL)
  #define PTHREADS 0
#elif __cplusplus < 201101
  #define PTHREADS 1
#else
  #define PTHREADS 0
#endif

#if PTHREADS
  #include <pthread.h>
#endif

#define ALIGNMENT_MASK	 (sizeof(ptr) - 1u)
#define MAX_REQUEST_SIZE 1000
#define DATA_BLOCK_SIZE	 8000


#if defined(NO_THREADS)

ON_INIT([] { debugstr("tempmem: single-threaded\n"); });

// current pool and linked list of all pools:
static TempMemPool*		   pool = new TempMemPool;
static inline TempMemPool* get_current_pool() { return pool; }
static inline void		   set_current_pool(TempMemPool* p) { pool = p; }

#elif PTHREADS

static pthread_key_t tempmem_key; // key for per-thread TempMemPool

static inline TempMemPool* get_current_pool()
{
	return reinterpret_cast<TempMemPool*>(pthread_getspecific(tempmem_key));
}

static inline void set_current_pool(TempMemPool* pool)
{
	int err = pthread_setspecific(tempmem_key, pool); // may fail with ENOMEM (utmost unlikely)
	if (unlikely(err))
	{
		fprintf(stderr, "tempmem: pthread_setspecific: %s\n", strerror(err));
		abort();
	}
}

static void deallocate_pool(void* pool)
{
	// note: not called for the main thread (Linux tested 2019-11)
	delete reinterpret_cast<TempMemPool*>(pool);
}

ON_INIT([] {
	int err = pthread_key_create(&tempmem_key, deallocate_pool);
	if (unlikely(err))
	{
		fprintf(stderr, "tempmem: pthread_key_create: %s\n", strerror(err));
		abort();
	}
});

#else // thread_local

// current pool and linked list of all pools:
static thread_local struct CurrentPoolPtr
{
	TempMemPool* pool = nullptr;
	CurrentPoolPtr() : pool(new TempMemPool) { debugstr("tempmem: thread-local ctor\n"); }
	~CurrentPoolPtr()
	{
		debugstr("tempmem: thread-local dtor\n");
		while (pool) delete pool;
	}
} current_pool;

static inline TempMemPool* get_current_pool() { return current_pool.pool; }
static inline void		   set_current_pool(TempMemPool* p) { current_pool.pool = p; }
#endif


struct TempMemData
{
	TempMemData* prev;
	char		 data[0];
};

static inline TempMemData* new_tempmemdata(uint32 size)
{
	return reinterpret_cast<TempMemData*>(new char[sizeof(TempMemData) + size]);
}

static inline void delete_tempmemdata(TempMemData* data) { delete[] reinterpret_cast<char*>(data); }


// ---- ctor / dtor ------------------------------------

TempMemPool::TempMemPool() noexcept : size(0), data(nullptr)
{
	prev = get_current_pool();
	set_current_pool(this);
}

TempMemPool::~TempMemPool() noexcept
{
	purge();
	set_current_pool(prev);
}


// ---- Member functions -------------------------------

void TempMemPool::purge() noexcept
{
	// purge all memory in this pool
	// all memory retrieved from this pool becomes invalid!

	while (data != nullptr)
	{
		TempMemData* prev = data->prev;
		delete_tempmemdata(data);
		data = prev;
	}
	size = 0;
}

char* TempMemPool::alloc(uint bytes) noexcept
{
	// allocate memory in this pool

	if (bytes <= size) // fits in current buffer?
	{
		size -= bytes;
		return data->data + size;
	}

	else if (bytes <= MAX_REQUEST_SIZE) // small request?
	{
		TempMemData* newdata = new_tempmemdata(DATA_BLOCK_SIZE);
		assert((uintptr_t(newdata) & ALIGNMENT_MASK) == 0);
		newdata->prev = data;
		data		  = newdata;
		size		  = DATA_BLOCK_SIZE - bytes;
		return newdata->data + size;
	}

	else // large request
	{
		TempMemData* newdata = new_tempmemdata(bytes);
		assert((uintptr_t(newdata) & ALIGNMENT_MASK) == 0);
		if (data)
		{
			newdata->prev = data->prev; // neuen Block 'unterheben'
			data->prev	  = newdata;
		}
		else
		{
			newdata->prev = nullptr;
			data		  = newdata;
			assert(size == 0);
		}
		return newdata->data;
	}
}

TempMemPool* TempMemPool::getPool() noexcept
{
	// Get the current temp mem pool
	// if there is no pool, then it is created.

#if PTHREADS
	TempMemPool* pool = get_current_pool();
	return pool ? pool : new TempMemPool();
#else
	return get_current_pool(); // current_pool ctor allocates a pool so there is always a pool in place
#endif
}

TempMemPool* TempMemPool::getXPool() noexcept
{
	// Get the surrounding temp mem pool
	// if there is no surrounding pool, then it is created.
	// If also no current pool exists, then 2 pools are created.

	TempMemPool* pool = getPool();
	TempMemPool* prev = pool->prev;
	if (!prev)
	{
		prev	   = new TempMemPool(); // automatically create 'outer' pool
		prev->prev = nullptr;			// 'outer' pool 'unterheben'.
		pool->prev = prev;
		set_current_pool(pool); // aktuellen Pool erneut als 'aktuell' markieren
	}
	return prev;
}


// ---- Global functions -------------------------------

char* tempmem(uint size) noexcept
{
	// Allocate temp memory
	// in the thread's current tempmem pool

	return TempMemPool::getPool()->allocMem(size);
}

char* tempstr(uint len) noexcept
{
	// Allocate a temp cstring
	// in the thread's current tempmem pool

	return TempMemPool::getPool()->allocStr(len);
}

char* xtempmem(uint size) noexcept
{
	// Allocate temp memory
	// from the surrounding pool

	return TempMemPool::getXPool()->allocMem(size);
}

char* xtempstr(uint len) noexcept
{
	// Allocate a temp cstring
	// in the surrounding pool

	return TempMemPool::getXPool()->allocStr(len);
}

static char null	 = 0;
str			emptystr = &null;

str dupstr(cstr s) noexcept
{
	// Create copy of string in tempmem

	if (unlikely(!s || !*s)) return emptystr;
	str dest = temp<char>(uint(strlen(s)) + 1);
	return strcpy(dest, s);
}

str xdupstr(cstr s) noexcept
{
	// Create copy of string in the outer tempmem pool

	if (unlikely(!s || !*s)) return emptystr;
	str dest = xtemp<char>(uint(strlen(s)) + 1);
	return strcpy(dest, s);
}

str newstr(uint n) noexcept
{
	// allocate char[]
	// deallocate with delete[]
	// presets terminating 0

	str c = new char[n + 1];
	c[n]  = 0;
	return c;
}

str newcopy(cstr s) noexcept
{
	// allocate char[]
	// deallocate with delete[]
	// returns NULL if source string is NULL

	if (s) return strcpy(new char[strlen(s) + 1], s);
	else return nullptr;
}

#ifdef DEBUG
namespace TempMemTest
{
static_assert((sizeof(TempMemData) & (ALIGNMENT_MASK)) == 0, "");

ON_INIT([] {
	// check assumptions:
	ptr p1 = new char[17], p2 = new char[15];
	assert((uintptr_t(p1) & ALIGNMENT_MASK) == 0);
	assert((uintptr_t(p2) & ALIGNMENT_MASK) == 0);
	delete[] p1;
	delete[] p2;

	char s[] = {1, 2, 3, 4};
	p1		 = strcpy(s, "abc");
	assert(p1 == s);   // must return dest addr
	assert(s[3] == 0); // must copy final '\0'
	p2 = newcopy(p1);
	assert(p1 != p2 && eq(p2, p1));
	delete[] p2;
});
} // namespace TempMemTest
#endif
