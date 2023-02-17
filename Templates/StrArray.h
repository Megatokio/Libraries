#pragma once
// Copyright (c) 2015 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Array.h"


// ___________________________________________________________
// array of c-strings allocated with new/delete:


class StrArray : protected Array<str>
{
	typedef Array<str> SUPER;

protected:
	void release(uint i) noexcept { delete[] data[i]; }
	void release(uint a, uint e) noexcept
	{
		while (a < e) release(a++);
	}
	void release_all() noexcept { release(0, cnt); }
	void retain(uint i) throws { data[i] = newcopy(data[i]); }
	void retain(uint a, uint e) throws
	{
		while (a < e) retain(a++);
	}
	void retain_all() throws { retain(0, cnt); }

public:
	// see https://stackoverflow.com/questions/11562/how-to-overload-stdswap
	static void swap(StrArray& a, StrArray& b) noexcept { Array<str>::swap(a, b); }

	virtual ~StrArray() noexcept { release_all(); }
	StrArray() noexcept : SUPER() {}
	StrArray(StrArray&& q) noexcept : SUPER(std::move(q)) { assert(q.data == nullptr); }
	explicit StrArray(const StrArray& q) throws : SUPER(q) { retain_all(); }
	explicit StrArray(const Array<str>& q) throws : SUPER(q) { retain_all(); }
	explicit StrArray(const Array<cstr>& q) throws : StrArray(reinterpret_cast<const Array<str>&>(q)) {}
	StrArray& operator=(StrArray&& q) noexcept
	{
		swap(*this, q);
		return *this;
	}
	StrArray& operator=(const StrArray& q) throws { return *this = StrArray(q); }
	StrArray& operator=(const Array<str>& q) throws { return *this = StrArray(q); }
	StrArray& operator=(const Array<cstr>& q) throws { return *this = StrArray(q); }
	explicit StrArray(uint cnt, uint max = 0) throws : SUPER(cnt, max) {}
	StrArray(str* q, uint cnt) throws : SUPER(q, cnt) { retain_all(); }
	StrArray(cstr* q, uint cnt) throws : StrArray(const_cast<str*>(q), cnt) {}

	StrArray copyofrange(uint a, uint e) const throws;

	// access data members:
	using SUPER::count;
	using SUPER::data;
	using SUPER::getData;
	using SUPER::operator[];
	using SUPER::first;
	using SUPER::last;
	//	uint		count   	() const		noexcept { return cnt; }
	//	str const*	getData		() const		noexcept { return data; }
	//	str*		getData		()				noexcept { return data; }
	//	str const&	operator[]	(uint i) const	asserts  { assert(i<cnt); return data[i]; }
	//	str&		operator[]	(uint i)		asserts  { assert(i<cnt); return data[i]; }
	//	str const&	first		() const		asserts  { assert(cnt); return data[0]; }
	//	str&		first		()				asserts  { assert(cnt); return data[0]; }
	//	str const&	last		() const		asserts  { assert(cnt); return data[cnt-1]; }
	//	str&		last		()				asserts  { assert(cnt); return data[cnt-1]; }

	using SUPER::operator==; // compare values
	using SUPER::operator!=; // compare values
	bool operator==(const Array<cstr>& q) const noexcept { return operator==(reinterpret_cast<const SUPER&>(q)); }
	bool operator!=(const Array<cstr>& q) const noexcept { return operator!=(reinterpret_cast<const SUPER&>(q)); }
	bool operator==(const StrArray& q) const noexcept { return SUPER::operator==(q); }	 // compare values
	bool operator!=(const StrArray& q) const noexcept { return SUPER::operator!=(q); }	 // compare values
	bool contains(cstr s) const noexcept { return SUPER::contains(const_cast<str>(s)); } // compare values
	uint indexof(cstr s) const noexcept { return SUPER::indexof(const_cast<str>(s)); }	 // compare values

	// resize:
	using SUPER::grow;
	//str&	grow        ()					throws;
	//void	grow		(uint newcnt)		throws;
	//void	grow		(uint cnt, uint max) throws;
	void shrink(uint newcnt) noexcept
	{
		release(newcnt, cnt);
		SUPER::shrink(newcnt);
	}
	void resize(uint newcnt) throws
	{
		shrink(newcnt);
		grow(newcnt);
	}
	void drop() noexcept
	{
		assert(cnt);
		release(--cnt);
	}
	str pop() noexcept
	{
		assert(cnt);
		return data[--cnt];
	} // returns ownership
	void purge() noexcept
	{
		release_all();
		SUPER::purge();
	}
	str	 append(cstr q) throws { return grow() = newcopy(q); }
	str	 append(str q) throws { return grow() = newcopy(q); }
	void appendifnew(cstr q) throws
	{
		if (!contains(q)) append(q);
	} // compares values
	StrArray& operator<<(cstr q) throws
	{
		append(q);
		return *this;
	}
	StrArray& operator<<(int n) throws { return operator<<(tostr(n)); } // for StrArray.test.cpp
	void	  append(str* q, uint n) throws { append(StrArray(q, n)); }
	void	  append(cstr* q, uint n) throws { append(StrArray(q, n)); }
	void	  append(StrArray&& q) throws
	{
		SUPER::append(q);
		q.cnt = 0;
	}
	void append(const StrArray& q) throws { append(StrArray(q)); }

