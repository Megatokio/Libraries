// Copyright (c) 2018 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Names.h"


/*
	class Names provides uint identifier IDs for strings.
	get(cstr) returns the ID and
	get(uint) returns the string.

	Intended use: for compilers.
	If compiler instances can run in parallel (to compile a bunch of files),
	then each instance must have it's own Names. (else each lookup must be wrapped with a mutex.)
	When linking 2 files, their Names must be merged, which will assign new IDs to the added names.
*/


// standard names as defined in NameID.h:
static constexpr cstr stdnames[]
#define M(A, B) B
#include "NameID.h"
	static constexpr uint32 num_stdnames = NELEM(stdnames);
static_assert(num_stdnames > 0 && *stdnames[0] == 0, ""); // first name must be the empty string


Names::Names() : pool(nullptr), pool_ptr(nullptr), pool_end(nullptr)
{
	// setup hashmap with standard names
	// => standard names have fixed IDs as enumerated in "NameID.h"
	// standard names (the keys) are not copied into the mempool
	// names are never removed from hashmap
	// => position in hashmap.items[] / hashmap.keys[] won't change
	// => names for IDs can be looked up with hashmap.keys[ID]
	// IDs for names are looked up with hashmap.get() / hashmap.add()

	for (uint32 id = 0; id < num_stdnames; id++) { hashmap.add(stdnames[id], NameID(id)); }

	assert(hashmap.count() == num_stdnames); // assert: no idf added twice
}

Names::~Names() { delete pool; }

Names::Names(Names&& q) : pool(q.pool), pool_ptr(q.pool_ptr), pool_end(q.pool_end), hashmap(std::move(q.hashmap))
{
	pool	 = nullptr;
	pool_ptr = pool_end = nullptr; // safety only
}

Names& Names::operator=(Names&& q)
{
	assert(this != &q);
	this->~Names();
	new (this) Names(std::move(q));
	return *this;
}

void Names::purge()
{
	if (hashmap.count() == num_stdnames) return;
	this->~Names();
	new (this) Names();
}

NameID Names::add(cstr name)
{
	// add name if not in list
	// return ID

	const uint32 mapsize = hashmap.count();
	NameID		 id		 = hashmap.get(name, NameID(mapsize));
	if (id != mapsize) return id; // name found

	// not found => add to map:

	size_t slen = strlen(name) + 1;
	if (pool_ptr + slen > pool_end)
	{
		assert(slen <= sizeof(pool->data));

		pool	 = new Chunk(pool);
		pool_ptr = pool->data;
		pool_end = pool->data + sizeof(pool->data);
	}

	memcpy(pool_ptr, name, slen);
	hashmap.add(pool_ptr, id);
	pool_ptr += slen;

	return id;
}

Array<NameID> Names::merge(const Names& q)
{
	// append names from another Names map.
	// returns array with old-to-new ID mappings.

	Array<NameID> map(q.hashmap.count());

	for (uint32 id = 0; id < map.count(); id++) { map[id] = add(q.get(NameID(id))); }

	return map;
}

#define MAGIC 0x2355

void Names::serialize(FD& fd) const
{
	uint32 a = num_stdnames;
	uint32 e = hashmap.count();

	fd.write_uint16(MAGIC);
	fd.write_uint32(a);
	fd.write_uint32(e);

	for (uint32 id = a; id < e; id++) { fd.write_nstr(get(NameID(id))); }
}

void Names::deserialize(FD& fd)
{
	uint32 m = fd.read_uint16();
	uint32 a = fd.read_uint32();
	uint32 e = fd.read_uint32();

	if (m != MAGIC) throw DataError("Names: wrong MAGIC");
	if (a != num_stdnames) throw DataError("Names: wrong num_stdnames");

	purge();

	for (uint32 id = a; id < e; id++) { add(fd.read_nstr()); }
}
