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
	try
	{
		PushAudioData(AudioData16);
		return;
	}
	catch(UnsupportedAudioFormatException& e)
	{
		//	fall through to below
	}
	catch(std::runtime_error& e)
	{
		OnError(e.what());
		throw;
	}
	catch(...)
	{
		OnError("Unknown exception in PushData<16>");
		throw;
	}
	
	try
	{
		std::vector<float> DataFloats;
		auto AudioDataFloat = AudioData16.ConvertSamples(DataFloats);
		PushAudioData( AudioDataFloat );
		return;
	}
	catch(std::runtime_error& e)
	{
		OnError(e.what());
		throw;
	}
	catch(...)
	{
		OnError("Unknown exception in PushData<16>");
		throw;
	}
}

void Decoder_t::PushData(AudioDataView_t<float> AudioDataFloat)
{
	try
	{
		PushAudioData(AudioDataFloat);
		return;
	}
	catch(UnsupportedAudioFormatException& e)
	{
		//	fall through to below
	}
	catch(std::runtime_error& e)
	{
		OnError(e.what());
		throw;
	}
	catch(...)
	{
		OnError("Unknown exception in PushData<float>");
		throw;
	}
	
	try
	{
		std::vector<int16_t> Data16s;
		auto AudioData16 = AudioDataFloat.ConvertSamples(Data16s);
		PushAudioData( AudioData16 );
		return;
	}
	catch(std::runtime_error& e)
	{
		OnError(e.what());
		throw;
	}
	catch(...)
	{
		OnError("Unknown exception in PushData<16>");
		throw;
	}
}
