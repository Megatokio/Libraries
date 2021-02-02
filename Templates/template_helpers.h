#pragma once
/*	Copyright  (c)	Günter Woigk 2018 - 2021
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

// select type T1 or T2:
template<bool, typename T1, typename T2> struct select_type             { typedef T2 type; };
template<typename T1, typename T2>       struct select_type<true,T1,T2> { typedef T1 type; };


template<typename T>
struct has_oper_star
{
	// test whether type T has member function operator*():
	// has_oper_star<T>::value = true|false

	struct Foo{};
	template<typename C> static Foo test(...);
	template<typename C> static decltype(*std::declval<C>()) test(int);

	static constexpr bool value = !std::is_same<Foo,decltype(test<T>(99))>::value;
};

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

template<typename T, typename U=void*>
struct has_serialize_w_data
{
	// test whether type T has member function void T::serialize(FD&,void*):
	// has_serialize<T>::value = true|false

	struct Foo{};
	template<typename C, typename D> static Foo test(...);
	template<typename C, typename D> static decltype(std::declval<C>().serialize(std::declval<FD&>(),std::declval<D>())) test(int);

	static constexpr bool value = std::is_same<void,decltype(test<T,U>(99))>::value;
};

template<typename T, typename U=void*>
struct has_deserialize_w_data
{
	// test whether type T has member function void T::deserialize(FD&, void*):
	// has_deserialize<T>::value = true|false

	struct Foo{};
	template<typename C, typename D> static Foo test(...);
	template<typename C, typename D> static decltype(std::declval<C>().deserialize(std::declval<FD&>(),std::declval<D>())) test(int);

	static constexpr bool value = std::is_same<void,decltype(test<T,U>(99))>::value;
};

namespace DebugTemplateHelpers{
  struct Foo{};
  struct FooWithPrint{void print(FD&,cstr)const;};
  struct FooWithSerialize{void serialize(FD&)const;};
  struct FooWithDeserialize{void deserialize(FD&);};
  struct FooWithSerializeWithData{void serialize(FD&,void*)const;};
  struct FooWithDeserializeWithData{void deserialize(FD&,Foo&);};

  static_assert(has_print<FooWithPrint>::value,"booboo");
  static_assert(!has_print<Foo>::value,"booboo");

  static_assert(has_serialize_w_data<FooWithSerializeWithData>::value,"booboo");
  static_assert(!has_serialize_w_data<Foo>::value,"booboo");

  static_assert(has_deserialize_w_data<FooWithDeserializeWithData,Foo&>::value,"booboo");
  static_assert(!has_deserialize_w_data<Foo>::value,"booboo");

  static_assert(has_serialize<FooWithSerialize>::value,"booboo");
  static_assert(!has_serialize<Foo>::value,"booboo");

  static_assert(has_deserialize<FooWithDeserialize>::value,"booboo");
  static_assert(!has_deserialize<Foo>::value,"booboo");

  static_assert(has_oper_star<int*>::value,"");
  static_assert(!has_oper_star<int>::value,"");
  static_assert(!has_oper_star<int&>::value,"");
};

}; // namespace




