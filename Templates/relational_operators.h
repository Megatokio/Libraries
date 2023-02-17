#pragma once
// Copyright (c) 2017 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"
#include "template_helpers.h"
#include <type_traits>


/*	DATA TYPE			VALUE				IDENTITY		SORT		INDEXOF/CONTAINS/REMOVE
	-------------------	-------------------	---------------	-----------	-----------------------
	basic				eq ge gt == >= >		 			value		value    eq ==
	inline object		eq ge gt == >= >					value		value    eq ==
	char*				eq ge gt strcmp()	is == 			value		value    eq
	pointer to object	eq ge gt (*p)		is == 			value		identity is ==
	RCPtr				eq ge gt (*p)		is == ptr()		value		identity is ==
	smart pointer		eq ge gt (*p)		is == get() 	value		identity is ==
*/

#define cT T const

// general case:

#define Bool typename std::enable_if<std::is_arithmetic<T>::value || std::is_enum<T>::value, bool>::type
template<typename T>
inline Bool eq(T a, T b) noexcept
{
	return a == b;
}
template<typename T>
inline Bool ne(T a, T b) noexcept
{
	return a != b;
}
template<typename T>
inline Bool gt(T a, T b) noexcept
{
	return a > b;
}
template<typename T>
inline Bool lt(T a, T b) noexcept
{
	return a < b;
}
template<typename T>
inline Bool ge(T a, T b) noexcept
{
	return a >= b;
}
template<typename T>
inline Bool le(T a, T b) noexcept
{
	return a <= b;
}
#undef Bool

// specialization for classes:

#define Bool typename std::enable_if<std::is_class<T>::value && !kio::has_oper_star<T>::value, bool>::type
template<typename T>
inline Bool eq(cT& a, cT& b) noexcept
{
	return a == b;
}
template<typename T>
inline Bool ne(cT& a, cT& b) noexcept
{
	return a != b;
}
template<typename T>
inline Bool gt(cT& a, cT& b) noexcept
{
	return a > b;
}
template<typename T>
inline Bool lt(cT& a, cT& b) noexcept
{
	return a < b;
}
template<typename T>
inline Bool ge(cT& a, cT& b) noexcept
{
	return a >= b;
}
template<typename T>
inline Bool le(cT& a, cT& b) noexcept
{
	return a <= b;
}
#undef Bool

// specialization for pointer classes:

#define Bool typename std::enable_if<std::is_class<T>::value && kio::has_oper_star<T>::value, bool>::type
template<class T>
inline Bool eq(cT& a, cT& b) noexcept
{
	return a && b ? eq(*a, *b) : a == b;
}
template<class T>
inline Bool ne(cT& a, cT& b) noexcept
{
	return a && b ? ne(*a, *b) : a != b;
}
template<class T>
inline Bool gt(cT& a, cT& b) noexcept
{
	return a && b ? gt(*a, *b) : !!a;
}
template<class T>
inline Bool lt(cT& a, cT& b) noexcept
{
	return a && b ? lt(*a, *b) : !!b;
}
template<class T>
inline Bool ge(cT& a, cT& b) noexcept
{
	return a && b ? ge(*a, *b) : !b;
}
template<class T>
inline Bool le(cT& a, cT& b) noexcept
{
	return a && b ? le(*a, *b) : !a;
}
template<class T>
inline Bool is(cT& a, cT& b) noexcept
{
	return a == b;
}
#undef Bool

// specialization for cstr:

extern bool gt(cstr a, cstr b) noexcept; // in cstrings.cpp
extern bool lt(cstr a, cstr b) noexcept; // in cstrings.cpp
extern bool ne(cstr a, cstr b) noexcept; // in cstrings.cpp
extern bool eq(cstr a, cstr b) noexcept; // in cstrings.cpp
extern bool ge(cstr a, cstr b) noexcept; // in cstrings.cpp
extern bool le(cstr a, cstr b) noexcept; // in cstrings.cpp
inline bool is(cstr a, cstr b) noexcept { return a == b; }

