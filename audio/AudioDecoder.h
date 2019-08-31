#pragma once
/*	Copyright  (c)	Günter Woigk 2015 - 2019
					mailto:kio@little-bat.de

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Permission to use, copy, modify, distribute, and sell this software and
	its documentation for any purpose is hereby granted without fee, provided
	that the above copyright notice appear in all copies and that both that
	copyright notice and this permission notice appear in supporting
	documentation, and that the name of the copyright holder not be used
	in advertising or publicity pertaining to distribution of the software
	without specific, written prior permission.  The copyright holder makes no
	representations about the suitability of this software for any purpose.
	It is provided "as is" without express or implied warranty.

	THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
	EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
	PERFORMANCE OF THIS SOFTWARE.

	initially based on
		libaudiodecoder - Native Portable Audio Decoder Library
		libaudiodecoder API Header File
		http://www.oscillicious.com/libaudiodecoder
		Copyright (c) 2010-2012 Albert Santoni, Bill Good, RJ Ryan
 */

#include "kio/kio.h"
//#include "Templates/MTObject.h"
#include "Templates/RCObject.h"

#include <AudioToolbox/AudioToolbox.h>
#include "CAStreamBasicDescription.h"

#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
#include <CoreServices/CoreServices.h>
#include <CoreAudio/CoreAudioTypes.h>
#include <AudioToolbox/AudioFile.h>
#include <AudioToolbox/AudioFormat.h>
#else
#include "CoreAudioTypes.h"
#include "AudioFile.h"
#include "AudioFormat.h"
#endif







class AudioDecoder : public RCObject
{
	cstr	filename;
	uint64	num_frames;				// total number of frames (~samples) in file
	float64	frames_per_second;		// sample rate in file
	uint	num_channels;			// number of channels in file
	uint	sizeof_sample;			// sample size in file (2=int16, 4=float32)

    ExtAudioFileRef	audiofile;		// OSX handle for audio file
    CAStreamBasicDescription file_format;	// file format data
    CAStreamBasicDescription client_format;	// client format data

	uint32	frame_position;			// current play position in file [samples]

	uint32	read_buffer(void* bu, uint num_channels, uint32 num_frames, uint sizeof_sample) noexcept(false);

public:		AudioDecoder();
			~AudioDecoder();


	// Open file for decoding
	// if there is already a file open, then it will be closed first.
	void open(cstr filename) noexcept(false);

	// close file. you may reuse the decoder.
	OSStatus close();

	// Read a maximum of max_frames of audio into a buffer.
	//	Samples are returned as 32-bit floats, with stereo interlacing if num_channels=2.
	//	Returns the number of samples read, 0 at end of file.
	uint32 read(float32*, uint32 max_frames, uint num_channels) noexcept(false);

	// Read a maximum of max_frames of audio into a buffer.
	//	Samples are returned as 16-bit signed integers, with stereo interlacing if num_channels=2.
	//	Returns the number of samples read, 0 at end of file.
	uint32 read(int16*, uint32 max_frames, uint num_channels) noexcept(false);

	// Seek to a sample in the file
	void seekSamplePosition(uint32 frame_position) noexcept(false);

	// Get the current playback position in samples
	uint32	getSamplePosition()		const { return frame_position; }

	// Get the number of sample frames in the file. This will be a good estimate of the
	//	number of samples you can get out of read(), though you should not rely on it
	//	being perfectly accurate always. e.g. it might be slightly inaccurate with VBR MP3s
	// better read() until read() returns 0
	uint64	numSamples()	        const { return num_frames; }

	// get size of samples in the audio file
	uint	sizeofSample()			const { return sizeof_sample; }

	// Get number of channels in the audio file
	uint	numChannels()			const { return num_channels; }

	// Get sample rate of the audio file
	float64	samplesPerSecond()		const { return frames_per_second; }

	// Get the play time of the audio file [seconds]
	float64	playtime()				const { return num_frames / frames_per_second; }

	// get filename:
	cstr	getFilename()			const { return filename; }

	// Get a NULL-terminated list of the filetypes supported by the decoder, by extension
	static cstr* supportedFileExtensions();
};

















