#pragma once


#include "PopMondegreen.hpp"
#include <string>
#include "PopJson/PopJson.hpp"



class ListenerParams_t
{
public:
	ListenerParams_t(PopJson::Json_t& Params);
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
	
protected:
	ListenerParams_t	mParams;
};
