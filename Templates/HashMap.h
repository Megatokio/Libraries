#pragma once
/*	Copyright  (c)	Günter Woigk 2014 - 2019
					mailto:kio@little-bat.de

	This file is free software

 	This program is distributed in the hope that it will be useful,
 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	• Redistributions of source code must retain the above copyright notice,
	  this list of conditions and the following disclaimer.
	• Redistributions in binary form must reproduce the above copyright notice,
	  this list of conditions and the following disclaimer in the documentation
	  and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
	OR BUSINESS IDXERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
	WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
	OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
	ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "kio/kio.h"
#include "Templates/Array.h"
#include "hash/sdbm_hash.h"
#include "kio/util/msbit.h"


/*	Template class HashMap stores Objects with Keys.

	Keys (Names) must be unique.
	The HashMap retains ownership of the Objects.

	Keys are stored as flat copy. (e.g. c-strings or objects are not cloned.)
	The passed keys are stored internally and must remain valid throughout lifetime of the HashMap.
	This may be accomplished by using Objects with a data member 'name' and use this 'name' as key.

	The HashMap creator should be passed the expected size of the final map.
	If the map grows larger then it will be resized on the fly which will take some time.
	Better be generous because a larger hashmap results in less collisions and therefore faster access;
	the fixed costs are hashmap.count() * sizeof(int).


	in map[] sind die indizes der zugehörigen daten in keys/items gespeichert.
	wg. uneindeutigkeit muss bei zugriff immer noch der key aus keys[] verglichen werden.
	der hashwert gibt den startpunkt in map[] an, ab dem nach den zugehörigen daten gesucht werden kann
	ist der erste wert falsch, wird mit dem nächsten eintrag in map[] weiterprobiert
	bis ein endmarker erreicht wurde. dann: n.ex.
	endmarker ist gesetztes bit31 auf einem index.
	unbenutzte slots in map[] haben den wert FREE und haben auch bit31 gesetzt.
	wird ein eintrag gelöscht, wird der index auf FREE gesetzt, bit31 bleibt aber erhalten.
	ggf. wird bit31 im vor-index ebenfalls gesetzt. (s.u.)


	template arguments:

		class KEY		must be a flat type for HashMap. Keys are compared with eq().
		class ITEM		must be a flat type for HashMap.
*/


#ifndef ArrayMAX
#define ArrayMAX	0x40000000u		/* max size  ((not count)) */
#endif


namespace kio
{
	inline uint hash (cstr key)			{ return sdbm_hash(key); }

	inline uint hash (uint32 key)		{ return key ^ (key>>16); }
	inline uint hash (int32 key)		{ return hash(uint32(key)); }

	inline uint hash (uint64 key)		{ return hash(uint32(key) ^ uint32(key>>32)); }
	inline uint hash (int64 key)		{ return hash(uint64(key)); }

//	inline uint hash (uint key)			{ return key ^ (key>>16); }
//	inline uint hash (int key)			{ return hash(uint32(key)); }

#ifndef _LINUX
#if _sizeof_long == 8
	inline uint hash (ulong key)		{ return hash(uint64(key)); }
	inline uint hash (long key)			{ return hash(uint64(key)); }
#elif _sizeof_long == 4
	inline uint hash (ulong key)		{ return hash(uint32(key)); }
	inline uint hash (long key)			{ return hash(uint32(key)); }
#else
	#error
#endif
#endif

	// note: uint  == uint32
	// but:  ulong != uint64
	// and:  std::size_t == ulong != uint64
	// without the definition of hash(ulong) the compiler cannot resolve a call to hash(ulong)   :-(
	inline uint hash (void const* key)	{ return hash(std::size_t(key)); }
}

template<class KEY, class ITEM>
class HashMap
{
private:
	Array<ITEM>	items;			// stored items
	Array<KEY>	keys;			// their keys
	int*		map;			// hash -> index conversion array
	uint		mask;			// map size -1; map size must be 1<<N

