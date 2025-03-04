#include "WaveDecoder.hpp"


WaveDecoder_t::WaveDecoder_t(std::function<void(AudioDataView_t<float>,bool Eof)> OnData) :
	mOnDataFloat	( OnData )
{
}

WaveDecoder_t::WaveDecoder_t(std::function<void(AudioDataView_t<int16_t>,bool Eof)> OnData) :
	mOnData16	( OnData )
{
}

void WaveDecoder_t::PushData(std::span<uint8_t> FileData)
{
}

void WaveDecoder_t::PushEndOfData()
{
}

void WaveDecoder_t::OnOutputData(AudioDataView_t<float> Data)
{
	if ( mOnDataFloat )
	{
		mOnDataFloat( Data, false );
	}
	
	if ( mOnData16 )
	{
		//	convert to 16
		throw std::runtime_error("todo: convert to 16 bit");
	}
}

void WaveDecoder_t::OnOutputData(AudioDataView_t<int16_t> Data)
{
	if ( mOnDataFloat )
	{
		throw std::runtime_error("todo: convert to float");
	}
	
	if ( mOnData16 )
	{
		mOnData16( Data, false );
	}
}

void WaveDecoder_t::OnOutputEof()
{
	if ( mOnData16 )
		mOnData16( {}, true );
	
	if ( mOnDataFloat )
		mOnDataFloat( {}, true );
}
