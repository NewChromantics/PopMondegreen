#pragma once

#include <span>
#include "Timecode.hpp"



template<typename TYPE>
class AudioDataView_t
{
public:
	std::span<TYPE>		mSamples;

	//	we need this meta for audio source output
	int					mSamplesPerSecond = 0;	//	eg 44100 
	int					mChannelCount = 0; 
	
	//	whilst not part of audio data... it goes along with it!
	Timecode_t			mTime;
};