	static constexpr int BIT31 = INT_MIN;	// 0x80000000 mask for 'end-of-thread' marker bit
	static constexpr int FREE  = -1;		// 			  value for free slots in map[] (BIT31 set)
	static constexpr uint16 MAGIC = 0x9C0A;
	static constexpr uint16 BYTESWAPPED_MAGIC = 0x0A9C;

private:
	void	clearmap  ()					noexcept { memset(&map[0],-1,(mask+1)*sizeof(map[0])); }
	int		indexof	  (KEY) const			noexcept; // find index in items[]; -1 if not found
	void	resizemap (uint)				throws;

public:
	static constexpr uint maxCount1 = Array<ITEM>::maxCount;
	static constexpr uint maxCount2 = Array<KEY>::maxCount;
	static constexpr uint maxCount  = maxCount1<maxCount2?maxCount1:maxCount2;

	// see https://stackoverflow.com/questions/11562/how-to-overload-stdswap
	static void swap (HashMap& a, HashMap& b) noexcept;

	explicit HashMap (uint max=1<<10)	throws;		// default: for up to 1024 items without resizing
	explicit HashMap (HashMap const&)	throws;
	HashMap (HashMap&&)					noexcept;
	~HashMap ()							noexcept { delete[] map; }
	HashMap& operator= (HashMap&&)		noexcept;
	HashMap& operator= (HashMap const&)	throws;

// get internal data:
	uint		getMapSize () const			noexcept { return mask+1; }
	int const*	getMap	   () const			noexcept { return map; }

	Array<KEY> const&  getKeys	() const	noexcept { return keys; }
	Array<ITEM>&	   getItems	()			noexcept { return items; }
	Array<ITEM> const& getItems	() const	noexcept { return items; }

// get items:
	uint		count		() const		noexcept { return items.count(); }
	bool		contains	(KEY key) const	noexcept { return indexof(key) != -1; }	// uses KEY::eq()
	ITEM		get			(KEY key, ITEM dflt) const noexcept;						// uses KEY::eq()
	ITEM&		get			(KEY key)		noexcept { int i=indexof(key); assert(i!=-1); return items[i]; } // uses KEY::eq()
	ITEM const&	get			(KEY key) const	noexcept { int i=indexof(key); assert(i!=-1); return items[i]; } // uses KEY::eq()
	ITEM&		operator[]	(KEY key)		noexcept { int i=indexof(key); assert(i!=-1); return items[i]; } // uses KEY::eq()
	ITEM const&	operator[]	(KEY key) const	noexcept { int i=indexof(key); assert(i!=-1); return items[i]; } // uses KEY::eq()

// add / remove items:
	void		purge		()				noexcept;
	HashMap&	add			(KEY, ITEM)		throws;		// overwrites if key already exists
	void		remove		(KEY)			noexcept;	// silently does nothing if key does not exist

// misc:
	bool operator== (HashMap const& q) const noexcept;							// uses KEY::eq() and ITEM::ne()
	bool operator!= (HashMap const& q) const noexcept { return !operator==(q); }	// uses KEY::eq() and ITEM::ne()

// read / write file:
	void print		(FD&, cstr indent)	const throws;
	void serialize	(FD&) const			throws;
	void deserialize (FD&)				throws;
};



// -----------------------------------------------------------------------
//				   	I M P L E M E N T A T I O N S
// -----------------------------------------------------------------------


template<class KEY,class ITEM>
inline str tostr(HashMap<KEY,ITEM> const& hashmap)
{
	// return 1-line description of hashmap for debugging and logging:
	return usingstr("HashMap[%u]", hashmap.count());
}


template<class KEY,class ITEM>
inline void HashMap<KEY,ITEM>::swap (HashMap<KEY,ITEM>& a, HashMap<KEY,ITEM>& b) noexcept
{
	std::swap(a.items,b.items);
	std::swap(a.keys,b.keys);
	std::swap(a.map,b.map);
	std::swap(a.mask,b.mask);
}

