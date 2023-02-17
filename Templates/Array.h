#pragma once
// Copyright (c) 2010 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"
#include "sort.h"
#include "template_helpers.h"
#include "unix/FD.h"
#include <memory>
#include <stdexcept>
#include <type_traits>

/*	Array for items and cstr

	assumptions:
	items must not have virtual member functions. (must not have a vtable)
	new items are initialized with zero.
	items are moved around in memory with memcpy().
	items which have been moved with the move creator are not destroyed with ~T()
	except this new(), ~T() and swap() are called.
	item's eq(), ne(), gt() should be implemented for operator==() and sort() etc.
	sort() uses gt(T const&,T const&) for classes else gt(T,T)

	operator[] aborts on failed index check!

	specializations for Array<str> and Array<cstr> with allocation in TempMem.
	see StrArray.h for strings with new[] & delete[].
*/

#ifndef ArrayMAX
  #define ArrayMAX 0x40000000u /* max size  ((not count)) */
#endif


// ------------------------------------------------------------
//				template class "Array"
// ------------------------------------------------------------

template<typename T>
class Array
{
protected:
	uint max, cnt;
	T*	 data;

	T*			allocate(uint n) throws;
	void		memmove(uint z, uint q, uint n) noexcept { ::memmove(ptr(data + z), cptr(data + q), n * sizeof(T)); }
	static void memcopy(T* z, const void* q, uint n) noexcept { ::memcpy(ptr(z), cptr(q), n * sizeof(T)); }
	void		memclr(uint z, uint n) noexcept { ::memset(ptr(data + z), 0, n * sizeof(T)); }
	void		deallocate(T* data) noexcept { delete[] ptr(data); }

public:
	static constexpr uint maxCount = ArrayMAX / sizeof(T);

	// see https://stackoverflow.com/questions/11562/how-to-overload-stdswap
	static void swap(Array& a, Array& b) noexcept
	{
		std::swap(a.cnt, b.cnt);
		std::swap(a.max, b.max);
		std::swap(a.data, b.data);
	}

	~Array() noexcept
	{
		for (uint i = 0; i < cnt; i++) data[i].~T();
		deallocate(data);
	}
	Array() noexcept : max(0), cnt(0), data(nullptr) {}
	Array(Array&& q) noexcept : max(q.max), cnt(q.cnt), data(q.data)
	{
		q.max = q.cnt = 0;
		q.data		  = nullptr;
	}
	Array(const Array& q) throws;
	Array& operator=(Array&& q) noexcept
	{
		swap(*this, q);
		return *this;
	}
	Array& operator=(const Array& q) throws { return operator=(Array(q)); }
	explicit Array(uint cnt, uint max = 0) throws;
	Array(const T* q, uint n) throws : Array(0u, n)
	{
		for (uint i = 0; i < n; i++) new (data + i) T(*q++);
		cnt = n;
	}

	Array copyofrange(uint a, uint e) const throws;

	// access data members:
	uint	 count() const noexcept { return cnt; }
	const T* getData() const noexcept { return data; }
	T*		 getData() noexcept { return data; }
	const T& operator[](uint i) const noexcept
	{
		assert(i < cnt);
		return data[i];
	}
	T& operator[](uint i) noexcept
	{
		assert(i < cnt);
		return data[i];
	}
	const T& operator[](int i) const noexcept
	{
		assert(uint(i) < cnt);
		return data[i];
	}
	T& operator[](int i) noexcept
	{
		assert(uint(i) < cnt);
		return data[i];
	}
	const T& first() const noexcept
	{
		assert(cnt);
		return data[0];
	}
	T& first() noexcept
	{
		assert(cnt);
		return data[0];
	}
	const T& last() const noexcept
	{
		assert(cnt);
		return data[cnt - 1];
	}
	T& last() noexcept
	{
		assert(cnt);
		return data[cnt - 1];
	}

	bool operator==(const Array& q) const noexcept; // uses ne()
	bool operator!=(const Array& q) const noexcept; // uses ne()
	bool operator<(const Array& q) const noexcept;	// uses eq() and lt()
	bool operator>(const Array& q) const noexcept;	// uses eq() and gt()
	bool operator>=(const Array& q) const noexcept { return !operator<(q); }
	bool operator<=(const Array& q) const noexcept { return !operator>(q); }

