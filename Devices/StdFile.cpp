// Copyright (c) 2024 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "StdFile.h"
#include <cstring>

namespace kio
{

static cstr modestr_for_mode(FileOpenMode mode)
{
	static_assert(FileOpenMode::READ == 1 + 16, "");
	static_assert(FileOpenMode::WRITE == 2 + 32, "");
	static_assert(FileOpenMode::APPEND == 2 + 4, "");

	// not WRITE => open for reading only
	if ((mode & 2) == 0) return "r";

	// WRITE or READWRITE:
	if (mode & 4) return mode & 1 ? "a+" : "a"; // append
	else if (mode & NEW) return "wx+";			// readwrite
	else return mode & TRUNCATE ? "w+" : "r+";	// readwrite
}

StdFile::StdFile(cstr fpath, FileOpenMode mode) : File(mode)
{
	file = fopen(fpath, modestr_for_mode(mode));
	if (!file) throw "could not open file";
}

StdFile::StdFile(FILE* file, Flags flags) : //
	File(flags | Flags::dont_close),
	file(file)
{}

StdFile::~StdFile()
{
	if (flags & dont_close) return; // stdin, out, err

	FILE* f = file;
	file	= nullptr;

	while (f)
	{
		if (fclose(f) == 0) break;
		if (errno == EINTR) continue;
		debugstr("close() failed\n");
		break;
	}
}

uint32 StdFile::ioctl(IoCtl cmd, void* arg1, void* arg2)
{
	return File::ioctl(cmd, arg1, arg2); //
}

__noreturn void StdFile::throw_error()
{
	Error e = ferror(file) ? strerror(errno) : END_OF_FILE;
	clearerr(file);
	throw e;
}

SIZE StdFile::read(void* data, SIZE size, bool partial)
{
	SIZE nbytes = SIZE(fread(data, 1, size, file));
	if (nbytes == size) return nbytes;
	if (partial && nbytes > 0) return nbytes;
	if (ferror(file) || eof_pending()) throw_error();
	set_eof_pending();
	return 0;
}

char StdFile::getc()
{
	int c = fgetc(file);
	if (c >= 0) return char(c);
	throw_error();
}

int StdFile::getc(__unused uint timeout)
{
	int c = fgetc(file);
	if (c >= 0) return char(c);
	if (ferror(file) || eof_pending()) throw_error();
	set_eof_pending();
	return -1;
}

SIZE StdFile::write(const void* data, SIZE size, bool partial)
{
	SIZE nbytes = SIZE(fwrite(data, 1, size, file));
	if (nbytes == size) return nbytes;
	if (partial && nbytes > 0) return nbytes;
	throw_error();
}

void StdFile::putc(char c)
{
	if (fputc(c, file) < 0) throw_error(); //
}

ADDR StdFile::getSize() const noexcept
{
	clearerr(file);
	long fpos = ftell(file);
	fseek(file, 0, SEEK_END);
	long fsize = ftell(file);
	fseek(file, fpos, SEEK_SET);
	if (!ferror(file))
	{
		if _constexpr (sizeof(ADDR) >= sizeof(fsize)) return ADDR(fsize);
		if (fsize == ADDR(fsize)) return ADDR(fsize);
		debugstr("StdFile: file size exceeds 4GB\n");
		return 0xffffffffu;
	}
	debugstr("StdFile: %s\n", strerror(errno));
	return 0;
	//clearerr(file);
	//throw strerror(errno);
}

ADDR StdFile::getFpos() const noexcept
{
	clearerr(file);
	long fpos = ftell(file);
	if (!ferror(file))
	{
		if _constexpr (sizeof(ADDR) >= sizeof(fpos)) return ADDR(fpos);
		if (fpos == ADDR(fpos)) return ADDR(fpos);
		debugstr("StdFile: file position beyond 4GB\n");
		return 0xffffffffu;
	}
	return 0;
	//clearerr(file);
	//throw dupstr(strerror(errno));
}

void StdFile::setFpos(ADDR pos)
{
	clearerr(file);
	clear_eof_pending();
	fseek(file, long(pos), SEEK_SET);
}

void StdFile::close()
{
	FILE* f = file;
	file	= nullptr;

	while (f)
	{
		if (fclose(f) == 0) break;
		if (errno != EINTR) throw_error();
	}
}

} // namespace kio

/*
































*/
