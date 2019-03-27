
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

	2012-01-02	kio	handle EAGAIN
	2015-02-24  kio rework


    basic use:
        logline(format, ...)

    compacting:
        repeated messages are kept for up to 1 second and then
        either printed as usual (max. 1 repetition)
        or printed as "last message repeated N times"

    multi threading:
        supported
        messages are marked with a thread id

    incremental messsage composing
        supported
        but deprecated

    indenting
        supported
        but doesn't look good with multiple threads
 */

#include "kio/kio.h"
#include <fcntl.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/param.h>
#include "../Templates/sort.h"

#ifndef LOGFILE
#error define LOGFILE in settings.h if you include log.cpp
#endif


#if defined(HAVE_SYS_DIRENT_H)
	#include <sys/dirent.h>
#elif defined(HAVE_DIRENT_H)
	#include <dirent.h>
#else
	#define	DT_UNKNOWN	 0
	#define	DT_FIFO		 1
	#define	DT_CHR		 2
	#define	DT_DIR		 4
	#define	DT_BLK		 6
	#define	DT_REG		 8
	#define	DT_LNK		10
	#define	DT_SOCK		12
	#define	DT_WHT		14
#endif


//	abort application with _exit(3)
//
void panic( cstr format, va_list )	__attribute__((__noreturn__));
void panic( cstr formatstring, ... )	__attribute__((__noreturn__));
void panic( int error_number )		__attribute__((__noreturn__));


/*	normalize path (except for symlinks)
	create missing directories
	return normalized path
*/
static cstr create_path( cstr path )
{
    if (!path||!*path)
    {
		errno = ENOTDIR;
		return "";
	}
	if(lastchar(path) != '/')			// directory path must end with '/'
	{
		path = catstr(path,"/");
	}
    if(path[0]=='~' && path[1]=='/')	// home dir
    {
		cstr h = getenv("HOME");
		if(h) path = catstr(h, path+1);
	}
    if(path[0]!='/')					// relative path
    {
        char bu[MAXPATHLEN];
        cstr wd = getcwd(bu,MAXPATHLEN);
        if(!wd) { if(errno==ENOENT) errno=ENOTDIR; return path; }
        assert(wd[0]=='/' || wd[0]==0);
        path = catstr(wd, "/", path);
    }

    errno = noerror;

	struct stat fs;
	int err = lstat(path, &fs);		// lstat(): follow last symlink
	if(!err && (fs.st_mode >> 12) == DT_DIR) return path;

	// normalize "./" and "../" and create directories:
	for(uint a=0;;)
	{
		cptr p = strchr(path+a+1,'/'); if(p==NULL) break;
		uint e = p-path;

		if(e-a==1)						// --> "//"
		{
			path = catstr(substr(path,p-1),p);
			continue;
		}
		if(e-a==2 && path[e-1]=='.')	// --> "./"
		{
			path = catstr(substr(path,p-2),p);
			continue;
		}
		if(e-a==3 && path[e-1]=='.' && path[e-2]=='.')	// --> "../"
		{
			if(a>0) while(path[--a] != '/') {}
			path = catstr(substr(path,path+a), path+e);
			continue;
		}

		a = e;

		int err = lstat(substr(path,p),&fs);
		if(err || (fs.st_mode >> 12) != DT_DIR)
		{
			mode_t z = umask(0);
			err = mkdir(path,0777);
			umask(z);
			if(err) return substr(path,p+1);	// errno set
		}
	}

	errno = ok;
	return path;
}


/*  ___________________________________________________________________
	application settings:
	#define LOGFILE_ROTATION etc. in "settings.h"
	or call openLogfile(..)
*/

#ifndef LOGFILE_ROTATION
#define LOGFILE_ROTATION NEVER
#endif

#ifndef LOGFILE_TIMESTAMP_WITH_DATE
#define LOGFILE_TIMESTAMP_WITH_DATE  (LOGFILE_ROTATION != DAILY)
#endif

#ifndef LOGFILE_TIMESTAMP_WITH_MSEC
#define LOGFILE_TIMESTAMP_WITH_MSEC yes
#endif