	void remove(cstr item, bool fast = 0) noexcept { removeitem(item, fast); }
	void remove(uint idx, bool fast = 0) noexcept { removeat(idx, fast); }

	void removeat(uint i, bool fast = 0) noexcept
	{
		assert(i < cnt);
		release(i);
		SUPER::removeat(i, fast);
	}
	void removerange(uint a, uint e) noexcept;
	void removeitem(cstr s, bool fast = 0) noexcept
	{
		uint i = indexof(s);
		if (i != ~0u) removeat(i, fast);
	} // remove by value

	void insertat(uint idx, cstr s) throws;
	void insertat(uint idx, const cstr* q, uint n) throws;
	void insertat(uint idx, const StrArray&) throws;
	void insertat(uint idx, StrArray&&) throws;
	void insertrange(uint a, uint e) throws { SUPER::insertrange(a, e); }
	void insertsorted(cstr s) throws; // uses gt()

	using SUPER::revert;
	using SUPER::rol;
	using SUPER::ror;
	using SUPER::rsort;
	using SUPER::shuffle;
	using SUPER::sort;
	//void 	revert	(uint a, uint e)		noexcept { SUPER::revert(a,e); }
	//void 	rol		(uint a, uint e)		noexcept { SUPER::rol(a,e); }
	//void 	ror		(uint a, uint e)		noexcept { SUPER::ror(a,e); }
	//void 	shuffle	(uint a, uint e)		noexcept { SUPER::shuffle(a,e); }
	//void 	sort	(uint a, uint e)		noexcept { SUPER::sort(a,e); }
	//void 	rsort	(uint a, uint e)		noexcept { SUPER::rsort(a,e); }
	//void 	sort	(uint a, uint e, bool(*gt)(str,str)) noexcept { SUPER::sort(a,e,gt); }
	//void 	revert	()						noexcept { SUPER::revert(0,cnt); }
	//void 	rol		()						noexcept { SUPER::rol(0,cnt); }
	//void 	ror		()						noexcept { SUPER::ror(0,cnt); }
	//void 	shuffle	()						noexcept { SUPER::shuffle(0,cnt); }
	//void 	sort	()						noexcept { SUPER::sort(0,cnt); }
	//void 	rsort	()						noexcept { SUPER::rsort(0,cnt); }
	//void 	sort	(bool(*gt)(str,str))	noexcept { SUPER::sort(0,cnt,gt); }

	using SUPER::print;
	using SUPER::serialize;
	//void	print (FD&, cstr indent) const throws;
	//void	serialize	(FD&) const throws;
	void deserialize(FD&) throws;
};


inline str tostr(const StrArray& array)
{
	// return 1-line description of object for debugging and logging:
	return usingstr("StrArray[%u]", array.count());
}


inline StrArray StrArray::copyofrange(uint a, uint e) const throws
{
	// create a copy of a range of data of this

	if (e > cnt) e = cnt;
	if (a >= e) return StrArray();

	return StrArray(data + a, e - a);
}

inline void StrArray::removerange(uint a, uint e) noexcept
{
	// remove range of data

	if (e > cnt) e = cnt;
	if (a >= e) return;

	release(a, e);
	memmove(a, e, cnt - e);
	cnt -= e - a;
}

inline void StrArray::insertat(uint idx, cstr s) throws
{
	// insert item at index
	// idx ≤ cnt

	assert(idx <= cnt);

	grow();
	memmove(idx + 1, idx, cnt - idx - 1);
	data[idx] = newcopy(s);
}

inline void StrArray::insertat(uint idx, const cstr* q, uint n) throws
{
	// insert source array at index
	// idx ≤ cnt

	SUPER::insertat(idx, const_cast<str*>(q), n);
	retain(idx, idx + n);
}

inline void StrArray::insertat(uint idx, const StrArray& q) throws
{
	// insert source array at index
	// idx ≤ cnt

	assert(this != &q);

	SUPER::insertat(idx, q);
	retain(idx, idx + q.cnt);
}

inline void StrArray::insertat(uint idx, StrArray&& q) throws
{
	// insert source array at index
	// idx ≤ cnt

	assert(this != &q);

	SUPER::insertat(idx, q);
	q.cnt = 0;
}

inline void StrArray::insertsorted(cstr q) throws
{
	uint i = cnt;
	while (i-- && gt(data[i], q)) {}
	insertat(i + 1, q);
}

inline void StrArray::deserialize(FD& fd) throws
{
	SUPER::deserialize(fd);
	retain_all();
}
