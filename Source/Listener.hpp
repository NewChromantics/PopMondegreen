#pragma once


#include "PopMondegreen.hpp"
#include <string>

class ListenerParams_t
{
public:
	ListenerParams_t(std::string_view Json);
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
	
	
	ListenerParams_t	mParams;
};