#ifndef LOGFILE_LOG_TO_CONSOLE
 #ifdef NDEBUG
 #define LOGFILE_LOG_TO_CONSOLE	no
 #else
 #define LOGFILE_LOG_TO_CONSOLE	yes
 #endif
#endif

#ifndef LOGFILE_MAX_LOGFILES
#define LOGFILE_MAX_LOGFILES	10
#endif

#ifndef LOGFILE_BASE_DIRECTORY
#define LOGFILE_BASE_DIRECTORY	"/var/log/"
#endif

#ifndef LOGFILE_AUX_DIRECTORY
#define LOGFILE_AUX_DIRECTORY	"/tmp/"
#endif



/*  ___________________________________________________________________
    const data:
*/

static const time_t seconds_per_day = 60*60*24;

static const cstr fmt00 = "%02u:%02u:%02u";
static const cstr fmt01 = "%02u:%02u:%02u.%03u";
static const cstr fmt10 = "%04u-%02u-%02u %02u:%02u:%02u";
static const cstr fmt11 = "%04u-%02u-%02u %02u:%02u:%02u.%03u";

bool timestamp_with_date = no;
bool timestamp_with_msec = no;

#define MAXINDENT 40u
static const char _spaces[/*MAXINDENT+1*/] = "                                        ";
static_assert(sizeof(_spaces)==MAXINDENT+1,"bitte Leerzeichen nachzählen...");
#define indentstr(N) (_spaces+MAXINDENT-min(N,MAXINDENT))

#define ABORTED	2
#define	PANICED	3
#define PANIC_REPEATED	4

#undef  ABORT
#define	ABORT	panic



/*  ___________________________________________________________________
	static data
*/

// thread-specific data:
static pthread_key_t logfile_key;			// key for per-thread LogFile

// File descriptor for log file
static int  fd = -1;						// default: no log file
       bool log2console = yes;				// default: log to console

// Logrotate setting:
static LogRotation 	logrotate = NEVER;
static double 		logrotate_when = 0.0; 	// 0 = not initialized, 1e99 = never
static uint 		max_logfiles = 99;
static cstr 		logdir = NULL;

// Existing LogFile instances:
static class LogFile** logfiles = NULL;		// Pointers to all existing LogFile instances
static uint			logfiles_cnt = 0;
static uint			logfiles_max = 0;

// Synchronizer:
static pthread_mutex_t mutex;				// access lock



/*  ___________________________________________________________________
	forward references:
*/
static void write2log(int thread_id, double when, uint indent, cstr msg);
static void init();



//	===================================================================
//				class LogFile
//				per-thread log state
//	===================================================================


class LogFile
{
public:
    uint		thread_id;				// my serial number for this thread (for display in logfile)
    uint		indent;					// message indentation
    double		when;					// timestamp of last message printed
    uint		composition;			// index in msg[]
    uint		repetitions;			// counter for unlogged repeating messages
    char		msg[256];				// for msg composition & repetition

	void		write2log(cstr msg)		{ ::write2log(thread_id,when,indent,msg); }
	void		print_repetitions();
	void		print_pending();

				LogFile();
				~LogFile();

    void		valog(double now, cstr fmt, va_list);	// log line with timestamp etc.
    void		vaadd(cstr fmt, va_list);				// compose log line
    void		nl(double now);							// finalize and print composed log line
};



/*  ___________________________________________________________________
	helpers:
*/

#ifndef NDEBUG
static void lock()   { int e = pthread_mutex_lock(&mutex);	if(e) panic("LogFile:lock failed");   }
static void unlock() { int e = pthread_mutex_unlock(&mutex);if(e) panic("LogFile:unlock failed"); }
#else
inline void lock()   { pthread_mutex_lock(&mutex);	 }
inline void unlock() { pthread_mutex_unlock(&mutex); }
#endif

// get (the best guess of) the current thread id
// without creating a LogFile instance
// for abort() and panic()
//
static int quick_id()
{
	if(logrotate_when==0.0) return 0;
	LogFile* logfile = (LogFile*) pthread_getspecific( logfile_key );
	return logfile ? logfile->thread_id : logfiles_cnt;
}

