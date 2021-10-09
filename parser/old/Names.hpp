#pragma once
// Copyright (c) 2018 - 2021 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

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






