// specialization for pointers to objects:

template<class T>
inline bool eq(cT* a, cT* b) noexcept
{
	return a && b ? *a == *b : a == b;
}
template<class T>
inline bool ne(cT* a, cT* b) noexcept
{
	return a && b ? *a != *b : a != b;
}
template<class T>
inline bool gt(cT* a, cT* b) noexcept
{
	return a && b ? *a > *b : !!a;
}
template<class T>
inline bool lt(cT* a, cT* b) noexcept
{
	return a && b ? *a < *b : !!b;
}
template<class T>
inline bool ge(cT* a, cT* b) noexcept
{
	return a && b ? *a >= *b : !b;
}
template<class T>
inline bool le(cT* a, cT* b) noexcept
{
	return a && b ? *a <= *b : !a;
}
template<class T>
inline bool is(cT* a, cT* b) noexcept
{
	return a == b;
}

// map pointers to non-const types to const types:
// at least the non-const-non-const variants are required for sort()
//    to use gt() or lt() to compare by value (*a) <=> (*b)

template<class T>
inline bool eq(cT* a, T* b) noexcept
{
	return eq(a, const_cast<cT*>(b));
}
template<class T>
inline bool ne(cT* a, T* b) noexcept
{
	return ne(a, const_cast<cT*>(b));
}
template<class T>
inline bool gt(cT* a, T* b) noexcept
{
	return gt(a, const_cast<cT*>(b));
}
template<class T>
inline bool lt(cT* a, T* b) noexcept
{
	return lt(a, const_cast<cT*>(b));
}
template<class T>
inline bool ge(cT* a, T* b) noexcept
{
	return ge(a, const_cast<cT*>(b));
}
template<class T>
inline bool le(cT* a, T* b) noexcept
{
	return le(a, const_cast<cT*>(b));
}
template<class T>
inline bool is(cT* a, T* b) noexcept
{
	return is(a, const_cast<cT*>(b));
}

template<class T>
inline bool eq(T* a, cT* b) noexcept
{
	return eq(const_cast<cT*>(a), b);
}
template<class T>
inline bool ne(T* a, cT* b) noexcept
{
	return ne(const_cast<cT*>(a), b);
}
template<class T>
inline bool gt(T* a, cT* b) noexcept
{
	return gt(const_cast<cT*>(a), b);
}
template<class T>
inline bool lt(T* a, cT* b) noexcept
{
	return lt(const_cast<cT*>(a), b);
}
template<class T>
inline bool ge(T* a, cT* b) noexcept
{
	return ge(const_cast<cT*>(a), b);
}
template<class T>
inline bool le(T* a, cT* b) noexcept
{
	return le(const_cast<cT*>(a), b);
}
template<class T>
inline bool is(T* a, cT* b) noexcept
{
	return is(const_cast<cT*>(a), b);
}

template<class T>
inline bool eq(T* a, T* b) noexcept
{
	return eq(const_cast<cT*>(a), const_cast<cT*>(b));
}
template<class T>
inline bool ne(T* a, T* b) noexcept
{
	return ne(const_cast<cT*>(a), const_cast<cT*>(b));
}
template<class T>
inline bool gt(T* a, T* b) noexcept
{
	return gt(const_cast<cT*>(a), const_cast<cT*>(b));
}
template<class T>
inline bool lt(T* a, T* b) noexcept
{
	return lt(const_cast<cT*>(a), const_cast<cT*>(b));
}
template<class T>
inline bool ge(T* a, T* b) noexcept
{
	return ge(const_cast<cT*>(a), const_cast<cT*>(b));
}
template<class T>
inline bool le(T* a, T* b) noexcept
{
	return le(const_cast<cT*>(a), const_cast<cT*>(b));
}
template<class T>
inline bool is(T* a, T* b) noexcept
{
	return is(const_cast<cT*>(a), const_cast<cT*>(b));
}

#undef cT