// flush unwrittencontents to logfile
// mutex must be locked
//
static void flush_logfiles()
{
	for(uint i=0; i<logfiles_cnt; i++)
	{
		if(logfiles[i]) logfiles[i]->print_pending();
	}
}

// get the LogFile instance for the current thread
// test for initial initialization
// test for log rotation
// creates LogFile for current thread if required
//
// if called with getLogfile(0.0) then the test for log rotation is omitted
//
static LogFile* getLogfile(double now)
{
	if(now >= logrotate_when)	// => init or rotate log
	{
		if(logrotate_when==0.0) init();									// first call
		else if(logrotate==NEVER) logrotate_when = 1e99;				// should never happen
		else openLogfile(logdir, logrotate, max_logfiles, log2console,	// rotate log
						 timestamp_with_date, timestamp_with_msec);
	}

	LogFile* logfile = (LogFile*) pthread_getspecific( logfile_key );
	return logfile ? logfile : new LogFile();
}



/*  ___________________________________________________________________
	Print formatted log message to stderr and/or logfile.
    The message should not be longer than 260 characters.
    Format: "[<thread_id>] <timestamp> <indentation> <message>\n".
    lock should be locked.
*/
static void write2log(int thread_id, double when, uint indent, cstr msg)
{
    char sbu[260+MAXINDENT+40];

    for(cptr nl; (nl = strchr(msg,'\n')); msg = nl+1)
    {
        strcpy(sbu, msg, min(uint(NELEM(sbu)), uint(nl-msg+1)));
        write2log(thread_id,when,indent,sbu);
    }

	char timestamp[40];

    if(timestamp_with_date)
    {
	    time_t sec = (time_t)when;
        tm dt; gmtime_r(&sec,&dt);

        uint y = dt.tm_year+1900;
        uint m = dt.tm_mon+1;
        uint d = dt.tm_mday;

		if(timestamp_with_msec)
			sprintf(timestamp, fmt11, y, m, d, dt.tm_hour, dt.tm_min, dt.tm_sec, uint((when-sec)*1000));
		else
			sprintf(timestamp, fmt10, y, m, d, dt.tm_hour, dt.tm_min, dt.tm_sec);
    }
    else
    {
	    time_t sec  = time_t(when);
        uint   minu = uint(sec/60);
        uint   hour = uint(minu/60);

		if(timestamp_with_msec)
	        sprintf(timestamp, fmt01, hour%24, minu%60, uint(sec%60), uint((when-sec)*1000));
		else
	        sprintf(timestamp, fmt00, hour%24, minu%60, uint(sec%60));
    }

	uint sz = snprintf(sbu, NELEM(sbu), "[%u] %s%s  %s\n", thread_id, timestamp, indentstr(indent), msg);
	if(sz >= NELEM(sbu)) { sz = NELEM(sbu) -1; sbu[sz-1]='\n'; }

    if(fd!=-1)  // write to file
    {
		for(uint i=0; i<sz;)
        {
            int n = int(write(fd,sbu+i,sz-i));
            if(n>=0) { i+=n; continue; }
            if(errno==EINTR) continue;
            close(fd); fd=-1;
			panic("writing to logfile failed",errno);
        }
    }

    if(log2console) // print to stderr
    {
		for(uint i=0; i<sz;)
        {
            int n = int(write(2,sbu+i,sz-i));
            if(n>=0) { i+=n; continue; }
            if(errno==EINTR) continue;
            //log2console = no;
			panic("logging to stderr failed",errno);
        }
    }
}



/*  ___________________________________________________________________
	print error in case of emergeny:
	- logfile may be not yet initialized
	- lock may be locked
	- normal logfile may be broken and stderr may be /dev/null
	  => print to file "/tmp/Panic.log"
	- in an atexit() registered function
*/

