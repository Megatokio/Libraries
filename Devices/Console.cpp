// Copyright (c) 2023 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Console.h"
#include "cstrings/tempmem.h"
#include <dirent.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/ioctl.h> // linux, mac, who else?
#include <sys/stat.h>
#include <termios.h>


/*	this implementation requires POSIX 2008

	Console input requires non-blocking unbuffered input with no echo.
	A FILE cannot be set to any of this, if it isn't already so.
	Therefore we must use UNIX/POSIX file handling. 

	#ifdef _POSIX_C_SOURCE
	stdio:   https://man7.org/linux/man-pages/man3/stdio.3.html
	fcntl:   https://manpages.debian.org/unstable/manpages-de-dev/fcntl.2.de.html
	termios: https://man7.org/linux/man-pages/man0/termios.h.0p.html
	poll:    https://man7.org/linux/man-pages/man3/poll.3p.html
*/

uint classify_file(int fd = 0 /*stdin*/) noexcept
{
	// classify file
	// returns a type id as defined in <dirent.h>
	struct stat data;
	if (fstat(fd, &data)) return DT_UNKNOWN;
	else return data.st_mode >> 12; // acc. to <bits/stat.h>
}


namespace kio
{

static int	   original_fl_state[2];
static termios original_tc_state;
static int	   original_state_valid = 0;
static int	   fl_state[2];

static inline void set_non_blocking(int fd, bool f)
{
	assert(uint(fd) <= 1);
	int& state = fl_state[fd];
	if ((f ? state : ~state) & O_NONBLOCK) return;
	state ^= O_NONBLOCK;
	fcntl(0, F_SETFL, state);
}
static void set_tcstate(int fd, uint set, uint res = 0)
{
	static termios tc_state;
	tcgetattr(fd, &tc_state);
	tc_state.c_lflag |= set;
	tc_state.c_lflag &= ~res;
	tcsetattr(fd, TCSANOW, &tc_state);
}
static void res_tcstate(int fd, uint res)
{
	set_tcstate(fd, 0, res); //
}
static int get_console_width()
{
	struct winsize ws;
	return ioctl(1 /*stdout*/, TIOCGWINSZ, &ws) ? 0 : ws.ws_col;
}


// *************************************************************

Console::Console() : SerialDevice(readwrite)
{
	if (++original_state_valid == 1)
	{
		original_fl_state[0] = fl_state[0] = fcntl(0, F_GETFL);
		original_fl_state[1] = fl_state[1] = fcntl(1, F_GETFL);
		tcgetattr(0, &original_tc_state);
	}
	res_tcstate(0, ICANON); // disable input buffer
	res_tcstate(0, ECHO);	// disable echo (is enabled by clear ICANON!)
}
Console::~Console()
{
	if (--original_state_valid == 0)
	{
		fcntl(0, F_SETFL, original_fl_state[0]);
		fcntl(1, F_SETFL, original_fl_state[1]);
		tcsetattr(0, TCSANOW, &original_tc_state);
	}
}

uint32 Console::ioctl(IoCtl ctl, void* arg1, void* arg2)
{
	switch (ctl.cmd)
	{
	case IoCtl::FLUSH_OUT: return 0;
	case IoCtl::FLUSH_IN:
	{
		uint t = classify_file();
		if (t == DT_CHR || t == DT_FIFO || t == DT_SOCK)
		{
			char bu[100];
			while (read(bu, sizeof(bu), true) == sizeof(bu)) {}
		}
		return 0;
	}
	default: return SerialDevice::ioctl(ctl, arg1, arg2);
	}
}

SIZE Console::write(const void* data, SIZE size, bool partial)
{
	// write `size` bytes
	// throw on error
	// returns number of bytes actually written

	const int fd = 1; // stdout
	set_non_blocking(fd, partial);

	for (uint cnt = 0;;)
	{
		auto n = ::write(fd, cptr(data) + cnt, size - cnt);
		if (n >= 0)
		{
			cnt += n;
			if (cnt == size) return cnt;
			if (partial) return cnt;
			continue;
		}

		if (errno == EINTR) continue;				 // system call interrupted by signal.
		if (errno == EAGAIN || errno == EWOULDBLOCK) // not ready. non-blocking dev only
		{
			if (partial) return cnt; // EAGAIN -> it must be partial
			usleep(5000);			 // safety only. this should not happen.
			continue;
		}
		throw strerror(errno); // everything else
	}
}

SIZE Console::read(void* data, SIZE size, bool partial)
{
	// read `size` bytes
	// throw on error
	// returns number of bytes actually read

	const int fd = 0; // stdin
	set_non_blocking(fd, partial);

	for (uint cnt = 0;;)
	{
		auto n = ::read(fd, ptr(data) + cnt, size - cnt);
		if (n >= 0)
		{
			cnt += n;
			if (cnt == size) return cnt;
			if (n) continue;
			if (partial) return cnt;
			throw END_OF_FILE;
		}

		if (errno == EINTR) continue;				 // system call interrupted by signal.
		if (errno == EAGAIN || errno == EWOULDBLOCK) // not ready. non-blocking dev only
		{
			if (partial) return cnt; // EAGAIN -> it must be partial
			usleep(5000);			 // safety only. this should not happen.
			continue;
		}
		throw strerror(errno); // everything else
	}
}

char Console::getc()
{
	Console::read(&last_char, 1, false);
	return last_char;
}

int Console::getc(uint timeout_us)
{
	uint n = read(&last_char, 1, true);
	if (n) return last_char;
	if (timeout_us == 0) return -1;

	int timeout_ms = (timeout_us + 999) / 1000;

	pollfd fds {0, POLLIN, 0};
	int	   e = poll(&fds, 1, timeout_ms);
	if (e < 0) throw strerror(errno);

	n = read(&last_char, 1, true);
	return n ? last_char : -1;
}


cstr lineInput(SerialDevice* console, cstr prompt, cstr oldtext, int epos)
{
	// enter a new line or edit an existing line of text by the user.
	// input is either ascii or ucs1. utf-8 needs a wrapper, e.g. class Utf8Decoder.
	// returns a tempstr

	// TODO
	// positioning is still a little bit buggy.
	// cursor left

	enum {
		BACKSPACE	  = 8,
		RUBOUT		  = 0x7f,
		NL			  = 10,
		RETURN		  = 13,
		ESC			  = 0x1b,
		CSI			  = 0x9b, // C1 version of ESC[
		KEY_BACKSPACE = BACKSPACE,
		KEY_DELETE,
		KEY_INSERT,
		KEY_ARROW_UP,
		KEY_ARROW_DOWN,
		KEY_ARROW_RIGHT,
		KEY_ARROW_LEFT,
	};

	if (!oldtext) oldtext = "";
	assert(epos <= int(strlen(oldtext)));

	console->puts(prompt);

	// annoyingly BS in the linux terminal does not wrap.
	// esc [ <N> j is ignored
	// esc [ <N> D stops at left border (as it should)
	static constexpr bool vt102_works	= yes;
	const int			  width			= get_console_width();
	const int			  strlen_prompt = int(strlen(prompt));

	// oldtext = usingstr("w=%i", width);
	// epos	= int(strlen(oldtext));

	auto left = [&](int n) {
		if (vt102_works && width != 0 && n != 0)
		{
			int col	 = (strlen_prompt + epos + n) % width;
			int zcol = (strlen_prompt + epos) % width;

			if (col == 0) // crsr may actually stand in col80 of prev line
			{			  // and also crsr right doesn't work in this case ...
				char c = epos + n < int(strlen(oldtext)) ? oldtext[epos + n] : ' ';
				console->putc(c);
				col = 1;
				n += 1;
			}

			if (zcol < col) { console->printf("\x1b[%iD", col - zcol); }
			if (zcol > col) { console->printf("\x1b[%iC", zcol - col); }
			n += zcol - col;

			assert(n % width == 0);
			if (n) console->printf("\x1b[%iA", n / width);
		}
		else
		{
			while (n--) console->putc(BACKSPACE); //
		}
	};

	auto right = [&](int n) {
		cptr p = oldtext + epos;
		while (n--) console->putc(*p++);
	};

	auto puts = [=](cstr s) { console->puts(s); };
	auto putc = [=](char c) { console->putc(c); };
	auto getc = [=]() { return console->getc(); };

	puts(oldtext);
	left(int(strlen(oldtext)) - epos);

	while (1)
	{
		//showCursor();
		int c = console->getc();
	a:
		if (is_printable(char(c)))
		{
			oldtext = catstr(leftstr(oldtext, epos), charstr(char(c)), oldtext + epos);
			puts(oldtext + epos++);
			left(int(strlen(oldtext)) - epos);
			continue;
		}

		// else it's a control code:
		switch (c)
		{
		case RETURN:
		case NL:
			puts(oldtext + epos);
			putc('\n');
			return oldtext;
		case RUBOUT: c = KEY_DELETE; break;		  // REMOVE
		case BACKSPACE: c = KEY_BACKSPACE; break; // '<---' key
		case ESC:
		case CSI:
			if (c == ESC) c = getc();
			if (c == '[') c = getc();
			switch (c)
			{
			case '2': // ESC[2~  -> INSERT
				c = getc();
				if (c != '~') debugstr("{ESC,2}");
				if (c != '~') goto a;
				c = KEY_INSERT;
				break;
			case '3': // ESC[3~  -> REMOVE
				c = getc();
				if (c != '~') debugstr("{ESC,3}");
				if (c != '~') goto a;
				c = KEY_DELETE;
				break;
			case 0x48:							  // POS1
			case 'A': c = KEY_ARROW_UP; break;	  // ESC[A
			case 0x46:							  // END
			case 'B': c = KEY_ARROW_DOWN; break;  // ESC[B
			case 'C': c = KEY_ARROW_RIGHT; break; // ESC[C
			case 'D': c = KEY_ARROW_LEFT; break;  // ESC[D
			default: debugstr("{ESC,0x%02x}", uint(c)); continue;
			}
			break;
		default: debugstr("{0x%02x}", uint(c)); continue;
		}

		switch (c)
		{
		case KEY_INSERT: // TODO: toggle INSERT mode?
			oldtext = catstr(leftstr(oldtext, epos), charstr(char(c)), oldtext + epos);
			puts(oldtext + epos);
			left(int(strlen(oldtext)) - epos);
			continue;
		case KEY_BACKSPACE:
			if (epos == 0) break;
			epos--;
			left(1);
			[[fallthrough]];
		case KEY_DELETE:
			if (oldtext[epos] == 0) break;
			oldtext = catstr(leftstr(oldtext, epos), oldtext + epos + 1);
			puts(oldtext + epos);
			putc(' ');
			left(int(strlen(oldtext)) - epos + 1);
			break;
		case KEY_ARROW_LEFT:
			if (epos > 0)
			{
				epos--;
				left(1);
			}
			break;
		case KEY_ARROW_RIGHT:
			if (oldtext[epos] != 0) putc(oldtext[epos++]);
			break;
		case KEY_ARROW_UP:
		{
			int n = width ? min(width, epos) : epos;
			epos -= n;
			left(n);
			break;
		}
		case KEY_ARROW_DOWN:
		{
			int n = int(strlen(oldtext));
			n	  = width && epos + width < n ? width : n - epos;
			right(n);
			epos += n;
			break;
		}
		default: debugstr("{%s}", tostr(c & 0xff));
		}
	}
}


} // namespace kio


/*




































*/
