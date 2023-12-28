#pragma once

#include <stdint.h>
#include <limits>

class Timecode_t
{
private:
	const static uint32_t	InvalidMilliseconds = std::numeric_limits<uint32_t>::max();
	
public:
	static Timecode_t	Invalid()	{	return Timecode_t();	}
	
public:
	Timecode_t(uint32_t Milliseconds=InvalidMilliseconds) :
		mMilliseconds	( Milliseconds )
	{
	}
	
	bool			IsValid()	{	return mMilliseconds != InvalidMilliseconds;	}
	
	uint32_t		mMilliseconds = InvalidMilliseconds;
};

