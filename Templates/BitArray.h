// Copyright (c) 2023 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Array.h"


class BitRef
{
	uint8& p;
	uint   i;

public:
	BitRef(uint8* p, uint i) noexcept : p(*p), i(i) {}
	BitRef(uint8& p, uint i) noexcept : p(p), i(i) {}

	operator bool() const noexcept { return (p >> i) & 1; }

	void operator=(bool bit) noexcept { p = uint8((p & ~(1 << i)) | bit << i); }
	void operator|=(bool bit) noexcept { p |= bit << i; }
	void operator&=(bool bit) noexcept { p &= ~(bit << i); }
	void operator^=(bool bit) noexcept { p ^= bit << i; }
	void set() noexcept { p |= 1 << i; }
	void res() noexcept { p &= 1 << i; }
	void toggle() noexcept { p ^= 1 << i; }
};

using BitArray = Array<bool>;

template<>
class Array<bool> : private Array<uint8>
{
	using Super = Array<uint8>;

	static bool _get(const uint8* data, uint i) noexcept { return (data[i >> 3] >> (i & 7)) & 1; }
	static void _set(uint8* data, uint i) noexcept { data[i >> 3] |= 1 << (i & 7); }
	static void _xor(uint8* data, uint i) noexcept { data[i >> 3] ^= 1 << (i & 7); }
	static void _res(uint8* data, uint i) noexcept { data[i >> 3] &= ~(1 << (i & 7)); }
	static void _setbit(uint8& data, uint i, bool bit) noexcept { data = uint8((data & ~(1 << i)) | bit << i); }
	static void _set(uint8* data, uint i, bool bit) noexcept { _setbit(data[i >> 3], i & 7, bit); }

public:
	~Array() noexcept		  = default;
	Array() noexcept		  = default;
	Array(Array&& q) noexcept = default;
	Array(const Array& q) throws; // TODO
	Array& operator=(Array&& q) noexcept	= default;
	Array& operator=(const Array& q) throws = default;
	explicit Array(uint cnt, uint max = 0) throws;	// TODO
	Array copyofrange(uint a, uint e) const throws; // TODO

	// access data members:
	uint		 count() const noexcept { return cnt; }
	const uint8* getData() const noexcept { return data; }
	uint8*		 getData() noexcept { return data; }

	bool operator[](uint i) const noexcept
	{
		assert(i < cnt);
		return _get(data, i);
	}
	BitRef operator[](uint i) noexcept
	{
		assert(i < cnt);
		return BitRef(data, i);
	}
	bool get(uint i) const noexcept
	{
		assert(i < cnt);
		return _get(data, i);
	}
	void set(uint i) noexcept
	{
		assert(i < cnt);
		_set(data, i);
	}
	void res(uint i) noexcept
	{
		assert(i < cnt);
		_res(data, i);
	}
	void set(uint i, bool bit) noexcept
	{
		assert(i < cnt);
		_set(data, i, bit);
	}

	bool first() const noexcept { return get(0); }
	bool last() const noexcept { return get(cnt - 1); }
	//BitRef first() noexcept { return operator[](0); }
	//BitRef last() noexcept { return operator[](cnt - 1); }

	bool operator==(const Array& q) const noexcept; // TODO
	bool operator!=(const Array& q) const noexcept; // TODO
	bool operator<(const Array& q) const noexcept;	// TODO
	bool operator>(const Array& q) const noexcept;	// TODO
	bool operator>=(const Array& q) const noexcept { return !operator<(q); }
	bool operator<=(const Array& q) const noexcept { return !operator>(q); }

	// resize:
	void growmax(uint newmax) throws;			// TODO
	void grow(uint newcnt, uint newmax) throws; // TODO
	void grow(uint newcnt) throws;				// TODO
	void shrink(uint newcnt) noexcept;			// TODO
	void resize(uint newcnt) throws
	{
		grow(newcnt);
		shrink(newcnt);
	}
	void drop() noexcept
	{
		assert(cnt);
		cnt -= 1;
	}
	bool pop() noexcept
	{
		assert(cnt);
		return _get(data, --cnt);
	}
	void purge() noexcept
	{
		max = cnt = 0;
		delete[] data;
		data = nullptr;
	}
	void append(bool bit) throws
	{
		growmax(cnt + 1);
		_set(data, cnt++, bit);
	}
	Array& operator<<(bool bit) throws
	{
		append(bit);
		return *this;
	}
	void append(const uint8* q, uint n) throws
	{
		growmax(cnt + n);
		for (uint i = 0; i < n; i++) { _set(data, cnt + i, _get(q, i)); }
		cnt += n;
	}
	void append(const Array& q) throws
	{
		assert(this != &q);
		append(q.data, q.cnt);
	}

	void removeat(uint idx) noexcept;		   //TODO
	void removerange(uint a, uint e) noexcept; //TODO
	void remove(uint idx) noexcept { removeat(idx); }

	void insertat(uint idx, bool) throws;					//TODO
	void insertat(uint idx, const uint8* q, uint n) throws; //TODO
	void insertat(uint idx, const Array&) throws;			//TODO
	void insertrange(uint a, uint e) throws;				//TODO

	void revert(uint a, uint e) noexcept;  // revert items in range [a..[e //TODO
	void rol(uint a, uint e) noexcept;	   // roll left  range  [a..[e //TODO
	void ror(uint a, uint e) noexcept;	   // roll right range  [a..[e //TODO
	void shuffle(uint a, uint e) noexcept; // shuffle range [a..[e //TODO

	void revert() noexcept { revert(0, cnt); }
	void rol() noexcept { rol(0, cnt); }
	void ror() noexcept { ror(0, cnt); }
	void shuffle() noexcept { shuffle(0, cnt); }

	void swap(uint i, uint j) noexcept
	{
		assert(i < cnt && j < cnt);
		if (_get(data, i) != _get(data, j))
		{
			_xor(data, i);
			_xor(data, j);
		}
	}
};


/*































*/
