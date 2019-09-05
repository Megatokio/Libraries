/*	Copyright  (c)	Günter Woigk 1999 - 2019
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


	custom exceptions
*/

#include "kio/kio.h"
#include "unix/s_type.h"
#include "unix/FD.h"


// helper
static cstr filename(cstr file) noexcept
{
	cstr p = strrchr(file,'/');
	return p ? p+1 : file;
}


// ---------------------------------------------
//			any_error
// ---------------------------------------------

any_error::any_error(cstr format, ...) noexcept
: error(customerror)
{
	va_list va;
	va_start(va,format);
	text = newcopy(usingstr(format,va));
	va_end(va);
}

any_error::any_error (cstr msg, va_list va)	noexcept
: error(customerror),
  text(newcopy(usingstr(msg,va)))
{}

any_error::any_error (int error, cstr msg) noexcept
: error(error),
  text(newcopy(msg))
{}

any_error::any_error (any_error&& q) noexcept
: std::exception(q),
  error(q.error),
  text(q.text)
{
	q.text=nullptr;
}

any_error::any_error(any_error const& q) noexcept
: std::exception(q),
  error(q.error),
  text(newcopy(q.text))
{}

any_error::~any_error () noexcept
{
	delete[] text;
}

cstr any_error::what() const noexcept
{
	return text ? text : errorstr(error);
}


// ---------------------------------------------
//			internal_error
// ---------------------------------------------

cstr internal_error::what() const noexcept
{
	return usingstr( "%s line %u: %s",
		filename(file), line, text ? text : errorstr(error) );
}


// ---------------------------------------------
//			limit_error
// ---------------------------------------------

limit_error::limit_error (cstr where, ulong sz, ulong max) noexcept
: any_error(limiterror)
{
	text = newcopy(usingstr( "%s: size %lu exceeds maximum of %lu", where, sz, max ));
}


// ---------------------------------------------
//			data_error
// ---------------------------------------------

data_error::data_error (cstr msg, ...) noexcept
: any_error(dataerror)
{
	va_list va;
	va_start(va,msg);
	text = newcopy(usingstr(msg,va));
	va_end(va);
}


// ---------------------------------------------
//			file_error
// ---------------------------------------------

file_error::file_error (cstr path, int error) noexcept
: any_error(error),
  filepath(newcopy(path)),
  fd(-1)
{}

file_error::file_error (cstr path, int error, cstr msg) noexcept
: any_error(error,msg),
  filepath(newcopy(path)),
  fd(-1)
{}

file_error::file_error (cstr path, int fd, int error) noexcept
: any_error(error),
  filepath(newcopy(path)),
  fd(fd)
{}

file_error::file_error (cstr path, int fd, int error, cstr msg) noexcept
: any_error(error,msg),
  filepath(newcopy(path)),
  fd(fd)
{}

file_error::file_error (const FD& fd, int error) noexcept
: any_error(error),
  filepath(newcopy(fd.filepath())),
  fd(fd.file_id())
{}

file_error::file_error (const FD& fd, int error, cstr msg) noexcept
: any_error(error,msg),
  filepath(newcopy(fd.filepath())),
  fd(fd.file_id())
{}

file_error::file_error(file_error const& q) noexcept
: any_error(q),
  filepath(newcopy(q.filepath)),
  fd(q.fd)
{}

file_error::file_error(file_error&& q) noexcept
: any_error(std::move(q)),
  filepath(q.filepath),
  fd(q.fd)
{
	q.filepath = nullptr;
}

file_error::~file_error () noexcept
{
	delete[] filepath;
}

cstr file_error::what() const noexcept
{
	if(text) return usingstr("%s in file \"%s\" (%s)",errorstr(error),filepath,text);
	else	 return usingstr("%s in file \"%s\"",     errorstr(error),filepath);
}










