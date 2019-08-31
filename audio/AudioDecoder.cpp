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

	note:
		There is a bug in ExtFileAudio in OSX 10.4 (and maybe later)
		see: http://cocoadev.com/ExtAudioFile
		This results in a little offset of the sample seek position in ExtAudioFileSeek() for compressed files
		because the frames added for better decompression are not skipped.
		This offset is tiny, hopefully fixed today and will only add some silent samples at the beginning of the file.
		This is no longer fixed. I just call seek(0) in open() to avoid a mismatch after a later seek().
*/

#include "AudioDecoder.h"



AudioDecoder::AudioDecoder()
:
	filename(NULL),
	num_frames(0),
	frames_per_second(0),
	num_channels(0),
	sizeof_sample(0),
	audiofile(NULL),
	frame_position(0)
{
	bzero(&client_format,sizeof(client_format));
}


AudioDecoder::~AudioDecoder()
{
	close();
}


/*	close audio file.
	the decoder may be reused.
*/
OSStatus AudioDecoder::close()
{
	delete[] filename;
	filename = NULL;

	OSStatus e = audiofile ? ExtAudioFileDispose(audiofile) : ok;
	audiofile = NULL;

	return e;
}


/*	open audio file for reading
	extracts various metrics
	e.g. you can then call numChannels(), numSamples(), samplesPerSecond(), sizeofSample()
*/
void AudioDecoder::open(cstr filename) noexcept(false)
{
	xlogIn("AudioDecoder::open(%s)", filename);

	// close existing file if open
    OSStatus err = close(); (void)err;

    // open the audio file
    CFStringRef urlStr = CFStringCreateWithCString(kCFAllocatorDefault, filename, kCFStringEncodingUTF8);
    CFURLRef urlRef = CFURLCreateWithFileSystemPath(NULL, urlStr, kCFURLPOSIXPathStyle, false);
    err = ExtAudioFileOpenURL(urlRef, &audiofile);
    CFRelease(urlStr);
    CFRelease(urlRef);
	if(err) { audiofile=NULL; throw any_error("AudioDecoder: Error opening file."); }
	this->filename = newcopy(filename);
	frame_position = 0;
	client_format.mBitsPerChannel = 0;

	// get the input file format
	uint32 size = sizeof(file_format);
	err = ExtAudioFileGetProperty(audiofile, kExtAudioFileProperty_FileDataFormat, &size, &file_format);
	if(err) throw any_error("AudioDecoder: Error getting file format.");

	// get the total number of frames of the audio file
	// a frame is the set of samples with 1 sample per channel
	size = sizeof(num_frames);
	err	= ExtAudioFileGetProperty(audiofile, kExtAudioFileProperty_FileLengthFrames, &size, &num_frames);
	if(err) throw any_error("AudioDecoder: Error getting number of frames.");

	num_channels		= file_format.mChannelsPerFrame;
	frames_per_second	= file_format.mSampleRate;
	sizeof_sample		= file_format.mBytesPerFrame / num_channels;
}


/*	set file position of audio file to desired sample position
*/
void AudioDecoder::seekSamplePosition(uint32 frameposition) noexcept(false)
{
    OSStatus err = ExtAudioFileSeek(audiofile, frameposition);
	if(err) throw any_error("AudioDecoder: seek(%u) failed: %i",uint(frameposition), int(err));
    frame_position = frameposition;
}


/*	read buffer from file
	updates client data format if it has changed or is not yet set
	reads num_frames of num_channels of sizeof_sample into buffer bu[]
	note: if the file has more channels than requested, then the additional channels are simply ignored
		  if the file has less channels than requested, then the missing channels are cleared with silence
	returns number of frames actually read
*/
//private
uint32 AudioDecoder::read_buffer(void* bu, uint numchannels, uint32 numframes, uint sizeofsample) noexcept(false)
{
	if(client_format.mBitsPerChannel!=sizeofsample<<3 || client_format.mChannelsPerFrame!=numchannels)
	{
		// setup output format:
		bzero(&client_format, sizeof(AudioStreamBasicDescription));
		client_format.mSampleRate		= file_format.mSampleRate;
		client_format.mFormatID			= kAudioFormatLinearPCM;
		client_format.mFormatFlags		= sizeofsample==sizeof(float32) ?
					kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked | kAudioFormatFlagIsFloat :
					kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked | kAudioFormatFlagIsSignedInteger;
		client_format.mFramesPerPacket	= 1;
		client_format.mChannelsPerFrame = numchannels;
		client_format.mBitsPerChannel	= sizeofsample * 8;
		client_format.mBytesPerFrame	= sizeofsample * numchannels;
		client_format.mBytesPerPacket	= sizeofsample * numchannels;

		// set output format on audio file:
		uint32 size  = sizeof(client_format);
		OSStatus err = ExtAudioFileSetProperty(audiofile, kExtAudioFileProperty_ClientDataFormat, size, &client_format);
		if(err) throw any_error("AudioDecoder: Error when setting ClientDataFormat: %i",int(err));

		seekSamplePosition(frame_position);	// security
	}

	AudioBufferList	audiobufferlist;
	audiobufferlist.mNumberBuffers = 1;
	audiobufferlist.mBuffers[0].mNumberChannels = numchannels;
	audiobufferlist.mBuffers[0].mDataByteSize = numframes * numchannels * sizeofsample;
	audiobufferlist.mBuffers[0].mData = bu;
	OSStatus err = ExtAudioFileRead (audiofile, &numframes, &audiobufferlist);	// reads & sets num_frames
	if(err) throw any_error("AudioDecoder: failed to read from file");
	return numframes;
}


