#include "Decoder.hpp"
#include <mutex>

DecoderParams_t::DecoderParams_t(PopJson::Json_t& Params)
{
}

Decoder_t::Decoder_t(DecoderParams_t Params) :
	mParams		( Params )
{
}


void Decoder_t::OnError(std::string_view Error)
{
	OutputData_t Output;
	Output.mError = Error;
	OnOutputData(Output);
}


void Decoder_t::OnOutputData(OutputData_t Data)
{
	if ( Data.mOutputTime == PopMondegreen::EventTime_t::Invalid() )
	{
		Data.mOutputTime = PopMondegreen::EventTime_t::Now();
	}

	std::scoped_lock Lock(mOutputLock);
	mOutputs.push_back( Data );
}


OutputData_t Decoder_t::PopData()
{
	std::scoped_lock Lock(mOutputLock);
	if ( mOutputs.empty() )
		return {};
	
	auto First = *mOutputs.begin();
	mOutputs.erase( mOutputs.begin() );
	return First;
}

