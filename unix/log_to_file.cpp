
// Copyright (c) 1999 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/*	basic use:
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

#include "Templates/sort.h"
#include "kio/kio.h"
#include <dirent.h>
#include <fcntl.h>
#include <memory>
#include <mutex>
#include <pthread.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>


#if defined(HAVE_SYS_DIRENT_H)
  #include <sys/dirent.h>
#elif defined(HAVE_DIRENT_H)
  #include <dirent.h>
#else
  #define DT_UNKNOWN 0
  #define DT_FIFO	 1
  #define DT_CHR	 2
  #define DT_DIR	 4
  #define DT_BLK	 6
  #define DT_REG	 8
  #define DT_LNK	 10
  #define DT_SOCK	 12
  #define DT_WHT	 14
#endif


/*  ___________________________________________________________________
	const data:
*/

static constexpr time_t seconds_per_day = 60 * 60 * 24;

static constexpr char fmt00[] = "%02i:%02i:%02i";
static constexpr char fmt01[] = "%02i:%02i:%02i.%03i";
static constexpr char fmt10[] = "%04i-%02i-%02i %02i:%02i:%02i";
static constexpr char fmt11[] = "%04i-%02i-%02i %02i:%02i:%02i.%03i";

static constexpr int  MAXINDENT = 40u;
static constexpr char _spaces[] = "                                        ";
static_assert(sizeof(_spaces) == MAXINDENT + 1, "bitte Leerzeichen nachzählen...");
#define indentstr(N) (_spaces + MAXINDENT - min(N, MAXINDENT))

static constexpr int ABORTED		= 2;
static constexpr int PANICED		= 3;
static constexpr int PANIC_REPEATED = 4;


/*  ___________________________________________________________________
	static data
*/

static cstr	 appl_name			 = "noname";
static bool	 timestamp_with_date = no;
static bool	 timestamp_with_msec = no;
static auto* time_r				 = &localtime_r;

// thread-specific data:
static thread_local std::unique_ptr<class LogFile> my_logfile = nullptr;

// File descriptor for log file
static int		  fd		  = -1;	 // default: no log file
bool			  log2console = yes; // default: log to console
static std::mutex mutex;			 // mutually exclusive access to logfile
using lock_guard = std::lock_guard<std::mutex>;

// Logrotate setting:
static LogRotation logrotate	  = NEVER;
static double	   logrotate_when = 1e99; // 1e99 = never
static uint		   max_logfiles	  = 99;
static cstr		   logdir		  = nullptr;

// Existing LogFile instances:
static class LogFile** logfiles		= nullptr; // Pointers to all existing LogFile instances
static uint			   logfiles_cnt = 0;
static uint			   logfiles_max = 0;


/*  ___________________________________________________________________
	forward references:
*/

static void write2log(uint thread_id, double when, int indent, cstr msg);
static void open_logfile();
static uint quick_id();


// =====================================================================
// =====================================================================
// =====================================================================


//	abort application with _exit(3)

__noreturn __printflike(1, 0) void panic(cstr fmt, va_list va)
{
	// print error in case of emergency:
	// - logfile may be not yet initialized
	// - lock may be locked
	// - normal logfile may be broken and stderr may be /dev/null
	//   => print to file "/tmp/Panic.log"
	// - in an atexit() registered function

	static bool repeat = no;
	if (repeat) _exit(PANIC_REPEATED);
	repeat = yes;

	(void)mutex.try_lock();

	char zbu[280];
	vsnprintf(zbu, NELEM(zbu), fmt, va);
	char msg[308];
	snprintf(msg, NELEM(msg), "%s: Panic: %s", appl_name, zbu);

	log2console = yes;
	if (fd == -1)
	{
		if (fd > 2) close(fd);
		mode_t z = umask(0);
		// note: wg. macos SIERRA: umask(0): else write permission is removed from passed permission flags
		// note: wg. macos SIERRA: 0666: must be writable for all, else user1 can't write to logfile created by user2
		fd = open("/tmp/Panic.log", O_WRONLY | O_CREAT | O_APPEND, 0666);
		umask(z);
	}

	write2log(quick_id(), now(), 0, msg);
	_exit(PANICED);
}

