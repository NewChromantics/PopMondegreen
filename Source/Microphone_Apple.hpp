#pragma once

#include <functional>
#include "Listener.hpp"


class AudioSource_t
{
public:
	AudioSource_t(std::function<void(AudioDataView_t)> OnSamples);
	
protected:
	void		OnSamples(AudioDataView_t Data);
	
private:
	std::function<void(AudioDataView_t)>	mOnSamples;
};


class AvfAudioSource;


class Microphone_t : public AudioSource_t
{
public:
	static constexpr auto	Name = "Microphone";
public:
	Microphone_t(std::function<void(AudioDataView_t)> OnSamples);
		
protected:
	std::shared_ptr<AvfAudioSource>	mAvfSource;
};
