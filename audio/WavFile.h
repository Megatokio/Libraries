#pragma once
// Copyright (c) 2018 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"
#include "unix/FD.h"


class WavFile
{
public:
	FD	   fd;
	uint32 current_frame; // current index in frame data associated with fd.fpos

	// header data:
	// all values are written in little-endian (Intel) byte order (LSB first)
	//uint32 riff_tag;				// 'RIFF'
	//uint32 riff_size;				// overall size of file in bytes

	//uint32 wave_tag;				// "WAVE"
	//uint32 fmt_tag;  				// "fmt\0" with trailing null char
	//uint32 fmt_size;				// length of the format data: 16

	uint16 sample_format;	  // 1: PCM, 3: IEEE float, 6: 8bit alaw, 7: 8bit ulaw
	uint16 num_channels;	  // number of channels = samples per frame: 1, 2
	uint32 frames_per_second; // samples rate
	uint32 bytes_per_second;  // bytes_per_frame * frames_per_second
	uint16 bytes_per_frame;	  // ceil(num_channels * bits_per_sample / 8)
	uint16 bits_per_sample;	  // bits per sample: 8, 16, 32

	//uint32 data_tag;				// 'DATA'
	uint32 data_size; // samples_per_file * bytes_per_sample: size of audio data

	// derived data:
	double total_playtime; // seconds in file
	uint32 total_frames;   // frames = samples * num_channels in file
	off_t  data_start;	   // file position
	bool   is_valid;


	enum // SampleFormat
	{
		PCM		  = 1,
		IEEEFLOAT = 3,
		ALAW	  = 6, // 8-bit ITU-T G.711 A-law
		ULAW	  = 7  // 8-bit ITU-T G.711 µ-law
	};

	enum FrameFormat {
		DEFAULT = 0, // MONO or STEREO depending on num_channels
		MONO	= 1,
		STEREO	= 2,
		LEFTCHANNEL,
		RIGHTCHANNEL,
	};

private:
	void read_header() throws;

	// read and convert to desired sample format:
	void read_samples(Array<int8>& dest, uint32 cnt) throws;
	void read_samples(Array<int16>& dest, uint32 cnt) throws;
	void read_samples(Array<int32>& dest, uint32 cnt) throws;
	void read_samples(Array<float32>& dest, uint32 cnt) throws;

public:
	WavFile(cstr filepath) noexcept;

	void seekFramePosition(uint32 idx);

	// read cnt frames
	// returns MONO or STEREO
	template<typename T>
	FrameFormat readFrames(Array<T>& dest, uint32 cnt, FrameFormat fmt = DEFAULT) throws;

	cstr tostr() const;
};


template<typename T>
WavFile::FrameFormat WavFile::readFrames(Array<T>& dest, uint32 cnt, FrameFormat fmt) throws
{
	assert(is_valid);
	assert(bits_per_sample == 8 || bits_per_sample == 16 || bits_per_sample == 32);
	assert(num_channels == 1 || num_channels == 2);

	if (fmt == DEFAULT) fmt = FrameFormat(num_channels);
	cnt = min(cnt, total_frames - current_frame);

	if (num_channels == 1)
	{
		if (fmt == STEREO)
		{
			Array<T> zbu;
			read_samples(zbu, cnt);
			mono_to_stereo(zbu, dest);
			return STEREO;
		}
		else
		{
			read_samples(dest, cnt);
			return MONO;
		}
	}
	else // 2 channels
	{
		if (fmt == STEREO)
		{
			read_samples(dest, cnt * 2);
			return STEREO;
		}
		else
		{
			Array<T> zbu;
			read_samples(zbu, cnt * 2);

			if (fmt == LEFTCHANNEL) left_channel(zbu, dest);
			else if (fmt == RIGHTCHANNEL) right_channel(zbu, dest);
			else stereo_to_mono(zbu, dest);
			return MONO;
		}
	}
}