__printflike(1, 2) __noreturn void panic(cstr fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	panic(fmt, va);
	//va_end(va);
}

__noreturn void panic(int err) { panic("%s", strerror(err)); }

__noreturn static void panic(cstr where, int err) { panic("%s: %s", where, strerror(err)); }


static cstr create_path(cstr path) noexcept
{
	//	normalize path (except for symlinks)
	//	create missing directories
	//	return normalized path

	if (!path || !*path)
	{
		errno = ENOTDIR;
		return "";
	}
	if (lastchar(path) != '/') // directory path must end with '/'
	{
		path = catstr(path, "/");
	}
	if (path[0] == '~' && path[1] == '/') // home dir
	{
		cstr h = getenv("HOME");
		if (h) path = catstr(h, path + 1);
	}
	if (path[0] != '/') // relative path
	{
		char bu[MAXPATHLEN];
		cstr wd = getcwd(bu, MAXPATHLEN);
		if (!wd)
		{
			if (errno == ENOENT) errno = ENOTDIR;
			return path;
		}
		assert(wd[0] == '/' || wd[0] == 0);
		path = catstr(wd, "/", path);
	}

	errno = noerror;

	struct stat fs;
	int			err = lstat(path, &fs); // lstat(): follow last symlink
	if (!err && (fs.st_mode >> 12) == DT_DIR) return path;

	// normalize "./" and "../" and create directories:
	for (uint a = 0;;)
	{
		cptr p = strchr(path + a + 1, '/');
		if (p == nullptr) break;
		uint e = uint(p - path);

		if (e - a == 1) // --> "//"
		{
			path = catstr(substr(path, p - 1), p);
			continue;
		}
		if (e - a == 2 && path[e - 1] == '.') // --> "./"
		{
			path = catstr(substr(path, p - 2), p);
			continue;
		}
		if (e - a == 3 && path[e - 1] == '.' && path[e - 2] == '.') // --> "../"
		{
			if (a > 0)
				while (path[--a] != '/') {}
			path = catstr(substr(path, path + a), path + e);
			continue;
		}

		a = e;

		int err = lstat(substr(path, p), &fs);
		if (err || (fs.st_mode >> 12) != DT_DIR)
		{
			mode_t z = umask(0);
			err		 = mkdir(path, 0777);
			umask(z);
			if (err) return substr(path, p + 1); // errno set
		}
	}

	errno = ok;
	return path;
}

//	===================================================================
//				class LogFile
//				per-thread log state
//	===================================================================


class LogFile
{
public:
	uint   thread_id;	// my serial number for this thread (for display in logfile)
	int	   indent;		// message indentation
	double when;		// timestamp of last message printed
	uint   composition; // index in msg[]
	uint   repetitions; // counter for unlogged repeating messages
	char   msg[400];	// for msg composition & repetition

	void write2log(cstr msg) { ::write2log(thread_id, when, indent, msg); }
	void print_repetitions();
	void print_pending();

	LogFile() noexcept;
	~LogFile();

	__printflike(3, 0) void valog(double now, cstr fmt, va_list); // log line with timestamp etc.
	__printflike(2, 0) void vaadd(cstr fmt, va_list);			  // compose log line
	void nl(double now);										  // finalize and print composed log line
};

