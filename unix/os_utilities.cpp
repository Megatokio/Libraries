/*	Copyright  (c)	Günter Woigk 2001 - 2019
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

    2003-08-08 kio	enhanced SysLoad() for all (?) platforms
    2003-08-09 kio	ExecCmd()
    2003-11-25 kio	save&restore stdin/stderr settings
    2011-01-17 kio	rewritten some procs for sysctl()
    2011-06-19 kio	modifications for LINUX
*/


#include <thread>
#include	"../kio/kio.h"

#ifdef HAVE_TIME_H
#include	<time.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include	<sys/time.h>
#endif

#include	<sys/param.h>
#include	<sys/resource.h>
#include	<termios.h>
#include	<pwd.h>

#ifdef HAVE_MACHINE_VMPARAM_H
#include	<machine/vmparam.h>
#endif

#ifdef HAVE_NETDB_H
#include	<netdb.h>
#endif

#ifdef HAVE_SYS_SYSCTL_H
#include	<sys/sysctl.h>
#endif

#ifdef HAVE_SYS_LOADAVG_H
#include	<sys/loadavg.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include	<sys/wait.h>
#endif

#ifdef _MACOSX
//#include "/usr/include/mach/i386/kern_return.h"		NOT PRESENT IN SIERRA
#define panic __mach_panic
#include <mach/mach.h>
#undef panic
#endif

#include	"os_utilities.h"




extern char **environ;					// was required by: Mac OSX (pre 10.5 ?)


cstr getUser()
{
    struct passwd* p = getpwuid(getuid());
    return p ? p->pw_name : nullptr;
}

cstr getEffUser()
{
    struct passwd* p = getpwuid(geteuid());
    return p ? p->pw_name : nullptr;
}


/* ===========================================================
        machine information
            for more sysctl calls see:
            <sys/sysctl.h>
            man sysctl
   ========================================================= */


cstr hostName()
{
#ifdef _BSD
    char s[MAXHOSTNAMELEN];
    size_t size = MAXHOSTNAMELEN;
    int mib[4] = { CTL_KERN, KERN_HOSTNAME };
    sysctl( mib, 2, s, &size, nullptr, 0);
    return dupstr(s);

#else
  #if !defined(MAXHOSTNAMELEN)
    #define MAXHOSTNAMELEN 256
  #endif
    char s[MAXHOSTNAMELEN];
    gethostname(s,MAXHOSTNAMELEN);
    return dupstr(s);
#endif
}

uint numCPUs()
{
#if 1
	uint n = std::thread::hardware_concurrency();
	return n?n:1;
#else
#ifdef _BSD
    int    n;
    size_t size = sizeof(n);
    int mib[4] = { CTL_HW, HW_AVAILCPU };
    sysctl( mib, 2, &n, &size, NULL, 0);
    if(n>=1) return n;

    mib[1] = HW_NCPU;
    sysctl( mib, NELEM(mib), &n, &size, NULL, 0 );

    return n>=1 ? n : 1;

#elif defined(_LINUX)||defined(_SOLARIS)
    return sysconf(_SC_NPROCESSORS_ONLN);
#else
    #fixme!
#endif
#endif
}

// get sysload values
// ranges: 1, 5, and 15 minutes
//
// returns the number of processes in the system run queue (waiting for) execution.
// kio 2016-02-20: i have never seen values below 1.0 on OS X 10.9 and assume that
// the sysload is updated on a user thread which doesn't subtract itself.
//
void sysLoad ( double load[3] )
{
	int n = getloadavg(load, 3);
	if(n<3)
	{
        if(n<1) load[0] = 1.0;
        if(n<2) load[1] = load[0];
                load[2] = load[1];
	}
#ifdef _MACOSX
	static bool fixit = true;
	if(fixit)
	{
		if(load[0]<0.99) fixit = false;
		else { load[0]--; load[1]--; load[2]--; }
	}
#endif
}


#ifdef _MACOSX
// get the CPU load
// returned value 0.0 .. 1.0
// normalized (independent) uf num CPUs
double cpuLoad()
{
	static double old_busy  = 0;
	static double old_total = 0;

    natural_t num_cpus;
    processor_info_array_t cpu_info;
    mach_msg_type_number_t info_cnt;
	kern_return_t err;

    err = host_processor_info(mach_host_self(), PROCESSOR_CPU_LOAD_INFO, &num_cpus, &cpu_info, &info_cnt);
    if(err != KERN_SUCCESS) { logline("host_processor_info returned %u", uint(err)); return 0; }

	double new_busy  = 0;
	double new_total = 0;

	for(uint cpu = 0; cpu < num_cpus; ++cpu)
	{
		integer_t* info = &cpu_info[cpu*CPU_STATE_MAX];
		new_busy  += info[CPU_STATE_USER] + info[CPU_STATE_SYSTEM] + info[CPU_STATE_NICE];
		new_total += new_busy + info[CPU_STATE_IDLE];
	}

	size_t info_size = sizeof(*cpu_info) * info_cnt;
	vm_deallocate(mach_task_self(), vm_address_t(cpu_info), info_size);

	double rval = (new_busy-old_busy) / (new_total-old_total);
	old_busy = new_busy;
	old_total = new_total;
	return rval / num_cpus;
}
#endif


