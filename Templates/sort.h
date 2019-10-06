#pragma once
/*	Copyright  (c)	Günter Woigk 1998 - 2019
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
#include "relational_operators.h"


template<bool, typename T>
struct RefOrValue
{
	typedef T ArgType;
	typedef bool(*CompareFunction)(T,T);
};
template<typename T>
struct RefOrValue<true, T>
{
	typedef T const& ArgType;
	typedef bool(*CompareFunction)(T const&,T const&);
};

// macro returns the type of the compare function for item type:
#define COMPARATOR(T) typename RefOrValue<std::is_class<T>::value,T>::CompareFunction
#define REForVALUE(T) typename RefOrValue<std::is_class<T>::value,T>::ArgType


// undefine macros needed by old sort.h. TODO: remove
#undef TYPE
#undef SWAP
#undef GT

// ------------------------------------------------------------
//					Sort range [a ... [e
// ------------------------------------------------------------

template<typename TYPE> inline void sort(TYPE* a, TYPE* e, COMPARATOR(TYPE) gt)
#include "sorter.h"

template<typename TYPE> inline void sort(TYPE* a, TYPE* e)  { sort(a,e,gt); }
template<typename TYPE> inline void rsort(TYPE* a, TYPE* e) { sort(a,e,lt); }










