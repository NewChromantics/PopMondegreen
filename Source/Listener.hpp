#pragma once


#include "PopMondegreen.hpp"
#include <string>
#include "PopJson/PopJson.hpp"



class AudioDataView_t
{
public:
	int					mSampleRate = 0;
	int					mChannelCount = 0;
	std::span<float>	mSamples;
};

class AudioData_t
{
public:
	int					mSampleRate = 0;
	int					mChannelCount = 0;
	std::vector<float>	mSamples;
};



class ListenerParams_t
{
public:
	ListenerParams_t(PopJson::Json_t& Params);
	
	std::string		mModelUrl;
};

class TimestampedText_t
{
public:
	PopMondegreen::EventTime_t	mTime;
	std::string					mText;
};

class ListenerOutputMeta_t
{
public:
	PopMondegreen::EventTime_t	mNowTime;
	
	//	output here a load of strings with timestamps for
	//	essentially subtitles
	std::vector<TimestampedText_t>	mTexts;
};




class Listener_t
{
public:
	Listener_t(ListenerParams_t Params);
	
	virtual std::string	GetName()=0;
	
	virtual void		PushData(AudioDataView_t Data)=0;
	
protected:
	ListenerParams_t	mParams;
};