template<class KEY,class ITEM>
HashMap<KEY,ITEM>::HashMap (uint max) throws
:
	items(),
	keys(),
	map(nullptr),
	mask(0)
{
	// create HashMap with preallocated items[] and initial map[] size
	// there will be no reallocation of items[] and  no reallocation and reindexing of map[] up to max items
	// note: if int==int16 then maximum for max is 0x7FFE (32766)

	assert(max>0);
	assert(max<=maxCount);

	uint mapsize = 4u << msbit(max-1);	// mapsize = 2 * max!
	map = new int[mapsize];
	mask = mapsize-1;
	clearmap();

	items.grow(0,max);
	keys.grow(0,max);
}

template<class KEY,class ITEM>
HashMap<KEY,ITEM>::HashMap (HashMap&& q) noexcept
:
	items(std::move(q.items)),
	keys(std::move(q.keys)),
	map(q.map),
	mask(q.mask)
{
	q.mask = 0;
	q.map = nullptr;
	q.map = new int[1];		// size = mask+1 => can't be = 0
	q.map[0] = FREE;		// q.clearmap();
}

template<class KEY,class ITEM>
HashMap<KEY,ITEM>::HashMap (HashMap const& q) throws
:
	items(),
	keys(),
	map(nullptr),
	mask(0)
{
	items = q.items;
	keys = q.keys;
	map = new int[q.mask+1];
	mask = q.mask;
	memcpy (&map[0], &q.map[0], (mask+1)*sizeof(map[0]));
}

template<class KEY,class ITEM>
HashMap<KEY,ITEM>& HashMap<KEY,ITEM>::operator= (HashMap&& q) noexcept
{
	swap (*this, q);
	return *this;
}

template<class KEY,class ITEM>
HashMap<KEY,ITEM>& HashMap<KEY,ITEM>::operator= (HashMap const& q) throws
{
	if (this != &q)
	{
		this->~HashMap();
		new(this) HashMap(q);
	}
	return *this;
}

template<class KEY,class ITEM>
inline ITEM HashMap<KEY,ITEM>::get (KEY key, ITEM dflt) const noexcept
{
	int idx = indexof(key);
	return idx == -1 ? std::move(dflt) : items[idx];
}

template<class KEY,class ITEM>
void HashMap<KEY,ITEM>::purge() noexcept
{
	// clear HashMap
	// the map[] is not resized
	// but all entries are cleared to FREE

	items.purge();
	keys.purge();
	clearmap();
}

template<class KEY,class ITEM>
void HashMap<KEY,ITEM>::resizemap(uint newsize) throws
{
	xlogline("HashMap: grow map to %i",newsize);

	assert(newsize >= items.count());
	assert(newsize == 1 << msbit(newsize));
	assert(newsize <= min(uint(BIT31), uint(ArrayMAX/sizeof(map[0]))));	// also catches size=0

	mask = newsize-1;

// allocate and clear map:
	int* newmap = new int[newsize];
	delete[] map;
	map = newmap;
	clearmap();

// put all items back into map:
	for(uint idx=0,e=items.count(); idx<e; idx++)
	{
		uint i = kio::hash(keys[idx]);
		while(map[i&mask] != FREE)
			map[i++&mask] &= ~BIT31;	// clear end-of-thread marker on this index
		map[i&mask] = idx + BIT31;		// store index, set end-of-thread marker
	}
}

template<class KEY,class ITEM>
int HashMap<KEY,ITEM>::indexof(KEY key) const noexcept
{
	// search for key
	// returns index in items[] or -1

	uint i = kio::hash(key);
	int idx = map[i&mask];
	if(idx==FREE) return -1;

	for(;;)
	{
		bool fin = idx<0;
		idx &= ~BIT31;
		if(eq(keys[idx],key)) return idx;	// found
		if(fin) return -1;					// end of thread => not found
		idx = map[++i&mask];
		assert(idx!=FREE);
	}
}

