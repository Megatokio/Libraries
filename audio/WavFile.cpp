// Copyright (c) 2018 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#define LOGLEVEL 1
#define SAFETY	 1
#include "WavFile.h"
#include "Templates/Array.h"
#include "audio/convert_audio.h"
#include "kio/kio.h"
#include "unix/FD.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


WavFile::WavFile(cstr filepath) noexcept
{
	is_valid = no;
	try
	{
		fd.open_file_r(filepath);
		read_header();
		xlogline("new WavFile:\n%s", tostr());
	}
	catch (AnyError& e)
	{
		xlogline("new WavFile(\"%s\"): %s", fd.filepath(), e.what());
	}
}

cstr WavFile::tostr() const
{
	uint sf	   = sample_format;
	cstr sfstr = sf == PCM		 ? "PCM" :
				 sf == IEEEFLOAT ? "IEEE FLOAT" :
				 sf == ALAW		 ? "A-LAW" :
				 sf == ULAW		 ? "µLAW" :
								   usingstr("UNKNOWN (%u)", sf);

	return usingstr(
		"format type:       %s\n"
		"num. channels:     %u\n"
		"sample rate:       %u/sec\n"
		"byte rate:         %u/sec\n"
		"frame size         %u bytes\n"
		"bits per sample:   %u\n"
		"total data size:   %u bytes\n"
		"total frames:      %u\n"
		"total playtime:    %.3f sec",
		sfstr, num_channels, frames_per_second, bytes_per_second, bytes_per_frame, bits_per_sample, data_size,
		total_frames, total_playtime);
}

void WavFile::read_header() throws
{
	is_valid = no;

	// read RIFF and WAVE header
	{
		uint32 riff_tag	 = fd.read_uint32_x();
		uint32 riff_size = fd.read_uint32_z();
		(void)riff_size;
		uint32 wave_tag = fd.read_uint32_x();

		if (riff_tag != 'RIFF') throw DataError("not a RIFF file");
		if (wave_tag != 'WAVE') throw DataError("not a WAVE file");
	}

	// read FMT chunk:
	{
		uint32 fmt_tag	= fd.read_uint32_x();
		uint32 fmt_size = fd.read_uint32_z();

		if (fmt_tag != 'fmt ') throw DataError("FMT chunk not found");
		if (fmt_size < 16) throw DataError("FMT chunk broken");

		sample_format	  = fd.read_uint16_z();
		num_channels	  = fd.read_uint16_z();
		frames_per_second = fd.read_uint32_z();
		bytes_per_second  = fd.read_uint32_z();
		bytes_per_frame	  = fd.read_uint16_z();
		bits_per_sample	  = fd.read_uint16_z();
		if (fmt_size > 16)
		{
			uint32 n = fmt_size - 16;
			n += n & 1;
			fd.skip_bytes(n); // PCM=16, IEEE=18, other longer
		}

		uint z = sample_format;
		if (z != PCM && z != ALAW && z != ULAW && z != IEEEFLOAT) throw DataError("unknown sample format: %u", z);

		z = bits_per_sample;
		if (z != 8 && z != 16 && z != 32) throw DataError("unsupported sample size: %u bits", z);

		if (num_channels != 1 && num_channels != 2) throw DataError("unsupported number of channels: %u", num_channels);

		if (bytes_per_frame * frames_per_second != bytes_per_second)
			throw DataError(
				"inconsistent values for bytes/frame, frames/second and bytes/second"
				" (%u,%u,%u)",
				bytes_per_frame, frames_per_second, bytes_per_second);

		z = (num_channels * bits_per_sample + 7) / 8;
		if (z != bytes_per_frame)
			throw DataError(
				"inconsistent values for num_channels, bits/sample and bytes/frame"
				" (%u,%u,%u)",
				num_channels, bits_per_sample, bytes_per_frame);
	}

	// read DATA header
	{
		uint32 data_tag = fd.read_uint32_x();

		while (data_tag != 'data')
		{
			uint32 n = fd.read_uint32_z();
			n += n & 1;
			if (fd.file_remaining() < n + 8) throw DataError("DATA chunk not found");
			fd.skip_bytes(n);
			data_tag = fd.read_uint32_x();
		}

		data_size  = fd.read_uint32_z();
		data_start = fd.file_position();

		if (data_size / bytes_per_frame * bytes_per_frame != data_size)
			throw DataError("total data size is not an integral multiple of bytes/frame");

		if (fd.file_remaining() < data_size) throw DataError("DATA truncated");
	}

	total_playtime = double(data_size) / bytes_per_second;
	total_frames   = data_size / bytes_per_frame;

	current_frame = 0;
	is_valid	  = yes;
}

