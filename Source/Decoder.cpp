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

	mModelUrl = Params[PopMondegreen_OptionKey_ModelUrl].GetString();
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

void Decoder_t::PushData(AudioDataView_t<int16_t> AudioData16)
{
	std::vector<float> DataFloats;
	AudioData16.ConvertSamples(DataFloats);
	
	AudioDataView_t<float> AudioDataFloat;
	AudioDataFloat.mSamples = DataFloats;
	AudioDataFloat.mSamplesPerSecond = AudioData16.mSamplesPerSecond;
	AudioDataFloat.mChannelCount = AudioData16.mChannelCount;
	AudioDataFloat.mTime = AudioData16.mTime;

	PushData( AudioDataFloat );
}

void Decoder_t::PushData(AudioDataView_t<float> AudioDataFloat)
{
	std::vector<int16_t> Data16s;
	AudioDataFloat.ConvertSamples(Data16s);
	
	AudioDataView_t<int16_t> AudioData16;
	AudioData16.mSamples = Data16s;
	AudioData16.mSamplesPerSecond = AudioDataFloat.mSamplesPerSecond;
	AudioData16.mChannelCount = AudioDataFloat.mChannelCount;
	AudioData16.mTime = AudioDataFloat.mTime;
	
	PushData( AudioData16 );}
