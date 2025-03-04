#pragma once

#include <functional>
#include "Decoder.hpp"


class AudioSource_t
{
public:
	AudioSource_t(std::function<void(AudioDataView_t<float>)> OnSamples);
	
protected:
	void		OnSamples(AudioDataView_t<float> Data);
	
private:
	std::function<void(AudioDataView_t<float>)>	mOnSamples;
};


class AvfAudioSource;


class Microphone_t : public AudioSource_t
{
public:
	static constexpr auto	Name = "Microphone";
public:
	Microphone_t(std::function<void(AudioDataView_t<float>)> OnSamples);
		
protected:
	std::shared_ptr<AvfAudioSource>	mAvfSource;
};
