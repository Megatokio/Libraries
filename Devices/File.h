// Copyright (c) 2023 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "SerialDevice.h"


namespace kio
{

/* Interface class `File`

	EOF handling in read():
	Since File is a subclass of SerialDevice a function may expect a serial device which provides data
	on an irregular basis, most notably reading less or no data does not indicate a problem.
	On the other hand reading less or no data from a file surely indicates there is no more data coming.
	=>	read() with partial == false always throws.
		read() with partial == true throws END_OF_FILE if called again after the previous call returnd 0 bytes.
		getc(timeout), if reimplemented, must behave accordingly.
		A subclass may manage this using the protected functions File::eof_pending() etc..
		setFpos() must clear eof_pending.
*/
class File : public SerialDevice
{
protected:
	File(Flags flags) noexcept : SerialDevice(flags) {}
	File(FileOpenMode mode) noexcept : SerialDevice(flags_for_mode(mode)) {}

public:
	// noexcept: subclasses must catch all exceptions in the d'tor.
	virtual ~File() noexcept override = default;

	virtual uint32 ioctl(IoCtl cmd, void* arg1 = nullptr, void* arg2 = nullptr) override;

	// virtual SIZE read(char* data, SIZE, bool partial = false);
	// virtual SIZE write(const char* data, SIZE, bool partial = false);

	virtual int	 getc(uint timeout_us) override;
	virtual char getc() override;
	// virtual str	gets();
	// virtual void putc(char);
	// virtual void puts(cstr);
	// virtual void printf(cstr fmt, ...) __printflike(2, 3);

	// close()    close the file even if a file error occurs.
	// set_fpos() may set fpos beyond file end if this is possible.

	virtual ADDR getSize() const noexcept = 0;
	virtual ADDR getFpos() const noexcept = 0;
	virtual void setFpos(ADDR)			  = 0;
	virtual void close()				  = 0;
	virtual void truncate() { throw "truncate() not supported"; }

	void skipBytes(off_t n);
	bool is_at_eof() const noexcept { return getFpos() >= getSize(); }

protected:
	bool eof_pending() const noexcept { return flags & EOF_PENDING; }
	void set_eof_pending() noexcept { flags = flags | EOF_PENDING; }
	void clear_eof_pending() noexcept { flags = flags & ~EOF_PENDING; }

private:
	static Flags flags_for_mode(FileOpenMode mode) noexcept;
};


} // namespace kio


/*






























*/
