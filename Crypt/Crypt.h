/*	Copyright  (c)	Günter Woigk 2013 - 2013
                    mailto:kio@little-bat.de

    This file is free software

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    • Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    • Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
    OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
    OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef _Crypt_h_
#define _Crypt_h_


#include "kio/kio.h"
#include "rng/MersenneTwister64.h"


class Crypt : public MersenneTwister64
{

    str 	encrypt_path(str path);
    str 	decrypt_path(str path);
    void	crypt(uint8 bu[], uint sz);
    Crypt*	spread_key2();


public:
    // Constructor mit einfachem ulong als Seed
    // Bei seed=0 wird ein zufälliges Seed generiert.
    explicit Crypt(uint64 seed)						:MersenneTwister64(seed){}
    explicit Crypt(uint32 seed)						:MersenneTwister64(seed){}

    // Constructor mit uint64[] Array als Seed
    // Das Seed kann bis zu 312 Einträge lang sein.
    Crypt(uint64 seed[], uint count)				:MersenneTwister64(seed,count){}

    // Constructor mit einem Passwort als Seed
    // Das Passwort kann bis zu 2496 Zeichen lang sein.
    // NOTE: die Initialisierung auf BIG-ENDIAN ist zu der auf LITTLE-ENDIAN Maschinen inkompatibel!
    explicit Crypt(cstr seed)						:MersenneTwister64(seed){}
    explicit Crypt(cstr key1, cstr key2)			:MersenneTwister64(key1){apply_key2(key2);}

    // Constructor basierend auf anderem Crypter mit Sub-Key.
    Crypt(const Crypt& q, cstr key2)				:MersenneTwister64(q){apply_key2(key2);}
    Crypt(const Crypt& q, uint64 key2)				:MersenneTwister64(q){apply_key2(key2);}
    Crypt(const Crypt& q, uint32 key2)				:MersenneTwister64(q){apply_key2(key2);}
    Crypt(const Crypt& q, uint16 key2)				:MersenneTwister64(q){apply_key2(key2);}
    Crypt(const Crypt& q, uint64 key2[], uint cnt)	:MersenneTwister64(q){apply_key2(key2,cnt);}

    // Copy Creator und Zuweisungs-Operator:
    Crypt(const Crypt& q)							:MersenneTwister64(q){}
    Crypt& operator= (const Crypt& q)				{ return (Crypt&) MersenneTwister64::operator=(q); }

    // Initialisierungserweiterung mit einem weiteren Key:
    Crypt*	apply_key2(uint64);
    Crypt*	apply_key2(uint32);
    Crypt*	apply_key2(uint16);
    Crypt*	apply_key2(uint64[],uint32 count);
    Crypt*	apply_key2(cstr);
    Crypt*	apply_path(cstr s)						{ (void)encrypt_path((str)s); return this; }

    // Encrypt & decrypt:
    void	encrypt(uint8 bu[], uint sz)			{ crypt(bu,sz); }
    void	decrypt(uint8 bu[], uint sz)			{ crypt(bu,sz); }
    str		encrypt(cstr) const;					// encrypt and encode base85
    str		decrypt(cstr) const;					// decode base85 and decrypt
    str 	encryptPath(cstr path) const			{ return Crypt(*this).encrypt_path((str)path); }
    str 	decryptPath(cstr path) const			{ return Crypt(*this).decrypt_path((str)path); }
};




#endif


















