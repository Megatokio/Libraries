#pragma once
// Copyright (c) 2013 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"
#include "rng/MersenneTwister64.h"


class Crypt : public MersenneTwister64
{
	str	   encrypt_path(str path);
	str	   decrypt_path(str path);
	str	   encrypt_path(cstr path) { return encrypt_path(dupstr(path)); }
	str	   decrypt_path(cstr path) { return decrypt_path(dupstr(path)); }
	void   crypt(uint8 bu[], uint sz);
	Crypt* spread_key2();


public:
	// Constructor mit einfachem ulong als Seed
	// Bei seed=0 wird ein zufälliges Seed generiert.
	explicit Crypt(uint64 seed) : MersenneTwister64(seed) {}
	explicit Crypt(uint32 seed) : MersenneTwister64(seed) {}

	// Constructor mit uint64[] Array als Seed
	// Das Seed kann bis zu 312 Einträge lang sein.
	Crypt(uint64 seed[], uint count) : MersenneTwister64(seed, count) {}

	// Constructor mit einem Passwort als Seed
	// Das Passwort kann bis zu 2496 Zeichen lang sein.
	// NOTE: die Initialisierung auf BIG-ENDIAN ist zu der auf LITTLE-ENDIAN Maschinen inkompatibel!
	explicit Crypt(cstr seed) : MersenneTwister64(seed) {}
	explicit Crypt(cstr key1, cstr key2) : MersenneTwister64(key1) { apply_key2(key2); }

	// Constructor basierend auf anderem Crypter mit Sub-Key.
	Crypt(const Crypt& q, cstr key2) : MersenneTwister64(q) { apply_key2(key2); }
	Crypt(const Crypt& q, uint64 key2) : MersenneTwister64(q) { apply_key2(key2); }
	Crypt(const Crypt& q, uint32 key2) : MersenneTwister64(q) { apply_key2(key2); }
	Crypt(const Crypt& q, uint16 key2) : MersenneTwister64(q) { apply_key2(key2); }
	Crypt(const Crypt& q, uint64 key2[], uint cnt) : MersenneTwister64(q) { apply_key2(key2, cnt); }

	// Copy Creator und Zuweisungs-Operator:
	Crypt(const Crypt& q) : MersenneTwister64(q) {}
	Crypt& operator=(const Crypt& q)
	{
		MersenneTwister64::operator=(q);
		return *this;
	}

	// Initialisierungserweiterung mit einem weiteren Key:
	Crypt* apply_key2(uint64);
	Crypt* apply_key2(uint32);
	Crypt* apply_key2(uint16);
	Crypt* apply_key2(uint64[], uint32 count);
	Crypt* apply_key2(cstr);
	Crypt* apply_path(cstr s)
	{
		(void)encrypt_path(s);
		return this;
	}

	// Encrypt & decrypt:
	void encrypt(uint8 bu[], uint sz) { crypt(bu, sz); }
	void decrypt(uint8 bu[], uint sz) { crypt(bu, sz); }
	str	 encrypt(cstr) const; // encrypt and encode base85
	str	 decrypt(cstr) const; // decode base85 and decrypt
	str	 encryptPath(cstr path) const { return Crypt(*this).encrypt_path(path); }
	str	 decryptPath(cstr path) const { return Crypt(*this).decrypt_path(path); }
};
