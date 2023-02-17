// Copyright (c) 2019 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#undef NDEBUG
#define SAFETY	 2
#define LOGLEVEL 1
#include "kio/kio.h"
#include "main.h"
#include "util/count1bits.h"
#include "util/msbit.h"
#include "util/swap.h"

void test_kio_util(uint& num_tests, uint& num_errors)
{
	logIn("test kio/util/*");

	TRY // validate byte order:
		const char abcd[5] = "abcd";
	assert(peek4X(abcd) == 'abcd');
	assert(peek4Z(abcd) == 'dcba');
	END

		TRY assert(count1bits(int(0x01020304)) == 5);
	assert(count1bits(uint(0x01020304)) == 5);
	assert(count1bits(int32(0x01020304)) == 5);
	assert(count1bits(uint32(0x01020304)) == 5);
	assert(count1bits(int16(0x0304)) == 3);
	assert(count1bits(uint16(0x0304)) == 3);
	assert(count1bits(int8(0x3b)) == 5);
	assert(count1bits(uint8(0x3b)) == 5);
	assert(count1bits(0xf000c000a000555f) == 18);
	assert(count1bits(uint64(0)) == 0);
	assert(count1bits(uint32(0)) == 0);
	assert(count1bits(uint16(0)) == 0);
	assert(count1bits(uint8(0)) == 0);
	assert(count1bits(0xffffffffffffffff) == 64);
	assert(count1bits(uint32(0xffffffff)) == 32);
	assert(count1bits(uint16(0xffff)) == 16);
	assert(count1bits(uint8(0xff)) == 8);

	for (int64 a = 0x01f20304938a2f70, n = 0; n < 64; n++, a = (a << 1) + (a < 0)) { assert(count1bits(a) == 24); }
	for (int32 a = 0x01f20304, n = 0; n < 32; n++, a = (a << 1) + (a < 0)) { assert(count1bits(a) == 9); }
	for (int16 a = 0x01f2, n = 0; n < 16; n++, a = int16((a << 1) + (a < 0))) { assert(count1bits(a) == 6); }
	for (int8 a = 0x2f, n = 0; n < 8; n++, a = int8((a << 1) + (a < 0))) { assert(count1bits(a) == 5); }
	END

		TRY uint8 bu[] = {0xa8, 0x30, 0xc0, 0x00,		 0xa0, 0x00, 0x55, 0x5f /*19*/, 0xF0, 0xF0, 0xF0, 0xF0,
						  0xF0, 0xF0, 0xF0, 0xF0 /*32*/, 0x0a, 0xaa, 0x1b, 0xbb,		0x2c, 0xcc, 0x3d, 0xef /*35*/,
						  0xff, 0xff, 0xff, 0xff,		 0xff, 0xff, 0xff, 0xff /*64*/, 0x00, 0x00, 0x00, 0x00,
						  0x00, 0x00, 0x00, 0x00 /*0*/,	 0x62, 0x73, 0x65, 0x25,		0x72, 0x83, 0x57, 0x87 /*31*/};

	assert(count1bits(u64ptr(bu), 6) == 19 + 32 + 35 + 64 + 31);
	assert(count1bits(u32ptr(bu), 12) == 19 + 32 + 35 + 64 + 31);
	assert(count1bits(u16ptr(bu), 24) == 19 + 32 + 35 + 64 + 31);
	assert(count1bits(u8ptr(bu), 48) == 19 + 32 + 35 + 64 + 31);
	assert(count1bitsL(u8ptr(bu), 48 * 8) == 19 + 32 + 35 + 64 + 31);
	assert(count1bitsR(u8ptr(bu), 48 * 8) == 19 + 32 + 35 + 64 + 31);

	assert(count1bits(u32ptr(bu) + 1, 12 - 2) == 19 - 7 + 32 + 35 + 64 + 31 - 16);
	assert(count1bits(u16ptr(bu) + 1, 24 - 2) == 19 - 5 + 32 + 35 + 64 + 31 - 9);
	assert(count1bits(u8ptr(bu) + 1, 48 - 2) == 19 - 3 + 32 + 35 + 64 + 31 - 4);
	assert(count1bitsL(u8ptr(bu) + 1, (48 * 8) - 8 - 3) == 19 - 3 + 32 + 35 + 64 + 31 - 3);
	assert(count1bitsR(u8ptr(bu) + 2, (48 * 8) - 16 - 3) == 19 - 5 + 32 + 35 + 64 + 31 - 1);
	END

		TRY assert(msbit(0x20000000) == 29);
	assert(msbit(0x3fffffff) == 29);
	assert(msbit(0x20000000u) == 29);
	assert(msbit(0x3fffffffu) == 29);
	assert(msbit(int16(0x2000)) == 13);
	assert(msbit(int16(0x3fff)) == 13);
	assert(msbit(uint8(0x20)) == 5);
	assert(msbit(int8(0x3f)) == 5);
	assert(msbit(0) == 0);
	for (int i = 0; i < 32; i++) { assert(msbit(1 << i) == i); }
	END

		TRY assert(reqBits(0) == 0);
	assert(reqBits(1) == 0);
	assert(reqBits(2) == 1);
	assert(reqBits(3) == 2);
	assert(reqNibbles(0) == 0);
	assert(reqNibbles(1) == 0);
	assert(reqNibbles(2) == 1);
	assert(reqNibbles(16) == 1);
	assert(reqNibbles(17) == 2);
	assert(reqBytes(0) == 0);
	assert(reqBytes(1) == 0);
	assert(reqBytes(2) == 1);
	assert(reqBytes(256) == 1);
	assert(reqBytes(257) == 2);
	END

		TRY assert(swap(uint16(0x1234)) == 0x3412);
	assert(swap(uint32(0x12345678)) == 0x78563412);
	assert(swap(uint64(0x123456789abcdef0)) == 0xf0debc9a78563412);
	END
}
