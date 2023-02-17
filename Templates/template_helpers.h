#pragma once
// Copyright (c) 2018 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"

namespace kio
{

// select type T1 or T2:
template<bool, typename T1, typename T2>
struct select_type
{
	typedef T2 type;
};
template<typename T1, typename T2>
struct select_type<true, T1, T2>
{
	typedef T1 type;
};


template<typename T>
struct has_oper_star
{
	// test whether type T has member function operator*():
	// has_oper_star<T>::value = true|false

	struct Foo
	{};
	template<typename C>
	static Foo test(...);
	template<typename C>
	static decltype(*std::declval<C>()) test(int);

	static constexpr bool value = !std::is_same<Foo, decltype(test<T>(99))>::value;
};

template<typename T>
struct has_print
{
	// test whether type T has member function void print(FD&):
	// has_print<T>::value = true|false

	struct Foo
	{};
	template<typename C>
	static Foo test(...);
	template<typename C>
	static decltype(std::declval<C>().print(std::declval<FD&>(), "")) test(int);

	static constexpr bool value = std::is_same<void, decltype(test<T>(99))>::value;
};

template<typename T>
struct has_serialize
{
	// test whether type T has member function void serialize(FD&):
	// has_serialize<T>::value = true|false

	struct Foo
	{};
	template<typename C>
	static Foo test(...);
	template<typename C>
	static decltype(std::declval<C>().serialize(std::declval<FD&>())) test(int);

	static constexpr bool value = std::is_same<void, decltype(test<T>(99))>::value;
};

template<typename T>
struct has_deserialize
{
	// test whether type T has member function void deserialize(FD&):
	// has_deserialize<T>::value = true|false

	struct Foo
	{};
	template<typename C>
	static Foo test(...);
	template<typename C>
	static decltype(std::declval<C>().deserialize(std::declval<FD&>())) test(int);

	static constexpr bool value = std::is_same<void, decltype(test<T>(99))>::value;
};

template<typename T, typename U = void*>
struct has_serialize_w_data
{
	// test whether type T has member function void T::serialize(FD&,void*):
	// has_serialize<T>::value = true|false

	struct Foo
	{};
	template<typename C, typename D>
	static Foo test(...);
	template<typename C, typename D>
	static decltype(std::declval<C>().serialize(std::declval<FD&>(), std::declval<D>())) test(int);

	static constexpr bool value = std::is_same<void, decltype(test<T, U>(99))>::value;
};

template<typename T, typename U = void*>
struct has_deserialize_w_data
{
	// test whether type T has member function void T::deserialize(FD&, void*):
	// has_deserialize<T>::value = true|false

	struct Foo
	{};
	template<typename C, typename D>
	static Foo test(...);
	template<typename C, typename D>
	static decltype(std::declval<C>().deserialize(std::declval<FD&>(), std::declval<D>())) test(int);

	static constexpr bool value = std::is_same<void, decltype(test<T, U>(99))>::value;
};

namespace DebugTemplateHelpers
{
struct Foo
{};
struct FooWithPrint
{
	void print(FD&, cstr) const;
};
struct FooWithSerialize
{
	void serialize(FD&) const;
};
struct FooWithDeserialize
{
	void deserialize(FD&);
};
struct FooWithSerializeWithData
{
	void serialize(FD&, void*) const;
};
struct FooWithDeserializeWithData
{
	void deserialize(FD&, Foo&);
};

static_assert(has_print<FooWithPrint>::value, "booboo");
static_assert(!has_print<Foo>::value, "booboo");

static_assert(has_serialize_w_data<FooWithSerializeWithData>::value, "booboo");
static_assert(!has_serialize_w_data<Foo>::value, "booboo");

static_assert(has_deserialize_w_data<FooWithDeserializeWithData, Foo&>::value, "booboo");
static_assert(!has_deserialize_w_data<Foo>::value, "booboo");

static_assert(has_serialize<FooWithSerialize>::value, "booboo");
static_assert(!has_serialize<Foo>::value, "booboo");

static_assert(has_deserialize<FooWithDeserialize>::value, "booboo");
static_assert(!has_deserialize<Foo>::value, "booboo");

static_assert(has_oper_star<int*>::value, "");
static_assert(!has_oper_star<int>::value, "");
static_assert(!has_oper_star<int&>::value, "");
}; // namespace DebugTemplateHelpers

}; // namespace kio