LogFile::LogFile() noexcept : indent(0), when(0.0), composition(0), repetitions(0), msg {0}
{
	lock_guard {mutex};

	// find a free slot / thread_id:
	uint i = 0;
	while (i < logfiles_cnt && logfiles[i] != nullptr) i++;

	// no free slot => append at end of list
	if (i == logfiles_cnt)
	{
		// list full => grow list
		if (logfiles_cnt == logfiles_max)
		{
			logfiles_max += 20;
			LogFile** z = new LogFile*[logfiles_max];
			memcpy(z, logfiles, logfiles_cnt * sizeof(LogFile*));
			delete[] logfiles;
			logfiles = z;
		}
		logfiles_cnt++;
	}

	thread_id	= i;
	logfiles[i] = this;

	my_logfile.reset(this); // set the thread_local variable
}

LogFile::~LogFile()
{
	lock_guard {mutex};

	// flush pending output:
	print_pending();

	// because the thread_id will be recycled we must note the thread change.
	// this will only be logged if there actually was s.th. logged on this thread
	// because otherwise there is no LogFile instance which can be deleted.
	when = now();
	write2log("---thread terminated---");

	// remove this from logfiles[]:
	logfiles[thread_id] = nullptr;
}

void LogFile::print_repetitions()
{
	// print pending repetitions
	// to be called at log rotation.
	// mutex must be locked

	assert(repetitions);
	assert(!composition);

	if (repetitions > 1) sprintf(msg, "last msg repeated %u times", repetitions);
	write2log(msg);
	repetitions = 0;
	msg[0]		= 0; // avoid that first msg in the new logfile is a repetition msg
}

void LogFile::print_pending()
{
	// print pending repetitions and unfinished compositions
	// to be called at exit.
	// mutex must be locked

	if (repetitions) print_repetitions();
	else if (composition)
	{
		when = now();
		write2log(msg);
		composition = 0;
	}
}

void LogFile::vaadd(cstr format, va_list va)
{
	// start or append to composed log message
	// a final valog() or nl() is required to print it
	// the timestamp is not set: it will be set by the function which actually prints it
	// called by log()

	lock_guard {mutex};

	if (repetitions) print_repetitions();

	uint maxlen = NELEM(msg) - composition;
	int	 n		= vsnprintf(msg + composition, maxlen, format, va);
	assert(n >= 0);
	if (n < 0) n = snprintf(msg + composition, maxlen, "[[printf format error]]");
	uint num_added = min(uint(n), maxlen);

	// break message before it get's too long:
	while (ptr p = ptr(memchr(msg + composition, '\n', num_added)))
	{
		*p++ = 0; // overwrites the nl
		when = now();
		write2log(msg);
		num_added	= uint(msg + composition + num_added - p);
		composition = 0;
		memmove(msg, p, num_added);
	}

	composition += num_added;
}

void LogFile::valog(double now, cstr format, va_list va)
{
	// log line with timestamp etc.
	// print string to logfile
	// may print buffered repetitions first
	// may be buffered as a repetition
	// may finalize & print a composed message

	lock_guard {mutex};

	if (composition) // finalize and print a composed message
	{
		vsnprintf(msg + composition, NELEM(msg) - composition, format, va);
		composition = 0;
	}
	else
	{
		char z[NELEM(msg)];
		vsnprintf(z, NELEM(z), format, va);
		if (strcmp(msg, z) == 0) // message repeats:
		{
			when = now; // timestamp of last repetition
			repetitions++;
			return;
		}
		if (repetitions) print_repetitions();
		strcpy(msg, z);
	}

	when = now;
	write2log(msg);
}

void LogFile::nl(double now)
{
	// print empty line or
	// finalize and print composed log message if any, else print an empty line
	// may print buffered repetitions first

	lock_guard {mutex};

	if (composition) // finalize and print composed message
	{
		composition = 0;
	}
	else
	{
		if (repetitions) print_repetitions();
		msg[0] = 0;
	}

	when = now;
	write2log(msg);
}


// ====================================================================
// ====================================================================
// ====================================================================


static uint quick_id()
{
	// get (the best guess of) the current thread id
	// without creating a LogFile instance
	// for abort() and panic()

	if (logdir == nullptr) return 0; // not initialized
	return my_logfile ? my_logfile->thread_id : logfiles_cnt;
}