time_t intCurrentTime()
{
	struct timespec tv;
	clock_gettime(CLOCK_REALTIME, &tv);
	return tv.tv_sec;
}

//double now()			now in log.cpp
//{
//    struct timeval tv;
//    gettimeofday ( &tv, NULL );
//    return tv.tv_sec + tv.tv_usec/1000000.0;
//}


time_t bootTime ( )
{
#ifdef _BSD
    int mib[4] = { CTL_KERN, KERN_BOOTTIME };
    struct timeval data;
    size_t size = sizeof(data);
    sysctl ( mib, 2, &data, &size, nullptr, 0 );

    return data.tv_sec;

#elif defined(_LINUX)
    // Better way to do is use jiffies or get_cycles function
    // time being will read /proc/uptime
    FILE *fp;
    long ut;
    if((fp=fopen("/proc/uptime","r"))==NULL)
    {
        printf("Cannot open file /proc/uptime\n");	// -> FILE Stdout
        return(-1);
    }
    fscanf(fp,"%ld",&ut);
    fclose(fp);
    return intCurrentTime()-ut;
#else
    #fixme!
#endif
}


#ifdef _LINUX
# include <sys/sysinfo.h>
#endif

#ifdef _BSD
# include <mach/task.h>
# include <mach/mach_init.h>
#endif

#ifdef _WINDOWS
# include <windows.h>
#else
# include <sys/resource.h>
#endif


// The amount of memory currently being used by this process, in bytes.
// By default, returns the full virtual arena, but if resident=true,
// it will report just the resident set in RAM (if supported on that OS).
// http://stackoverflow.com/questions/372484/how-do-i-programmatically-check-memory-use-in-a-fairly-portable-way-c-c
//
size_t memoryUsage (bool resident)
{
#if defined(_LINUX)
    // Ugh, getrusage doesn't work well on Linux.  Try grabbing info
    // directly from the /proc pseudo-filesystem.  Reading from
    // /proc/self/statm gives info on your own process, as one line of
    // numbers that are: virtual mem program size, resident set size,
    // shared pages, text/code, data/stack, library, dirty pages.  The
    // mem sizes should all be multiplied by the page size.
    //
    FILE* file = fopen("/proc/self/statm", "r");
    if (!file) return 0;

    unsigned long vm = 0;
    fscanf (file, "%ul", &vm);  // Just need the first num: vm size
    fclose (file);
    return (size_t)vm * getpagesize();

#elif defined(_BSD)
    // Inspired by:
    // http://miknight.blogspot.com/2005/11/resident-set-size-in-mac-os-x.html
    //
    struct task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;
    task_info(current_task(), TASK_BASIC_INFO, (task_info_t)&t_info, &t_info_count);
    return (resident ? t_info.resident_size : t_info.virtual_size);

#elif defined(_WINDOWS)
    // According to MSDN...
    // see msdn.microsoft.com/en-us/library/ms683219(VS.85).aspx and the linked example
    //
    PROCESS_MEMORY_COUNTERS counters;
    if (GetProcessMemoryInfo (GetCurrentProcess(), &counters, sizeof (counters)))
        return counters.PagefileUsage;
    else return 0;

#else
    #fixme!
#endif
}










