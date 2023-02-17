// Copyright (c) 1999 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/*
	custom exceptions
*/

#include "exceptions.h"
#include "kio/kio.h"
#include "unix/FD.h"
#include "unix/s_type.h"


// helper
static cstr filename(cstr file) noexcept
{
	cstr p = strrchr(file, '/');
	return p ? p + 1 : file;
}


// ---------------------------------------------
//			AnyError
// ---------------------------------------------

AnyError::AnyError(cstr msg, va_list va) noexcept : msg(newcopy(usingstr(msg, va))), err(customerror) {}

AnyError::AnyError(cstr format, ...) noexcept : err(customerror)
{
	va_list va;
	va_start(va, format);
	msg = newcopy(usingstr(format, va));
	va_end(va);
}

AnyError::AnyError(int e, cstr msg) noexcept : msg(newcopy(msg)), err(e) {}

AnyError::AnyError(AnyError&& q) noexcept : std::exception(std::move(q)), msg(q.msg), err(q.err) { q.msg = nullptr; }

AnyError::AnyError(const AnyError& q) noexcept : std::exception(q), msg(newcopy(q.msg)), err(q.err) {}

AnyError::~AnyError() noexcept { delete[] msg; }

cstr AnyError::what() const noexcept { return msg ? msg : errorstr(err); }


// ---------------------------------------------
//			InternalError
// ---------------------------------------------

InternalError::InternalError(cstr file, uint line, cstr txt) noexcept :
	AnyError(internalerror, usingstr("%s line %u: %s", filename(file), line, txt))
{}

InternalError::InternalError(cstr file, uint line, int e) noexcept : InternalError(file, line, errorstr(e)) {}


// ---------------------------------------------
//			LimitError
// ---------------------------------------------

LimitError::LimitError(cstr where, ulong sz, ulong max) noexcept :
	AnyError(limiterror, usingstr("%s: size %lu exceeds maximum of %lu", where, sz, max))
{}


// ---------------------------------------------
//			DataError
// ---------------------------------------------

DataError::DataError(cstr txt, ...) noexcept : AnyError(dataerror)
{
	va_list va;
	va_start(va, txt);
	msg = newcopy(usingstr(txt, va));
	va_end(va);
}


// ---------------------------------------------
//			FileError
// ---------------------------------------------

FileError::FileError(cstr path, int e) noexcept : AnyError(e), filepath(newcopy(path)), fd(-1) {}

FileError::FileError(cstr path, int e, cstr txt) noexcept : AnyError(e, txt), filepath(newcopy(path)), fd(-1) {}

FileError::FileError(int fd, cstr path, int error) noexcept : AnyError(error), filepath(newcopy(path)), fd(fd) {}

FileError::FileError(int fd, cstr path, int error, cstr txt) noexcept :
	AnyError(error, txt),
	filepath(newcopy(path)),
	fd(fd)
{}

FileError::FileError(const FD& fd, int error) noexcept :
	AnyError(error),
	filepath(newcopy(fd.filepath())),
	fd(fd.file_id())
{}

FileError::FileError(const FD& fd, int error, cstr txt) noexcept :
	AnyError(error, txt),
	filepath(newcopy(fd.filepath())),
	fd(fd.file_id())
{}

FileError::FileError(const FileError& q) noexcept : AnyError(q), filepath(newcopy(q.filepath)), fd(q.fd) {}

FileError::FileError(FileError&& q) noexcept : AnyError(std::move(q)), filepath(q.filepath), fd(q.fd)
{
	q.filepath = nullptr;
}

FileError::~FileError() noexcept { delete[] filepath; }

cstr FileError::what() const noexcept
{
	if (msg) return usingstr("%s in file \"%s\" (%s)", errorstr(err), filepath, msg);
	else return usingstr("%s in file \"%s\"", errorstr(err), filepath);
}
