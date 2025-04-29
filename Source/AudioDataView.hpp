#pragma once

#include <span>
#include "Timecode.hpp"


template<typename TYPE>
class AudioDataView_t;


//	in WaveDecoder.cpp
AudioDataView_t<float>		Audio16ToFloat(AudioDataView_t<int16_t>& Audio,std::vector<float>& Storage);
AudioDataView_t<int16_t>	AudioFloatTo16(AudioDataView_t<float>& Audio,std::vector<int16_t>& Storage);
AudioDataView_t<float>		AudioFloatResample(AudioDataView_t<float>& Audio,std::vector<float>& Storage);



template<typename TYPE>
class AudioDataView_t
{
public:
	template<typename TARGETTYPE>
	AudioDataView_t<TARGETTYPE>		ConvertSamples(std::vector<TARGETTYPE>& Storage);
	
	AudioDataView_t<TYPE>			Resample(int NewSamplesPerSecond,std::vector<TYPE>& Storage);
	
public:
	std::span<TYPE>		mSamples;

	//	we need this meta for audio source output
	int					mSamplesPerSecond = 0;	//	eg 44100 
	int					mChannelCount = 0; 
	
	//	whilst not part of audio data... it goes along with it!
	Timecode_t			mTime;
};


template<>
template<>
inline AudioDataView_t<float> AudioDataView_t<int16_t>::ConvertSamples(std::vector<float>& Storage)
{
	return Audio16ToFloat( *this, Storage );
}


template<>
template<>
inline AudioDataView_t<int16_t> AudioDataView_t<float>::ConvertSamples(std::vector<int16_t>& Storage)
{
	return AudioFloatTo16( *this, Storage );
}

template<>
inline AudioDataView_t<float> AudioDataView_t<float>::Resample(int NewSamplesPerSecond,std::vector<float>& Storage)
{
	return AudioFloatResample( *this, Storage );
}
