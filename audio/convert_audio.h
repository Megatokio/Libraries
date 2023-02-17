#pragma once
// Copyright (c) 2018 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Templates/Array.h"
#include "unix/FD.h"


/* =========================================================

	A U D I O   C O N V E R T E R S :

	primary encodings: int16 and float32
	other encodings: int8, int32, uint8, uint16, uint32, A-Law, µLaw

	if there is an encoding A -> B, then there is also an encoding B -> A.

	float <-> int8, int16, int32

	int8  <-> uint8, int16, int32,  float
	int16 <-> int8,  uint8, uint16, int32, float, ALaw, µLaw
	int32 <-> int8,  int16, uint32, float

	uint8  <-> int8, int16
	uint16 <-> int16
	uint32 <-> int32

	A-Law <-> int16
	µLaw  <-> int16
*/

extern void convert_audio(const int8* q, uint8* z, uint cnt);
extern void convert_audio(const int8* q, int16* z, uint cnt);
extern void convert_audio(const int8* q, int32* z, uint cnt);
extern void convert_audio(const int8* q, float32* z, uint cnt);

extern void convert_audio(const uint8* q, int16* z, uint cnt);

extern void convert_audio(const int16* q, int8* z, uint cnt);
extern void convert_audio(const int16* q, uint8* z, uint cnt);
extern void convert_audio(const int16* q, uint16* z, uint cnt);
extern void convert_audio(const int16* q, int32* z, uint cnt);
extern void convert_audio(const int16* q, float32* z, uint cnt);

extern void convert_audio(const int32* q, uint32* z, uint cnt);
extern void convert_audio(const int32* q, int8* z, uint cnt);
extern void convert_audio(const int32* q, int16* z, uint cnt);
extern void convert_audio(const int32* q, float32* z, uint cnt);

extern void convert_audio(const float32* q, int8* z, uint cnt);
extern void convert_audio(const float32* q, int16* z, uint cnt);
extern void convert_audio(const float32* q, int32* z, uint cnt);

inline void convert_audio(const uint8* q, int8* z, uint cnt)
{
	convert_audio(reinterpret_cast<const int8*>(q), reinterpret_cast<uint8*>(z), cnt);
}
inline void convert_audio(const uint16* q, int16* z, uint cnt)
{
	convert_audio(reinterpret_cast<const int16*>(q), reinterpret_cast<uint16*>(z), cnt);
}
inline void convert_audio(const uint32* q, int32* z, uint cnt)
{
	convert_audio(reinterpret_cast<const int32*>(q), reinterpret_cast<uint32*>(z), cnt);
}

// these templates resolve to the above convert_audio() functions:
template<typename T, typename U>
inline void convert_audio(const Array<T>& q, Array<U>& z)
{
	uint cnt = q.count();
	if (z.count() != cnt)
	{
		z.purge();
		z.grow(cnt);
	}
	convert_audio(q.getData(), z.getData(), cnt);
}
template<typename T, typename U>
inline void convert_sample(T q, U& z)
{
	convert_audio(&q, &z, 1);
}


extern void convert_to_ALaw(const int16* q, int8* z, uint cnt);
extern void convert_to_uLaw(const int16* q, int8* z, uint cnt);
extern void convert_ALaw(const int8* q, int16* z, uint cnt);
extern void convert_uLaw(const int8* q, int16* z, uint cnt);

inline void convert_to_ALaw(const Array<int16>& q, Array<int8>& z)
{
	uint cnt = q.count();
	if (z.count() != cnt)
	{
		z.purge();
		z.grow(cnt);
	}
	convert_to_ALaw(q.getData(), z.getData(), cnt);
}
inline void convert_to_uLaw(const Array<int16>& q, Array<int8>& z)
{
	uint cnt = q.count();
	if (z.count() != cnt)
	{
		z.purge();
		z.grow(cnt);
	}
	convert_to_uLaw(q.getData(), z.getData(), cnt);
}
inline void convert_ALaw(const Array<int8>& q, Array<int16>& z)
{
	uint cnt = q.count();
	if (z.count() != cnt)
	{
		z.purge();
		z.grow(cnt);
	}
	convert_ALaw(q.getData(), z.getData(), cnt);
}
inline void convert_uLaw(const Array<int8>& q, Array<int16>& z)
{
	uint cnt = q.count();
	if (z.count() != cnt)
	{
		z.purge();
		z.grow(cnt);
	}
	convert_uLaw(q.getData(), z.getData(), cnt);
}


// helpers for stereo_to_mono():
template<typename T>
inline T average_sample(T a, T b)
{
	return (a + b) >> 1;
}
inline float32 average_sample(float32 a, float32 b) { return (a + b) / 2; }
inline int32   average_sample(int32 a, int32 b) { return int32((int64(a) + b) >> 1); }
inline uint32  average_sample(uint32 a, uint32 b) { return uint32((uint64(a) + b) >> 1); }

template<typename T>
void stereo_to_mono(const Array<T>& q, Array<T>& z)
{
	assert(q.count() % 2 == 0);

	uint zcnt = q.count() / 2;
	if (z.count() != zcnt)
	{
		z.purge();
		z.grow(zcnt);
	}

	const T* qp = q.getData();
	T*		 zp = z.getData();

	while (zcnt--)
	{
		T a	  = *qp++;
		T b	  = *qp++;
		*zp++ = average_sample(a, b);
	}
}

template<typename T>
void mono_to_stereo(const Array<T>& q, Array<T>& z)
{
	uint qcnt = q.count();
	if (z.count() != qcnt * 2)
	{
		z.purge();
		z.grow(qcnt * 2);
	}

	const T* qp = q.getData();
	T*		 zp = z.getData();

	while (qcnt--)
	{
		T a	  = *qp++;
		*zp++ = a;
		*zp++ = a;
	}
}

template<typename T>
void one_channel(const Array<T>& q, Array<T>& z, uint idx)
{
	assert(q.count() % 2 == 0);

	uint zcnt = q.count() / 2;
	if (z.count() != zcnt)
	{
		z.purge();
		z.grow(zcnt);
	}

	const T* qp = q.getData() + idx;
	T*		 zp = z.getData();

	while (zcnt--)
	{
		*zp++ = *qp;
		qp += 2;
	}
}

template<typename T>
inline void left_channel(const Array<T>& q, Array<T>& z)
{
	one_channel(q, z, 0);
}

template<typename T>
inline void right_channel(const Array<T>& q, Array<T>& z)
{
	one_channel(q, z, 1);
}
