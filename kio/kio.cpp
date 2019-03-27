/*	Copyright  (c)	Günter Woigk 2017 - 2019
					mailto:kio@little-bat.de

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Permission to use, copy, modify, distribute, and sell this software and
	its documentation for any purpose is hereby granted without fee, provided
	that the above copyright notice appear in all copies and that both that
	copyright notice and this permission notice appear in supporting
	documentation, and that the name of the copyright holder not be used
	in advertising or publicity pertaining to distribution of the software
	without specific, written prior permission.  The copyright holder makes no
	representations about the suitability of this software for any purpose.
	It is provided "as is" without express or implied warranty.

	THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
	EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
	PERFORMANCE OF THIS SOFTWARE.
*/

#include "kio/kio.h"
#include <time.h>


/*	get error string for system or custom error number:
*/
cstr errorstr( int err )
{
	// custom error texts:
	static const cstr ETEXT[] =
	{
	#define  EMAC(A,B)	B
	#include "error_emacs.h"
	};

	if(err==0)						return "no error";
//	if(err==-1)						return "unknown error (-1)";	strerror: "Unknown error: -1"
	if(uint(err-EBAS)<NELEM(ETEXT)) return ETEXT[err-EBAS];
	else							return strerror(err);
}


/*	get time in seconds:
*/
double now()
{
	struct timespec tv;
	clock_gettime(CLOCK_REALTIME, &tv);
    return tv.tv_sec + tv.tv_nsec * 1e-9;
}



#ifndef LOGFILE
// ****************************************************
//				Logging to stderr
// ****************************************************

void logline(cstr format, va_list va)
{
	vfprintf(stderr,catstr(format,"\n"),va);
}

void log(cstr format, va_list va)
{
	vfprintf(stderr,format,va);
}

void logNl()
{
	fprintf(stderr,"\n");
}

void logline(cstr format, ...)
{
    va_list va;
    va_start(va,format);
    logline(format,va);
    va_end(va);
}

void log(cstr format, ...)
{
    va_list va;
    va_start(va,format);
    log(format,va);
    va_end(va);
}

static uint indent;			// message indentation		TODO

// log line and add indentation for the following lines
// the indentation will be undone by the d'tor
// For use with macro LogIn(...)
//
LogIndent::LogIndent( cstr format, ... )
{
	va_list va;
	va_start(va,format);
		logline(format,va);
		indent += 2;
	va_end(va);
}

LogIndent::~LogIndent()
{
	indent -= 2;
}


// ****************************************************
//	print error in case of emergeny:
//	- in an atexit() registered function
// ****************************************************

#define ABORTED	2

void abort(cstr fmt, va_list va) // __attribute__((__noreturn__));
{
	if(lastchar(fmt)!='\n') fmt = catstr(fmt,"\n");
    vfprintf(stderr, fmt, va);
    fprintf(stderr, "aborted.\n");
	_exit(ABORTED);
}

void abort(cstr fmt, ...) // __attribute__((__noreturn__));
{
	va_list va;
	va_start(va,fmt);
    abort(fmt, va);
	//va_end(va);
}

void abort( int error ) // __attribute__((__noreturn__));
{
    abort("%s",strerror(error));
}

#endif


