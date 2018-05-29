/*	Copyright  (c)	Günter Woigk 1999 - 2018
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
#include "kio/kio.h"
#include "unix/s_type.h"



#if 0
char custom_errmsg[256];
int	 custom_error = 0;


cstr ETEXT[] =
{
#define		EMAC(a,b)	b
#include	"error_emacs.h"
};


/* ===================================================
        Get Error Text:
        does not allocate memory
   =================================================== */

cstr ErrorStr(int err, bool custom)
{
    if(err==0)						return "no error";
    if(err==-1)						return "unknown error (-1)";
    if(custom && err==custom_error)	return custom_errmsg;
    if(uint(err-EBAS)<NELEM(ETEXT)) return ETEXT[err-EBAS];
    else							return strerror(err);
}


/* ===================================================
        Set Error:
   =================================================== */

// unconditional:
//
void ForceError( int err, cstr msg )
{
    strncpy(custom_errmsg,msg,255);
    errno = custom_error = err;
}

// conditional:
//
void SetError( int err, cstr msg )
{
    if(!errno) ForceError(err,msg);
}
#endif



/* ===================================================
        Exception Messages:
   =================================================== */

#include "../unix/FD.h"

file_error::file_error(class FD& fd, int error) noexcept
: any_error(error),filepath(fd.filepath()),fd(fd.file_id())
{}

//file_error::file_error(class FD& fd, cstr msg) noexcept
//: any_error(errno?errno:customerror,msg),filepath(fd.filepath()),fd(fd.file_id())
//{}

file_error::file_error(class FD& fd, int error, cstr msg) noexcept
: any_error(error,msg),filepath(fd.filepath()),fd(fd.file_id())
{}


// helper
static
cstr filename(cstr file) noexcept
{
    cstr p = strrchr(file,'/');
    return p ? p+1 : file;
}

//virtual
cstr internal_error::what() const noexcept
{
    return usingstr( "%s line %u: %s",
        filename(file), line, text ? text : errorstr(error) );
}

//virtual
cstr limit_error::what() const noexcept
{
    return usingstr( "%s: size %u exceeds maximum of %u", text, sz, max );
}

//ctor with format string:
any_error::any_error(cstr format, ...) noexcept
:
	error(customerror)
{
    va_list va;
    va_start(va,format);
    text = usingstr(format,va);
    va_end(va);
}

//virtual
//	error 	-> errorstr
//	text	-> text
// 	beides	-> errorstr (text)
//
// 			wenn error und text angegeben sind, wird angenommen,
//			dass der text zusätzlicher ein hinweis ist
//
cstr any_error::what() const noexcept
{
    return text
        ? error==customerror || error==-1
            ? text
            : usingstr("%s (%s)",errorstr(error),text)
        : errorstr(error);
}

//virtual
// text==0	-> errorstr: file="filepath"
// text!=0	-> errorstr: file="filepath" (text)
//
// 			wenn auch ein text angegeben ist, wird angenommen,
//			dass dieser zusätzlicher ein hinweis ist
//
cstr file_error::what() const noexcept
{
    return usingstr( text ? "%s: file = \"%s\" (%s)"
                          : "%s: file = \"%s\"",
        errorstr(error), filepath, text);
}


data_error::data_error (cstr msg, ...) noexcept
:
	any_error(dataerror)
{
    va_list va;
    va_start(va,msg);
    text = usingstr(msg,va);
    va_end(va);
}