template<class KEY,class ITEM>
HashMap<KEY,ITEM>& HashMap<KEY,ITEM>::add(KEY key, ITEM item) throws
{
	// add item for key
	// if key alredy exists, then overwrite

a:	uint mask = this->mask;					// for rapid access
	uint i = kio::hash(key);						// i = index in map[]
	int  idx = map[i&mask];					// idx = index in items[]
	if(idx==FREE) goto b;					// map[i] is free => quick action!

	// search for existing key:
	for(;;)
	{
		bool fin = idx<0;
		idx &= ~BIT31;
		if(eq(keys[idx],key)) 				// key exists => overwrite & exit:
		{
			items[idx] = std::move(item);	// overwrite item at idx
			keys[idx] = key;				// also overwrite key, if KEY==cstr then the key may be kept alive by it's item
			return *this;
		}
		if(fin) break;						// no more chances: key does not yet exist
		idx = map[++i&mask];				// inspect next map[i] / items[idx]
		assert(idx!=FREE);					// BIT31 must be set on last entry of thread: we can't run into a free slot
	}

	// check whether it's time to grow the map[]:
	if(items.count()*2 > mask)
	{
		resizemap(mask*2+2);
		goto a;
	}

	// find free slot in map[]
	// and clear end-of-thread markers until free slot found:
	do { map[i&mask] &= ~BIT31; } while(map[++i&mask] != FREE);

	// map[i] is free
	// get free index in items[]:
	// store index in map[i],
	// store key+item in keys[] and items[]:

	//map[(i-1)&mask] &= ~BIT31;  			// clear end-of-thread marker on previous index
b:	map[i&mask] = items.count() + BIT31;	// store index, set end-of-thread marker
	items.append(std::move(item));			// store item at index
	keys.append(key);						// store key at index

	return *this;
}

template<class KEY,class ITEM>
void HashMap<KEY,ITEM>::remove(KEY key) noexcept
{
	// remove key
	// silently ignores if key does not exist

	uint mask = this->mask;					// for rapid access
	bool fin;
	int idx;

	uint i = kio::hash(key);						// i = index in map[]
	idx = map[i&mask];						// idx = index in items[]
	if(idx==FREE) return;					// not found

	for(;;)
	{
		fin = idx<0;						// end-of-thread marker
		idx &= ~BIT31;						// real index
		if(eq(keys[idx],key)) break;		// item found at map[i] / items[idx]
		if(fin) return;						// end of thread => not found
		idx = map[++i&mask];				// next i / idx
	}

	// item has been found at map[i] / items[idx]:
	// fin indicates that map[i] is at the end of the thread

	// move items.last() into gap:
	int idx2 = items.count()-1;
	if(idx != idx2)
	{
		// find index i2 in map[] for moved item:
		uint i2 = kio::hash(keys[idx2]);			// i2 = index in map[]
		for(;;++i2)
		{
			//assert(map[i2&mask] != FREE);
			if((map[i2&mask] & ~BIT31) == idx2) break;
			assert(map[i2&mask] >= 0);		// must exist
		}

		// move item and point map[i2] to new location:
		items[idx] = std::move(items[idx2]);
		keys[idx]  = keys[idx2];
		map[i2&mask] += idx - idx2;			// keep bit31
	}
	items.drop();
	keys.drop();

	// map[i] is a free slot
	// fin tells whether map[i] is at the end of the thread
	// search remainder of thread for entries which should be moved closer to their nominal position in map[]:

	uint j = i;
	while(!fin)								// fin tells whether map[j] is the end of the thread
	{
		idx = map[++j&mask];				// inspect map[j] / items[idx]
		assert(idx!=FREE);
		fin = idx<0;						// update fin for j
		idx &= ~BIT31;

		uint j0 = kio::hash(keys[idx]);			// j0 = nominal position of j in map[]
		if(j==j0) continue;					// on it's nominal position

		if(((j-j0)&mask) >= ((j-i)&mask))	// j0<=i ?
		{
			map[i&mask] = idx;				// move idx from map[j] to map[i], bit31 cleared
			i = j;							// now map[i] is a free slot
		}
	}

	// map[i] is a free slot.
	// after i there is no entry with a nominal position before i
	// => truncate thread here

	map[(i-1)&mask] |= BIT31;				// map[i-1] := end of thread
	map[i&mask] = FREE;						// map[i] := free
}

