#pragma once

#include <span>
#include "Timecode.hpp"


template<typename TYPE>
class AudioDataView_t;


//	in WaveDecoder.cpp
void Audio16ToFloat(AudioDataView_t<int16_t>& Audio,std::vector<float>& Storage);
void AudioFloatTo16(AudioDataView_t<float>& Audio,std::vector<int16_t>& Storage);
void AudioFloatResample(AudioDataView_t<float>& Audio,std::vector<float>& Storage);



template<typename TYPE>
class AudioDataView_t
{
public:
	template<typename TARGETTYPE>
	void				ConvertSamples(std::vector<TARGETTYPE>& Storage);
	
	void				Resample(int NewSamplesPerSecond,std::vector<TYPE>& Storage);
	
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
inline void AudioDataView_t<int16_t>::ConvertSamples(std::vector<float>& Storage)
{
	Audio16ToFloat( *this, Storage );
}


template<>
template<>
inline void AudioDataView_t<float>::ConvertSamples(std::vector<int16_t>& Storage)
{
	AudioFloatTo16( *this, Storage );
}

template<>
inline void AudioDataView_t<float>::Resample(int NewSamplesPerSecond,std::vector<float>& Storage)
{
	AudioFloatResample( *this, Storage );
}
