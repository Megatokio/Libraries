#pragma once
// Copyright (c) 2001 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"
#include "s_type.h" // --> classify_file()
#include <utility>	// --> std::swap()
#define TPL template<class T>
class FD;


// ------------------------------------------------------------
//			Non-Class Methods:
// ------------------------------------------------------------


extern void copy(FD& q, FD& z, off_t count);


// ------------------------------------------------------------
//			The File Class:
// ------------------------------------------------------------


class FD
{
	// ==== data member ======================================

	int	 fd	   = -1;	  // unix file descriptor
	cstr fpath = nullptr; // allocated copy


	// ==== public member functions ==========================
public:
	// c'tor and d'tor:
	FD() noexcept {}
	FD(cstr path, int flags = 'r', mode_t perm = 0664) { open_file(path, flags, perm); }
	FD(int fd, cstr fname) noexcept : fd(fd), fpath(newcopy(fname)) {}
	~FD() noexcept;

	FD(const FD&) noexcept;			   // only for std i/o !
	FD& operator=(const FD&) noexcept; // only for std i/o !

	FD(FD&&) noexcept;
	FD& operator=(FD&&) noexcept;

	// standard input and output:
	static FD _stdin;  // 2021-03-09 changed from "stdin"
	static FD _stdout; // because stdin, stdout, stderr are macros by C89/C99
	static FD _stderr;

	// open file:
	// mode can be found in fnctl.h
	// most frequently used: O_RDONLY, O_WRONLY, O_RDWR, O_CREAT, O_EXCL, O_APPEND, O_TRUNC

	int	 open_file(cstr path, int mode, mode_t perm = 0664);
	void open_file_r(cstr path) { open_file(path, 'r', 0664); }						// must exist.
	void open_file_w(cstr path, mode_t perm = 0664) { open_file(path, 'w', perm); } // truncates file!
	void open_file_a(cstr path, mode_t perm = 0664) { open_file(path, 'a', perm); } // append mode.
	void open_file_n(cstr path, mode_t perm = 0664) { open_file(path, 'n', perm); } // new: must not exist.
	void open_file_m(cstr path, mode_t perm = 0664) { open_file(path, 'm', perm); } // modify: r&w
	void open_tempfile(); // tempfile open for r/w. will vanish after close

	void set_file_id(int fd, cstr fpath) noexcept __attribute__((__deprecated__));

	int set_blocking(bool) noexcept;
	int set_async(bool) noexcept;

	// File mode groups
	// see stat.h
	enum {
		readable   = 0444, // readable by owner, group & others
		writable   = 0222, // writable by owner, group & others
		executable = 0111, // executable/listable by owner, group & others
		owner	   = 0700,
		group	   = 0070,
		others	   = 0007,
		nobody	   = 0000
	};
	int set_permissions(mode_t perm) noexcept;
	int set_permissions(mode_t who, mode_t perm) noexcept; // set perm for owner|group|others

	// close & truncate file:
	off_t resize_file(off_t size);
	off_t truncate_file() { return resize_file(file_position()); }
	int	  close_file(bool _throw = 1);
	void  swap(FD& z) noexcept
	{
		std::swap(fd, z.fd);
		std::swap(fpath, z.fpath);
	}
	void swap_fd(FD& z) noexcept { std::swap(fd, z.fd); }

	// information:
	time_t file_mtime() const noexcept; // modification time
	time_t file_atime() const noexcept; // last access time
	time_t file_ctime() const noexcept; // last status change time
	off_t  file_size() const noexcept;
	bool   is_writable() const noexcept;   // acc. to mode bits. 	TODO: wprot fs etc.
	s_type classify_file() const noexcept; // note: s_type defined in "s_type.h"
	bool   is_file() const noexcept { return classify_file() == s_file; }
	bool   is_dir() const noexcept { return classify_file() == s_dir; }
	bool   is_tty() const noexcept { return classify_file() == s_tty; }
	cstr   filepath() const noexcept { return fpath; }
	cstr   filename() const noexcept
	{
		cstr p = fpath;
		if (p) p = strrchr(fpath, '/');
		return p ? p + 1 : fpath;
	}
	int	 file_id() const noexcept { return fd; }
	bool is_valid() const noexcept { return fd >= 0; }