static void flush_logfiles()
{
	// flush unwritten contents to logfile
	// mutex must be locked

	for (uint i = 0; i < logfiles_cnt; i++)
	{
		if (logfiles[i]) logfiles[i]->print_pending();
	}
}

static LogFile* get_logfile(double now)
{
	// get the LogFile instance for the current thread
	// test for initial initialization
	// test for log rotation
	// creates LogFile for current thread if required
	//
	// if called with getLogfile(0.0) then the test for log rotation is omitted
	// if called before openLogfile() then logrotate_when==1e99 then also no log rotation

	if (now >= logrotate_when) open_logfile(); // rotate logfile

	return my_logfile ? my_logfile.get() : new LogFile();
}

static void write2log(uint thread_id, double when, int indent, cstr msg)
{
	// Print formatted log message to stderr and/or logfile.
	// The message should not be longer than 260 characters.
	// Format: "[<thread_id>] <timestamp> <indentation> <message>\n".
	// lock should be locked.

	char sbu[260 + MAXINDENT + 40];

	for (cptr nl; (nl = strchr(msg, '\n')); msg = nl + 1)
	{
		strcpy(sbu, msg, min(uint(NELEM(sbu)), uint(nl - msg + 1)));
		write2log(thread_id, when, indent, sbu);
	}

	char timestamp[40];

	time_t sec = time_t(when);
	tm	   dt;
	time_r(&sec, &dt);

	if (timestamp_with_date)
	{
		int y = dt.tm_year + 1900;
		int m = dt.tm_mon + 1;
		int d = dt.tm_mday;

		if (timestamp_with_msec)
			sprintf(timestamp, fmt11, y, m, d, dt.tm_hour, dt.tm_min, dt.tm_sec, int((when - double(sec)) * 1000));
		else sprintf(timestamp, fmt10, y, m, d, dt.tm_hour, dt.tm_min, dt.tm_sec);
	}
	else
	{
		if (timestamp_with_msec)
			sprintf(timestamp, fmt01, dt.tm_hour, dt.tm_min, dt.tm_sec, int((when - double(sec)) * 1000));
		else sprintf(timestamp, fmt00, dt.tm_hour, dt.tm_min, dt.tm_sec);
	}

	uint sz = uint(snprintf(sbu, NELEM(sbu), "[%u] %s%s  %s\n", thread_id, timestamp, indentstr(indent), msg));
	if (sz >= NELEM(sbu))
	{
		assert(int(sz) >= 0);
		sz			= NELEM(sbu);
		sbu[sz - 1] = '\n';
	}

	if (fd != -1) // write to file
	{
		for (uint i = 0; i < sz;)
		{
			int n = int(write(fd, sbu + i, sz - i)); // <-- cancellation point => throw
			if (n >= 0)
			{
				i += uint(n);
				continue;
			}
			if (errno == EINTR) continue;
			close(fd);
			fd = -1;
			panic("writing to logfile failed", errno);
		}
	}

	if (log2console) // print to stderr
	{
		for (uint i = 0; i < sz;)
		{
			int n = int(write(2, sbu + i, sz - i));
			if (n >= 0)
			{
				i += uint(n);
				continue;
			}
			if (errno == EINTR) continue;
			//log2console = no;
			panic("logging to stderr failed", errno);
		}
	}
}

static void atexit_actions()
{
	// Flush logfile and stop timer thread
	// called at application termination
	// this function is registered with atexit()

	lock_guard {mutex};

	flush_logfiles();
	if (fd != -1) write2log(quick_id(), now(), 0, "\nExit: Logfile closed\n");
}

