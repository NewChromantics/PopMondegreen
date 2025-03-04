#pragma once

#include <chrono>
#include <span>
#include "AudioDataView.hpp"


namespace PopMondegreen
{
	class EventTime_t;


	void					ReadFile(std::string_view Filename,std::function<void(std::span<uint8_t>,bool Eof)> OnChunk);
	std::vector<uint8_t>	ReadFile(std::string_view Filename);
}

class PopMondegreen::EventTime_t
{
public:
	typedef std::chrono::time_point<std::chrono::steady_clock> Base;

	static EventTime_t		Invalid()	{	return EventTime_t( Base::min() );	}
	static EventTime_t		Now()		{	return EventTime_t( Base::clock::now() );	}
	
	EventTime_t() :
		mTimePoint	( Base::min() )
	{
	}
	EventTime_t(Base TimePoint) :
		mTimePoint	( TimePoint )
	{
	}
	
	inline bool	operator==(const EventTime_t& that) const
	{
		return this->mTimePoint == that.mTimePoint;
	}
	
private:
	Base		mTimePoint;
};


//	c++ interface to c funcs
void	PopMondegreen_PushData(int32_t Instance,AudioDataView_t<int16_t>& Data);
void	PopMondegreen_PushData(int32_t Instance,AudioDataView_t<float>& Data);
