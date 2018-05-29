/*
    Original C-source:

    A C-program for MT19937-64 (2004/9/29 version).
    Coded by Takuji Nishimura and Makoto Matsumoto.

    Copyright (C) 2004, Makoto Matsumoto and Takuji Nishimura,
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote
        products derived from this software without specific prior written
        permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    References:
    T. Nishimura, ``Tables of 64-bit Mersenne Twisters''
        ACM Transactions on Modeling and
        Computer Simulation 10. (2000) 348--357.
    M. Matsumoto and T. Nishimura,
        ``Mersenne Twister: a 623-dimensionally equidistributed
        uniform pseudorandom number generator''
        ACM Transactions on Modeling and
        Computer Simulation 8. (Jan. 1998) 3--30.

    Any feedback is very welcome.
        http://www.math.hiroshima-u.ac.jp/~m-mat/MT/emt.html
        email: m-mat @ math.sci.hiroshima-u.ac.jp (remove spaces)


    This is a 64-bit version of Mersenne Twister pseudorandom number generator.

    Before using, initialize the state by using init_genrand64(seed)
    or init_by_array64(init_key, key_length).


    thread-safe encapulation in C++ class by kio@little-bat.de
*/

#ifndef _MersenneTwister64_h_
#define _MersenneTwister64_h_


#include "kio/kio.h"
#include <stdint.h>


inline uint32	random32();
inline int32	random31();
inline uint64	random64();
inline int64	random63();


// number of values per mt[] buffer NOT emitted a random value
// in order to keep the state of the PRNG secret:
// this is for use in encryption only
// reasonable values: 8 .. 56
//
#ifndef MTI0
#define MTI0	8
#endif

// Scheiß Makro...
#ifndef INT64_C
#  if _sizeof_long==4
#  define INT64_C(c) (c ## LL)
#  endif
#endif

class MersenneTwister64
{
protected:
    enum { NN=312, MM=156 };

    // state
    uint64	mt[NN];
    uint 	mti;			// running index in mt[]
    uint	mti0;			// reset value for mti

//	MersenneTwister64*	spread_key2();


public:
    // Constructor mit uint64[] Array als Seed
    // Das Seed kann bis zu 312 Einträge lang sein.
    MersenneTwister64(uint64 seed[], uint count)				{ mti0=MTI0; init(seed,count); }

    // Constructor mit einem Passwort als Seed
    // Das Passwort kann bis zu 2496 Zeichen lang sein.
    // NOTE: die Initialisierung auf BIG-ENDIAN ist zu der auf LITTLE-ENDIAN Maschinen inkompatibel!
    explicit MersenneTwister64(cstr seed)						{ mti0=MTI0; init(seed); }

    // Constructor mit einfachem uint64 oder uint32 als Seed
    // Bei seed=0 wird ein zufälliges Seed generiert.
    explicit MersenneTwister64(uint64 seed)						{ mti0=MTI0; init(seed); }
    explicit MersenneTwister64(uint32 seed)						{ mti0=MTI0; init(seed); }

    // Copy Creator und Zuweisungs-Operator:
    MersenneTwister64(const MersenneTwister64& q)				{ memcpy(mt,q.mt,sizeof(mt)); mti=q.mti; mti0=q.mti0; }
    MersenneTwister64& operator= (const MersenneTwister64& q)	{ memcpy(mt,q.mt,sizeof(mt)); mti=q.mti; mti0=q.mti0; return *this; }

    // Re-Initialisierung
    void 	init(uint32 seed);				// re-initialize with seed
    void 	init(uint64 seed);				// re-initialize with seed
    void 	init(uint64 key[], uint cnt);	// re-initialize with array
    void 	init(cstr password);			// re-initialize with string
    void	next_table();					// calculate next table and reset mti

    void	reset_mti()						{ mti = mti0; }
    void	setCryptoSeedsize(uint n)		{ mti0 = n & 63; }

    // Get random number:
    uint32 	random32();						// random number in range [0, 2^32-1]
    uint64 	random64();						// random number in range [0, 2^64-1]
    int32 	random31();						// random number in range [0, 2^31-1]
    int64 	random63();						// random number in range [0, 2^63-1]
    double 	randomReal1();					// random number in range [0,1]
    double 	randomReal2();					// random number in range [0,1)
    double 	randomReal3();					// random number in range (0,1)
};





// Inline-Implementierungen:


inline uint64 MersenneTwister64::random64()
{
    if(mti >= NN) next_table();
    uint64 x = mt[mti++];

    x ^= (x >> 29) & INT64_C(0x5555555555555555U);
    x ^= (x << 17) & INT64_C(0x71D67FFFEDA60000U);
    x ^= (x << 37) & INT64_C(0xFFF7EEE000000000U);
    x ^= (x >> 43);

    return x;
}

inline uint32 	MersenneTwister64::random32()		{ return  (uint32)random64(); }
inline int32  	MersenneTwister64::random31()		{ return  (int32)random64();  }
inline int64  	MersenneTwister64::random63()		{ return  (random64() >> 1 ); }
inline double 	MersenneTwister64::randomReal1()	{ return  (random64() >> 11)        * (1.0/9007199254740991.0); }
inline double 	MersenneTwister64::randomReal2()	{ return  (random64() >> 11)        * (1.0/9007199254740992.0); }
inline double 	MersenneTwister64::randomReal3()	{ return ((random64() >> 12) + 0.5) * (1.0/4503599627370496.0); }


extern MersenneTwister64 static_twister;

inline uint32	random32()	{ return static_twister.random32(); }
inline int32	random31()	{ return static_twister.random31(); }
inline uint64	random64()	{ return static_twister.random64(); }
inline int64	random63()	{ return static_twister.random63(); }


#endif















