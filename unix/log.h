#pragma once
/*	Copyright  (c)	Günter Woigk 1999 - 2019
					mailto:kio@little-bat.de

	This file is free software

 	This program is distributed in the hope that it will be useful,
 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	• Redistributions of source code must retain the above copyright notice,
	  this list of conditions and the following disclaimer.
	• Redistributions in binary form must reproduce the above copyright notice,
	  this list of conditions and the following disclaimer in the documentation
	  and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
	OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
	WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
	OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
	ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	log to logfile or stderr
	2015-02 kio branched from kio/log.cpp


	this file is included from kio/kio.h

*/


//	–––––––––––––––––––––––––––––––––––––––––––––––––––
//	Logfile:
//	–––––––––––––––––––––––––––––––––––––––––––––––––––

#include "kio/kio.h"
#ifndef LOGFILE
#error define LOGFILE in settings.h if you include log.h/log.cpp
// else don't include log.h/log.cpp and logging will be done in kio.cpp
#endif


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









