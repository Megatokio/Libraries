// Copyright (c) 2018 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "NameID.h"
#include "Templates/Array.h"
#include "Templates/HashMap.h"


namespace kio
{

static constexpr char data_error[] = "data error in file";

class Names
{
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

	void serialize(class SerialDevice*, void* = nullptr) const; // use Names.cpp
	void deserialize(class SerialDevice*, void* = nullptr);

	void serialize(class FD&, void* = nullptr) const; // use Names_FD.cpp
	void deserialize(class FD&, void* = nullptr);

private:
	struct Chunk* pool	   = nullptr;
	char*		  pool_ptr = nullptr;
	char*		  pool_end = nullptr;

	HashMap<cstr, NameID> hashmap;
};

} // namespace kio