/* ==== Execute External Command File ===========================
*/
/* ----	execute external command ----------------------------
        argv[]  must be a 0-terminated list
        argv[0]	hard path to command
        envv[]  is either NULL or a 0-terminated list of environment variables
                in case of NULL the current global environ[] list is passed
        returns	stdout output of command called
                returned string must be delete[]ed
                output to stderr is still printed to stderr
                return code is passed implicitely in errno
        returns NULL if exec failed
                then errno is set
        errno:	noerr: ok
                errno: exec() failed
                childreturnederror:		 returned error code is part of custom ErrorText()
                childterminatedbysignal: signal number is part of custom ErrorText()
*/
str execCmd ( str const argv[], str const envv[] )
{
    assert(argv && argv[0] && *argv[0]);

    errno=ok;

    const int R=0,W=1;
    int pipout[2];
    if ( pipe(pipout) != 0 )
    {
        assert(errno!=ok);
        return nullptr;
    }

// preset result to "nothing"
    char* result = nullptr;

// it seems, that the child can mess up the stdin stream settings
// (and maybe stderr too) especially if execve() fails
// so save them here and restore them afterwards							kio 2003-11-25
    struct termios old_stderr_termios;
    struct termios old_stdin_termios;
    bool restore_stdin  = tcgetattr(0,&old_stdin_termios ) != -1;
    bool restore_stderr = tcgetattr(2,&old_stderr_termios) != -1;

    // TODO: ExecCmd() should search for cmd file prior to spawning

// *** DOIT ***
    pid_t child_id = fork();
    switch ( child_id )
    {
    case -1:// error happened:
            close(pipout[R]);
            close(pipout[W]);
            assert(errno!=ok);
            break;

    case 0:	// child process:
        {
            close(pipout[R]);	// close unused fd
            close(1);			// close stdout
            dup(pipout[W]);		// becomes lowest unused fileid: stdout
            close(pipout[W]);	// close unused dup

            // call cmd:
            xlogline("ExecCmd(): exec %s",argv[0]);
            execve(argv[0],argv,envv?envv:(str const *)environ);

            // call failed. try path completion
            str path = getenv("PATH");
            if (path&&*argv[0]!='/')
            {
                for(;;)
                {
                    while(*path==':') path++;
                    char* dp = strchr(path,':');
                    if (!dp) dp = strchr(path,0);
                    if(dp==path) break;						// end of env.PATH reached: finally no success

                    str cmd = catstr(substr(path,dp),"/",argv[0]);
                    xlogline("ExecCmd(): exec %s",cmd);
                    execve(cmd,argv,envv?envv:(str const *)environ);	// no return if success
                    path = dp;
                }
            }

            xlogline( "ExecCmd(%s) failed: %s", quotedstr(argv[0]), strerror(errno) );
            if(errno==EFAULT) {xlogline("ExecCmd(): HINT: make shure the argv[] is NULL terminated!");}	// kio 2013-03-31
            exit(errno);
        }

    default:// parent process
        {
            close(pipout[W]);	// close unused fd

            ssize_t result_size = 0;
            ssize_t result_used = 0;
            int		status;

            for(;;)
            {
                if (result_used+100>result_size)
                {
                    result_size += result_size/8 + 4000;
                    char* newbu = newstr(int(result_size));
                    memcpy ( newbu,result,result_used );
                    delete[] result;
                    result = newbu;
                }

                ssize_t n = read ( pipout[R], result+result_used, result_size-result_used-1 ); //log("[%i]",int(n));

                if(n>0)		  { result_used += n; }
                else if(n==0) { if(waitpid(child_id,&status,0)==child_id) { errno=ok; break; } }
                else		  { if (errno!=EINTR&&errno!=EAGAIN) break; }
            }

            close(pipout[R]);	// close fd

            if (errno==ok)
            {
                if ( WIFEXITED(status) )			// child process returned normally
                {
                    if ( WEXITSTATUS(status)!=0 )	// child process returned error code
                    {
						errno = childreturnederror;
//                        SetError
//                        (	childreturnederror,
//                            usingstr( "%s returned exit code %i", quotedstr(argv[0]), int(WEXITSTATUS(status)) )
//                        );
                    }
                }
                else if ( WIFSIGNALED(status) )		// child process terminated by signal
                {
						errno = childterminatedbysignal;
//                        SetError
//                        (	childterminatedbysignal,
//                            usingstr( "%s terminated by signal %i", quotedstr(argv[0]), int(WTERMSIG(status)) )
//                        );
                }
                else IERR();
            }

            result[result_used] = 0;
        }
    }

// restore stream settings
    if (restore_stderr) { tcsetattr(2,TCSADRAIN,&old_stderr_termios); xlogline("ExecCmd(): restored stderr"); }
    if (restore_stdin)  { tcsetattr(0,TCSADRAIN,&old_stdin_termios);  xlogline("ExecCmd(): restored stdin");  }

// return result
    return result;
}

str execCmd ( cstr cmd, ... )
{
    cstr* argv = new cstr[100];
    argv[0] = cmd;
    uint argc = 1;

    va_list va;
    va_start(va,cmd);

    cstr arg;
    do
    {
        if ((argc%100)==0)
        {
            cstr* newargv = new cstr[argc+100];
            memcpy(newargv,argv,argc*sizeof(cstr));
            delete[]argv;
            argv=newargv;
        }

        arg = va_arg(va,cstr);
        argv[argc++] = arg;
    }
    while (arg);

    va_end(va);

    str result = execCmd((char*const*)argv);
    delete[]argv;
    return result;
}












