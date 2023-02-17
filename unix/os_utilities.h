#pragma once
// Copyright (c) 2001 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"


/* ----	system info ----------------------
*/
extern cstr	  hostName();
extern uint	  numCPUs() noexcept;
extern time_t bootTime();

extern void	  sysLoad(double load[3]); // ranges: 1, 5, and 15 minutes
extern size_t memoryUsage(bool resident = true);
extern double cpuLoad();

extern time_t intCurrentTime();
//extern double	now				();		// was: floatCurrentTime() .. now in kio.cpp
inline time_t upTime() { return intCurrentTime() - bootTime(); }

#ifdef _UNIX
inline uid_t getUID() { return getuid(); }
inline gid_t getGID() { return getgid(); }
inline uid_t getEffUID() { return geteuid(); }
inline gid_t getEffGID() { return getegid(); }
#endif
extern cstr getUser();
extern cstr getEffUser();


extern str execCmd(/* cmd = argv[0] */ const str argv[], const str envv[] = nullptr);
extern str execCmd(cstr cmd, ... /* argv[1] ... NULL */);