void panic(cstr fmt, va_list va) // __attribute__((__noreturn__));
{
	static bool repeat = no;
	if(repeat) _exit(PANIC_REPEATED);
	repeat = yes;

	pthread_mutex_trylock(&mutex);

    char zbu[280];	vsnprintf(zbu, NELEM(zbu), fmt, va);
    char msg[300];	snprintf (msg, NELEM(msg), "%s: Panic: %s", APPL_NAME, zbu);

	log2console = yes;
	if(fd==-1)
	{
		if(fd>2) close(fd);
		mode_t z = umask(0);
		// note: wg. macos SIERRA: umask(0): else write permission is removed from passed permission flags
		// note: wg. macos SIERRA: 0666: must be writable for all, else user1 can't write to logfile created by user2
		fd = open("/tmp/Panic.log", O_WRONLY|O_CREAT|O_APPEND,0666);
		umask(z);
	}

	write2log(quick_id(),now(),0,msg);
	_exit(PANICED);
}

void panic(cstr fmt, ...) // __attribute__((__noreturn__));
{
	va_list va;
	va_start(va,fmt);
    panic(fmt, va);
	//va_end(va);
}

void panic( int error ) // __attribute__((__noreturn__));
{
    panic("%s",strerror(error));
}

static void panic(cstr where, uint err) // __attribute__((__noreturn__));
{
    panic("%s: %s",where,strerror(err));
}



/*  ___________________________________________________________________
	GLOBAL: abort with message
	note: must not be called within locked mutex!
		  macros like assert, IERR, TODO etc. call abort()!
		  atexit_actions() locks the mutex
	note: must not be called from functions registered with atexit()
		  atexit_actions() may call logline(), so this basically appies to almost everything in this file
*/

void abort( cstr format, va_list va )	// __attribute__((__noreturn__))
{
	char bu[300]; snprintf(bu,NELEM(bu),"%s\naborted.",format);
	getLogfile(0.0)->valog(now(),format,va);
	//abort();
	exit(ABORTED);				// ~ abort() but no crash report
}

void abort( cstr format, ... )	// __attribute__((__noreturn__))
{
	va_list va;
	va_start(va,format);
	abort(format,va);
	//va_end(va);
}

void abort( int error )			// __attribute__((__noreturn__));
{
    abort("%s",strerror(error));
}



/*  ___________________________________________________________________
    Init & Shutdown:
*/

// Deallocate LogFile instance of current thread
// called at thread termination if pointer in logfile_key is != NULL
// this function is registered with pthread_key_create()
//
static void delete_logfile(void* logfile)
{
    delete (LogFile*) logfile;
}


// Flush logfile and stop timer thread
// called at application termination
// this function is registered with atexit()
//
static void atexit_actions()
{
	lock();

		flush_logfiles();
		if(fd!=-1) write2log(quick_id(),now(),0,"\nExit: Logfile closed\n");

    unlock();
}


// initialize
// must only be called once
// creates a pthread key for per-thread LogFiles
// registers atexit_actions
//
static void init_once(void)
{
	int e = pthread_mutex_init(&mutex,NULL);
	if(e) panic("init_mutex", e);

	// create key:
    e = pthread_key_create( &logfile_key, delete_logfile );
    if(e) panic("create_key", e);

	// set flag to non-zero: initialized
    logrotate_when = 1e99;

	// prepare flushing on exit:
	atexit(atexit_actions);

// open logfile if set in settings.h:
	if(LOGFILE_ROTATION != NEVER)
	{
		openLogfile(
			NULL/*dirpath*/, LOGFILE_ROTATION, LOGFILE_MAX_LOGFILES, LOGFILE_LOG_TO_CONSOLE,
			LOGFILE_TIMESTAMP_WITH_DATE, LOGFILE_TIMESTAMP_WITH_MSEC );
	}
}


// initialize
// calls init_once() only once
// global functions should call "if(logrotate_when==0) init();" before doing anything else
//
static void init()
{
	IFDEBUG( char bu[2]; snprintf(bu,2,"ss"); if(bu[1]) panic("snprintf"); )

	// initialize:
	static pthread_once_t once_control = PTHREAD_ONCE_INIT;
    int e = pthread_once( &once_control, init_once );
    if(e) panic("init once", e);
}



/*  ___________________________________________________________________
    c'tor, d'tor
*/

