#include "Decoder.hpp"
#include <mutex>
#include "PopMondegreen.h"
#include <iostream>

DecoderParams_t::DecoderParams_t(PopJson::Json_t& Params)
{
	mApiKey = Params[PopMondegreen_OptionKey_ApiKey].GetString();
	mApiRegion = Params[PopMondegreen_OptionKey_ApiRegion].GetString();
	
	if ( Params.HasKey(PopMondegreen_OptionKey_UseApiMicrophone) )
		mUseApiMicrophone = Params[PopMondegreen_OptionKey_UseApiMicrophone].GetBool();
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

	std::cerr << "Got output @" << Data.mStartTime.mMilliseconds << "ms; " << Data.mData << std::endl;
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

void Decoder_t::PushData(AudioDataView_t<int16_t> Data)
{
	throw std::runtime_error("todo: convert audio data to float");
}

void Decoder_t::PushData(AudioDataView_t<float> Data)
{
	throw std::runtime_error("todo: convert audio data to 16bit");
}
