#pragma once
// Copyright (c) 2008 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/*	Temporary Memory Pool
	=====================

	Provides fast memory for temporary strings, e.g. for return values or in expressions.
	Any moderately-sized data which does not have a destructor can be stored in temp mem.
	See cstrings/ for the main use case.

	• pools are thread local and thus thread safe
	• automatic pool creation and destruction for each thread
	• local pools can be nested to bulk-purge local temp strings


	Basic Usage
	-----------

	Temporary memory pools are created automatically for every thread and purged when the
	thread terminates.

	Local pools can be created to bulk-purge local temp strings only in the pool's dtor,
	e.g. when a function uses cstring functions and processes large text files.

	Use tempstr(), tempmem() and temp<T> to retrieve memory from the pool:

	tempstr(len)
	  • not cleared, not aligned
	  • 1 byte for c-string delimiter is added and cleared to 0

	tempmem(size)
	  • not cleared, aligned to sizeof(ptr)

	temp<T>(count)
	  • not cleared, aligned to sizeof(T)

	You cannot return results from a function allocated in a local tempmem pool.
	use xtempstr() and xtempmem(), which allocate memory in the surrounding pool.


	Accessing Pools Directly
	------------------------

	• if you have created a local pool, then you can use the member functions alloc(), allocStr(),
	  allocMem() and purge() directly with this instance.

	• TempMemPool::getPool() retrieves and may create the current pool, it never returns NULL.

	• TempMemPool::getXtPool() similarly retrieves and may create the current surrounding pool.


	Cave At
	-------

	TempMemPool uses a pthread_key. This is unique within the application, but might collide with
	keys defined somewhere else. Basically this may be interrupt threads issued from the OS,
	such as sound interrupts. Therefore you should define a local TempMemPool at each entry point
	of external threads into your application. ((evtl. this is just paranoid.))

	Don't store tempmem cstrings in exceptions unless you are shure that the exception is caught
	within the current tempmem pool's scope. Better use new/delete for cstrings in exceptions,
	because you don't know how deep they will fall.

	Don't store temp mem strings in objects which live longer than the current pool.
	Don't mix storage of new-allocated and temp mem strings in the same pointer.
	Don't pass temp strings from your pool to other threads, except you know you live longer.
*/

#include "kio/kio.h"
struct TempMemData;


class TempMemPool
{
	uint		 size;
	TempMemData* data;
	TempMemPool* prev;

	TempMemPool(const TempMemPool&)	   = delete;
	void operator=(const TempMemPool&) = delete;

public:
	TempMemPool() noexcept;
	~TempMemPool() noexcept;

	void  purge() noexcept;
	char* alloc(uint size) noexcept; // unaligned, uncleared

	char* allocStr(uint len) noexcept
	{
		// get a 0-terminated string with strlen len
		// allocate 1 byte more and set it to 0
		// string contents are not cleared

		char* p = alloc(len + 1);
		p[len]	= 0;
		return p;
	}

	char* allocMem(uint bytes) noexcept
	{
		// allocate 'bytes' bytes of data aligned to sizeof(ptr)
		// memory contents are not cleared

		static const uint mask = sizeof(ptr) - 1;
		this->size &= ~mask;			// align what we have
		bytes = (bytes + mask) & ~mask; // align requested size
		return alloc(bytes);			// now must be aligned as well
	}

	template<typename T>
	T* alloc(uint count) noexcept
	{
		// get 'count' elements of type T aligned to sizeof(T)
		// memory contents are not cleared

		this->size &= ~(sizeof(T) - 1); // align
		return reinterpret_cast<T*>(alloc(count * sizeof(T)));
	}

	static TempMemPool* getPool() noexcept;
	static TempMemPool* getXPool() noexcept;
};


// #######################################################################

extern str emptystr; // non-const version of ""

extern char* tempmem(uint size) noexcept;  // allocate in current pool: aligned, not cleared
extern char* xtempmem(uint size) noexcept; // allocate in outer pool

extern str tempstr(uint size) noexcept;	 // allocate in current pool: 0-terminated, not cleared
extern str xtempstr(uint size) noexcept; // allocate in outer pool

extern str dupstr(cstr) noexcept;  // copy string into the current pool
extern str xdupstr(cstr) noexcept; // copy string into the outer pool

extern str newstr(uint n) noexcept; // allocate memory with new[]
extern str newcopy(cstr) noexcept;	// allocate memory with new[] and copy string


template<typename T>
inline T* temp(uint count) noexcept
{
	// allocate 'count' elements of type T in current pool
	// aligned to sizeof(T), not cleared

	return TempMemPool::getPool()->alloc<T>(count);
}

template<typename T>
inline T* xtemp(uint count) noexcept
{
	// allocate 'count' elements of type T in outer pool
	// aligned to sizeof(T), not cleared

	return TempMemPool::getXPool()->alloc<T>(count);
}

inline void __attribute((deprecated)) purgeTempMem() noexcept
{
	// Purge current pool
	// deprecated: better create a local pool

	TempMemPool::getPool()->purge();
}