void WavFile::read_samples(Array<int8>& z, uint32 cnt) throws
{
	if (z.count() != cnt)
	{
		z.purge();
		z.grow(cnt);
	}

	if (sample_format == PCM && bits_per_sample == 8) // unsigned bytes
	{
		Array<uint8> q(cnt);
		fd.read_data(q.getData(), cnt);
		convert_audio(q, z);
	}
	else if (sample_format == PCM && bits_per_sample == 16) // signed
	{
		Array<int16> q(cnt);
		fd.read_data_z(q.getData(), cnt);
		convert_audio(q, z);
	}
	else if (sample_format == PCM && bits_per_sample == 32) // signed
	{
		Array<int32> q(cnt);
		fd.read_data_z(q.getData(), cnt);
		convert_audio(q, z);
	}
	else // float, A-Law, µLaw
	{
		Array<int16> q(cnt);
		read_samples(q, cnt);
		convert_audio(q, z);
	}
}

void WavFile::read_samples(Array<int16>& z, uint32 cnt) throws
{
	if (z.count() != cnt)
	{
		z.purge();
		z.grow(cnt);
	}

	if (sample_format == PCM && bits_per_sample == 8)
	{
		Array<uint8> q(cnt);
		fd.read_data(q.getData(), cnt);
		convert_audio(q, z);
	}
	else if (sample_format == PCM && bits_per_sample == 16) { fd.read_data_z(z.getData(), cnt); }
	else if (sample_format == PCM && bits_per_sample == 32)
	{
		Array<int32> q(cnt);
		fd.read_data_z(q.getData(), cnt);
		convert_audio(q, z);
	}
	else if (sample_format == IEEEFLOAT && bits_per_sample == 32)
	{
		Array<float32> q(cnt);
		fd.read_data_z(q.getData(), cnt);
		convert_audio(q, z);
	}
	else if (sample_format == ALAW)
	{
		assert(bits_per_sample == 1);
		Array<int8> q(cnt);
		fd.read_data(q.getData(), cnt);
		convert_ALaw(q, z);
	}
	else if (sample_format == ULAW)
	{
		assert(bits_per_sample == 1);
		Array<int8> q(cnt);
		fd.read_data(q.getData(), cnt);
		convert_uLaw(q, z);
	}
	else IERR();
}

void WavFile::read_samples(Array<int32>& z, uint32 cnt) throws
{
	if (z.count() != cnt)
	{
		z.purge();
		z.grow(cnt);
	}

	if (sample_format == PCM && bits_per_sample == 32) { fd.read_data_z(z.getData(), cnt); }
	else if (sample_format == IEEEFLOAT && bits_per_sample == 32)
	{
		Array<float32> q(cnt);
		fd.read_data_z(q.getData(), cnt);
		convert_audio(q, z);
	}
	else
	{
		Array<int16> q(cnt);
		read_samples(q, cnt);
		convert_audio(q, z);
	}
}

void WavFile::read_samples(Array<float32>& z, uint32 cnt) throws
{
	if (z.count() != cnt)
	{
		z.purge();
		z.grow(cnt);
	}

	if (sample_format == PCM && bits_per_sample == 32)
	{
		Array<int32> q(cnt);
		fd.read_data_z(q.getData(), cnt);
		convert_audio(q, z);
	}
	else if (sample_format == IEEEFLOAT && bits_per_sample == 32) { fd.read_data_z(z.getData(), cnt); }
	else
	{
		Array<int16> q(cnt);
		read_samples(q, cnt);
		convert_audio(q, z);
	}
}

void WavFile::seekFramePosition(uint32 idx) throws
{
	idx = min(idx, total_frames);

	fd.seek_fpos(data_start + idx * bytes_per_frame);
	current_frame = idx;
}
