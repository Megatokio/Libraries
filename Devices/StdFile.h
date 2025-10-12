// Copyright (c) 2024 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "File.h"

namespace kio
{

class StdFile : public File
{
	NO_COPY_MOVE(StdFile);

public:
	StdFile(cstr fpath, FileOpenMode = READ);
	StdFile(FILE*, Flags); // intended for stdin|out|err: file will not be closed in dtor

	virtual ~StdFile() override;
	virtual uint32 ioctl(IoCtl cmd, void* arg1 = nullptr, void* arg2 = nullptr) override;

	virtual SIZE read(void* data, SIZE, bool partial = false) override;
	virtual SIZE write(const void* data, SIZE, bool partial = false) override;

	using File::read;
	using File::write;

	virtual int	 getc(uint timeout_us) override;
	virtual char getc() override;
	// virtual str	gets();
	virtual void putc(char) override;
	// virtual SIZE puts(cstr)override;
	// virtual SIZE printf(cstr fmt, ...) __printflike(2, 3);

	virtual ADDR getSize() const noexcept override;
	virtual ADDR getFpos() const noexcept override;
	virtual void setFpos(ADDR) override;
	virtual void close() override;
	virtual void truncate() override { throw "truncate() not supported"; }

private:
	FILE* file;
	void  throw_error();
};


} // namespace kio