	uint indexof(REForVALUE(T) item) const noexcept; // compare using '==' except str/cstr: 'eq'
	bool contains(REForVALUE(T) item) const noexcept { return indexof(item) != ~0u; } // uses indexof()

	// resize:
	void growmax(uint newmax) throws;
	T&	 grow() throws
	{
		growmax(cnt + 1);
		return *new (&data[cnt++]) T();
	}
	void grow(uint cnt, uint max) throws;
	void grow(uint newcnt) throws;
	void shrink(uint newcnt) noexcept;
	void resize(uint newcnt) throws
	{
		grow(newcnt);
		shrink(newcnt);
	}
	void drop() noexcept
	{
		assert(cnt);
		data[--cnt].~T();
	}
	T pop() noexcept
	{
		assert(cnt);
		return std::move(data[--cnt]);
	}
	void purge() noexcept
	{
		for (uint i = 0; i < cnt; i++) data[i].~T();
		deallocate(data);
		max = cnt = 0;
		data	  = nullptr;
	}
	T& append(T q) throws
	{
		growmax(cnt + 1);
		return *new (&data[cnt++]) T(std::move(q));
	}
	void appendifnew(T q) throws
	{
		if (!contains(q)) append(std::move(q));
	} // uses indexof()
	Array& operator<<(T q) throws
	{
		append(std::move(q));
		return *this;
	}
	void append(const T* q, uint n) throws
	{
		growmax(cnt + n);
		for (uint i = 0; i < n; i++) new (&data[cnt + i]) T(*q++);
		cnt += n;
	}
	void append(const Array& q) throws
	{
		assert(this != &q);
		append(q.data, q.cnt);
	}

	void removeitem(REForVALUE(T) item, bool fast = 0) noexcept // uses indexof()
	{
		uint i = indexof(item);
		if (i != ~0u) removeat(i, fast);
	}
	void removeat(uint idx, bool fast = 0) noexcept;
	void removerange(uint a, uint e) noexcept;

	// define overloaded remove() for item and idx:
	//	 void remove (item, bool);
	//	 void remove (idx, bool);
	// for fundamental types this is ambiguous and cannot even be defined for T=uint
	// we cannot make the definitions optional, based on type T
	//   -> make unusable definitions for fundamental types!
	//   -> make usable definitions for non_fundamental types!
	struct Foo1
	{
		Foo1() = delete;
	};
	struct Foo2
	{
		Foo2() = delete;
	};
	void remove(
		typename kio::select_type<std::is_fundamental<T>::value, Foo1, REForVALUE(T)>::type item,
		bool																				fast = 0) noexcept
	{
		removeitem(item, fast);
	}
	void remove(typename kio::select_type<std::is_fundamental<T>::value, Foo2, uint>::type idx, bool fast = 0) noexcept
	{
		removeat(idx, fast);
	}

	void insertat(uint idx, T) throws;
	void insertat(uint idx, const T* q, uint n) throws;
	void insertat(uint idx, const Array&) throws;
	void insertrange(uint a, uint e) throws;
	void insertsorted(T) throws; // uses gt()

	void revert(uint a, uint e) noexcept;  // revert items in range [a..[e
	void rol(uint a, uint e) noexcept;	   // roll left  range  [a..[e
	void ror(uint a, uint e) noexcept;	   // roll right range  [a..[e
	void shuffle(uint a, uint e) noexcept; // shuffle range [a..[e
	void sort(uint a, uint e) noexcept
	{
		if (e > cnt) e = cnt;
		if (a < e) ::sort(data + a, data + e);
	}
	void rsort(uint a, uint e) noexcept
	{
		if (e > cnt) e = cnt;
		if (a < e) ::rsort(data + a, data + e);
	}
	void sort(uint a, uint e, COMPARATOR(T) gt) noexcept
	{
		if (e > cnt) e = cnt;
		if (a < e) ::sort(data + a, data + e, gt);
	}

	void revert() noexcept { revert(0, cnt); }
	void rol() noexcept { rol(0, cnt); }
	void ror() noexcept { ror(0, cnt); }
	void shuffle() noexcept { shuffle(0, cnt); }
	void sort() noexcept
	{
		if (cnt) ::sort(data, data + cnt);
	} // uses gt()
	void rsort() noexcept
	{
		if (cnt) ::rsort(data, data + cnt);
	} // uses gt()
	void sort(COMPARATOR(T) gt) noexcept
	{
		if (cnt) ::sort(data, data + cnt, gt);
	}

