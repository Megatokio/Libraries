/*	Copyright  (c)	Günter Woigk 2008 - 2019
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
#include <pthread.h>
#include "tempmem.h"


#define ALIGNMENT_MASK	(_MAX_ALIGNMENT-1)
static const uint MAX_REQ_SIZE = 500;
static const uint BUFFER_SIZE = 8000;


// ----	Initialization ---------------------------------

static pthread_key_t tempmem_key;	// key for per-thread TempMemPool

static void deallocate_pool (void* pool)
{
	// note: not called for the main thread (Linux tested 2019-11)

	xlogline("tempmem: delete pool");
	delete reinterpret_cast<TempMemPool*>(pool);
}

DEBUG_INIT_MSG
ON_INIT([]
{
	xlogline("tempmem: create key");
	int err = pthread_key_create( &tempmem_key, deallocate_pool );
	if (err) abort( "tempmem: init: %s", strerror(err) );
});


// ---- ctor / dtor ------------------------------------

TempMemPool::TempMemPool() noexcept :
	size(0),
	data(nullptr)
{
	xlogIn("new TempMemPool");

	prev = reinterpret_cast<TempMemPool*>(pthread_getspecific( tempmem_key ));
	xxlogline("  prev pool = %lx",(ulong)prev);
	xxlogline("  this pool = %lx",(ulong)this);
	//int err =
	pthread_setspecific( tempmem_key, this );				// may fail with ENOMEM (utmost unlikely)
	//if (err) {abort("new TempMemPool: ",strerror(err));}	// then this pool is not registered
}															// and getPool() keeps using the outer pool

TempMemPool::~TempMemPool() noexcept
{
	xlogIn("delete TempMemPool");
	xxlogline("  this pool = %lx",(ulong)this);
	xxlogline("  prev pool = %lx",(ulong)prev);

	purge();
	int err = pthread_setspecific( tempmem_key, prev );		// may fail with ENOMEM (utmost unlikely)
	if (err) { abort("delete TempMemPool: %s",strerror(err)); }
}


// ---- Member functions -------------------------------

void TempMemPool::purge () noexcept		// Purge() == destroy + create pool
{
	// purge all memory in this pool

	xlogIn("TempMemPool::Purge");
	xxlog("  this pool = %lx ",(ulong)this);

	while (data != nullptr)
	{
		xxlog(".");
		TempMemData* prev = data->prev;
		delete[] reinterpret_cast<char*>(data); data = prev;
	}
	size = 0;
	xxlogline(" ok");
}

char* TempMemPool::alloc (uint bytes) noexcept
{
	// allocate memory in this pool

	if (bytes <= size)					// fits in current buffer?
	{
		size -= bytes;
		return data->data + size;
	}
	else if (bytes <= MAX_REQ_SIZE)		// small request?
	{
		TempMemData* newdata = reinterpret_cast<TempMemData*>(new char[ sizeof(TempMemData) + BUFFER_SIZE ]);
		xxlogline("tempmem new data = $%lx",(ulong)newdata);
		assert( (uintptr_t(newdata) & ALIGNMENT_MASK) == 0 );
		newdata->prev = data;
		data = newdata;
		size = BUFFER_SIZE-bytes;
		return newdata->data + size;
	}
	else								// large request
	{
		TempMemData* newdata = reinterpret_cast<TempMemData*>(new char[ sizeof(TempMemData) + bytes ]);
		xxlogline("tempmem new data = $%lx", ulong(newdata));
		assert( (uintptr_t(newdata) & ALIGNMENT_MASK) == 0 );
		if (data)
		{
			newdata->prev = data->prev;		// neuen Block 'unterheben'
			data->prev = newdata;
		}
		else
		{
			newdata->prev = nullptr;
			data = newdata;
			size = 0;
		}
		return newdata->data;
	}
}

char* TempMemPool::allocMem (uint bytes) noexcept
{
	// allocate aligned memory in this pool

	char* p = alloc(bytes);
	if (data->prev && p == data->prev->data)	// wurde "large request" 'untergehoben' ?
	{
		return p;
	}
	else
	{
		uint n = size & ALIGNMENT_MASK;
		size -= n;
		return p-n;
	}
}

TempMemPool* TempMemPool::getPool() noexcept
{
	// Get the current temp mem pool
	// if there is no pool, then it is created.

	TempMemPool* pool = reinterpret_cast<TempMemPool*>(pthread_getspecific( tempmem_key ));
	return pool ? pool : new TempMemPool();
}

TempMemPool* TempMemPool::getXPool() noexcept
{
	// Get the surrounding temp mem pool
	// if there is no surrounding pool, then it is created.
	// If also no current pool exists, then 2 pools are created.

	TempMemPool* pool = getPool();
	TempMemPool* prev = pool->prev;
	if ( !prev )
	{
		prev = new TempMemPool();					// automatically create 'outer' pool
		prev->prev = nullptr;						// 'outer' pool 'unterheben'.
		pool->prev = prev;
		pthread_setspecific( tempmem_key, pool );	// aktuellen Pool erneut als 'aktuell' markieren
	}												// note: *might* fail with ENOMEM (utmost unlikely)
	return prev;									// in which case we keep on using the outer pool
}

char* tempmem (uint size) noexcept
{
	// Allocate temp memory
	// in the thread's current tempmem pool

	return TempMemPool::getPool()->allocMem(size);
}

char* tempstr (uint len) noexcept
{
	// Allocate a temp cstring
	// in the thread's current tempmem pool

	return TempMemPool::getPool()->allocStr(len);
}

char* xtempmem (uint size) noexcept
{
	// Allocate temp memory
	// from the surrounding pool

	return TempMemPool::getXPool()->allocMem(size);
}

char* xtempstr (uint len) noexcept
{
	// Allocate a temp cstring
	// in the surrounding pool

	return TempMemPool::getXPool()->allocStr(len);
}

void purgeTempMem() noexcept
{
	// Purge current pool

	TempMemPool::getPool()->purge();
}

#ifndef NDEBUG
namespace TempMemTest
{
	static_assert((sizeof(TempMemData)&(ALIGNMENT_MASK)) == 0, "");

	static struct T
	{
		T()
		{
			// check assumptions:
			ptr p1 = new char[17], p2 = new char[15];
			assert( (uintptr_t(p1)&ALIGNMENT_MASK)==0 );
			assert( (uintptr_t(p2)&ALIGNMENT_MASK)==0 );
			delete[] p1;
			delete[] p2;
		}
	} dummy;
}
#endif




































