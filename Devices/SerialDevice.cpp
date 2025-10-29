// Copyright (c) 2023 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "SerialDevice.h"
#include "cstrings/tempmem.h"
#include <memory>
#include <stdarg.h>
#include <string.h>
#include <time.h>


namespace kio
{

uint32 SerialDevice::ioctl(IoCtl cmd, void*, void*)
{
	// default implementation

	switch (cmd.cmd)
	{
	case IoCtl::FLUSH_OUT: return 0;
	default: throw INVALID_ARGUMENT;
	}
}

int SerialDevice::getc(uint timeout_us)
{
	// default implementation for getc() with timeout
	// FIXME: the code assumes that we will be interrupted by a signal,
	//		  else the whole timeout will be waited if no byte is immediately available!

	if (read(&last_char, 1, true) == 0)
	{
		timespec dur;
		dur.tv_sec	= timeout_us / 1000000;
		dur.tv_nsec = (timeout_us % 1000000) * 1000;

		while (read(&last_char, 1, true) == 0)
		{
			int result = nanosleep(&dur, &dur);
			if (result == 0) return -1; // timed out
			if (errno != EINTR) throw strerror(errno);
		}
	}
	return last_char;
}

char SerialDevice::getc()
{
	if (read(&last_char, 1, true) == 0)
	{
		timespec dur;
		dur.tv_sec	= 0;
		dur.tv_nsec = 50 * 1000; // 50 µs
		while (read(&last_char, 1, true) == 0) { nanosleep(&dur, nullptr); }
	}
	return last_char;
}

str SerialDevice::gets(uint line_ends)
{
	// read string from Device
	// handles DOS line ends

	char buffer[gets_max_len];
	char last_eol = this->last_char;

	uint i = 0;
	while (i < gets_max_len)
	{
		char c = getc();
		if (uchar(c) >= 32 || (1 << c) & ~line_ends)
		{
			buffer[i++] = c;
			continue;
		}

		// skip cr after nl or vice versa:
		if (i == 0 && c == 23 - last_eol)
		{
			last_eol = 0;
			continue;
		}

		// line end:
		break;
	}

	str s = tempstr(i);
	memcpy(s, buffer, i);
	return s;
}

void SerialDevice::putc(char c)
{
	write(&c, 1); //
}

void SerialDevice::puts(cstr s)
{
	if (s) write(s, SIZE(strlen(s))); //
}

void SerialDevice::printf(cstr fmt, ...)
{
	char bu[100];

	va_list va;
	va_start(va, fmt);
	int n = vsnprintf(bu, sizeof(bu), fmt, va);
	va_end(va);

	assert(n >= 0);
	SIZE size = uint(n);
	if (size < 100) return (void)write(bu, size);

	//TempMemSave _;
	TempMemPool _;

	ptr bp = tempstr(size);

	va_start(va, fmt);
	vsnprintf(bp, size + 1, fmt, va);
	va_end(va);

	write(bp, size);
}

void SerialDevice::write_nstr(cstr s)
{
	if (s)
	{
		uint32 len = uint32(strlen(s));

		if (len >= 253)
			if (len >> 16)
			{
				write<uint8>(255);
				write_LE<uint32>(len);
			}
			else
			{
				write<uint8>(254);
				write_LE<uint16>(uint16(len));
			}
		else { write<uint8>(uint8(len)); }

		write(s, len);
	}
	else write<uint8>(253); // 253 => nullptr !
}

str SerialDevice::read_nstr()
{
	uint32 len = read<uint8>();
	if (len >= 253)
	{
		if (len == 253) return nullptr;
		else len = len == 255 ? read_LE<uint32>() : read_LE<uint16>();
	}
	str s = tempstr(len);
	read(s, len);
	return s;
}


} // namespace kio


/*






































*/
