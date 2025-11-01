// Copyright (c) 2025 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "SerialDevice.h"

namespace kio
{

// class to decode utf-8 input and encode to utf-8 output.
// The 'local' character encoding is 'ucs1' (1 byte per char)

class Utf8Decoder : public SerialDevice
{
public:
	Utf8Decoder(SerialDevicePtr dest);
	virtual uint32 ioctl(IoCtl cmd, void* arg1 = nullptr, void* arg2 = nullptr) override;
	virtual SIZE   read(void* data, SIZE, bool partial = false) override;
	virtual SIZE   write(const void* data, SIZE, bool partial = false) override;

private:
	uint			pending_inbytes = 0;
	char			inbytes[6];
	char			pending_outbyte = 0;
	char			_padding[1];
	SerialDevicePtr dest;
};

} // namespace kio
