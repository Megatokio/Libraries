// Copyright (c) 2017 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"
#include <cmath>


// ****************************************************
//				Logging to stderr
// ****************************************************

void logline(cstr format, va_list va) { vfprintf(stderr, catstr(format, "\n"), va); }

void log(cstr format, va_list va) { vfprintf(stderr, format, va); }

void logNl() { fprintf(stderr, "\n"); }

void logline(cstr format, ...)
{
	va_list va;
	va_start(va, format);
	logline(format, va);
	va_end(va);
}

void log(cstr format, ...)
{
	va_list va;
	va_start(va, format);
	log(format, va);
	va_end(va);
}

void LogIndent::indent(int) { } // TODO


// ****************************************************
//	print error in case of emergency:
//	- in an atexit() registered function
// ****************************************************

static constexpr int ABORTED = 2;

__noreturn void abort(cstr fmt, va_list va)
{
	if (lastchar(fmt) != '\n') fmt = catstr(fmt, "\n");
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "aborted.\n");
	exit(ABORTED);
}

__noreturn void abort(cstr fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	abort(fmt, va);
	//va_end(va);
}

namespace kio
{
__noreturn void panic(cstr fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	abort(fmt, va);
	//va_end(va);
}
} // namespace kio