template<class KEY,class TYPE>
bool HashMap<KEY,TYPE>::operator== (HashMap const& q) const noexcept
{
	if (keys.count() != q.keys.count()) return false;
	for (uint i = keys.count(); i--;)
	{
		int qi = q.indexof(keys[i]);
		if (qi == -1 || ne(items[i],q.items[qi])) return false;
	}
	return true;
}

template<class KEY,class TYPE>
void HashMap<KEY,TYPE>::serialize (FD& fd) const throws
{
	fd.write_uint16_z(MAGIC);
	items.serialize(fd);
	keys.serialize(fd);
}

template<class KEY,class TYPE>
void HashMap<KEY,TYPE>::deserialize (FD& fd) throws
{
	// deserialize: supports reading back on byte swapped host. (if items support this.)
	uint m = fd.read_uint16_z();
	if (m != MAGIC && m != BYTESWAPPED_MAGIC) throw data_error("HashMap<T,U>: wrong magic");

	items.deserialize(fd);
	keys.deserialize(fd);
	if (items.count() != keys.count()) throw data_error("HashMap<T,U>: key/item mismatch");

	uint mapsize = items.count() < 8 ? 16 : 4u << msbit(items.count()-1);	// mapsize = 2 * max!
	resizemap(mapsize);
}


// ____ print() ____

template<typename KEY, typename ITEM>
inline typename std::enable_if<kio::has_print<ITEM>::value,void>::type
/*void*/ print (FD& fd, HashMap<KEY,ITEM>const& hashmap, cstr indent) throws
{
	// pretty print with indentation
	// this function is called by HashMap<K,T>::print() for classes T which implement T::print()

	KEY const* keys = hashmap.getKeys().getData();
	ITEM const* items = hashmap.getItems().getData();

	fd.write_fmt("%sHashMap[%u]\n",indent,hashmap.count());
	indent = catstr("  ",indent);
	for (uint i=0; i<hashmap.count(); i++)
	{
		fd.write_fmt("%s[%2u] [#%8x] %s = ", indent, i, kio::hash(keys[i]), tostr(keys[i]));
		items[i].print(fd,"");
	}
}

template<typename KEY, typename ITEM>
inline typename std::enable_if<!kio::has_print<ITEM>::value,void>::type
/*void*/ print (FD& fd, HashMap<KEY,ITEM>const& hashmap, cstr indent) throws
{
	// pretty print with indentation
	// this function is called by HashMap<K,T>::print() for classes T which don't implement T::print()

	KEY const* keys = hashmap.getKeys().getData();
	ITEM const* items = hashmap.getItems().getData();

	fd.write_fmt("%sHashMap[%u]\n",indent,hashmap.count());
	indent = catstr("  ",indent);
	for (uint i=0; i<hashmap.count(); i++)
	{
		fd.write_fmt("%s[%2u] [#%8x] %s = %s\n", indent, i, kio::hash(keys[i]), tostr(keys[i]), tostr(items[i]));
	}
}

template<typename KEY, typename ITEM>
void HashMap<KEY,ITEM>::print (FD& fd, cstr indent) const throws
{
	// pretty print with indentation
	// this template will find the above print(FD&,HashMap<T,U>const&,cstr)

	::print(fd,*this,indent);
}



































