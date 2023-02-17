// Copyright (c) 2020 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once

enum CpuID {
	CpuDefault,	   // Z80, not actively selected
	CpuZ80,		   // Z80 without IXCB illegal opcodes
	CpuZ80_ixcbr2, // Z80 with "bit 0,(ix+dis),r" option
	CpuZ80_ixcbxh, // Z80 with "bit 0,xh" option
	Cpu8080,
	CpuZ180
};
