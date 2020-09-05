/*	Copyright  (c)	Günter Woigk 2013 - 2020
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY, NOT EVEN THE
	IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE
	AND IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY DAMAGES
	ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/

#include "md5_tools.h"
#include "md5.h"
#include "unix/FD.h"


void calc_md5(FD& fd, uint64 start, uint64 count, uint8 md5[16]) throws
{
	MD5Context md5_context;
	MD5Init(&md5_context);

	const uint bsize = 8*1024;
	fd.seek_fpos(start);
	uint8 bu[bsize];

	while (count)
	{
		uint32 n = bsize; if (count<bsize) n = (uint32)count;
		n = fd.read_bytes(bu,n);
		count -= n;
		MD5Update(&md5_context,bu,n);
	}

	MD5Final(&md5_context,md5);
}


void calc_md5(const uint8 data[], uint32 count, uint8 md5[16])
{
	MD5Context md5_context;
	MD5Init(&md5_context);
	MD5Update(&md5_context,data,count);
	MD5Final(&md5_context,md5);
}


void check_md5(const uint8 data[], uint32 count, const uint8 md5[16], cstr msg) throws
{
	uint8 mymd5[16];
	calc_md5(data,count,mymd5);
	if (memcmp(md5,mymd5,16)) throw DataError(md5error,msg);
}