LogFile::LogFile()
:
    indent(0),
    when(0.0),
    composition(0),
    repetitions(0)
{
    msg[0] = 0;

	lock();

		// find a free slot / thread_id:
		uint i=0;
		while(i<logfiles_cnt && logfiles[i]!=NULL) i++;

		// no free slot => append at end of list
		if(i==logfiles_cnt)
		{
			// list full => grow list
			if(logfiles_cnt==logfiles_max)
			{
				logfiles_max += 20;
				LogFile** z = new LogFile*[logfiles_max];
				memcpy(z, logfiles, logfiles_cnt*sizeof(LogFile*));
				delete[] logfiles; logfiles = z;
			}
			logfiles_cnt++;
		}

		thread_id = i;
		logfiles[i] = this;

	unlock();

    int e = pthread_setspecific(logfile_key, this);
    if(e) panic("new",e);
}


LogFile::~LogFile()
{
	lock();

		// flush pending output:
		print_pending();

		// because the thread_id will be recycled we must note the thread change.
		// this will only be logged if there actually was s.th. logged on this thread
		// because otherwise there is no LogFile instance which can be deleted.
		when = now();
		write2log("---thread terminated---");

		// remove this from logfiles[]:
		logfiles[thread_id] = NULL;

	unlock();
}



/*  ___________________________________________________________________
    member functions
*/


// print pending repetitions
// to be called at log rotation.
// mutex must be locked
//
void LogFile::print_repetitions()
{
	assert(repetitions);
	assert(!composition);

    if(repetitions>1) sprintf(msg, "last msg repeated %u times", repetitions);
    write2log(msg);
    repetitions = 0;
    msg[0] = 0;		// avoid that first msg in the new logfile is a repetition msg
}


// print pending repetitions and unfinished compositions
// to be called at exit.
// mutex must be locked
//
void LogFile::print_pending()
{
	if(repetitions) print_repetitions(); else
	if(composition)
	{
		when = now();
		write2log(msg);
		composition = 0;
	}
}


// start or append to composed log message
// a final valog() or nl() is required to print it
// the timestamp is not set: it will be set by the function which actually prints it
// called by log()
//
void LogFile::vaadd(cstr format, va_list va)
{
    lock();

		if(repetitions) print_repetitions();
		composition += vsnprintf(msg+composition, NELEM(msg)-composition, format, va);
		if(composition>=NELEM(msg)) composition = NELEM(msg);

    unlock();
}


// log line with timestamp etc.
// print string to logfile
// may print buffered repetitions first
// may be buffered as a repetition
// may finalize & print a composed message
//
void LogFile::valog(double now, cstr format, va_list va)
{
	lock();

		if(composition)		// finalize and print a composed message
		{
			vsnprintf(msg+composition, NELEM(msg)-composition, format, va);
			composition = 0;
		}
		else
		{
			char z[NELEM(msg)];
			vsnprintf(z, NELEM(z), format, va);
			if(strcmp(msg,z)==0)	// message repeats:
			{
				when = now;			// timestamp of last repetition
				repetitions++;
				goto x;
			}
			if(repetitions) print_repetitions();
			strcpy(msg,z);
		}

		when = now;
		write2log(msg);

x:	unlock();
}


// print empty line or
// finalize and print composed log message if any, else print an empty line
// may print buffered repetitions first
//
void LogFile::nl(double now)
{
    lock();

		if(composition)		// finalize and print composed message
		{
			composition = 0;
		}
		else
		{
			if(repetitions) print_repetitions();
			msg[0] = 0;
		}

		when = now;
		write2log(msg);

    unlock();
}



/*  ___________________________________________________________________
    global functions:
*/

