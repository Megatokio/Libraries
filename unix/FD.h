#pragma once
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
*/

#include "kio/kio.h"
#include "s_type.h"				// --> classify_file()
#include <utility>				// --> std::swap()
#define TPL template<class T>
#define SOT sizeof(T)
#define THF	noexcept(false)		// file_error
#define EXT extern
#define INL inline
class FD;


// ------------------------------------------------------------
//			Non-Class Methods:
// ------------------------------------------------------------


EXT	void copy( FD& q, FD& z, off_t count ) THF;



// ------------------------------------------------------------
//			The File Class:
// ------------------------------------------------------------


class FD
{
// ==== data member ======================================

	int 	fd;				// unix file descriptor
	cstr   	fpath;			// allocated copy


// ==== private member functions =========================

	FD&		operator= 		(FD const&);					// prohibit
			FD				(FD const&);					// prohibit: don't pass FD by value - always pass FD by reference!
															// for moving fd from one FD to another use operator=()


// ==== public member functions ==========================
public:

// c'tor and d'tor:
			FD				()											noexcept	:fd(-1),fpath(nullptr) {}
			FD				(cstr path, int flags='r', int perm=0664) 	THF	:fd(-1),fpath(nullptr) { open_file(path,flags,perm); }
			FD				(int fd, cstr fname)						noexcept	:fd(fd),fpath(newcopy(fname)){}
			~FD				()											noexcept;

// standard input and output:
	static	FD	stdin;
	static	FD	stdout;
	static	FD	stderr;

// open file:
// mode can be found in fnctl.h
// most frequently used: O_RDONLY, O_WRONLY, O_RDWR, O_CREAT, O_EXCL, O_APPEND, O_TRUNC

	int 	open_file		(cstr path, int mode, int perm=0664) THF;
	void	open_file_r		(cstr path)						THF		{ open_file(path, 'r', 0664); }		// must exist.
	void	open_file_w		(cstr path, int perm=0664)		THF		{ open_file(path, 'w', perm); }		// truncates file!
	void	open_file_a		(cstr path, int perm=0664)		THF		{ open_file(path, 'a', perm); }		// append mode.
	void	open_file_n		(cstr path, int perm=0664)		THF		{ open_file(path, 'n', perm); }		// new: must not exist.
	void	open_file_m		(cstr path, int perm=0664)		THF		{ open_file(path, 'm', perm); }		// modify: r&w
	void	open_tempfile	() 								THF;	// tempfile open for r/w. will vanish after close

	void	set_file_id		(int fd, cstr fpath)			noexcept;	// deprecated
	void	operator= 		(FD&)							noexcept;	// *MOVE* fd: sets q.fd = -1;  except for stdin,out,err!

	int     set_blocking    (bool)							noexcept;
	int 	set_async		(bool)							noexcept;

	// File mode groups
	// see stat.h
	enum
	{
		readable	= 0444,		// readable by owner, group & others
		writable	= 0222,		// writable by owner, group & others
		executable	= 0111,		// executable/listable by owner, group & others
		owner		= 0700,
		group		= 0070,
		others		= 0007,
		nobody		= 0000
	};
	int		set_permissions	(mode_t perm)				noexcept;
	int		set_permissions	(mode_t who, mode_t perm)	noexcept;	// set perm for owner|group|others

// close & truncate file:
	off_t 	resize_file		(off_t size)				THF;
	off_t	truncate_file	()							THF 	{ return resize_file(file_position()); }
	int		close_file		(bool _throw=1)				THF;
	void	swap			(FD& z)						noexcept	{ std::swap(fd,z.fd); std::swap(fpath,z.fpath); }
	void	swap_fd			(FD& z)						noexcept	{ std::swap(fd,z.fd); }

// information:
	time_t	file_mtime		() const					noexcept;	// modification time
	time_t	file_atime		() const					noexcept;	// last access time
	time_t	file_ctime		() const					noexcept;	// last status change time
	off_t	file_size		() const					noexcept;
	bool	is_writable		() const					noexcept;	// acc. to mode bits. 	TODO: wprot fs etc.
	s_type	classify_file	() const					noexcept;	// note: s_type defined in "s_type.h"
	bool	is_file			() const					noexcept	{ return classify_file()==s_file; }
	bool	is_dir			() const					noexcept	{ return classify_file()==s_dir; }
	bool	is_tty			() const					noexcept	{ return classify_file()==s_tty; }
	cstr	filepath		() const					noexcept	{ return fpath; }
	int		file_id			() const					noexcept	{ return fd; }
	bool	is_valid		() const					noexcept	{ return fd>=0; }

