/*	Copyright  (c)	Günter Woigk 2013 - 2019
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


    convert binary data into 7-bit ascii string
    the size grows by 25% (round up)

    the following characters are not used:

    must not include:
        / 	general path separator
        "	WebDAV Strato fails
        %	WebDAV Strato fails
        ?	WebDAV Strato fails
        127	rubbout, may be not printable
        .	only if 1st char: hides file (unix)
        *	might be taken as a wild card
        \ 	might be used to escape the next char(s)
        | 	might me used for pipe (unix)
        & 	might be used for html entities (Strato?)
        `	unix? strato?

    not left out due to requirement for 85 characters (only 11 of 96 can be left out):

        :	might be used for path separator (dos) or protocol separator (url)
        @	might be used for email address

    the encoded strings are suitable for file names on most systems.
*/

#include "kio/kio.h"
#include "kio/peekpoke.h"
#include "base85.h"


/* base85 encoding translation tables:
*/
static uint8 base85[86] = " !#$'()+,-0123456789:;<=>@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_abcdefghijklmnopqrstuvwxyz{}~";
//						  " !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~"

#define x 0xff
static uint8 unbase85[128] =
{
    x,x,x,x,x,x,x,x,         x,x,x,x,x,x,x,x,			// 00 - 0F
    x,x,x,x,x,x,x,x,         x,x,x,x,x,x,x,x,			// 10 - 1F
    0,1,x,2,3,x,x,4,		 5,6,x,7,8,9,x,x,			// ' ' - '/'
    10,11,12,13,14,15,16,17, 18,19,20,21,22,23,24,x,	// '0' - '?'
    25,26,27,28,29,30,31,32, 33,34,35,36,37,38,39,40,	// '@' - 'O'
    41,42,43,44,45,46,47,48, 49,50,51,52,x,53,54,55,	// 'P' - '_'
    x,56,57,58,59,60,61,62,  63,64,65,66,67,68,69,70,	// '`' - 'o'
    71,72,73,74,75,76,77,78, 79,80,81,82,x,83,84,x		// 'p' - 127
};
#undef x



uint sizeAfterBase85Encoding(uint qlen)
{
    return qlen/4*5 + (qlen&3) + ((qlen&3)!=0);
}

void encodeBase85( cuptr q, uint qlen, uptr z, uint zlen )
{
    assert(zlen>=sizeAfterBase85Encoding(qlen));

    uint8 const* e = q + (qlen & ~3);

    while(q<e)
    {
        uint32 c = peek4Z(q); q+=4;		// first char -> low byte
        *z++ = base85[c%85]; c /= 85;
        *z++ = base85[c%85]; c /= 85;
        *z++ = base85[c%85]; c /= 85;
        *z++ = base85[c%85]; c /= 85;
        *z++ = base85[c%85]; c /= 85;
    }
    if((qlen&=3))	// rest von 1, 2 oder 3 char?
    {
        uint32 c = q[0];
        if(qlen>1) c |= q[1]<<8;
        if(qlen>2) c |= q[2]<<16;

        *z++ = base85[c%85]; c /= 85;
        *z++ = base85[c%85]; c /= 85;
        if(qlen>1) { *z++ = base85[c%85]; c /= 85;
        if(qlen>2) { *z++ = base85[c%85]; c /= 85; }}
    }
}

uint sizeAfterBase85Decoding(uint qlen)
{
    return qlen/5*4 + qlen%5 -((qlen%5)!=0);
}

int decodeBase85( cuptr q, uint qlen, uptr z, uint zlen )
{
    assert(zlen>=sizeAfterBase85Decoding(qlen));

    if(qlen%5==1) return error;			// error: impossible length

    uptr e = z + (zlen & ~3);
    uint c1,c2,c3,c4,c5;

    while(z<e)
    {
        c5 = unbase85[*q++];
        c4 = unbase85[*q++];
        c3 = unbase85[*q++];
        c2 = unbase85[*q++];
        c1 = unbase85[*q++];

        if((c1|c2|c3|c4|c5)&0x80) return error;			// error: ill. byte in base85 buffer

        poke4Z(z, (((c1*85+c2)*85+c3)*85+c4)*85+c5);	// read low byte first, store low byte first
        z += 4;
    }

    uint rest = zlen&3;
    if(rest)
    {
        c5 = unbase85[q[0]];
        c4 = unbase85[q[1]];
        c3 = rest>1 ? unbase85[q[2]] : 0;
        c2 = rest>2 ? unbase85[q[3]] : 0;

        if((c2|c3|c4|c5)&0x80) return error;

        uint32 c = ((c2*85+c3)*85+c4)*85+c5;

        *z++ = c;
        if(rest>1) *z++ = c>>8;
        if(rest>2) *z++ = c>>16;
    }

    return ok;
}


#ifndef NDEBUG
void test_base85()
{
    assert(sizeAfterBase85Encoding(0)==0);
    assert(sizeAfterBase85Encoding(1)==2);
    assert(sizeAfterBase85Encoding(4)==5);
    assert(sizeAfterBase85Encoding(5)==7);

    encodeBase85(NULL,0,NULL,0);
    decodeBase85(NULL,0,NULL,0);

    cstr q = "1234567890ß´^qwertzuiopü+asdfghjklöä#<yxcvbnm,.-°!§$%&/()=?`QWERTZUIOPÜ*ASDFGHJKLÖÄ'YXCVBNM;:_~º∆∫®∂√Ω€t!\n&)(/B)";
    str  z = tempstr(sizeAfterBase85Encoding((uint)strlen(q)));
    str  r = dupstr(q);

    while(*q)
    {
        uint qlen = (uint)strlen(q)+1;
        uint zlen = sizeAfterBase85Encoding(qlen);
        uint rlen = qlen;

        assert(sizeAfterBase85Decoding(zlen)==rlen);

        encodeBase85((uptr)q,qlen,(uptr)z,zlen);
        decodeBase85((uptr)z,zlen,(uptr)r,rlen);

        assert(eq(q,r));

        q++;
    }
}
ON_INIT(test_base85);
#endif




