	bool data_available() const noexcept; // non blocking devices only

	// file position:
	off_t seek_fpos(off_t, int f = SEEK_SET);
	off_t seek_endoffile() { return seek_fpos(0, SEEK_END); }
	off_t skip_bytes(off_t signed_offset) { return seek_fpos(signed_offset, SEEK_CUR); }
	off_t rewind_file() { return seek_fpos(0, SEEK_SET); }

	off_t file_position() const;
	off_t file_remaining() const { return file_size() - file_position(); }
	bool  is_at_eof() const { return file_position() >= file_size(); }
	bool  is_near_eof(off_t proximity) const { return file_remaining() <= proximity; }


	// read/write c-strings:

	void   skip_utf8_bom();
	uint32 write_str(cstr p) { return p && *p ? write_bytes(p, uint32(strlen(p))) : 0; }
	uint32 write_fmt(cstr fmt, ...) __printflike(2, 3);
	str	   read_str(); // read from file up to EOF,0,10,13,10+13,13+10
	str	   read_new_str() { return newcopy(read_str()); }

	void read_file(Array<str>&, uint32 max = 1 << 28); // temp mem
	void read_file(Array<cstr>& a, uint32 max = 1 << 28) { read_file(reinterpret_cast<Array<str>&>(a), max); }
	void read_file(class StrArray&, uint32 max = 1 << 28); // new[]
	void write_file(const Array<str>&);
	void write_file(const Array<cstr>& a) { write_file(reinterpret_cast<const Array<str>&>(a)); }


	// read/write length-prefixed strings:
	// note: NULL pointers are restored as NULL!

	str	 read_nstr();	  // temp mem
	str	 read_new_nstr(); // new[]
	void write_nstr(cstr);


	// read/write native byte order:

	uint32 read_bytes(void* p, uint32 sz);		// throws at eof
	uint32 read_bytes(void* p, uint32 sz, int); // does not throw at eof

	TPL uint32 read_data(T* p); //	{ return read_bytes(p,sizeof(T)); }		// returns bytes written
	TPL uint32 read_data(T* p, uint32 cnt) { return read_bytes(p, cnt * sizeof(T)); }		  // returns bytes written
	TPL uint32 read_data(T* p, uint32 cnt, int) { return read_bytes(p, cnt * sizeof(T), 1); } // does not throw on eof

	TPL uint32 read(T& n) { return read_bytes(_ptr(&n), sizeof(T)); }
	TPL T	   read()
	{
		T n;
		read_bytes(_ptr(&n), sizeof(T));
		return n;
	}
	int8   read_int8() { return read<int8>(); }
	uint8  read_uint8() { return read<uint8>(); }
	char   read_char() { return read<char>(); }
	uchar  read_uchar() { return read<uchar>(); }
	int16  read_int16() { return read<int16>(); }
	uint16 read_uint16() { return read<uint16>(); }
	int32  read_int32() { return read<int32>(); }
	uint32 read_uint32() { return read<uint32>(); }
	int64  read_int64() { return read<int64>(); }
	uint64 read_uint64() { return read<uint64>(); }

	uint32	   write_bytes(const void* p, uint32 sz);
	TPL uint32 write_data(const T* p, uint32 cnt) { return write_bytes(p, cnt * sizeof(T)); } // returns bytes written
	TPL uint32 write_data(const T* p); //	{ return write_bytes(p,sizeof(T)); }		// returns bytes written

