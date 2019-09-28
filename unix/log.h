#pragma once
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
*/

/*	log to logfile or stderr
	kio/kio.h #includes log.h if #defined LOGFILE e.g. in settings.h
*/

#include "kio/kio.h"
#ifndef LOGFILE
#error define LOGFILE in settings.h if you include log.h/log.cpp
// else don't include log.h/log.cpp and logging will be done in kio.cpp
#endif

// required settings in settings.h:
//	 APPL_NAME: #define or const char* e.g. from argv[0]

// the following #defines can be set in settings.h:
//   #define LOGFILE_ROTATION				default: NEVER
//   #define LOGFILE_TIMESTAMP_WITH_DATE  	default: (LOGFILE_ROTATION != DAILY)
//   #define LOGFILE_TIMESTAMP_WITH_MSEC 	default: yes
//   #define LOGFILE_LOG_TO_CONSOLE			default: NDEBUG ? no : yes
//   #define LOGFILE_MAX_LOGFILES			default: 10
//   #define LOGFILE_BASE_DIRECTORY			default: "/var/log/"
//   #define LOGFILE_AUX_DIRECTORY			default: "/tmp/"


enum LogRotation { NEVER,DAILY,WEEKLY,MONTHLY };	// value for uint logrotate in openLogfile(..)
extern bool log2console;                            // write access allowed. default: log to console = true.
extern bool timestamp_with_date;					// typically set by openLogfile(…); default = no
extern bool timestamp_with_msec;					// typically set by openLogfile(…); default = no

// open logfile,
// define log rotation,
// define max. number of logfiles to keep,
// switch on|off logging to console:
extern void openLogfile(cstr dirpath, LogRotation, uint max_logfiles, bool log2console, bool with_date, bool with_msec);
extern void openLogfile(cstr dirpath, LogRotation, uint max_logfiles, bool log2console);