	bool	data_available	() const					noexcept;	// non blocking devices only

// file position:
	off_t	seek_fpos		(off_t, int f=SEEK_SET)		THF;
	off_t	seek_endoffile	()							THF		{ return seek_fpos(0,SEEK_END); }
	off_t	skip_bytes		(off_t signed_offset)		THF		{ return seek_fpos(signed_offset,SEEK_CUR); }
	off_t	rewind_file		()							THF		{ return seek_fpos(0,SEEK_SET); }

	off_t	file_position	()							THF		{ return seek_fpos(0,SEEK_CUR); }
	off_t	file_remaining	()							THF		{ return file_size() - file_position(); }
	bool	is_at_eof		()							THF		{ return file_position() >= file_size(); }
	bool	is_near_eof		(off_t proximity)			THF		{ return file_remaining() <= proximity; }


// read/write c-strings:

	uint32	write_str		(cstr p)					THF		{ return p&&*p ? write_bytes(p,uint32(strlen(p))) : 0; }
	uint32	write_fmt		(cstr fmt, ...)				THF		__printflike(2,3);
	str		read_str		()							THF;	// read from file up to EOF,0,10,13,10+13,13+10
	str		read_new_str	()							THF		{ return newcopy(read_str()); }

	void 	read_file		(Array<str>&, uint32 max=1<<28) 	THF; // temp mem
	void 	read_file		(Array<cstr>& a, uint32 max=1<<28) 	THF {read_file(reinterpret_cast<Array<str>&>(a),max);}
	void 	read_file		(class StrArray&, uint32 max=1<<28) THF; // new[]
	void 	write_file		(Array<str>&)	 					THF;


// read/write length-prefixed strings:
// note: NULL pointers are restored as NULL!

	str		read_nstr		()							THF;	// temp mem
	str		read_new_nstr	()							THF;	// new[]
	void	write_nstr		(cstr)						THF;


// read/write native byte order:

	uint32	read_bytes		(void* p, uint32 sz)		THF;	// throws at eof
	uint32	read_bytes		(void* p, uint32 sz, int)	THF;	// does not throw at eof

TPL uint32	read_data		(T* p)						THF;// 	{ return read_bytes(p,SOT); }		// returns bytes written
TPL	uint32	read_data		(T* p, uint32 cnt) 			THF 	{ return read_bytes(p,cnt*SOT); }	// returns bytes written
TPL	uint32	read_data		(T* p, uint32 cnt, int)	 	THF 	{ return read_bytes(p,cnt*SOT,1); }	// does not throw on eof

TPL uint32	read			(T& n)						THF		{ return read_bytes(_ptr(&n),SOT); }
TPL	T		read			()							THF		{ T n; read_bytes(_ptr(&n),SOT); return n; }
	int8	read_int8		()							THF		{ return read<int8>();   }
	uint8	read_uint8		()							THF		{ return read<uint8>();  }
	char	read_char		()							THF		{ return read<char>();   }
	uchar	read_uchar		()							THF		{ return read<uchar>();  }
	int16	read_int16		()							THF		{ return read<int16>();  }
	uint16	read_uint16		()							THF		{ return read<uint16>(); }
	int32	read_int32		()							THF		{ return read<int32>();  }
	uint32	read_uint32		()							THF		{ return read<uint32>(); }
	int64	read_int64		()							THF		{ return read<int64>();  }
	uint64	read_uint64		()							THF		{ return read<uint64>(); }

