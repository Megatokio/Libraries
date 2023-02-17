#pragma once
// Copyright (c) 1995 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"


/* ----	Calculate base 2 logarithm or 'position' of leftmost '1' bit -----------------
		2014-02-15: renamed log2 to msbit
		return value:
				msbit(n>0) = int(log(2,n))
		note:	msbit(n=1) = 0
		caveat:	msbit(n=0) = 0		// illegal argument!
*/
inline int msbit(uint8 n) noexcept
{
	int b = 0, i = 4;
	do {
		if (n >> i)
		{
			n >>= i;
			b += i;
		}
	}
	while ((i >>= 1));
	return b;
} // 0 ..  7
inline int msbit(uint16 n) noexcept
{
	int b = 0, i = 8;
	do {
		if (n >> i)
		{
			n >>= i;
			b += i;
		}
	}
	while ((i >>= 1));
	return b;
} // 0 .. 15
inline int msbit(uint32 n) noexcept
{
	int b = 0, i = 16;
	do {
		if (n >> i)
		{
			n >>= i;
			b += i;
		}
	}
	while ((i >>= 1));
	return b;
} // 0 .. 31
inline int msbit(uint64 n) noexcept
{
	int b = 0, i = 32;
	do {
		if (n >> i)
		{
			n >>= i;
			b += i;
		}
	}
	while ((i >>= 1));
	return b;
} // 0 .. 63

// this is a PITA
#include <type_traits>
#define only_if_int8 \
  typename std::enable_if<(std::is_integral<T>::value || std::is_enum<T>::value) && sizeof(T) == 1, int>::type = 1
#define only_if_int16 \
  typename std::enable_if<(std::is_integral<T>::value || std::is_enum<T>::value) && sizeof(T) == 2, int>::type = 1
#define only_if_int32 \
  typename std::enable_if<(std::is_integral<T>::value || std::is_enum<T>::value) && sizeof(T) == 4, int>::type = 1
#define only_if_int64 \
  typename std::enable_if<(std::is_integral<T>::value || std::is_enum<T>::value) && sizeof(T) == 8, int>::type = 1
#define only_if_integral_or_enum \
  typename std::enable_if<(std::is_integral<T>::value || std::is_enum<T>::value), int>::type = 1

template<typename T, only_if_int8>
inline int msbit(T n) noexcept
{
	return msbit(uint8(n));
}
template<typename T, only_if_int16>
inline int msbit(T n) noexcept
{
	return msbit(uint16(n));
}
template<typename T, only_if_int32>
inline int msbit(T n) noexcept
{
	return msbit(uint32(n));
}
template<typename T, only_if_int64>
inline int msbit(T n) noexcept
{
	return msbit(uint64(n));
}


/* ----	Calculate the number of digits required to print a number:
		return value:
				binaryDigits(n=0) = 1
				binaryDigits(n>0) = ceil(msbit(n+1))
*/
template<class T, only_if_integral_or_enum>
inline int binaryDigits(T number) noexcept
{
	return msbit(number) + 1;
} // result >= 1
template<class T, only_if_integral_or_enum>
inline int hexDigits(T number) noexcept
{
	return msbit(number) / 4 + 1;
} // result >= 1


/* ----	Calculate the number of digits required to store a numbers of a given range:
		used in GifEncoder

		return value:
				reqBits(n) = ceil(msbit(n))
		note:	reqBits(1) = ceil(msbit(1)) = 0
		caveat:	reqBits(0) = ceil(msbit(0)) = 0		// illegal range!
*/
template<class T, only_if_integral_or_enum>
inline int reqBits(T number) noexcept
{
	return number > 1 ? msbit(number - 1) + 1 : 0;
}
template<class T, only_if_integral_or_enum>
inline int reqNibbles(T number) noexcept
{
	return number > 1 ? msbit(number - 1) / 4 + 1 : 0;
}
template<class T, only_if_integral_or_enum>
inline int reqBytes(T number) noexcept
{
	return number > 1 ? msbit(number - 1) / 8 + 1 : 0;
}
