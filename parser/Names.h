// Copyright (c) 2018 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "NameID.h"
#include "Templates/Array.h"
#include "Templates/HashMap.h"


class Names
{
	struct Chunk
	{
		Chunk* prev;
		char   data[8 * 1024 - sizeof(Chunk*)];
		explicit Chunk(Chunk* p) : prev(p) {}
		~Chunk() { delete prev; }
	};
	Chunk* pool;
	char*  pool_ptr;
	char*  pool_end;

	HashMap<cstr, NameID> hashmap;

	Names(const Names&)			   = delete;
	Names& operator=(const Names&) = delete;

public:
	Names();
	~Names();
	Names(Names&& q);
	Names& operator=(Names&& q);

	NameID add(cstr name); // add name if n.ex.

	NameID get(cstr name) const { return hashmap.get(name); }	  // must exist
	cstr   get(NameID id) const { return hashmap.getKeys()[id]; } // must exist

	NameID operator[](cstr name) const { return hashmap.get(name); }	 // must exist
	cstr   operator[](NameID id) const { return hashmap.getKeys()[id]; } // must exist

	Array<NameID> merge(const Names&);
	void		  purge();

	void serialize(class FD&) const;
	void deserialize(class FD&);
};
