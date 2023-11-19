#include "WaveDecoder.hpp"


WaveDecoder_t::WaveDecoder_t(std::function<void(AudioDataView_t,bool Eof)> OnData) :
	mOnData	( OnData )
{
}

void WaveDecoder_t::PushData(std::span<uint8_t> FileData)
{
}

void WaveDecoder_t::PushEndOfData()
{
}

void WaveDecoder_t::OnOutputData(AudioDataView_t Data)
{
	mOnData( Data, false );
}

void WaveDecoder_t::OnOutputEof()
{
	mOnData( {}, true );
}
