#pragma once

#include <chrono>
#include <span>


class AudioDataView_t;

namespace PopMondegreen
{
	typedef std::chrono::time_point<std::chrono::steady_clock> EventTime_t;


	void					ReadFile(std::string_view Filename,std::function<void(std::span<uint8_t>,bool Eof)> OnChunk);
	std::vector<uint8_t>	ReadFile(std::string_view Filename);
}


//	c++ interface to c funcs
void	PopMondegreen_PushData(int32_t Instance,AudioDataView_t& Data);
