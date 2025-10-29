// Copyright (c) 2023 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "SerialDevice.h"

namespace kio
{

/*
	SerialDevice which uses stdin and stdout
	and expects it to be a user console (a terminal)
*/

using ConsolePtr = std::shared_ptr<class Console>;

class Console final : public SerialDevice
{
public:
	Console();
	~Console() override;

	virtual uint32 ioctl(IoCtl ctl, void* arg1 = nullptr, void* arg2 = nullptr) override;
	virtual SIZE   read(void* data, SIZE, bool partial = false) override;
	virtual SIZE   write(const void* data, SIZE, bool partial = false) override;

	virtual int	 getc(uint timeout_us) override;
	virtual char getc() override;
	//virtual str  gets(uint lineends = 1 << '\n') override;
	//virtual void putc(char) override;
	//virtual void puts(cstr) override;
	//virtual void printf(cstr fmt, ...) override __printflike(2, 3);
};


extern cstr lineInput(SerialDevice*, cstr prompt, cstr old_text = "", int epos = 0);


} // namespace kio