static void purge_old_logfiles(cstr fname)
{
	// helper:
	// mutex must not be locked

	xlogIn("LogFile:purge_old_logfiles");

	assert(startswith(fname, appl_name));
	assert(logdir != nullptr);
	assert(*logdir == '/');					 // must be full path. '/' must exist.
	assert(*(strchr(logdir, 0) - 1) == '/'); // must end with '/'

	DIR* dir = opendir(logdir); // note: sets sporadic errors
	if (!dir)
	{
		logline("LogFile:purge_old_logfiles: %s", strerror(errno));
		return;
	}

	// collect old log files:
	uint  v_cnt = 0;
	uint  v_max = 100;
	cstr* v		= new cstr[v_max];

	auto					logdirlen = strlen(logdir); // dirpath + '/'
	auto					fnamelen  = strlen(fname);
	std::unique_ptr<char[]> fp {new char[logdirlen + fnamelen + 1]};
	char*					filepath = fp.get();
	strcpy(filepath, logdir);

	for (;;)
	{
		dirent* direntry = readdir(dir);
		if (direntry == nullptr) break; // end of dir

		cstr filename = direntry->d_name;
		if (!startswith(filename, appl_name)) continue; // filename does not match
		auto filenamelen = strlen(filename);
		if (filenamelen != fnamelen) continue; // filename does not match

		strcpy(filepath + logdirlen, filename);
		struct stat filestat;
		if (lstat(filepath, &filestat)) continue; // skip on error
		if (!S_ISREG(filestat.st_mode)) continue; // not a regular file

		if (v_cnt == v_max)
		{
			v_max *= 2;
			cstr* z = new cstr[v_max];
			memcpy(z, v, v_cnt * sizeof(cstr));
			delete[] v;
			v = z;
		}
		v[v_cnt++] = newcopy(filename);
	}

	// delete old logfiles until max_logfiles remain:
	if (v_cnt > max_logfiles)
	{
		sort(v, v + v_cnt);

		for (uint i = 0; i < v_cnt - max_logfiles; i++)
		{
			assert(startswith(v[i], appl_name));
			strcpy(filepath + logdirlen, v[i]);
			remove(filepath);
		}
	}

	for (uint i = 0; i < v_cnt; i++) delete[] v[i];
	delete[] v;
	closedir(dir);
}

static void open_logfile()
{
	char filepath[1024];

	lock_guard {mutex};
	{
		if (debug) write2log(quick_id(), now(), 0, "+++openLogfile+++");

		// calculate current logfile filename
		// and when to rotate log:
		time_t when = now<time_t>() / seconds_per_day * seconds_per_day; // start of day
		tm	   dt;
		time_r(&when, &dt);
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

		switch (logrotate)
		{
		default: panic("ill. log rotation setting");

		case NEVER:
			logrotate_when = 1e99;
			snprintf(filepath, NELEM(filepath), "%s%s.log", logdir, appl_name);
			break;

		case DAILY:
			snprintf(
				filepath, NELEM(filepath), "%s%s-%04i-%02i-%02i.log", logdir, appl_name, //
				dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday);
			logrotate_when = double(when) + seconds_per_day;
			break;

		case MONTHLY:
			when -= (dt.tm_mday - 1) * seconds_per_day; // start of month
			snprintf(
				filepath, NELEM(filepath), "%s%s-%04i-%02i.log", logdir, appl_name, //
				dt.tm_year + 1900, dt.tm_mon + 1);
			dt.tm_mon += 1;
			dt.tm_mday	   = 1;
			logrotate_when = double(timegm(&dt));
			break;

		case WEEKLY:
		{
			int dow = (dt.tm_wday + 6) % 7; // day of week since monday [0-6]
			when -= dow * seconds_per_day;	// start of week
			dt.tm_yday -= dow;

			if (dt.tm_yday < -3) // week is accounted for last year
			{
				time_r(&when, &dt);
				dow = 0;
			} // recalculate tm_yday: dep. on. leap year
			else if (dt.tm_mon == 11 && dt.tm_mday - dow >= 32 - 3) // week is accounted for next year
			{
				dt.tm_year += 1;
				dt.tm_yday = 0;
			}

			snprintf(
				filepath, NELEM(filepath), "%s%s-%04i-week-%02i.log", logdir, appl_name, //
				dt.tm_year + 1900, 1 + (dt.tm_yday + 3) / 7);
			logrotate_when = double(when) + 7 * seconds_per_day;
			break;
		}
		}

		if (fd > 2) close(fd);

		// note: umask(0): else write permission is removed from passed permission flags
		// note: 0666: must be writable for all, else user1 can't write to logfile created by user2
		mode_t z = umask(0);
		fd		 = open(filepath, O_WRONLY | O_CREAT | O_APPEND, 0666);
		umask(z);
	}

	if (fd == -1) panic("open logfile \"%s\" failed: %s", filepath, strerror(errno));

	logline("------------------------------------\nLogfile opened");
	logline("logfile = %s\n", filepath);

	if (logrotate != NEVER && max_logfiles) purge_old_logfiles(filepath + strlen(logdir));
}


