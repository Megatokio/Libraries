#pragma once
/*	Copyright  (c)	Günter Woigk 2018 - 2019
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

namespace kio {

template<typename T>
struct has_print
{
	// test whether type T has member function void print(FD&):
	// has_print<T>::value = true|false

	struct Foo{};
	template<typename C> static Foo test(...);
	template<typename C> static decltype(std::declval<C>().print(std::declval<FD&>(),"")) test(int);

	static constexpr bool value = std::is_same<void,decltype(test<T>(99))>::value;
};

template<typename T>
struct has_serialize
{
	// test whether type T has member function void serialize(FD&):
	// has_serialize<T>::value = true|false

	struct Foo{};
	template<typename C> static Foo test(...);
	template<typename C> static decltype(std::declval<C>().serialize(std::declval<FD&>())) test(int);

	static constexpr bool value = std::is_same<void,decltype(test<T>(99))>::value;
};

template<typename T>
struct has_deserialize
{
	// test whether type T has member function void deserialize(FD&):
	// has_deserialize<T>::value = true|false

	struct Foo{};
	template<typename C> static Foo test(...);
	template<typename C> static decltype(std::declval<C>().deserialize(std::declval<FD&>())) test(int);

	static constexpr bool value = std::is_same<void,decltype(test<T>(99))>::value;
};


namespace DebugTemplateHelpers{
  struct Foo{};
  struct FooWithPrint{void print(FD&,cstr)const;};
  struct FooWithSerialize{void serialize(FD&)const;};
  struct FooWithDeserialize{void deserialize(FD&);};

  static_assert(has_print<FooWithPrint>::value,"booboo");
  static_assert(!has_print<Foo>::value,"booboo");

  static_assert(has_serialize<FooWithSerialize>::value,"booboo");
  static_assert(!has_serialize<Foo>::value,"booboo");

  static_assert(has_deserialize<FooWithDeserialize>::value,"booboo");
  static_assert(!has_deserialize<Foo>::value,"booboo");
};

}; // namespace




