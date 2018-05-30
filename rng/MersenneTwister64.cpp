/*
   A C-program for MT19937-64 (2004/9/29 version).
   Coded by Takuji Nishimura and Makoto Matsumoto.

   This is a 64-bit version of Mersenne Twister pseudorandom number
   generator.

   Before using, initialize the state by using init_genrand64(seed)
   or init_by_array64(init_key, key_length).

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


    thread-safe encapulation in C++ class by kio@little-bat.de
*/

#include "kio/kio.h"
#include "MersenneTwister64.h"
#include "hash/md5.h"


#define UM 		INT64_C(0xFFFFFFFF80000000U)	 	// Most significant 33 bits
#define LM 		INT64_C(0x000000007FFFFFFFU) 		// Least significant 31 bits

static 	uint64	mag01[2] = {0U, INT64_C(0xB5026F5AA96619E9U)};


/*	Globale Instanz für Zufallszahlen die nicht reproduzierbar sein müssen:
    reentrant
*/
MersenneTwister64 static_twister(INT64_C(5489U));




/* 	Initialize with array
    Recommended method
*/
void MersenneTwister64::init( uint64 seed[], uint seed_length )
{
    init(INT64_C(19650218U));

    uint64 i = 1;
    uint64 j = 0;
    uint64 k = min(uint(NN),seed_length);

    for(; k; k--)
    {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 62)) * INT64_C(3935559000370003845U))) + seed[j] + j; /* non linear */
        i++; j++;
        if (i>=NN) { mt[0] = mt[NN-1]; i=1; }
        if (j>=seed_length) j=0;
    }

    for(k=NN-1; k; k--)
    {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 62)) * INT64_C(2862933555777941757U))) - i; /* non linear */
        i++;
        if (i>=NN) { mt[0] = mt[NN-1]; i=1; }
    }

    mt[0] |= INT64_C(1U) << 63; 	// MSB is 1; assuring non-zero initial array
}


/*	Initialize with string
    the string can be up to 2496 characters long
    For passwords etc.
*/
void MersenneTwister64::init(cstr qstr)
{
    if(qstr==NULL||*qstr==0) { init(uint64(0)); return; }

    uint qlen = uint(strlen(qstr));
    uint zlen = (qlen+7)/8;

    uint64 z[zlen]; z[zlen-1] = 0;
    memcpy(z,qstr,qlen);
    init(z,zlen);
}


/* 	Initialize with uint32 seed
    Wenn nur 4 Bytes für ein Seed zur Verfügung stehen
*/
void MersenneTwister64::init( uint32 seed )
{
    init( seed ? uint64(~seed)<<32 | uint64(seed) : 0UL );
}


/* 	Initialize with uint64 seed
    Wenn nur 8 Bytes für ein Seed zur Verfügung stehen
*/
void MersenneTwister64::init( uint64 seed )
{
    while(!seed) seed = random64();

    mt[0] = seed;
    for (mti=1; mti<NN; mti++)
    {
        mt[mti] = (INT64_C(6364136223846793005U) * (mt[mti-1] ^ (mt[mti-1] >> 62)) + mti);
    }
}


/*	Helper: calculate next NN random numbers
*/
void MersenneTwister64::next_table()
{
    int i;
    uint64 x;

    for(i=0;i<NN-MM;i++)
    {
        x = (mt[i]&UM) | (mt[i+1]&LM);
        mt[i] = mt[i+MM] ^ (x>>1) ^ mag01[int(x)&1];
    }

    for(;i<NN-1;i++)
    {
        x = (mt[i]&UM) | (mt[i+1]&LM);
        mt[i] = mt[i+(MM-NN)] ^ (x>>1) ^ mag01[int(x)&1];
    }

    x = (mt[NN-1]&UM) | (mt[0]&LM);
    mt[NN-1] = mt[MM-1] ^ (x>>1) ^ mag01[int(x)&1];

    mti = mti0;
}

///*	Helper: spread secondary key across array mt[]
//*/
//MersenneTwister64* MersenneTwister64::spread_key2()
//{
//	for (mti=1; mti<NN; mti++)
//	{
//		mt[mti] ^= (INT64_C(6364136223846793005U) * (mt[mti-1] ^ (mt[mti-1] >> 62)) + mti);
//	}
//	next_table();
//	return this;
//}

