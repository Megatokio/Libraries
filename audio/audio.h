#pragma once
/*	Copyright  (c)	Günter Woigk 2018 - 2019
					mailto:kio@little-bat.de

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Permission to use, copy, modify, distribute, and sell this software and
	its documentation for any purpose is hereby granted without fee, provided
	that the above copyright notice appear in all copies and that both that
	copyright notice and this permission notice appear in supporting
	documentation, and that the name of the copyright holder not be used
	in advertising or publicity pertaining to distribution of the software
	without specific, written prior permission.  The copyright holder makes no
	representations about the suitability of this software for any purpose.
	It is provided "as is" without express or implied warranty.

	THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
	EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
	PERFORMANCE OF THIS SOFTWARE.
*/

#include "Libraries/kio/kio.h"
#include "Libraries/unix/FD.h"
#include "Libraries/Templates/Array.h"


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

extern void convert_audio(int8 const* q, uint8* z, uint cnt);
extern void convert_audio(int8 const* q, int16* z, uint cnt);
extern void convert_audio(int8 const* q, int32* z, uint cnt);
extern void convert_audio(int8 const* q, float32* z, uint cnt);

extern void convert_audio(uint8 const* q, int16* z, uint cnt);

extern void convert_audio(int16 const* q, int8* z, uint cnt);
extern void convert_audio(int16 const* q, uint8* z, uint cnt);
extern void convert_audio(int16 const* q, uint16* z, uint cnt);
extern void convert_audio(int16 const* q, int32* z, uint cnt);
extern void convert_audio(int16 const* q, float32* z, uint cnt);

extern void convert_audio(int32 const* q, uint32* z, uint cnt);
extern void convert_audio(int32 const* q, int8* z, uint cnt);
extern void convert_audio(int32 const* q, int16* z, uint cnt);
extern void convert_audio(int32 const* q, float32* z, uint cnt);

extern void convert_audio(float32 const* q, int8* z, uint cnt);
extern void convert_audio(float32 const* q, int16* z, uint cnt);
extern void convert_audio(float32 const* q, int32* z, uint cnt);

inline void convert_audio(uint8 const* q, int8* z, uint cnt)
{
	convert_audio(reinterpret_cast<int8 const*>(q), reinterpret_cast<uint8*>(z), cnt);
}
inline void convert_audio(uint16 const* q, int16* z, uint cnt)
{
	convert_audio(reinterpret_cast<int16 const*>(q), reinterpret_cast<uint16*>(z), cnt);
}
inline void convert_audio(uint32 const* q, int32* z, uint cnt)
{
	convert_audio(reinterpret_cast<int32 const*>(q), reinterpret_cast<uint32*>(z), cnt);
}

// these templates resolve to the above convert_audio() functions:
template<typename T,typename U>
inline void convert_audio(Array<T> const& q, Array<U>& z)
{
	uint cnt = q.count(); if(z.count() != cnt) { z.purge(); z.grow(cnt); }
	convert_audio(q.getData(),z.getData(),cnt);
}
template<typename T, typename U>
inline void convert_sample(T q, U& z)
{
	convert_audio(&q,&z,1);
}


extern void convert_to_ALaw(int16 const* q, int8* z, uint cnt);
extern void convert_to_uLaw(int16 const* q, int8* z, uint cnt);
extern void convert_ALaw(int8 const* q, int16* z, uint cnt);
extern void convert_uLaw(int8 const* q, int16* z, uint cnt);

inline void convert_to_ALaw(Array<int16> const& q, Array<int8>& z)
{
	uint cnt = q.count(); if(z.count() != cnt) { z.purge(); z.grow(cnt); }
	convert_to_ALaw(q.getData(),z.getData(),cnt);
}
inline void convert_to_uLaw(Array<int16> const& q, Array<int8>& z)
{
	uint cnt = q.count(); if(z.count() != cnt) { z.purge(); z.grow(cnt); }
	convert_to_uLaw(q.getData(),z.getData(),cnt);
}
inline void convert_ALaw(Array<int8> const& q, Array<int16>& z)
{
	uint cnt = q.count(); if(z.count() != cnt) { z.purge(); z.grow(cnt); }
	convert_ALaw(q.getData(),z.getData(),cnt);
}
inline void convert_uLaw(Array<int8> const& q, Array<int16>& z)
{
	uint cnt = q.count(); if(z.count() != cnt) { z.purge(); z.grow(cnt); }
	convert_uLaw(q.getData(),z.getData(),cnt);
}


//helpers for stereo_to_mono():
template<typename T> inline T average_sample(T a, T b) { return (a+b) >> 1; }
inline float32 average_sample(float32 a, float32 b) { return (a+b)/2; }
inline int32 average_sample(int32 a, int32 b) { return int32((int64(a)+b) >> 1); }
inline uint32 average_sample(uint32 a, uint32 b) { return uint32((uint64(a)+b) >> 1); }

template<typename T>
void stereo_to_mono(Array<T> const& q, Array<T>& z)
{
	assert(q.count() % 2 == 0);

	uint zcnt = q.count() / 2;
	if(z.count() != zcnt) { z.purge(); z.grow(zcnt); }

	T const* qp = q.getData();
	T*		 zp = z.getData();

	while(zcnt--)
	{
		T a = *qp++;
		T b = *qp++;
		*zp++ = average_sample(a,b);
	}
}

template<typename T>
static void mono_to_stereo(Array<T> const& q, Array<T>& z)
{
	uint qcnt = q.count();
	if(z.count() != qcnt*2) { z.purge(); z.grow(qcnt*2); }

	T const* qp = q.getData();
	T*		 zp = z.getData();

	while(qcnt--)
	{
		T a = *qp++;
		*zp++ = a;
		*zp++ = a;
	}
}

template<typename T>
void one_channel(Array<T> const& q, Array<T>& z, uint idx)
{
	assert(q.count() % 2 == 0);

	uint zcnt = q.count() / 2;
	if(z.count() != zcnt) { z.purge(); z.grow(zcnt); }

	T const* qp = q.getData() + idx;
	T*		 zp = z.getData();

	while(zcnt--)
	{
		*zp++ = *qp; qp += 2;
	}
}

template<typename T>
inline void left_channel(Array<T> const& q, Array<T>& z)
{
	one_channel(q,z,0);
}

template<typename T>
inline void right_channel(Array<T> const& q, Array<T>& z)
{
	one_channel(q,z,1);
}



































