/*	Copyright  (c)	Günter Woigk 2013 - 2018
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

#include "md5_tools.h"
#include "md5.h"
#include "../unix/FD.h"


void calc_md5(FD& fd, uint64 start, uint64 count, uint8 md5[16]) throw(file_error)
{
	MD5Context md5_context;
	MD5Init(&md5_context);

	const uint bsize = 8*1024;
	fd.seek_fpos(start);
	uint8 bu[bsize];

	while(count)
	{
		uint32 n = bsize; if(count<bsize) n = (uint32)count;
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


void check_md5(const uint8 data[], uint32 count, const uint8 md5[16], cstr msg) throw(data_error)
{
	uint8 mymd5[16];
	calc_md5(data,count,mymd5);
	if(memcmp(md5,mymd5,16)) throw data_error(md5error,msg);
}