/*	read samples from file into buffer
		num_channels may be 1 or 2 (mono or stereo)
		max_frames = buffer size = num_samples * num_channels
	if num_channels does not match, then channels are silently split or joined.
	If sample size is not yet set (_outputformat.mBitsPerChannel==0) or a previous read()
	was for a different sample size, then the output format is silently updated.
	returns number of frames actually read, which may be less than requested at file's end.
*/
uint32 AudioDecoder::read(int16* bu, uint32 maxframes, uint numchannels) noexcept(false)
{
	xlogIn("AudioDecoder::read(int16,%u,%u)", uint(maxframes),numchannels);
	assert(numchannels>=1 && numchannels<=7);

	typedef int16 sample;
	uint32 count,n;

	if(numchannels==1 && num_channels>1)	// stereo|surround -> mono
	{
		sample* z = bu;						// destination pointer for channel conversion
		sample  qbu[1024*2];				// 1024 frames * 2 channels

		for(count=0; count<maxframes; count+=n)		// frames read
		{
			n = read_buffer(qbu, 2, min(maxframes-count,1024u), sizeof(sample));
			if(n==0) break;								// end of file

			for(sample *q=qbu, *e=q+n*2; q<e; q+=2)
			{
				*z++ = sample((*q + *(q+1)) / 2);
			}
		}
	}
	else					// requested and file's num channels match; or
	  						// stereo|surround to stereo|surround: default behaviour is ok; or
	{						// mono to stereo|surround: manually copy left to right channel
		for(count=0; count<maxframes; count+=n)		// frames read
		{
			n = read_buffer(bu+count*numchannels, numchannels, maxframes-count, sizeof(sample));
			if(n==0) break;								// end of file

			if(num_channels==1 && numchannels>1)		// mono -> stereo|surround: only channel[0] was set
			{											// => copy left channel[0] to right channel[1]
				for(sample *q=bu+count*numchannels, *e=q+n*numchannels; q<e; q+=numchannels)
				{
					*(q+1) = *q; q += numchannels;
				}
			}
		}
	}

	frame_position += count;
	return count;
}


/*	read samples from file into buffer
		num_channels may be 1 or 2 (mono or stereo)
		max_frames = buffer size = num_samples * num_channels
	if num_channels does not match, then channels are silently split or joined.
	If sample size is not yet set (_outputformat.mBitsPerChannel==0) or a previous read()
	was for a different sample size, then the output format is silently updated.
	returns number of frames actually read, which may be less than requested at file's end.
*/
uint32 AudioDecoder::read(float32* bu, uint32 maxframes, uint numchannels) noexcept(false)
{
	xlogIn("AudioDecoder::read(float32,%u,%u)", uint(maxframes),numchannels);
	assert(numchannels>=1 && numchannels<=7);

	typedef float32 sample;
	uint32 count,n;

	if(numchannels==1 && num_channels>1)	// stereo|surround -> mono
	{
		sample* z = bu;						// destination pointer for channel conversion
		sample  qbu[1024*2];				// 1024 frames * 2 channels

		for(count=0; count<maxframes; count+=n)			// frames read
		{
			n = read_buffer(qbu, 2, min(maxframes-count,1024u), sizeof(sample));
			if(n==0) break;								// end of file

			for(sample *q=qbu,*e=q+n*2; q<e; q+=2)
			{
				*z++ = (*q + *(q+1)) / 2;
			}
		}
	}
	else					// requested and file's num channels match; or
	  						// stereo|surround to stereo|surround: default behaviour is ok; or
	{						// mono to stereo|surround: manually copy left to right channel
		for(count=0; count<maxframes; count+=n)			// frames read
		{
			n = read_buffer(bu+count*numchannels, numchannels, maxframes-count, sizeof(sample));
			if(n==0) break;								// end of file

			if(num_channels==1 && numchannels>1)		// mono -> stereo|surround: only channel[0] was set
			{											// => copy left channel[0] to right channel[1]
				for(sample *q=bu+count*numchannels, *e=q+n*numchannels; q<e; q+=numchannels)
				{
					*(q+1) = *q; q += numchannels;
				}
			}
		}
	}

	frame_position += count;
	return count;
}


// static
cstr* AudioDecoder::supportedFileExtensions()
{
//  http://developer.apple.com/library/mac/documentation/MusicAudio/Reference/AudioFileConvertRef/Reference/reference.html#//apple_ref/doc/c_ref/AudioFileTypeID
//	kAudioFileAIFFType	= 'AIFF',
//	kAudioFileAIFCType	= 'AIFC',
//	kAudioFileWAVEType	= 'WAVE',
//	kAudioFileSoundDesigner2Type = 'Sd2f',
//	kAudioFileNextType	= 'NeXT',
//	kAudioFileMP3Type	= 'MPG3',
//	kAudioFileMP2Type	= 'MPG2',
//	kAudioFileMP1Type	= 'MPG1',
//	kAudioFileAC3Type	= 'ac-3',
//	kAudioFileAAC_ADTSType = 'adts',
//	kAudioFileMPEG4Type = 'mp4f',
//	kAudioFileM4AType	= 'm4af',
//	kAudioFileCAFType	= 'caff',
//	kAudioFile3GPType	= '3gpp',
//	kAudioFile3GP2Type	= '3gp2',
//	kAudioFileAMRType	= 'amrf'

	static cstr list[] = {"mp3","m4a","mp2","aiff","wav",NULL};
	return list;
}