	TPL uint32 write(const T& n) { return write_bytes(_cptr(&n), sizeof(T)); } // ref wg. auto propagate
	uint32	   write_int8(int8 n) { return write_bytes(_cptr(&n), sizeof(n)); }
	uint32	   write_uint8(uint8 n) { return write_bytes(_cptr(&n), sizeof(n)); }
	uint32	   write_char(char n) { return write_bytes(_cptr(&n), sizeof(n)); }
	uint32	   write_uchar(uchar n) { return write_bytes(_cptr(&n), sizeof(n)); }
	uint32	   write_int16(int16 n) { return write_bytes(_cptr(&n), sizeof(n)); }
	uint32	   write_uint16(uint16 n) { return write_bytes(_cptr(&n), sizeof(n)); }
	uint32	   write_int32(int32 n) { return write_bytes(_cptr(&n), sizeof(n)); }
	uint32	   write_uint32(uint32 n) { return write_bytes(_cptr(&n), sizeof(n)); }
	uint32	   write_int64(int64 n) { return write_bytes(_cptr(&n), sizeof(n)); }
	uint32	   write_uint64(uint64 n) { return write_bytes(_cptr(&n), sizeof(n)); }
	uint32	   write_nl() { return write_uint8('\n'); }


	// always revert byte order:

	uint32 read_bytes_reverted(void* p, uint n);		// read bytes and revert order
	uint32 write_bytes_reverted(const void* p, uint n); // write bytes in reverted order

	uint32 read_data_reverted(void* p, uint n, uint sz);		// read bytes and revert order
	uint32 write_data_reverted(const void* p, uint n, uint sz); // write bytes in reverted order


	// read/write network byte order, msb first, big endian:

#ifdef __LITTLE_ENDIAN__
	uint32	   read_bytes_x(void* p, uint sz) { return (read_bytes_reverted(p, sz)); }
	uint32	   write_bytes_x(const void* p, uint sz) { return (write_bytes_reverted(p, sz)); }
	TPL uint32 read_data_x(T* p, uint cnt) { return (read_data_reverted(p, cnt, sizeof(T))); }
	TPL uint32 write_data_x(const T* p, uint cnt) { return (write_data_reverted(p, cnt, sizeof(T))); }
#else
	uint32	   read_bytes_x(void* p, uint sz) { return (read_bytes(p, sz)); }
	uint32	   write_bytes_x(const void* p, uint sz) { return (write_bytes(p, sz)); }
	TPL uint32 read_data_x(T* p, uint cnt) { return (read_data(p, cnt)); }
	TPL uint32 write_data_x(const T* p, uint cnt) { return (write_data(p, cnt)); }
#endif

	TPL uint32 read_x(T& n) { return read_bytes_x(&n, sizeof(T)); }
	TPL uint32 write_x(const T& n) { return write_bytes_x(&n, sizeof(T)); }

	int16	read_int16_x();
	uint16	read_uint16_x() { return uint16(read_int16_x()); }
	int32	read_int24_x();
	uint32	read_uint24_x();
	int32	read_int32_x();
	uint32	read_uint32_x() { return uint32(read_int32_x()); }
	int64	read_int64_x();
	uint64	read_uint64_x() { return uint64(read_int64_x()); }
	float32 read_float32_x()
	{
		float32 f;
		read_bytes_x(&f, 4);
		return f;
	}
	float64 read_float64_x()
	{
		float64 f;
		read_bytes_x(&f, 8);
		return f;
	}
	float128 read_float128_x()
	{
		float128 f;
		read_bytes_x(&f, 16);
		return f;
	}

	uint32 write_int16_x(int16 n);
	uint32 write_uint16_x(uint16 n) { return write_int16_x(int16(n)); }
	uint32 write_int24_x(int32 n);
	uint32 write_uint24_x(uint32 n) { return write_int24_x(int32(n)); }
	uint32 write_int32_x(int32 n);
	uint32 write_uint32_x(uint32 n) { return write_int32_x(int32(n)); }
	uint32 write_int64_x(int64 n);
	uint32 write_uint64_x(uint64 n) { return write_int64_x(int64(n)); }


