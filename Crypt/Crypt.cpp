// Copyright (c) 2013 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Crypt.h"
#include "cstrings/base85.h"
#include "hash/md5.h"
#include "kio/kio.h"


uint16 hash(cstr q)
{
	//	uint32 h = 0x0ff0;
	//	while (*s)
	//	{
	//		h = (h<<5) ^ *s++;
	//		while (h>0x10000) { if (h&1) h ^= 0x8408; h=h>>1; }
	//	}
	//	return h;

	uint n = 0xffff, c;

	while ((c = *(uptr)q++))
	{
		for (c += 0x0100; c > 1; c >>= 1) { n = (n ^ c) & 1 ? (n >> 1) ^ 0x8408 : n >> 1; }
	}

	return n;
}


/*	Helper: spread secondary key across array mt[]
*/
Crypt* Crypt::spread_key2()
{
	for (int i = 1; i < NN; i++) { mt[i] ^= (INT64_C(6364136223846793005U) * (mt[i - 1] ^ (mt[i - 1] >> 62)) + i); }
	next_table();
	return this;
}

/*	apply a secondary key ontop of an already initialized instance
*/
Crypt* Crypt::apply_key2(uint64 key2)
{
	mt[0] ^= key2;
	return spread_key2();
}

/*	apply a secondary key ontop of an already initialized instance
*/
Crypt* Crypt::apply_key2(uint32 key2)
{
	mt[0] ^= (uint64)~key2 << 32 | (uint64)key2;
	return spread_key2();
}

/*	apply a secondary key ontop of an already initialized instance
*/
Crypt* Crypt::apply_key2(uint16 key2)
{
	mt[0] ^= key2 * INT64_C(0x0135749a87a5c571U);
	return spread_key2();
}

/*	apply a secondary key ontop of an already initialized instance
*/
Crypt* Crypt::apply_key2(cstr qstr)
{
	if (qstr && *qstr)
	{
		uint qlen = (uint)strlen(qstr);
		uint zlen = qlen / 8;

		assert(zlen < NN);

		for (uint i = 0; i < zlen; i++) { mt[i] ^= ((uint64*)qstr)[i]; }
		for (uint i = qlen; i-- > zlen * 8;) { ((ptr)mt)[i] ^= qstr[i]; }
	}
	return spread_key2();
}

Crypt* Crypt::apply_key2(uint64 key2[], uint32 count)
{
	if (count > NN) count = NN;
	for (uint i = 0; i < count; i++) mt[i] ^= key2[i];
	return spread_key2();
}


/*	encrypt or decrypt buffer bu[sz]
*/
void Crypt::crypt(uint8 bu[], uint sz)
{
	for (uint64* p = ((uint64*)bu) + (sz >> 3); (uptr)p > bu;) { *--p ^= this->random64(); }

	if (sz & 7)
	{
		uint64 q = this->random64();
		while (sz & 7)
		{
			--sz;
			bu[sz] ^= ((uptr)&q)[sz & 7];
		}
	}
}


/*	encrypt a text string

	Anwendung: Verschlüsseln von Dateinamen

	in:	 this = mit einem bekannten, reproduzierbaren Status initialisiert
		 qstr = zu verschlüsselnder String
	out: verschlüsselter, base85-codierter String
		 der verschlüsselte String ist um 2 + 20% (aufgerundet) Zeichen länger
*/
str Crypt::encrypt(cstr q) const
{
	if (q == nullptr || *q == 0) return tempstr(0);

	// Create copy of data:
	uint  zlen = strLen(q);
	uint8 z[zlen + 2];
	memcpy(z + 2, q, zlen);
	poke2Z(z, hash(q));

	// Encrypt in place:
	Crypt(*this, peek2Z(z)).encrypt(z + 2, zlen);

	// Encode Base85:
	uint rlen = sizeAfterBase85Encoding(zlen + 2);
	str	 r	  = tempstr(rlen);
	encodeBase85(z, zlen + 2, (uptr)r, rlen);
	return r;
}

str Crypt::decrypt(cstr q) const
{
	// Decode Base85:
	uint qlen = strLen(q);
	uint zlen = sizeAfterBase85Decoding(qlen);
	if (zlen < 2) return zlen == 0 ? tempstr(0) : nullptr;
	str z	= tempstr(zlen);
	int err = decodeBase85((uptr)q, qlen, (uptr)z, zlen);
	if (err) return nullptr;

	// Decrypt in place:
	Crypt(*this, peek2Z(z)).decrypt((uptr)z + 2, zlen - 2);
	return z + 2;
}

/*	encrypt path
	Der Pfad wird an den "/" getrennt und abschnittsweise gecryptet.
	Der MT wird dabei so verändert, dass danach der nächste Pfadabschnitt,
	oder z.B. ein Directory-Eintrag, damit verschlüsselt werden kann.
*/
str Crypt::encrypt_path(str qpath)
{
	mti	  = 0;
	ptr p = strchr(qpath, '/');
	if (p == nullptr) return encrypt(qpath);

	*p			= 0;
	char* zpath = encrypt(qpath);
	apply_key2(qpath);
	*p = '/';

	return *(p + 1) ? catstr(zpath, "/", encrypt_path(p + 1)) : catstr(zpath, "/");
}


/*	decrypt path
	Der Pfad wird an den "/" getrennt und abschnittsweise gecryptet.
	Der MT wird dabei so verändert, dass danach der nächste Pfadabschnitt,
	oder z.B. ein Directory-Eintrag, damit verschlüsselt werden kann.
	Diese Status-Veränderung ist dabei die gleiche wie beim Verschlüsseln.
*/
str Crypt::decrypt_path(str qpath)
{
	mti	  = 0;
	ptr p = strchr(qpath, '/');
	if (p == nullptr) return decrypt(qpath);

	*p			= 0;
	char* zpath = decrypt(qpath);
	apply_key2(zpath);
	*p = '/';

	return *(p + 1) ? catstr(zpath, "/", decrypt_path(p + 1)) : catstr(zpath, "/");
}


#ifndef NDEBUG

namespace q
{
static void test_crypt()
{
	Crypt mt("67647567");
	cstr  q = "!nuzu&/V$ub6&%R65ggutftfgu6i7&TIÜP,,üpÄP,üp,)N(nc4§ßQ";
	str	  z = mt.encrypt(q);
	z		= mt.decrypt(z);
	assert(eq(q, z));

	mt.reset_mti();
	q = dupstr("abcde/fghij/12345"); // dupstr("/zt67b/rcRdz/älpm98/6565r68u/");
	z = mt.encryptPath(q);
	z = mt.decryptPath(z);
	assert(eq(q, z));
}

ON_INIT(test_crypt);
} // namespace q

#endif
