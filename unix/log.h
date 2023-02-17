#pragma once
// Copyright (c) 1999 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


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


enum LogRotation { NEVER, DAILY, WEEKLY, MONTHLY }; // value for uint logrotate in openLogfile(..)
extern bool log2console;							// write access allowed. default: log to console = true.
extern bool timestamp_with_date;					// typically set by openLogfile(…); default = no
extern bool timestamp_with_msec;					// typically set by openLogfile(…); default = no

// open logfile,
// define log rotation,
// define max. number of logfiles to keep,
// switch on|off logging to console:
extern void openLogfile(cstr dirpath, LogRotation, uint max_logfiles, bool log2console, bool with_date, bool with_msec);
extern void openLogfile(cstr dirpath, LogRotation, uint max_logfiles, bool log2console);
