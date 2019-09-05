#pragma once
/*	Copyright  (c)	Günter Woigk 2017 - 2019
  					mailto:kio@little-bat.de

	This file is free software.

 	Permission to use, copy, modify, distribute, and sell this software
 	and its documentation for any purpose is hereby granted without fee,
 	provided that the above copyright notice appears in all copies and
 	that both that copyright notice, this permission notice and the
 	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY, NOT EVEN THE
	IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE
	AND IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY DAMAGES
	ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/

#include "kio/kio.h"
#include <type_traits>


// general case:

#define Bool typename std::enable_if<!std::is_class<T>::value,bool>::type
template<typename T> Bool eq(T a, T b) noexcept	{ return a == b; }
template<typename T> Bool ne(T a, T b) noexcept	{ return a != b; }
template<typename T> Bool gt(T a, T b) noexcept	{ return a > b; }
template<typename T> Bool lt(T a, T b) noexcept	{ return a < b; }
template<typename T> Bool ge(T a, T b) noexcept	{ return a >= b; }
template<typename T> Bool le(T a, T b) noexcept	{ return a <= b; }
#undef Bool

// specialization for classes:

#define Bool typename std::enable_if<std::is_class<T>::value,bool>::type
template<typename T> Bool eq(T const& a, T const& b) noexcept  { return a == b; }
template<typename T> Bool ne(T const& a, T const& b) noexcept  { return a != b; }
template<typename T> Bool gt(T const& a, T const& b) noexcept  { return a > b; }
template<typename T> Bool lt(T const& a, T const& b) noexcept  { return a < b; }
template<typename T> Bool ge(T const& a, T const& b) noexcept  { return a >= b; }
template<typename T> Bool le(T const& a, T const& b) noexcept  { return a <= b; }
#undef Bool

// specialization for cstr:

extern bool gt(cstr a, cstr b)	noexcept;	// in cstrings.cpp
extern bool lt(cstr a, cstr b)	noexcept;	// in cstrings.cpp
extern bool ne(cstr a, cstr b)	noexcept;	// in cstrings.cpp
extern bool eq(cstr a, cstr b)	noexcept;	// in cstrings.cpp
extern bool ge(cstr a, cstr b)	noexcept;	// in cstrings.cpp
extern bool le(cstr a, cstr b)	noexcept;	// in cstrings.cpp

// specialization for pointers to objects:

template<class T> bool eq(T const* a, T const* b) noexcept	{ return a&&b ? *a == *b : a==b; }
template<class T> bool ne(T const* a, T const* b) noexcept	{ return a&&b ? *a != *b : a!=b; }
template<class T> bool gt(T const* a, T const* b) noexcept	{ return a&&b ? *a >  *b : a!=nullptr; }
template<class T> bool lt(T const* a, T const* b) noexcept	{ return a&&b ? *a <  *b : b!=nullptr; }
template<class T> bool ge(T const* a, T const* b) noexcept	{ return a&&b ? *a >= *b : b==nullptr; }
template<class T> bool le(T const* a, T const* b) noexcept	{ return a&&b ? *a <= *b : a==nullptr; }

// map pointers to non-const types to const types:

#define cT T const
template<class T> bool eq(cT* a, T* b)	noexcept { return eq(a,const_cast<cT*>(b)); }
template<class T> bool ne(cT* a, T* b)	noexcept { return ne(a,const_cast<cT*>(b)); }
template<class T> bool gt(cT* a, T* b)	noexcept { return gt(a,const_cast<cT*>(b)); }
template<class T> bool lt(cT* a, T* b)	noexcept { return lt(a,const_cast<cT*>(b)); }
template<class T> bool ge(cT* a, T* b)	noexcept { return ge(a,const_cast<cT*>(b)); }
template<class T> bool le(cT* a, T* b)	noexcept { return le(a,const_cast<cT*>(b)); }

template<class T> bool eq(T* a, cT* b)	noexcept { return eq(const_cast<cT*>(a),b); }
template<class T> bool ne(T* a, cT* b)	noexcept { return ne(const_cast<cT*>(a),b); }
template<class T> bool gt(T* a, cT* b)	noexcept { return gt(const_cast<cT*>(a),b); }
template<class T> bool lt(T* a, cT* b)	noexcept { return lt(const_cast<cT*>(a),b); }
template<class T> bool ge(T* a, cT* b)	noexcept { return ge(const_cast<cT*>(a),b); }
template<class T> bool le(T* a, cT* b)	noexcept { return le(const_cast<cT*>(a),b); }

template<class T> bool eq(T* a, T* b)	noexcept { return eq(const_cast<cT*>(a),const_cast<cT*>(b)); }
template<class T> bool ne(T* a, T* b)	noexcept { return ne(const_cast<cT*>(a),const_cast<cT*>(b)); }
template<class T> bool gt(T* a, T* b)	noexcept { return gt(const_cast<cT*>(a),const_cast<cT*>(b)); }
template<class T> bool lt(T* a, T* b)	noexcept { return lt(const_cast<cT*>(a),const_cast<cT*>(b)); }
template<class T> bool ge(T* a, T* b)	noexcept { return ge(const_cast<cT*>(a),const_cast<cT*>(b)); }
template<class T> bool le(T* a, T* b)	noexcept { return le(const_cast<cT*>(a),const_cast<cT*>(b)); }
#undef cT