	uint32	write_bytes		(void const* p, uint32 sz)	THF;
TPL uint32	write_data		(T const* p, uint32 cnt)	THF		{ return write_bytes(p,cnt*SOT); }	// returns bytes written
TPL uint32	write_data		(T const* p)				THF;// 	{ return write_bytes(p,SOT); }		// returns bytes written

TPL	uint32	write			(T const& n)				THF		{ return write_bytes(_cptr(&n),SOT); }	// ref wg. auto propagate
	uint32	write_int8		(int8 n)					THF		{ return write_bytes(_cptr(&n),sizeof(n)); }
	uint32	write_uint8		(uint8 n)					THF		{ return write_bytes(_cptr(&n),sizeof(n)); }
	uint32	write_char		(char n)					THF		{ return write_bytes(_cptr(&n),sizeof(n)); }
	uint32	write_uchar		(uchar n)					THF		{ return write_bytes(_cptr(&n),sizeof(n)); }
	uint32	write_int16		(int16 n)					THF		{ return write_bytes(_cptr(&n),sizeof(n)); }
	uint32	write_uint16	(uint16 n)					THF		{ return write_bytes(_cptr(&n),sizeof(n)); }
	uint32	write_int32		(int32 n)					THF		{ return write_bytes(_cptr(&n),sizeof(n)); }
	uint32	write_uint32	(uint32 n)					THF		{ return write_bytes(_cptr(&n),sizeof(n)); }
	uint32	write_int64		(int64 n)					THF		{ return write_bytes(_cptr(&n),sizeof(n)); }
	uint32	write_uint64	(uint64 n)					THF		{ return write_bytes(_cptr(&n),sizeof(n)); }
	uint32	write_nl		()							THF		{ return write_uint8('\n'); }


// always revert byte order:

	uint32	read_bytes_reverted(void* p, uint n)		THF;	// read bytes and revert order
	uint32	write_bytes_reverted(void const* p, uint n)	THF;	// write bytes in reverted order

	uint32	read_data_reverted(void* p, uint n, uint sz)		THF;	// read bytes and revert order
	uint32	write_data_reverted(void const* p, uint n, uint sz)	THF;	// write bytes in reverted order


// read/write network byte order, msb first, big endian:

#ifdef _LITTLE_ENDIAN
	uint32	read_bytes_x	(void* p, uint sz)			THF		{ return(read_bytes_reverted(p,sz)); }
	uint32	write_bytes_x	(void const* p, uint sz)	THF		{ return(write_bytes_reverted(p,sz)); }
TPL	uint32	read_data_x		(T* p, uint cnt)			THF		{ return(read_data_reverted(p,cnt,SOT)); }
TPL	uint32	write_data_x	(T const* p, uint cnt)		THF		{ return(write_data_reverted(p,cnt,SOT)); }
#else
	uint32	read_bytes_x	(void* p, uint sz)			THF		{ return(read_bytes(p,sz)); }
	uint32	write_bytes_x	(void const* p, uint sz)	THF		{ return(write_bytes(p,sz)); }
TPL	uint32	read_data_x		(T* p, uint cnt)			THF		{ return(read_data(p,cnt)); }
TPL	uint32	write_data_x	(T const* p, uint cnt)		THF		{ return(write_data(p,cnt)); }
#endif

TPL uint32	read_x			(T& n)						THF		{ return read_bytes_x(&n,SOT); }
TPL	uint32	write_x			(T const& n)				THF		{ return write_bytes_x(&n,SOT); }

	int16	read_int16_x	()							THF;
	uint16	read_uint16_x	()							THF		{ return uint16(read_int16_x()); }
	int32	read_int24_x	()							THF;
	uint32	read_uint24_x	()							THF;
	int32	read_int32_x	()							THF;
	uint32	read_uint32_x	()							THF		{ return uint32(read_int32_x()); }
	int64	read_int64_x	()							THF;
	uint64	read_uint64_x	()							THF		{ return uint64(read_int64_x()); }
	float32	read_float32_x	()							THF		{ float32 f; read_bytes_x(&f,4); return f; }
	float64	read_float64_x	()							THF		{ float64 f; read_bytes_x(&f,8); return f; }
	float128 read_float128_x()							THF		{ float128 f;read_bytes_x(&f,16);return f; }