// ====================================================================
// ====================================================================
// ====================================================================


void openLogfile(
	cstr appl_name, cstr logdir, LogRotation logrotate, uint max_logfiles, bool log2console, bool with_date,
	bool with_msec, bool utc_timestamps)
{
	// Set settings and open logfile
	// dirpath must start and end with '/', e.g. "/var/log/";

	assert(::logdir == nullptr);

	with_date = with_date && (logrotate != DAILY);
	if (!endswith(logdir, "/")) logdir = catstr(logdir, "/");

	time_r				= utc_timestamps ? &gmtime_r : &localtime_r;
	::appl_name			= newcopy(appl_name);
	::max_logfiles		= max_logfiles;
	::logrotate			= logrotate;
	timestamp_with_msec = with_msec;
	timestamp_with_date = with_date;

	// prepare flushing on exit:
	atexit(atexit_actions);

	logdir = create_path(catstr(logdir, appl_name));
	if (errno) logdir = create_path(catstr("/tmp/", appl_name));
	if (errno) panic("could not create log dir", errno);
	::logdir = newcopy(logdir);

	::log2console = log2console;
	open_logfile();
}

void logline(cstr format, va_list va)
{
	double now = ::now();
	get_logfile(now)->valog(now, format, va);
}

void logline(cstr format, ...)
{
	// log line with timestamp etc.

	va_list va;
	va_start(va, format);
	double now = ::now();
	get_logfile(now)->valog(now, format, va);
	va_end(va);
}

void log(cstr format, va_list va)
{
	// incrementally compose log line

	get_logfile(0.0)->vaadd(format, va);
}

void log(cstr format, ...)
{
	// incrementally compose log line

	va_list va;
	va_start(va, format);
	log(format, va);
	va_end(va);
}

void logNl()
{
	// print composed log line if any, else an empty line

	double now = ::now();
	get_logfile(now)->nl(now);
}

void LogIndent::indent(int di)
{
	// called in ctor after logline() and in dtor

	LogFile* l = my_logfile.get();
	assert(l);
	assert(l->indent + di >= 0);

	if (l->repetitions || l->composition)
	{
		lock_guard {mutex};
		l->print_pending();
	}

	l->indent += di;
}


// ====================================================================
// ====================================================================
// ====================================================================

// GLOBAL: abort with message
// note: must not be called within locked mutex!
// 	     macros like assert, IERR, TODO etc. call abort()!
//	     atexit_actions() locks the mutex
// note: must not be called from functions registered with atexit()
//	     atexit_actions() may call logline(), so this basically appies to almost everything in this file

__noreturn void abort(cstr format, va_list va)
{
	char bu[300];
	snprintf(bu, NELEM(bu), "%s\naborted.", format);
	get_logfile(0.0)->valog(now(), format, va);
	//abort();
	exit(ABORTED); // ~ abort() but no crash report
}

__noreturn void abort(cstr format, ...)
{
	va_list va;
	va_start(va, format);
	abort(format, va);
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


/*






































*/
