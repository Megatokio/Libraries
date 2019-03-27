/*	Copyright  (c)	Günter Woigk 1999 - 2019
                    mailto:kio@little-bat.de

    This file is free software

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    Permission to use, copy, modify, distribute, and sell this software and
    its documentation for any purpose is hereby granted without fee, provided
    that the above copyright notice appear in all copies and that both that
    copyright notice and this permission notice appear in supporting
    documentation, and that the name of the copyright holder not be used
    in advertising or publicity pertaining to distribution of the software
    without specific, written prior permission. The copyright holder makes no
    representations about the suitability of this software for any purpose.
    It is provided "as is" without express or implied warranty.

    THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
    INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
    EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
    CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
    DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
    TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
    PERFORMANCE OF THIS SOFTWARE.

    error handling

note:
    include errors.cpp:  support for cstrings only, does not require class String
    _or_    serrors.cpp: support for cstrings and class String strings


    1999-01-01	first work on this file
    2001-02-17	added possibility to #include custom error file
    2001-11-09	added support for class String
    2011-01-17	modified to allow simple clearing of errno in most cases
*/

#include "config.h"
#include "Libraries/kio/kio.h"
#include "Libraries/unix/s_type.h"
#include "Libraries/unix/FD.h"


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
  filepath(newcopy(path))
{}

file_error::file_error (cstr path, int error, cstr msg) noexcept
: any_error(error,msg),
  filepath(newcopy(path))
{}

file_error::file_error (const FD& fd, int error) noexcept
: any_error(error),
  filepath(newcopy(fd.filepath()))
{}

file_error::file_error (const FD& fd, int error, cstr msg) noexcept
: any_error(error,msg),
  filepath(newcopy(fd.filepath()))
{}

file_error::file_error(file_error const& q) noexcept
: any_error(q),
  filepath(newcopy(q.filepath))
{}

file_error::file_error(file_error&& q) noexcept
: any_error(q),
  filepath(q.filepath)
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










