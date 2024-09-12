// Copyright (c) 2023 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


namespace kio
{

template<typename T>
constexpr int find_sorted_insertion_point(const T data[], int a, int e, T x, bool(lt)(const T&, const T&))
{
	if (a >= e) return a;
	if (lt(x, data[a])) return a;

	if (lt(x, data[e - 1]))
	{
		while (a < e - 1)
		{
			int m = (a + e) >> 1;
			if (lt(x, data[m])) e = m;
			else a = m;
		}
	}
	return e;
}

template<typename T>
constexpr int find_sorted_insertion_point(const T data[], int a, int e, T x)
{
	// find insertion point for element x in sorted array data[a … [e.
	// 'same values' will be inserted behind the 'same value'.
	// the template uses
	//	 lt(T,T) if defined
	// else for pointers
	//	 *T < *T  or  lt(*T,*T) if defined
	// else T < T.

	if constexpr (has_operator_lt<T>::value)
	{
		bool (*_lt)(const T&, const T&) = [](const T& a, const T& b) { return lt(a, b); };
		return find_sorted_insertion_point(data, a, e, x, _lt);
	}
	else if constexpr (has_operator_star<T>::value)
	{
		if constexpr (has_operator_lt<typename std::remove_pointer_t<T>>::value)
		{
			bool (*_lt)(const T&, const T&) = [](const T& a, const T& b) { return lt(*a, *b); };
			return find_sorted_insertion_point(data, a, e, x, _lt);
		}
		else
		{
			bool (*_lt)(const T&, const T&) = [](const T& a, const T& b) { return *a < *b; };
			return find_sorted_insertion_point(data, a, e, x, _lt);
		}
	}
	else
	{
		if (a >= e) return a;
		if (x < data[a]) return a;

		if (x < data[e - 1])
		{
			while (a < e - 1)
			{
				int m = (a + e) >> 1;
				if (x < data[m]) e = m;
				else a = m;
			}
		}

		return e;
	}
}


static_assert(find_sorted_insertion_point("abcdefghijk", 0, 0, 'a') == 0);
static_assert(find_sorted_insertion_point("bcdefghijk", 0, 10, 'a') == 0);
static_assert(find_sorted_insertion_point("abcdefghijk", 0, 10, 'a') == 1);
static_assert(find_sorted_insertion_point("abcdefghijk", 1, 10, 'b') == 2);
static_assert(find_sorted_insertion_point("abcdeghijk", 0, 10, 'f') == 5);
static_assert(find_sorted_insertion_point("abcdefghijk", 1, 10, 'f') == 6);
static_assert(find_sorted_insertion_point("abcdefghijk", 0, 9, 'f') == 6);
static_assert(find_sorted_insertion_point("abcdefghijk", 1, 7, 'c') == 3);
static_assert(find_sorted_insertion_point("abbbbfghijk", 0, 10, 'b') == 5);
static_assert(find_sorted_insertion_point("abcdefghij", 1, 10, 'j') == 10);
static_assert(find_sorted_insertion_point("abcdefghij", 0, 10, 'k') == 10);
static_assert(find_sorted_insertion_point("abcdefghijk", 1, 7, 'a') == 1);
static_assert(find_sorted_insertion_point(&"abcdefghijk"[4], -4, 0, 'a') == -3);
static_assert(find_sorted_insertion_point(&"abcdefghijk"[10], 10, 1, 'r') == 10);

} // namespace kio