// helper:
// mutex must not be locked
static void purge_old_logfiles(cstr fname)
{
    xlogIn("LogFile:purge_old_logfiles");

    assert(startswith(fname,APPL_NAME));
    assert(logdir!=NULL);
	assert(*logdir=='/');                // must be full path. '/' must exist.
    assert(*(strchr(logdir,0)-1)=='/');  // must end with '/'

	DIR* dir = opendir(logdir);             // note: sets sporadic errors
	if(!dir) { logline("LogFile:purge_old_logfiles: %s",strerror(errno)); return; }

	// collect old log files:
	uint v_cnt = 0;
	uint v_max = 100;
	cstr* v = new cstr[v_max];

	uint logdirlen = uint(strlen(logdir));	// dirpath + '/'
    uint fnamelen  = uint(strlen(fname));
	char filepath[logdirlen+fnamelen+1];
    strcpy(filepath,logdir);

	for(;;)
	{
		dirent* direntry = readdir(dir);
		if(direntry==NULL) break;			// end of dir

		cstr filename = direntry->d_name;
        #ifdef _BSD
            uint filenamelen = direntry->d_namlen;
        #else
            uint filenamelen = strlen(direntry->d_name);
        #endif
		assert(filename[filenamelen]==0);

		if(filenamelen!=fnamelen) continue;				// filename does not match
		if(!startswith(filename, APPL_NAME)) continue;	// filename does not match

		strcpy(filepath+logdirlen,filename);
		struct stat filestat;
		if(lstat(filepath,&filestat)) continue;			// skip on error
		if(!S_ISREG(filestat.st_mode)) continue;		// not a regular file

		if(v_cnt==v_max)
		{
			v_max *= 2;
			cstr* z = new cstr[v_max];
			memcpy(z,v,v_cnt*sizeof(cstr));
			delete[] v;
			v = z;
		}
		v[v_cnt++] = newcopy(filename);
	}

	// delete old logfiles until max_logfiles remain:
	if(v_cnt>max_logfiles)
	{
		sort(v,v+v_cnt);

		for(uint i=0; i < v_cnt - max_logfiles; i++)
		{
			assert(startswith(v[i],APPL_NAME));
			strcpy(filepath+logdirlen,v[i]);
			remove(filepath);
		}
	}

	for(uint i=0;i<v_cnt;i++) delete[] v[i];
	delete[] v;
	closedir(dir);
}


/*  Open a specific file for log output
	This should only be called once from the outside
	it is called for log rotation from the timer thread too
	dirpath must start and end with '/', e.g. "/var/log/";
*/
void openLogfile(cstr dirpath, LogRotation logrotate, uint max_logfiles, bool log2console)
{
    openLogfile(dirpath, logrotate, max_logfiles, log2console, logrotate!=DAILY, no);
}