	uint32	write_int16_x	(int16 n)					THF;
	uint32	write_uint16_x	(uint16 n)					THF		{ return write_int16_x(int16(n)); }
	uint32	write_int24_x	(int32 n)					THF;
	uint32	write_uint24_x	(uint32 n)					THF		{ return write_int24_x(int32(n)); }
	uint32	write_int32_x	(int32 n)					THF;
	uint32	write_uint32_x	(uint32 n)					THF		{ return write_int32_x(int32(n)); }
	uint32	write_int64_x	(int64 n)					THF;
	uint32	write_uint64_x	(uint64 n)					THF		{ return write_int64_x(int64(n)); }


// read/write intel byte order (lsb first, little endian)

#ifdef _LITTLE_ENDIAN
	uint32	read_bytes_z	(void* p, uint sz)			THF		{ return(read_bytes(p,sz)); }
	uint32	write_bytes_z	(void const* p, uint sz)	THF		{ return(write_bytes(p,sz)); }
TPL	uint32	read_data_z		(T* p, uint cnt)			THF		{ return(read_data(p,cnt)); }
TPL	uint32	write_data_z	(T const* p, uint cnt)		THF		{ return(write_data(p,cnt)); }
#else
	uint32	read_bytes_z	(void* p, uint sz)			THF		{ return(read_bytes_reverted(p,sz)); }
	uint32	write_bytes_z	(void const* p, uint sz)	THF		{ return(write_bytes_reverted(p,sz)); }
TPL	uint32	read_data_z		(T* p, uint cnt)			THF		{ return(read_data_reverted(p,cnt,SOT)); }
TPL	uint32	write_data_z	(T const* p, uint cnt)		THF		{ return(write_data_reverted(p,cnt,SOT)); }
#endif

TPL uint32	read_z			(T& n)						THF		{ return read_bytes_z(_ptr(&n),SOT); }
TPL	uint32	write_z			(T const& n)				THF		{ return write_bytes_z(_cptr(&n),SOT); }

	int16	read_int16_z	()							THF;
	uint16	read_uint16_z	()							THF		{ return uint16(read_int16_z()); }
	int32	read_int24_z	()							THF;
	uint32	read_uint24_z	()							THF;
	int32	read_int32_z	()							THF;
	uint32	read_uint32_z	()							THF		{ return uint32(read_int32_z()); }
	int64	read_int64_z	()							THF;
	uint64	read_uint64_z	()							THF		{ return uint64(read_int64_z()); }
	float32	read_float32_z	()							THF		{ float32 f; read_bytes_z(&f,4); return f; }
	float64	read_float64_z	()							THF		{ float64 f; read_bytes_z(&f,8); return f; }
	float128 read_float128_z()							THF		{ float128 f;read_bytes_z(&f,16);return f; }

	uint32	write_int16_z	(int16 n)					THF;
	uint32	write_uint16_z	(uint16 n)					THF		{ return write_int16_z(int16(n)); }
	uint32	write_int24_z	(int32 n)					THF;
	uint32	write_uint24_z	(uint32 n)					THF		{ return write_int24_z(int32(n)); }
	uint32	write_int32_z	(int32 n)					THF;
	uint32	write_uint32_z	(uint32 n)					THF		{ return write_int32_z(int32(n)); }
	uint32	write_int64_z	(int64 n)					THF;
	uint32	write_uint64_z	(uint64 n)					THF		{ return write_int64_z(int64(n)); }


// read / write dynamically sizes integers
// intended for numbers which are expected but not guaranteed to be small
// int12 / uint12 are written as 2 bytes max.
// int16 / uint16 are written as 3 bytes max.
// int32 / uint32 are written as 5 bytes max.

	uint32	write_vuint32	(uint32)					THF;
	uint32	write_vint32	(int32)						THF;
	uint32	read_vuint32	()							THF;
	int32	read_vint32		()							THF;


// TTY

	int		get_terminal_size	(int& rows, int& cols)	noexcept;
	int		set_terminal_size	(int rows, int cols)	noexcept;	// does not work !?
	int		terminal_rows		()						noexcept;
	int		terminal_cols		()						noexcept;
	int		terminal_width		()						noexcept;	// pixel
	int		terminal_height		()						noexcept;	// pixel


// deprecated:
	uint32	read_int16_data_x	(int16*p,		uint32)		__attribute__((deprecated)); // use read_data_x(p,n)
	uint32	read_int16_data_z	(int16*p,		uint32)		__attribute__((deprecated)); // use read_data_z(p,n)
	uint32	write_int16_data_x	(const int16* p, uint32)	__attribute__((deprecated)); // use write_data_x(p,n)
	uint32	write_int16_data_z	(const int16* p, uint32)	__attribute__((deprecated)); // use write_data_z(p,n)
	uint32	write_uint16_data_x	(const uint16* p, uint32)	__attribute__((deprecated)); // use write_data_x(p,n)
	uint32	write_uint16_data_z	(const uint16* p, uint32)	__attribute__((deprecated)); // use write_data_z(p,n)
};

























