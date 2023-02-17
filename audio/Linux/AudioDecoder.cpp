// Copyright (c) 2023 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "../AudioDecoder.h"

// implementation of class AudioDecoder for Linux
// stubs only
// TODO


AudioDecoder::AudioDecoder() :
	filename(nullptr),
	num_frames(0),
	frames_per_second(0),
	num_channels(0),
	sizeof_sample(0),
	frame_position(0)
{
	debugstr("AudioDecoder\n");
}


AudioDecoder::~AudioDecoder()
{
	debugstr("~AudioDecoder");
	close();
}


/*	close audio file.
	the decoder may be reused.
*/
int AudioDecoder::close()
{
	delete[] filename;
	filename = nullptr;

	// TODO

	return ok;
}


/*	open audio file for reading
	extracts various metrics
	e.g. you can then call numChannels(), numSamples(), samplesPerSecond(), sizeofSample()
*/
void AudioDecoder::open(cstr filename)
{
	(void)filename;
	debugstr("AudioDecoder::open\n");
	TODO();
}


/*	set file position of audio file to desired sample position
 */
void AudioDecoder::seekSamplePosition(uint32 frameposition)
{
	(void)frameposition;
	debugstr("AudioDecoder::seekSamplePosition\n");
	TODO();
}


/*	read buffer from file
	updates client data format if it has changed or is not yet set
	reads num_frames of num_channels of sizeof_sample into buffer bu[]
	note: if the file has more channels than requested, then the additional channels are simply ignored
		  if the file has less channels than requested, then the missing channels are cleared with silence
	returns number of frames actually read
*/
uint32 AudioDecoder::read_buffer(void* bu, uint numchannels, uint32 numframes, uint sizeofsample)
{
	(void)bu;
	(void)numchannels;
	(void)numframes;
	(void)sizeofsample;
	TODO();
}


/*	read samples from file into buffer
		num_channels may be 1 or 2 (mono or stereo)
		max_frames = buffer size = num_samples * num_channels
	if num_channels does not match, then channels are silently split or joined.
	If sample size is not yet set (_outputformat.mBitsPerChannel==0) or a previous read()
	was for a different sample size, then the output format is silently updated.
	returns number of frames actually read, which may be less than requested at file's end.
*/
uint32 AudioDecoder::read(int16* bu, uint32 maxframes, uint numchannels)
{
	(void)bu;
	(void)maxframes;
	(void)numchannels;
	debugstr("AudioDecoder::read\n");
	assert(numchannels >= 1 && numchannels <= 7);
	TODO();
}


/*	read samples from file into buffer
		num_channels may be 1 or 2 (mono or stereo)
		max_frames = buffer size = num_samples * num_channels
	if num_channels does not match, then channels are silently split or joined.
	If sample size is not yet set (_outputformat.mBitsPerChannel==0) or a previous read()
	was for a different sample size, then the output format is silently updated.
	returns number of frames actually read, which may be less than requested at file's end.
*/
uint32 AudioDecoder::read(float32* bu, uint32 maxframes, uint numchannels)
{
	(void)bu;
	(void)maxframes;
	(void)numchannels;
	debugstr("AudioDecoder::read\n");
	assert(numchannels >= 1 && numchannels <= 7);
	TODO();
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

	static cstr list[] = {nullptr}; // {"mp3", "m4a", "mp2", "aiff", "wav", nullptr};
	return list;
}
