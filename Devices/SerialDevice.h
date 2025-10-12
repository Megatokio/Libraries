// Copyright (c) 2023 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "devices_types.h"

namespace kio
{

constexpr uint gets_max_len = 512; // max string length handled by gets()


/* Interface class `SerialDevice`
*/
class SerialDevice
{
public:
	uint16 rc		 = 0;
	char   last_char = 0; // used by gets()
	Flags  flags;

public:
	SerialDevice(Flags flags) noexcept : flags(flags) {}
	virtual ~SerialDevice() noexcept = default;
	NO_COPY_MOVE(SerialDevice);

	// sequential read/write
	//   partial=true:  transfer as much as possible without blocking. possibly none.
	//   partial=false: transfer all data or throw. possibly blocking.
	//
	// text i/o:
	//   blocking. default implementations use read() and write().

	virtual uint32 ioctl(IoCtl cmd, void* arg1 = nullptr, void* arg2 = nullptr);
	virtual SIZE   read(void* data, SIZE, bool partial = false);
	virtual SIZE   write(const void* data, SIZE, bool partial = false);

	virtual int	 getc(uint timeout_us);
	virtual char getc();
	virtual str	 gets(uint line_ends = (1 << 0) + (1 << 10) + (1 << 13));
	virtual void putc(char);
	virtual void puts(cstr);
	virtual void printf(cstr fmt, ...) __printflike(2, 3);

	void flushOut() { ioctl(IoCtl::FLUSH_OUT); }
	bool is_readable() const noexcept { return flags & readable; }
	bool is_writable() const noexcept { return flags & writable; }

	// ------- Convenience Methods ---------------

	template<typename T>
	T read()
	{
		T n;
		read(&n, sizeof(T));
		return n;
	}

	template<typename T>
	void read(T& n)
	{
		read(&n, sizeof(T));
	}

	template<typename T>
	void write(const T& n)
	{
		write(&n, sizeof(T));
	}

#if __cplusplus >= 201700
  #define _constexpr constexpr
#else
  #define _constexpr
#endif

	template<typename T, ByteOrder LE>
	static T reverted(T n)
	{
		static_assert(sizeof(T) <= 8, "");
		if _constexpr (LE == native_byteorder) return n;
		char* p = reinterpret_cast<char*>(&n);
		if _constexpr (sizeof(T) >= 2) std::swap(p[0], p[sizeof(T) - 1]);
		if _constexpr (sizeof(T) >= 4) std::swap(p[1], p[sizeof(T) - 2]);
		if _constexpr (sizeof(T) >= 8) std::swap(p[2], p[sizeof(T) - 3]);
		if _constexpr (sizeof(T) >= 8) std::swap(p[3], p[sizeof(T) - 4]);
		return n;
	}

	template<typename T>
	T read_BE()
	{
		return reverted<T, big_endian>(read<T>());
	}

	template<typename T>
	void read_BE(T& n)
	{
		n = read_BE<T>();
	}

	template<typename T>
	void write_BE(const T& n)
	{
		write<T>(reverted<T, big_endian>(n));
	}

	template<typename T>
	T read_LE()
	{
		return reverted<T, little_endian>(read<T>());
	}

	template<typename T>
	void read_LE(T& n)
	{
		n = read_LE<T>(n);
	}

	template<typename T>
	void write_LE(const T& n)
	{
		write<T>(reverted<T, little_endian>(n));
	}
};


inline SIZE SerialDevice::read(void*, SIZE, bool) { throw NOT_READABLE; }
inline SIZE SerialDevice::write(const void*, SIZE, bool) { throw NOT_WRITABLE; }

} // namespace kio