	void swap(uint i, uint j) noexcept
	{
		assert(i < cnt && j < cnt);
		std::swap(data[i], data[j]);
	}

	static const uint16 MAGIC			  = 0x3343;
	static const uint16 BYTESWAPPED_MAGIC = 0x4333;

	void print(FD&, cstr indent) const throws;
	void serialize(FD&, void* data = nullptr) const throws;
	void deserialize(FD&, void* data = nullptr) throws;
};


// -----------------------------------------------------------------------
//					  I M P L E M E N T A T I O N S
// -----------------------------------------------------------------------

template<typename T>
T* Array<T>::allocate(uint n) throws
{
	if (n <= maxCount) return n ? reinterpret_cast<T*>(new char[n * sizeof(T)]) : nullptr;
	throw std::length_error(usingstr("Array::allocate(): new count = %u exceeds maximum of %u", n, maxCount));
}

template<typename T>
Array<T>::Array(const Array& q) throws : Array()
{
	data = allocate(q.cnt);
	for (uint i = 0; i < q.cnt; i++) new (data + i) T(q.data[i]);
	cnt = max = q.cnt;
}

template<typename T>
Array<T>::Array(uint cnt, uint max) throws : Array()
{
	if (max < cnt) max = cnt;
	data = allocate(max);
	memclr(0, cnt);
	this->cnt = cnt;
	this->max = max;
}

template<typename T>
Array<T> Array<T>::copyofrange(uint a, uint e) const throws
{
	// create a copy of a range of data of this

	if (e > cnt) e = cnt;
	if (a >= e) return Array();

	return Array(data + a, e - a);
}

template<typename T>
bool Array<T>::operator==(const Array<T>& q) const noexcept
{
	// compare arrays

	if (cnt != q.cnt) return false;
	for (uint i = cnt; i--;)
	{
		if (ne(data[i], q.data[i])) return false;
	}
	return true;
}

template<typename T>
bool Array<T>::operator!=(const Array<T>& q) const noexcept
{
	// compare arrays

	if (cnt != q.cnt) return true;
	for (uint i = cnt; i--;)
	{
		if (ne(data[i], q.data[i])) return true;
	}
	return false;
}

template<typename T>
bool Array<T>::operator<(const Array& q) const noexcept // uses eq() and lt()
{
	static int f = 0;
	if (!f) logline("TODO: TEST Array<T>::operator< (Array const& q)");
	f = 1;

	uint end = min(cnt, q.cnt);
	uint i	 = 0;
	while (i < end && eq(data[i], q.data[i])) { i++; }
	if (i < end) return lt(data[i], q.data[i]);
	else return cnt < q.cnt;
}

template<typename T>
bool Array<T>::operator>(const Array& q) const noexcept // uses eq() and gt()
{
	static int f = 0;
	if (!f) logline("TODO: TEST Array<T>::operator> (Array const& q)");
	f = 1;

	uint end = min(cnt, q.cnt);
	uint i	 = 0;
	while (i < end && eq(data[i], q.data[i])) { i++; }
	if (i < end) return gt(data[i], q.data[i]);
	else return cnt > q.cnt;
}

template<typename T>
uint Array<T>::indexof(REForVALUE(T) item) const noexcept
{
	// find first occurance
	// using == (find pointers by identity)
	// or return ~0u

	for (uint i = 0; i < cnt; i++)
	{
		if (data[i] == item) return i;
	}
	return ~0u;
}

template<>
inline uint Array<cstr>::indexof(cstr item) const noexcept
{
	// specializations for str and cstr
	// use eq() --> compare contents

	for (uint i = 0; i < cnt; i++)
	{
		if (eq(data[i], item)) return i;
	}
	return ~0u;
}

template<>
inline uint Array<str>::indexof(str item) const noexcept
{
	for (uint i = 0; i < cnt; i++)
	{
		if (eq(data[i], item)) return i;
	}
	return ~0u;
}
template<typename T>
void Array<T>::growmax(uint newmax) throws
{
	// grow data[]
	// only grows, never shrinks
	//
	// newmax > maxCount: throws
	// newmax > max: grows data[] and copies old items

	if (newmax > max)
	{
		if (newmax < maxCount) newmax = ::min(maxCount, newmax + newmax / 8 + 4);

		T* newdata = allocate(newmax);
		memcopy(newdata, data, cnt);
		deallocate(data);
		data = newdata;
		max	 = newmax;
	}
}

