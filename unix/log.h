// Copyright (c) 1999 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/*
	log to logfile or stderr
	the source files are either "log_to_file.cpp" or "log_to_console.cpp".
*/

#pragma once
#include "kio/kio.h"


// log_to_file.cpp only!
enum LogRotation { NEVER, DAILY, WEEKLY, MONTHLY };
extern bool log2console; // may be switched on/off any time

// open logfile
// log_to_file.cpp only!
extern void openLogfile(
	cstr appl_name, cstr dirpath, LogRotation, uint max_logfiles = 10, //
	bool log2console = debug, bool with_date = 1, bool with_msec = 0, bool utc_timestamps = 0);


/*
	Logging depending on 'loglevel':
	`loglevel` must be #defined before any #include.
	note: if you want to switch loglevel at runtime
		  #undef 'loglevel' after all #includes and define a variable 'loglevel' instead.
*/

#if defined RELEASE
  #undef loglevel
#endif
#if !defined loglevel
  #define loglevel 0
#endif

extern void logline(cstr, ...) __printflike(1, 2);
extern void logline(cstr, va_list) __printflike(1, 0);
extern void log(cstr, ...) __printflike(1, 2);
extern void log(cstr, va_list) __printflike(1, 0);
extern void logNl();

#define xlog(...)	   (loglevel < 1 ? void(0) : log(__VA_ARGS__))
#define xlogline(...)  (loglevel < 1 ? void(0) : logline(__VA_ARGS__))
#define xdebugstr(...) (loglevel < 1 ? void(0) : debugstr(__VA_ARGS__))
#define xlogNl()	   (loglevel < 1 ? void(0) : logNl())

#define xxlog(...)		(loglevel < 2 ? void(0) : log(__VA_ARGS__))
#define xxlogline(...)	(loglevel < 2 ? void(0) : logline(__VA_ARGS__))
#define xxdebugstr(...) (loglevel < 2 ? void(0) : debugstr(__VA_ARGS__))
#define xxlogNl()		(loglevel < 2 ? void(0) : logNl())


// indent logging for the lifetime of a function:
struct LogIndent
{
	bool		f;
	static void indent(int);
	LogIndent(bool f, cstr fmt, ...) __printflike(3, 4) : f(f)
	{
		if (f)
		{
			va_list va;
			va_start(va, fmt);
			logline(fmt, va);
			va_end(va);
			indent(+2);
		}
	}
	~LogIndent()
	{
		if (f) indent(-2);
	}
};

#define logIn(...)	 LogIndent _log_indent(true, __VA_ARGS__)
#define xlogIn(...)	 LogIndent _log_indent(loglevel >= 1, __VA_ARGS__)
#define xxlogIn(...) LogIndent _log_indent(loglevel >= 2, __VA_ARGS__)


/*





















*/
