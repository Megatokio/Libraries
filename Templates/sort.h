#pragma once
// Copyright (c) 1998 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"
#include "relational_operators.h"
#include "template_helpers.h"


// macro returns the type of the compare function for item type:
#define REForVALUE(T) typename kio::select_type<std::is_class<T>::value, T const&, T>::type
#define COMPARATOR(T) \
  typename kio::select_type<std::is_class<T>::value, bool (*)(T const&, T const&), bool (*)(T, T)>::type


// undefine macros needed by old sort.h. TODO: remove
#undef TYPE
#undef SWAP
#undef GT

// ------------------------------------------------------------
//					Sort range [a ... [e
// ------------------------------------------------------------

template<typename TYPE>
inline void sort(TYPE* a, TYPE* e, COMPARATOR(TYPE) gt)
#include "sorter.h"

	template<typename TYPE>
	inline void sort(TYPE* a, TYPE* e)
{
	sort(a, e, gt);
}
template<typename TYPE>
inline void rsort(TYPE* a, TYPE* e)
{
	sort(a, e, lt);
}
