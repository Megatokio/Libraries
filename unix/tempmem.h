#pragma once
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


	Temporary Memory Pool
	=====================

	Provides fast memory for temporary strings, e.g. for return values or in expressions.
	Any moderately-sized data which does not have a destructor can be stored in temp mem.
	See cstrings/ for the main use case.

	• thread local / thread safe
	• pools can be nested
	• automatic pool creation


	Basic Usage
	-----------

	Temporary memory pools are created automatically for every thread and purged when the thread
	terminates or on demand with purgeTempMem().

	Use tempstr() and tempmem() to retrieve memory from the pool:

	• allocation with tempstr(len)
		• 1 byte for c-string delimiter is added and cleared to 0
		• string is not cleared, except delimiter
		• string is not aligned

	• allocation with tempmem(size)
		• memory is aligned to a multiple of MAX_ALIGNMENT (typically sizeof pointer)
		• memory is not cleared


	Nested Pools
	------------

	A function can create a local pool. Then all subsequent temp mem allocations come from the
	local pool and they are purged in one go when the pool is destroyed, e.g. when the function returns.

	• create a local instance of TempMemPool in the function body.
	• use tempstr() and tempmem() as usual: memory now comes from the local pool.
	• purgeTempMem() purges only memory in the local pool.
	• when the function returns, the local pool is destroyed and all memory allocated in it is released.

	• to return results in temp mem from this function while the local pool is alive,
	  use xtempstr() and xtempmem(), which allocate memory in the surrounding pool.


	Accessing Pools Directly
	------------------------

	• if you have created a local pool, then you can use the member functions allocStr(), allocMem()
	  and purge() directly with this instance.

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

extern	char*	tempmem (uint size)		noexcept;
extern	char*	tempstr (uint size)		noexcept;
inline	char*	tempmem (int size)		noexcept { assert(size>=0); return tempmem(uint(size)); }
inline	char*	tempstr (int size)		noexcept; // cstrings.h

extern	char*	xtempmem (uint size)	noexcept;
extern	char*	xtempstr (uint size)	noexcept;
inline	char*	xtempmem (int size)		noexcept { assert(size>=0); return xtempmem(uint(size)); }
inline	char*	xtempstr (int size)		noexcept { assert(size>=0); return xtempstr(uint(size)); }

extern	void	purgeTempMem ()			noexcept;


// #######################################################################

struct TempMemData
{
	TempMemData*	prev;
	char			data[0];
};


class TempMemPool
{
	uint			size;
	TempMemData*	data;
	TempMemPool*	prev;

					TempMemPool		(TempMemPool const&) = delete;
	void			operator=		(TempMemPool const&) = delete;

public:
					TempMemPool		()			noexcept;
					~TempMemPool	()			noexcept;

	void			purge			()			noexcept;
	char*			alloc			(uint size)	noexcept;
	char*			allocStr		(uint len)	noexcept;	// 0-terminated
	char*			allocMem		(uint size)	noexcept;	// aligned to _MAX_ALIGNMENT

static TempMemPool*	getPool			()			noexcept;
static TempMemPool*	getXPool		()			noexcept;
};


// #######################################################################

inline char* TempMemPool::allocStr (uint len) noexcept
{
	char* p = alloc(len+1);
	p[len] = 0;
	return p;
}








