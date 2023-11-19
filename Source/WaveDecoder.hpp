#pragma once

#include "Decoder.hpp"


class WaveDecoder_t
{
public:
	WaveDecoder_t(std::function<void(AudioDataView_t,bool Eof)> OnData);
	
	void				PushData(std::span<uint8_t> FileData);
	void				PushEndOfData();
	
protected:
	void				OnOutputData(AudioDataView_t Data);
	void				OnOutputEof();
	
	std::function<void(AudioDataView_t,bool Eof)>	mOnData;
};