template<typename T>
void Array<T>::grow(uint newcnt, uint newmax) throws
{
	// grow data[]
	// only grows, never shrinks
	//
	// newcnt > maxCount: throws
	// newmax > maxCount: throws
	// newmax > max: grows data[] and copies old items
	// newcnt > cnt: grows cnt and clears new items

	assert(newmax >= newcnt);

	if (newmax > max)
	{
		T* newdata = allocate(newmax);
		memcopy(newdata, data, cnt);
		deallocate(data);
		data = newdata;
		max	 = newmax;
	}

	if (newcnt > cnt)
	{
		memclr(cnt, newcnt - cnt);
		cnt = newcnt;
	}
}

template<typename T>
void Array<T>::grow(uint newcnt) throws
{
	// grow data[]
	// only grows, never shrinks
	//
	// newcnt > maxCount: throws
	// newcnt ≤ cnt: does nothing
	// newcnt > max: overallocates ~12%

	if (newcnt <= cnt) return;

	growmax(newcnt);

	memclr(cnt, newcnt - cnt);
	cnt = newcnt;
}

template<typename T>
void Array<T>::shrink(uint newcnt) noexcept
{
	// shrink data[]
	// does nothing if new count ≥ current count
	// may reallocate data[]

	if (newcnt >= cnt) return;

	for (uint i = newcnt; i < cnt; i++) data[i].~T();
	cnt = newcnt;

	if (newcnt + 4 < max - max / 8) // time to shrink data?
	{
		try
		{
			T* newdata = allocate(newcnt);
			memcopy(newdata, data, newcnt);
			deallocate(data);
			data = newdata;
			max	 = newcnt;
		}
		catch (std::bad_alloc&)
		{}
	}
}

template<typename T>
void Array<T>::removeat(uint idx, bool fast) noexcept
{
	// remove item at index
	// idx < cnt

	assert(idx < cnt);

	data[idx].~T();
	if (--cnt == idx) return;

	if (fast) { new (data + idx) T(std::move(data[cnt])); }
	else { memmove(idx, idx + 1, cnt - idx); }
}

template<typename T>
void Array<T>::removerange(uint a, uint e) noexcept
{
	// remove range of data

	if (e > cnt) e = cnt;
	if (a >= e) return;

	for (uint i = a; i < e; i++) data[i].~T();
	memmove(a, e, cnt - e);
	cnt -= e - a;
}

template<typename T>
void Array<T>::insertat(uint idx, T t) throws
{
	// insert item at index
	// idx ≤ cnt

	assert(idx <= cnt);

	growmax(cnt + 1);
	memmove(idx + 1, idx, cnt - idx);
	cnt++;
	new (data + idx) T(std::move(t));
}

template<typename T>
void Array<T>::insertat(uint idx, const T* q, uint n) throws
{
	// insert source array at index
	// idx ≤ cnt

	assert(idx <= cnt);
	if (n == 0) return;

	growmax(cnt + n);
	memmove(idx + n, idx, cnt - idx);
	cnt += n;
	for (uint i = 0; i < n; i++) new (data + idx + i) T(*q++);
}

template<typename T>
void Array<T>::insertat(uint idx, const Array& q) throws
{
	// insert source array at index
	// idx ≤ cnt

	assert(this != &q);

	insertat(idx, q.data, q.cnt);
}

template<typename T>
void Array<T>::insertrange(uint a, uint e) throws
{
	// insert space cleared with 0 into data
	// a ≤ cnt

	assert(a <= cnt);
	if (a >= e) return;

	growmax(cnt + (e - a));
	memmove(e, a, cnt - a);
	cnt += e - a;
	memclr(a, e - a);
}

template<typename T>
void Array<T>::insertsorted(T q) throws
{
	uint i = cnt;
	while (i-- && gt(data[i], q)) {}
	insertat(i + 1, std::move(q));
}

template<typename T>
void Array<T>::revert(uint a, uint e) noexcept
{
	// revert order of items in data[]

	if (e > cnt) e = cnt;
	if (a >= e) return;

	T* pa = data + a;
	T* pe = data + e - 1;

	do {
		std::swap(*pa, *pe);
	}
	while (++pa < --pe);
}

template<typename T>
void Array<T>::rol(uint a, uint e) noexcept
{
	// roll left range [a..[e
	// e ≤ cnt

	if (e > cnt) e = cnt;
	if (a >= e) return;

	T z(std::move(data[a]));
	memmove(a, a + 1, e - a - 1);
	new (&data[e - 1]) T(std::move(z));
}

