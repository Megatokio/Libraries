#pragma once
// Copyright (c) 2018 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Array.h"
#include "RCPtr.h"


// _________________________________________________________
// convenience subclasses for Array and HashMap using RCPtr:

template<class T>
class RCArray : public Array<RCPtr<T>>
{
protected:
	using Array<RCPtr<T>>::cnt;
	using Array<RCPtr<T>>::data;

public:
	explicit RCArray() throws {}
	explicit RCArray(uint cnt, uint max = 0) throws : Array<RCPtr<T>>(cnt, max) {}

	uint indexof(const T* item) const noexcept // find first occurance or return ~0u
	{										   // compares object addresses (pointers)
		for (uint i = 0; i < cnt; i++)
		{
			if (data[i].p == item) return i;
		}
		return ~0u;
	}

	bool contains(const T* item) const noexcept { return indexof(item) != ~0u; }
	void removeitem(const T* item, bool fast = 0) noexcept
	{
		uint i = indexof(item);
		if (i != ~0u) remove(i, fast);
	}

	void remove(const T* item, bool fast = 0) noexcept { removeitem(item, fast); }
	void remove(uint idx, bool fast = 0) noexcept { this->removeat(idx, fast); }

	using Array<RCPtr<T>>::append;
	RCPtr<T>& append(T* q) throws { return RCArray<T>::append(RCPtr<T>(q)); }

	using Array<RCPtr<T>>::operator<<;
	RCArray&			   operator<<(T* q) throws
	{
		append(q);
		return *this;
	}
};


// TODO: HashMap.h must be included first
template<class KEY, class ITEM>
class HashMap;
template<class KEY, class T>
class RCHashMap : public HashMap<KEY, RCPtr<T>>
{
public:
	// default: for up to 1024 items without resizing
	explicit RCHashMap(uint max = 1 << 10) throws : HashMap<KEY, RCPtr<T>>(max) {}
};
