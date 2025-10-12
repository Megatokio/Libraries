// Copyright (c) 2023 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "File.h"

namespace kio
{

Flags File::flags_for_mode(FileOpenMode mode) noexcept
{
	static_assert(FileOpenMode::READ == 1 + 16, "");  // READ has also bit EXIST
	static_assert(FileOpenMode::WRITE == 2 + 32, ""); // WRITE has also bit TRUNCATE
	static_assert(FileOpenMode::APPEND == 2 + 4, ""); // APPEND has also bit WRITE

	Flags flags = Flags(mode & 3);
	return mode & 4 ? flags | Flags::APPEND_MODE : flags;
}

uint32 File::ioctl(IoCtl cmd, void*, void*)
{
	// default implementation

	switch (cmd.cmd)
	{
	case IoCtl::CTRL_SYNC: return 0;
	default: throw INVALID_ARGUMENT;
	}
}

int File::getc(__unused uint timeout_us)
{
	SIZE count = read(&last_char, 1, true);
	if (count) return uchar(last_char);
	if (eof_pending()) throw END_OF_FILE;
	else set_eof_pending();
	return -1;
}

char File::getc()
{
	read(&last_char, 1, false);
	return last_char;
}

void File::skipBytes(off_t n)
{
	off_t fpos = getFpos() + n;
	if (fpos < 0) throw strerror(EINVAL);
	setFpos(ADDR(fpos));
}

} // namespace kio
