// Copyright (c) 2013 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "md5_tools.h"
#include "md5.h"
#include "unix/FD.h"


void calc_md5(FD& fd, uint64 start, uint64 count, uint8 md5[16]) throws
{
	MD5Context md5_context;
	MD5Init(&md5_context);

	const uint bsize = 8 * 1024;
	fd.seek_fpos(start);
	uint8 bu[bsize];

	while (count)
	{
		uint32 n = bsize;
		if (count < bsize) n = (uint32)count;
		n = fd.read_bytes(bu, n);
		count -= n;
		MD5Update(&md5_context, bu, n);
	}

	MD5Final(&md5_context, md5);
}


void calc_md5(const uint8 data[], uint32 count, uint8 md5[16])
{
	MD5Context md5_context;
	MD5Init(&md5_context);
	MD5Update(&md5_context, data, count);
	MD5Final(&md5_context, md5);
}


void check_md5(const uint8 data[], uint32 count, const uint8 md5[16], cstr msg) throws
{
	uint8 mymd5[16];
	calc_md5(data, count, mymd5);
	if (memcmp(md5, mymd5, 16)) throw DataError(md5error, msg);
}
