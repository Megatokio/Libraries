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
#include "Libraries/Templates/Array.h"
#include "audio.h"


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


// ======== convert int8 --> X: uint8, int16, int32, float ========

void convert_audio(int8 const* q, uint8* z, uint cnt)
{
	memcpy(z, q, cnt*sizeof(*q));

	for(uint i=0; i<cnt; i++)
	{
		z[i] ^= 0x80;
	}
}

void convert_audio(int8 const* q, int16* z, uint cnt)
{
	for(uint i=0; i<cnt; i++)
	{
		z[i] = int16( q[i] << 8 );
	}
}

void convert_audio(int8 const* q, int32* z, uint cnt)
{
	for(uint i=0; i<cnt; i++)
	{
		z[i] = q[i] << 24;
	}
}

void convert_audio(int8 const* q, float32* z, uint cnt)
{
	for(uint i=0; i<cnt; i++)
	{
		z[i] = (q[i] + 0.5f) / 0x80;
	}
}

// ======== convert uint8 --> X: int8, int16 ========

void convert_audio(uint8 const* q, int16* z, uint cnt)
{
	std::unique_ptr<int8[]> v(new int8[cnt]);
	convert_audio(q,v.get(),cnt);
	convert_audio(v.get(),z,cnt);
}

// ======== convert int16 --> X: uint16, int8, int32, float, uint8, ALaw, µLaw ========

void convert_audio(int16 const* q, int8* z, uint cnt)
{
	for(uint i=0; i<cnt; i++)
	{
		z[i] = q[i] >> 8;
	}
}

void convert_audio(int16 const* q, uint8* z, uint cnt)
{
	std::unique_ptr<int8[]> v(new int8[cnt]);
	convert_audio(q,v.get(),cnt);
	convert_audio(v.get(),z,cnt);
}

void convert_audio(int16 const* q, uint16* z, uint cnt)
{
	memcpy(z, q, cnt*sizeof(*q));

	for(uint i=0; i<cnt; i++)
	{
		z[i] ^= 0x8000;
	}
}

void convert_audio(int16 const* q, int32* z, uint cnt)
{
	for(uint i=0; i<cnt; i++)
	{
		z[i] = q[i] << 16;
	}
}

void convert_audio(int16 const* q, float32* z, uint cnt)
{
	for(uint i=0; i<cnt; i++)
	{
		z[i] = (q[i] + 0.5f) / 0x8000;
	}
}

void convert_to_ALaw(int16 const* q, int8* z, uint cnt)
{
	for(uint i=0; i<cnt; i++)
	{
		// encode a signed 16 bit sample into a single byte using A-Law encoding
		// NOTE: unlike other encoders/decoders which require signed 13 bit samples
		// this implementation encodes the full range of 16 bit, dropping 3 low bits instead.

		int16 sample = q[i];

		int sign = 0;
		if (sample<0)
		{
			sign = 0x80;		//sample = -sample; if(sample<0) { sample--; assert(sample>0); }
			sample = ~sample;	// '~' instead of '-' avoids overflow test for 0x8000
		}

		int exp = 7;
		for (int mask = 0x4000; ~sample & mask && exp; mask >>= 1) { --exp; }

		int mant = sample >> (exp?exp+3:4);
		z[i] = int8( 0x55 ^ sign ^ (exp << 4) ^ (mant & 0x0F) );
	}
}

void convert_to_uLaw(int16 const* q, int8* z, uint cnt)
{
	// Note: the original µLaw algorithm uses 14 bits
	//		 this versions uses full 16 bits
	// µLaw knows a positive 0 (u=255) and a negative 0 (u=127)
	// thus uLawEncoded(uLawDecoded(127)) == 255

	for(uint i=0; i<cnt; i++)
	{
		// encode a signed 16 bit sample into a single byte using µLaw encoding
		// NOTE: unlike other encoders/decoders which require signed 14 bit samples
		// this implementation encodes the full range of 16 bit, dropping 2 low bits instead.

		int16 sample = q[i];

		int sign = 0;
		if (sample<0)
		{
			sign = 0x80;
			sample = -sample;
		}

		sample += 33*4;		// ULAW_BIAS << 2
		if (sample < 0) sample = 0x7FFF;

		int exp = 7;
		for (int mask = 0x4000; ~sample & mask && exp >= 0; mask >>= 1) { exp--; }

		int mant = sample >> (exp + 3);

		z[i] = ~ int8(sign ^ (exp << 4) ^ (mant&0x0F));
	}
}

// ======== convert int32 --> X: uint32, int8, int16, float ========

void convert_audio(int32 const* q, uint32* z, uint cnt)
{
	memcpy(z, q, cnt*sizeof(*q));

	for(uint i=0; i<cnt; i++)
	{
		z[i] ^= 0x80000000;
	}
}

void convert_audio(int32 const* q, int8* z, uint cnt)
{
	for(uint i=0; i<cnt; i++)
	{
		z[i] = q[i] >> 24;
	}
}

void convert_audio(int32 const* q, int16* z, uint cnt)
{
	for(uint i=0; i<cnt; i++)
	{
		z[i] = q[i] >> 16;
	}
}

void convert_audio(int32 const* q, float32* z, uint cnt)
{
	for(uint i=0; i<cnt; i++)
	{
		z[i] = (q[i] + 0.5f) / 0x80000000;
	}
}

// ======== convert float --> X: int8, int16, int32 ========

static constexpr float f_min = -1.0f;
static constexpr float f_max = float(0xFFFFFF) / float(0x1000000);
static_assert(f_max < 1.0f,"");
static_assert(f_max*0x80 < 0x80,"");
static_assert(f_max*0x8000 < 0x8000,"");
static_assert(f_max*0x80000000 < 0x80000000,"");

void convert_audio(float32 const* q, int8* z, uint cnt)
{
	for(uint i=0; i<cnt; i++)
	{
		z[i] = int8( minmax(f_min,q[i],f_max) * 0x80 );
	}
}

void convert_audio(float32 const* q, int16* z, uint cnt)
{
	for(uint i=0; i<cnt; i++)
	{
		z[i] = int16( minmax(f_min,q[i],f_max) * 0x8000 );
	}
}

void convert_audio(float32 const* q, int32* z, uint cnt)
{
	for(uint i=0; i<cnt; i++)
	{
		z[i] = int32( minmax(f_min,q[i],f_max) * 0x80000000 );
	}
}

// ======== convert A-Law --> X: int16 ========

void convert_ALaw(int8 const* q, int16* z, uint cnt)
{
	for(uint i=0; i<cnt; i++)
	{
		int sample = q[i] ^ 0x55;

		int vz = +1;
		if(sample < 0)
		{
			sample &= 0x7F;
			vz = -1;
		}

		int exp = sample >> 4;
		int rval = exp ? (((sample&0x0F) << 3) | 0x0084) << exp : (sample<<4) + 8;

		z[i] = int16(vz * rval);
	}
}

// ======== convert µLaw --> X: int16 ========

void convert_uLaw(const int8* q, int16* z, uint cnt)
{
	for(uint i=0; i<cnt; i++)
	{
		int sample = ~ q[i];

		int vz = +1;
		if (sample < 0)
		{
			sample &= 0x7F;
			vz = -1;
		}

		int exp = sample >> 4;
		int rval = ((0x84 << exp) | ((sample & 0x0F) << (exp+3))) - 33*4;

		z[i] = int16(vz * rval);
	}
}





