void openLogfile(cstr dirpath, LogRotation logrotate, uint max_logfiles, bool log2console, bool with_date, bool with_msec)
{
	if(logrotate_when==0.0) init();

	dirpath = create_path(dirpath);
	if(errno) dirpath = create_path(LOGFILE_BASE_DIRECTORY APPL_NAME "/");
	if(errno) dirpath = create_path(LOGFILE_AUX_DIRECTORY APPL_NAME "/");
	if(errno) panic("could not create log dir",strerror(errno));

    char filepath[1024];
	lock();

        IFDEBUG( write2log(quick_id(),now(),0,"+++openLogfile+++"); )

		// flush repetitions:
		for(uint i=0; i<logfiles_cnt; i++)
		{
			LogFile* l = logfiles[i];
			if(l && l->repetitions) l->print_repetitions();
		}

		::max_logfiles = max_logfiles;
		::logrotate    = logrotate;
		::log2console  = log2console;
		timestamp_with_msec = with_msec;
		timestamp_with_date = with_date;
        if(logdir!=dirpath) { delete[] logdir; logdir = new char[strlen(dirpath)+1]; strcpy((ptr)logdir,dirpath); }

		// calculate current logfile filename
		// and when to rotate log:

		time_t when = (time_t)::now() / seconds_per_day * seconds_per_day;	// start of day
		tm dt; gmtime_r(&when,&dt);
			//	int	tm_sec;		/* seconds after the minute [0-60] */
			//	int	tm_min;		/* minutes after the hour [0-59] */
			//	int	tm_hour;	/* hours since midnight [0-23] */
			//	int	tm_mday;	/* day of the month [1-31] */
			//	int	tm_mon;		/* months since January [0-11] */
			//	int	tm_year;	/* years since 1900 */
			//	int	tm_wday;	/* days since Sunday [0-6] */
			//	int	tm_yday;	/* days since January 1 [0-365] */
			//	int	tm_isdst;	/* Daylight Savings Time flag */
			//	long tm_gmtoff;	/* offset from CUT in seconds */
			//	char *tm_zone;	/* timezone abbreviation */

		switch(logrotate)
		{
		default:
            panic("ill. log rotation setting");

		case NEVER:
			logrotate_when = 1e99;
			snprintf(filepath, NELEM(filepath), "%s%s.log", dirpath, APPL_NAME);
			break;

		case DAILY:
			snprintf( filepath, NELEM(filepath), "%s%s-%04u-%02u-%02u.log",
                                dirpath, APPL_NAME, dt.tm_year+1900, dt.tm_mon+1, dt.tm_mday);
			logrotate_when = when + seconds_per_day;
			break;

		case MONTHLY:
			when -= (dt.tm_mday-1) * seconds_per_day;		// start of month
			snprintf( filepath, NELEM(filepath), "%s%s-%04u-%02u.log",
                                dirpath, APPL_NAME, dt.tm_year+1900, dt.tm_mon+1 );
			dt.tm_mon += 1;
			dt.tm_mday = 1;
			logrotate_when = timegm(&dt);
			break;

		case WEEKLY:
		  {
			int dow = (dt.tm_wday+6) % 7;					// day of week since monday [0-6]
			when -= dow * seconds_per_day;					// start of week
			dt.tm_yday -= dow;

			if(dt.tm_yday<-3)								// week is accounted for last year
				{ gmtime_r(&when,&dt); dow=0; }				// recalculate tm_yday: dep. on. leap year
			else if(dt.tm_mon==11 && dt.tm_mday-dow>=32-3)	// week is accounted for next year
				{ dt.tm_year+=1; dt.tm_yday=0; }			//

			snprintf( filepath, NELEM(filepath), "%s%s-%04u-week-%02u.log",
                                dirpath, APPL_NAME, dt.tm_year+1900, 1+(dt.tm_yday+3)/7 );
			logrotate_when = when + 7 * seconds_per_day;
			break;
		  }
		}

		if(fd>2) close(fd);

		// note: wg. macos SIERRA: umask(0): else write permission is removed from passed permission flags
		// note: wg. macos SIERRA: 0666: must be writable for all, else user1 can't write to logfile created by user2
		mode_t z = umask(0);
		fd = open(filepath, O_WRONLY|O_CREAT|O_APPEND,0666);
		umask(z);

		if(fd==-1) panic("open logfile \"%s\" failed: %s", filepath, strerror(errno));

	unlock();

	logline("------------------------------------\nLogfile opened\n");

	if(logrotate!=NEVER && max_logfiles) purge_old_logfiles(filepath+strlen(dirpath));
}


// log line with timestamp etc.
//
void logline( cstr format, ... )
{
	va_list va;
	va_start(va,format);
	double now = ::now();
    getLogfile(now)->valog(now, format, va);
	va_end(va);
}


// incrementally compose log line
//
void log( cstr format, ... )
{
	va_list va;
	va_start(va,format);
    getLogfile(0.0)->vaadd(format, va);
	va_end(va);
}

void log(cstr format, va_list va)
{
    getLogfile(0.0)->vaadd(format, va);
}


// print composed log line if any, else an empty line
//
void logNl()
{
	double now = ::now();
    getLogfile(now)->nl(now);
}


// log line with timestamp etc. and add indentation for the following lines
// the indentation will be undone by the d'tor
// For use with macro LogIn(...)
//
LogIndent::LogIndent( cstr format, ... )
{
	va_list va;
	va_start(va,format);
		double now = ::now();
        LogFile* l = getLogfile(now);
		l->valog(now, format, va);
		if(l->repetitions || l->composition) { lock(); l->print_pending(); unlock(); }
		l->indent += 2;
	va_end(va);
}


// undo indentation
//
LogIndent::~LogIndent()
{
	LogFile* l = reinterpret_cast<LogFile*>(pthread_getspecific(logfile_key));
	if(l->repetitions || l->composition) { lock(); l->print_pending(); unlock(); }
	l->indent -= 2;
}











