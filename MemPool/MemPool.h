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


	Memory Pool
	===========

	This is a variant (and future base of) class TempMemPool.

	Provide memory for temporary strings, e.g. for return values or in expressions.
	Any moderately-sized data which does not have a destructor can be stored in temp mem.
*/

#include "../kio/kio.h"


class MemPool final
{
	static const size_t BUFFER_SIZE = 8000;
	static const size_t MAXREQ_SIZE = 400;

	size_t	freesize;
	char*	data;

	MemPool (const MemPool&)			= delete;
	MemPool (MemPool&&)					= delete;
	MemPool& operator= (const MemPool&)	= delete;
	MemPool& operator= (MemPool&&)		= delete;

	static ptr new_data (size_t)		noexcept;
	static void delete_data (ptr)		noexcept;
	static ptr& prev_data(ptr)			noexcept;

public:
	MemPool ()	noexcept;
	~MemPool ()	noexcept;

	void	purge ()					noexcept;
	char*	alloc (size_t size)			throws;
	char*	allocstr (size_t len )		throws;		// 0-terminated
	char*	allocmem (size_t size)		throws;		// aligned to _MAX_ALIGNMENT
	char*	dupstr (cstr)				throws;

	static void test();
};