template<typename T>
void Array<T>::ror(uint a, uint e) noexcept
{
	// roll right range [a..[e
	// e ≤ cnt
	// moves items in a way which also works for RCArray<T> and other sub clases

	if (e > cnt) e = cnt;
	if (a >= e) return;

	T z(std::move(data[e - 1]));
	memmove(a + 1, a, e - a - 1);
	new (&data[a]) T(std::move(z));
}

template<typename T>
void Array<T>::shuffle(uint a, uint e) noexcept
{
	// shuffle data in range [a..[e
	// moves items in a way which also works for RCArray<T> and other sub clases

	if (e > cnt) e = cnt;
	if (a >= e) return;

	T*	 p = data + a;
	uint n = e - a;

	for (uint i = 0; i < n; i++) { std::swap(p[i], p[random() % n]); }
}


// _____________________________________________________________________________________________________________
// it seems impossible to specialize a class template's member function for a group of types with common traits.
// therefore functionality is extracted into a global function which can templated and overloaded as needed.
// https://jguegant.github.io/blogs/tech/sfinae-introduction.html

template<typename T>
inline str tostr(const Array<T>& array)
{
	// return 1-line description of array for debugging and logging:
	return usingstr("Array<T>[%u]", array.count());
}
inline str tostr(const Array<cstr>& array) { return usingstr("Array<cstr>[%u]", array.count()); }
inline str tostr(const Array<str>& array) { return usingstr("Array<str>[%u]", array.count()); }

// ____ print() ____

template<typename T>
inline typename std::enable_if<kio::has_print<T>::value, void>::type
/*void*/
print(FD& fd, const Array<T>& array, cstr indent) throws
{
	// pretty print with indentation
	// this function is called by Array<T>::print() for classes which implement T::print()

	fd.write_fmt("%sArray[%u]\n", indent, array.count());
	indent = catstr("  ", indent);
	for (uint i = 0; i < array.count(); i++)
	{
		fd.write_fmt("%s[%2u] ", indent, i);
		array[i].print(fd, "");
	}
}

template<typename T>
inline typename std::enable_if<!kio::has_print<T>::value, void>::type
/*void*/
print(FD& fd, const Array<T>& array, cstr indent) throws
{
	// pretty print with indentation
	// this function is called by Array<T>::print() for types which don't implement T::print()

	fd.write_fmt("%sArray[%u]\n", indent, array.count());
	indent = catstr("  ", indent);
	for (uint i = 0; i < array.count(); i++) { fd.write_fmt("%s[%2u] %s\n", indent, i, tostr(array[i])); }
}

template<typename T>
inline void Array<T>::print(FD& fd, cstr indent) const throws
{
	// pretty print with indentation
	// this template will find the above print(FD&,Array<T>const&,cstr)

	::print(fd, *this, indent);
}

// ____ serialize() ____

template<typename T>
typename std::enable_if<std::is_fundamental<T>::value, void>::type
/*void*/
serialize(FD& fd, const Array<T>& array, void*) throws
{
	// used if type T is plain integer or float

	fd.write_uint16(array.MAGIC); // saved in host byte order for byte order test
	fd.write_uint32_z(array.count());
	fd.write_data(array.getData(), array.count());
}

template<>
inline void Array<cstr>::serialize(FD& fd, void*) const throws
{
	// specialization for T = cstr
	// strings are stored with length prefix

	fd.write_uint16_z(MAGIC);
	fd.write_uint32_z(count());
	for (uint i = 0; i < count(); i++) fd.write_nstr(data[i]);
}

template<>
inline void Array<str>::serialize(FD& fd, void*) const throws
{
	// specialization for T = cstr
	// strings are stored with length prefix

	fd.write_uint16_z(MAGIC);
	fd.write_uint32_z(count());
	for (uint i = 0; i < count(); i++) fd.write_nstr(data[i]);
}

template<typename T>
typename std::enable_if<kio::has_serialize<T>::value && !kio::has_serialize_w_data<T, void*>::value, void>::type
/*void*/
serialize(FD& fd, const Array<T>& array, void*) throws
{
	// used if type T has member function T::serialize(FD&)

	fd.write_uint16_z(array.MAGIC);
	fd.write_uint32_z(array.count());
	for (uint i = 0; i < array.count(); i++) { array[i].serialize(fd); }
}