	// read/write intel byte order (lsb first, little endian)

#ifdef __LITTLE_ENDIAN__
	uint32	   read_bytes_z(void* p, uint sz) { return (read_bytes(p, sz)); }
	uint32	   write_bytes_z(const void* p, uint sz) { return (write_bytes(p, sz)); }
	TPL uint32 read_data_z(T* p, uint cnt) { return (read_data(p, cnt)); }
	TPL uint32 write_data_z(const T* p, uint cnt) { return (write_data(p, cnt)); }
#else
	uint32	   read_bytes_z(void* p, uint sz) { return (read_bytes_reverted(p, sz)); }
	uint32	   write_bytes_z(const void* p, uint sz) { return (write_bytes_reverted(p, sz)); }
	TPL uint32 read_data_z(T* p, uint cnt) { return (read_data_reverted(p, cnt, sizeof(T))); }
	TPL uint32 write_data_z(const T* p, uint cnt) { return (write_data_reverted(p, cnt, sizeof(T))); }
#endif

	TPL uint32 read_z(T& n) { return read_bytes_z(_ptr(&n), sizeof(T)); }
	TPL uint32 write_z(const T& n) { return write_bytes_z(_cptr(&n), sizeof(T)); }

	int16	read_int16_z();
	uint16	read_uint16_z() { return uint16(read_int16_z()); }
	int32	read_int24_z();
	uint32	read_uint24_z();
	int32	read_int32_z();
	uint32	read_uint32_z() { return uint32(read_int32_z()); }
	int64	read_int64_z();
	uint64	read_uint64_z() { return uint64(read_int64_z()); }
	float32 read_float32_z()
	{
		float32 f;
		read_bytes_z(&f, 4);
		return f;
	}
	float64 read_float64_z()
	{
		float64 f;
		read_bytes_z(&f, 8);
		return f;
	}
	float128 read_float128_z()
	{
		float128 f;
		read_bytes_z(&f, 16);
		return f;
	}

	uint32 write_int16_z(int16 n);
	uint32 write_uint16_z(uint16 n) { return write_int16_z(int16(n)); }
	uint32 write_int24_z(int32 n);
	uint32 write_uint24_z(uint32 n) { return write_int24_z(int32(n)); }
	uint32 write_int32_z(int32 n);
	uint32 write_uint32_z(uint32 n) { return write_int32_z(int32(n)); }
	uint32 write_int64_z(int64 n);
	uint32 write_uint64_z(uint64 n) { return write_int64_z(int64(n)); }


	// read / write dynamically sizes integers
	// intended for numbers which are expected but not guaranteed to be small
	// int12 / uint12 are written as 2 bytes max.
	// int16 / uint16 are written as 3 bytes max.
	// int32 / uint32 are written as 5 bytes max.

	uint32 write_vuint32(uint32);
	uint32 write_vint32(int32);
	uint32 read_vuint32();
	int32  read_vint32();


	// TTY

	int get_terminal_size(int& rows, int& cols) noexcept;
	int set_terminal_size(int rows, int cols) noexcept; // does not work !?
	int terminal_rows() noexcept;
	int terminal_cols() noexcept;
	int terminal_width() noexcept;	// pixel
	int terminal_height() noexcept; // pixel


	// deprecated:
	uint32 read_int16_data_x(int16* p, uint32) __attribute__((deprecated));			 // use read_data_x(p,n)
	uint32 read_int16_data_z(int16* p, uint32) __attribute__((deprecated));			 // use read_data_z(p,n)
	uint32 write_int16_data_x(const int16* p, uint32) __attribute__((deprecated));	 // use write_data_x(p,n)
	uint32 write_int16_data_z(const int16* p, uint32) __attribute__((deprecated));	 // use write_data_z(p,n)
	uint32 write_uint16_data_x(const uint16* p, uint32) __attribute__((deprecated)); // use write_data_x(p,n)
	uint32 write_uint16_data_z(const uint16* p, uint32) __attribute__((deprecated)); // use write_data_z(p,n)
};
