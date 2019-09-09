#pragma once
/*	Copyright  (c)	Günter Woigk 2018 - 2019
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

#include "Templates/HashMap.h"
#include "Templates/Array.h"
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






