template<typename T>
typename std::enable_if<kio::has_serialize_w_data<T, void*>::value, void>::type
/*void*/
serialize(FD& fd, const Array<T>& array, void* data) throws
{
	// used if type T has member function T::serialize(FD&,void*)

	fd.write_uint16_z(array.MAGIC);
	fd.write_uint32_z(array.count());
	for (uint i = 0; i < array.count(); i++) { array[i].serialize(fd, data); }
}

template<typename T>
inline void Array<T>::serialize(FD& fd, void* data) const throws
{
	// this template will find the above serialize(FD&,Array<T>const&, void*)
	// for types which implement T::serialize(FD&) and
	// for types which implement T::serialize(FD&,void*) and
	// for fundamental types (integer and float)
	// other types need to specialize Array<T>::serialize(FD&,void*)
	// as Array<cstr> and Array<str> do.

	::serialize(fd, *this, data);
}

// ____ deserialize() ____

template<typename T>
typename std::enable_if<std::is_fundamental<T>::value, void>::type
/*void*/
deserialize(FD& fd, Array<T>& array, void*) throws
{
	// used if type T is plain integer or float

	array.purge();

	uint m = fd.read_uint16();
	if (m != array.MAGIC && m != array.BYTESWAPPED_MAGIC) throw DataError("Array<T>: wrong magic");

	uint n = fd.read_uint32_z();
	array.grow(n, n);
	T* data = array.getData();
	fd.read_data(data, n);

	if (sizeof(T) > 1 && m == array.BYTESWAPPED_MAGIC)
	{
		while (n--) { revert_bytes(data++, sizeof(T)); }
	}
}

template<>
inline void Array<cstr>::deserialize(FD& fd, void*) throws
{
	// specialization for T = cstr
	// strings are stored with length prefix
	// strings in Array<cstr> are not managed by the array
	// strings read with deserialize() are located in TempMem!
	// Hint: class StrArray manages the contained strings

	purge();

	uint m = fd.read_uint16_z();
	if (m != MAGIC) throw DataError("Array<cstr>: wrong magic");

	uint n = fd.read_uint32_z();
	growmax(n);
	while (count() < n) { append(fd.read_nstr()); }
}

template<>
inline void Array<str>::deserialize(FD& fd, void*) throws
{
	// specialization for T = str
	// strings are stored with length prefix
	// strings in Array<str> are not managed by the array
	// strings read with deserialize() are located in TempMem!
	// Hint: class StrArray manages the contained strings

	purge();

	uint m = fd.read_uint16_z();
	if (m != MAGIC) throw DataError("Array<str>: wrong magic");

	uint n = fd.read_uint32_z();
	growmax(n);
	while (count() < n) { append(fd.read_nstr()); }
}

template<typename T>
typename std::enable_if<kio::has_deserialize<T>::value && !kio::has_deserialize_w_data<T, void*>::value, void>::type
/*void*/
deserialize(FD& fd, Array<T>& array, void*) throws
{
	// used if type T has member function T::deserialize(FD&)

	array.purge();

	uint m = fd.read_uint16_z();
	if (m != array.MAGIC) throw DataError("Array<T>: wrong magic");

	uint n = fd.read_uint32_z();
	array.grow(n, n);

	for (uint i = 0; i < n; i++) { array[i].deserialize(fd); }
}

template<typename T>
typename std::enable_if<kio::has_deserialize_w_data<T, void*>::value, void>::type
/*void*/
deserialize(FD& fd, Array<T>& array, void* data) throws
{
	// used if type T has member function T::deserialize(FD&,void*)

	array.purge();

	uint m = fd.read_uint16_z();
	if (m != array.MAGIC) throw DataError("Array<T>: wrong magic");

	uint n = fd.read_uint32_z();
	array.grow(n, n);

	for (uint i = 0; i < n; i++) { array[i].deserialize(fd, data); }
}

template<typename T>
inline void Array<T>::deserialize(FD& fd, void* data) throws
{
	// this template will find the above deserialize(FD&,Array<T>&,void*)
	// for types which implement T::deserialize(FD&) and
	// for types which implement T::deserialize(FD&,void*) and
	// for fundamental types (integer and float)
	// other types need to specialize Array<T>::deserialize(FD&,void*)
	// as Array<cstr> and Array<str> do.

	::deserialize(fd, *this, data);
}
