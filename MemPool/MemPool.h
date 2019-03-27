#pragma once
/*	Copyright  (c)	Günter Woigk 2008 - 2019
  					mailto:kio@little-bat.de

 	This program is distributed in the hope that it will be useful,
 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 	Permission to use, copy, modify, distribute, and sell this software and
 	its documentation for any purpose is hereby granted without fee, provided
 	that the above copyright notice appear in all copies and that both that
 	copyright notice and this permission notice appear in supporting
 	documentation, and that the name of the copyright holder not be used
 	in advertising or publicity pertaining to distribution of the software
 	without specific, written prior permission.  The copyright holder makes no
 	representations about the suitability of this software for any purpose.
 	It is provided "as is" without express or implied warranty.

 	THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 	EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 	PERFORMANCE OF THIS SOFTWARE.


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








