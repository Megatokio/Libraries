// Copyright (c) 2025 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Utf8Decoder.h"
#include "cstrings/utf8.h"

namespace kio
{
using namespace utf8;

Utf8Decoder::Utf8Decoder(SerialDevicePtr dest) : SerialDevice(dest->flags), dest(std::move(dest)) {}


static cptr last_utf8char(cptr p, uint sz)
{
	// nullptr => text contained only fups (or was "")

	cptr e = p + sz - 1;
	while (e > p && utf8::is_fup(*e)) e--;
	return e >= p ? e : nullptr;
}

SIZE Utf8Decoder::read(void* _data, SIZE size, bool partial)
{
	if (size == 0) return 0;
	ucs1char* data	= reinterpret_cast<ucs1char*>(_data);
	uint	  total = 0;

	const uint busize = 100;
	char	   bu[busize];

	uint avail = pending_inbytes;
	if (avail) memcpy(bu, inbytes, avail);

	while (size > 0)
	{
		// read more:
		uint n = min(size, busize - avail);			 // bytes to read
		n	   = dest->read(bu + avail, n, partial); // actually read
		if (n == 0) break;							 // partial==true and input empty
		avail += n;

		// find start of last char:
		cptr p = last_utf8char(bu, avail);
		if (p == nullptr) // leer oder nur fups
		{
			avail = 0;
			continue;
		}

		// check if last char is truncated: (not yet fully read)
		uint rest = uint(bu + avail - p);
		if (rest == nominal_size(p)) rest = 0;

		// read and convert the fully received bytes:
		n = uint(utf8_to_ucs1(bu, avail - rest, data) - data);
		total += n;
		data += n;
		size -= n;

		// clear buffer except for the last truncated char:
		if (rest) memmove(bu, bu + avail - rest, rest);
		avail = rest;
	}

	if (avail)
	{
		assert(avail <= 5);
		assert(avail < nominal_size(&bu[0]));
		assert(!is_fup(bu[0]));
		for (uint i = 1; i < avail; i++) assert(is_fup(bu[i]));
		memcpy(inbytes, bu, avail);
	}
	pending_inbytes = avail;

	return total;
}

SIZE Utf8Decoder::write(const void* _data, SIZE size, bool partial)
{
	cptr data = cptr(_data);

	// first get rid of a pending output char:
	if (pending_outbyte)
	{
		SIZE n = dest->write(&pending_outbyte, 1, partial);
		if (n) pending_outbyte = 0;
		else return 0;
	}

	SIZE written = 0;

	while (size)
	{
		uint cnt = 0;
		while (cnt < size && is_ascii(data[cnt])) cnt++;

		if (cnt) // send ascii chars en block:
		{
			SIZE n = dest->write(data, cnt, partial);
			written += n;
			data += n;
			size -= n;
			if (n == cnt) continue;
			else break;
		}
		else // send non-ascii chars one per char:
		{
			char bu[2];
			char c = data[0];
			bu[0]  = char(0xC0 + (uchar(c) >> 6));
			bu[1]  = char(0x80 + (uchar(c) & 0x3F));
			SIZE n = dest->write(bu, 2, partial);
			if (n == 0) break; // nothing written
			written++;
			data++;
			size--;
			if (n == 2) continue; // all written
			pending_outbyte = bu[1];
			break;
		}
	}

	return written;
}

uint32 Utf8Decoder::ioctl(IoCtl cmd, void* arg1, void* arg2)
{
	switch (cmd.cmd)
	{
	case IoCtl::FLUSH_OUT:
		if (pending_outbyte) dest->write(nullptr, 0, false);
		break;
	case IoCtl::FLUSH_IN:
		pending_outbyte = 0; //
		break;
	default: //
		break;
	}
	return dest->ioctl(cmd, arg1, arg2);
}


} // namespace kio

/*































*/
