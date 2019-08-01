#pragma once
/*	Copyright  (c)	Günter Woigk 2008 - 2019
					mailto:kio@little-bat.de

	This file is free software
	License: BSD-2-Clause, see https://opensource.org/licenses/BSD-2-Clause


	Temporary Memory Pool
	=====================

	Provide memory for temporary strings, e.g. for return values or in expressions.
	Any moderately-sized data which does not have a destructor can be stored in temp mem.

	• thread safe
	• pools can be nested
	• automatic pool creation


	Basic Usage
	-----------

	Temporary memory pools are created automatically for every thread.

	tempstr() and tempmem() retrieve memory from the pool:

	• allocation with tempstr(len)
		• 1 byte for c-string delimiter is added and cleared to 0
		• string is not cleared, except delimiter
		• string is not aligned

	• allocation with tempmem(size)
		• memory is aligned to a multiple of 4
		• memory is not cleared

	Purge all temporary memory of the current thread with PurgeTempMem().


	Nested Pools
	------------

	A function can create it's own, local pool.
	Then all subsequent temp mem allocations come from this local pool
	until the pool is destroyed.

	• create a local instance of TempMemPool in the function body.

	• use tempStr() and tempMem() as usual: memory now comes from the new pool.

	• PurgeTempMem() purges only memory from the current pool.

	• when the function returns, the local TempMemPool instance is destroyed
	  and all temporary memory allocated since it's creation is released.

	• to return results from this function in temp mem, use TempXtStr() and TempXtMem(),
	  which allocate memory in the surrounding TempMemPool.


	Accessing Pools Directly
	------------------------

	• if you have created a local pool, then you can use AllocStr(), AllocMem() and Purge()
	  directly with this instance.

	• TempMemPool::GetPool() retrieves and may create the current pool, it never returns NULL.

	• TempMemPool::GetXtPool() similarly retrieves and may create the current surrounding pool.


	Cave At
	-------

	TempMemPool uses a pthread_key. This is unique within the application, but may collide with
	keys defined somewhere else. Basically this may be interrupt threads issued from the OS,
	such as sound interrupts. Therefore you should define a local TempMemPool at each entry point
	of external threads into your application.

	If you catch an Exception which contains a reference to a tempstr outside the scope of it's TempMemPool,
	then accessing the error message will crash.
	You must then pass a copy in the surrounding pool, e.g. made with xdupstr() (see "cstrings.h")
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



/* #######################################################################
*/

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



/* #######################################################################
*/

inline
char* TempMemPool::allocStr (uint len) noexcept
{
	char* p = alloc(len+1);
	p[len] = 0;
	return p;
}








