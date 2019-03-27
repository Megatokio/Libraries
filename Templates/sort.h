#pragma once
/*	Copyright  (c)	Günter Woigk 1998 - 2019
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
	OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
	WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
	OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
	ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "../kio/kio.h"
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










