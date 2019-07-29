#pragma once
/*	Copyright  (c)	Günter Woigk 2018 - 2019
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
*/

#include "Libraries/Templates/HashMap.h"
#include "Libraries/Templates/Array.h"
#include "IdfID.h"


class Names
{
	struct Chunk
	{
		Chunk* prev;
		char data[8*1024-sizeof(Chunk*)];
		explicit Chunk (Chunk* p) : prev(p) {}
		~Chunk(){ delete prev; }
	};
	Chunk* pool;
	char* pool_ptr;
	char* pool_end;

	const cstr* stdnames;
	const uint num_stdnames;

	HashMap<cstr,IdfID> hashmap;

	Names(const Names&) = delete;
	Names(const Names&&) = delete;
	Names& operator=(const Names&) = delete;
	Names& operator=(const Names&&) = delete;

public:
	Names(const cstr* stdnames, uint num_stdnames);
	~Names();

	IdfID add(cstr name);											// add name if n.ex.

	IdfID get(cstr name) const	{ return hashmap.get(name); }		// must exist
	cstr get(IdfID id) const	{ return hashmap.getKeys()[id]; }	// must exist

	IdfID operator[](cstr name) const	{ return hashmap.get(name); }		// must exist
	cstr operator[](IdfID id) const		{ return hashmap.getKeys()[id]; }	// must exist

	Array<IdfID> merge(const Names&);
	void purge();

	void serialize(class FD&) const;
	void deserialize(class FD&);
};






