///*	apply a secondary key ontop of an already initialized instance
//*/
//MersenneTwister64* MersenneTwister64::apply_key2(uint64 key2)
//{
//	mt[0] ^= key2;
//	return spread_key2();
//}
//
///*	apply a secondary key ontop of an already initialized instance
//*/
//MersenneTwister64* MersenneTwister64::apply_key2(uint32 key2)
//{
//	mt[0] ^= (uint64)~key2<<32 | (uint64)key2;
//	return spread_key2();
//}
//
///*	apply a secondary key ontop of an already initialized instance
//*/
//MersenneTwister64* MersenneTwister64::apply_key2(uint16 key2)
//{
//	mt[0] ^= key2 * INT64_C(0x0135749a87a5c571U);
//	return spread_key2();
//}
//
///*	apply a secondary key ontop of an already initialized instance
//*/
//MersenneTwister64* MersenneTwister64::apply_key2(cstr qstr)
//{
////	xlogline("apply key2 = \"%s\"",qstr);
//	if(qstr==NULL)
//		log("");
//
//	if(qstr&&*qstr)
//	{
//		uint qlen = (uint)strlen(qstr);
//		uint zlen = qlen/8;
//
//		XXASSERT(zlen<NN);
//
//		for(uint i=0;i<zlen; i++)
//		{
//			mt[i] ^= ((uint64*)qstr)[i];
//		}
//		for(uint i=qlen; i-->zlen*8; )
//		{
//			((ptr)mt)[i] ^= qstr[i];
//		}
//	}
//	return spread_key2();
//}
//
//MersenneTwister64* MersenneTwister64::apply_key2(uint64 key2[], uint32 count)
//{
//	if(count>NN) count = NN;
//	for(uint i=0;i<count;i++) mt[i] ^= key2[i];
//	return spread_key2();
//}






///*	encrypt or decrypt buffer bu[sz]
//*/
//void MersenneTwister64::crypt(uint8 bu[], uint sz)
//{
//	for(uint64* p = ((uint64*)bu)+(sz>>3); (uptr)p>bu;)
//	{
//		*--p ^= this->random64();
//	}
//
//	if(sz&7)
//	{
//		uint64 q = this->random64();
//		while(sz&7) { --sz; bu[sz] ^= ((uptr)&q)[sz&7]; }
//	}
//}
//
//
///*	encrypt a text string
//
//	Anwendung: Verschlüsseln von Dateinamen
//
//	in:	 this = mit einem bekannten, reproduzierbaren Status initialisiert
//		 qstr = zu verschlüsselnder String
//	out: verschlüsselter, base85-codierter String
//		 der verschlüsselte String ist um 20% (aufgerundet) Zeichen länger
//*/
//str MersenneTwister64::encrypt(cstr q)
//{
//	// Create copy of data:
//	uint   zlen = strLen(q);
//	uint8  z[zlen];
//	memcpy(z,q,zlen);
//
//	// Encrypt in place:
//	encrypt(z,zlen);
//
//	// Encode Base85:
//	uint rlen = sizeAfterBase85Encoding(zlen);
//	str r = tempstr(rlen);
//	encodeBase85(z,zlen,(uptr)r,rlen);
//	return r;
//}
//
//str	MersenneTwister64::decrypt(cstr q)
//{
//	// Decode Base85:
//	uint qlen = strLen(q);
//	uint zlen = sizeAfterBase85Decoding(qlen);
//	str  z  = tempstr(zlen);
//	int err = decodeBase85((uptr)q,qlen,(uptr)z,zlen);
//	if(err) return NULL;
//
//	// Decrypt in place:
//	decrypt((uptr)z,zlen);
//	return z;
//}
//
///*	encrypt path
//	Der Pfad wird an den "/" getrennt und abschnittsweise gecryptet.
//	Der MT wird dabei so verändert, dass danach der nächste Pfadabschnitt,
//	oder z.B. ein Directory-Eintrag, damit verschlüsselt werden kann.
//*/
//str MersenneTwister64::encrypt_path(char* qpath)
//{
//	mti = mti0;
//	ptr p = strchr(qpath,'/');
//	if(p==NULL) return encrypt(qpath);
//
//	*p=0;
//	char* zpath = encrypt(qpath);
//	apply_key2(qpath);
//	*p='/';
//
//	return *(p+1) ? catstr(zpath,"/",encrypt_path(p+1)) : catstr(zpath,"/");
//}
//
//
//
///*	decrypt path
//	Der Pfad wird an den "/" getrennt und abschnittsweise gecryptet.
//	Der MT wird dabei so verändert, dass danach der nächste Pfadabschnitt,
//	oder z.B. ein Directory-Eintrag, damit verschlüsselt werden kann.
//	Diese Status-Veränderung ist dabei die gleiche wie beim Verschlüsseln.
//*/
//str MersenneTwister64::decrypt_path(char* qpath)
//{
//	mti = mti0;
//	ptr p = strchr(qpath,'/');
//	if(p==NULL) return decrypt(qpath);
//
//	*p=0;
//	char* zpath = decrypt(qpath);
//	apply_key2(zpath);
//	*p='/';
//
//	return *(p+1) ? catstr(zpath,"/",decrypt_path(p+1)) : catstr(zpath,"/");
//}


//#ifndef NDEBUG
//
//namespace q
//{
//static void test_crypt()
//{
//	MersenneTwister64 mt("67647567");
//	cstr q = "!nuzu&/V$ub6&%R65ggutftfgu6i7&TIÜP,,üpÄP,üp,)N(nc4§ßQ";
//	str  z = mt.encrypt(q);
//	mt.reset_mti();
//	z = mt.decrypt(z);
//	assert(eq(q,z));
//
//	mt.reset_mti();
//	q = dupstr("abcde/fghij/12345"); // dupstr("/zt67b/rcRdz/älpm98/6565r68u/");
//	z = MersenneTwister64(mt).encryptPath(q);
//	z = MersenneTwister64(mt).decryptPath(z);
//	assert(eq(q,z));
//}
//
//ON_INIT(test_crypt);
//};
//
//#endif
































