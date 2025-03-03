#include "PopMondegreen.h"
#include "InstanceManager.hpp"
#include "Decoder.hpp"
#include <iostream>
#include "PopJson/PopJson.hpp"

#include "../Data/Wave/lana_loves_the_llama.h"
#include "Json11/json11.hpp"


#include "FakeDecoder.hpp"
//#include "WhisperDecoder.hpp"
#include "MicrosoftCognitiveDecoder.hpp"



namespace Soy
{
	void	StringToBuffer(const char* Source,char* Buffer,size_t BufferSize);
}


namespace PopMondegreen
{
	InstanceManager_t<Decoder_t>					DecoderInstanceManager;
	decltype(DecoderInstanceManager)::Instance_t	AllocDecoder(PopJson::Json_t& Params);

	constexpr int	VersionMajor = VERSION_MAJOR;
	constexpr int	VersionMinor = VERSION_MINOR;
	constexpr int	VersionPatch = VERSION_PATCH;
}


__export int32_t PopMondegreen_GetVersionThousand()
{
	int32_t Version = 0;
	Version += PopMondegreen::VersionMajor;
	Version *= 1000;

	Version += PopMondegreen::VersionMinor;
	Version *= 1000;

	Version += PopMondegreen::VersionPatch;

	return Version;
}


__export int32_t PopMondegreen_CreateInstance(const char* _OptionsJson, char* ErrorBuffer, int32_t ErrorBufferSize)
{
	try
	{
		if ( !_OptionsJson )
			_OptionsJson = "{}";
		std::string_view OptionsJson( _OptionsJson, std::strlen(_OptionsJson) );
		PopJson::Json_t Options(OptionsJson);
		auto Instance = PopMondegreen::AllocDecoder(Options);

		return Instance;
	}
	catch (std::exception& e)
	{
		Soy::StringToBuffer( e.what(), ErrorBuffer, ErrorBufferSize );
		return PopMondegreen_NullInstance;
	}
}

__export void PopMondegreen_FreeInstance(int32_t Instance)
{
	try
	{
		PopMondegreen::DecoderInstanceManager.Free(Instance);
	}
	catch (std::exception& e)
	{
		std::cerr << __FUNCTION__ << " exception; " << e.what() << std::endl;
	}
}


void Soy::StringToBuffer(const char* Source,char* Buffer,size_t BufferSize)
{
	//	copy into nothing, is safe
	if ( Buffer == nullptr )
		return;
	if ( BufferSize == 0 )
		return;
	
	int Len = 0;
	for ( Len=0;	Source && Len<BufferSize-1;	Len++ )
	{
		if ( Source[Len] == '\0' )
			break;
		Buffer[Len] = Source[Len];
	}
	Buffer[std::min<ssize_t>(Len,BufferSize-1)] = '\0';
}

decltype(PopMondegreen::DecoderInstanceManager)::Instance_t PopMondegreen::AllocDecoder(PopJson::Json_t& Params)
{
	auto DecoderName = Params.GetValue("Name").GetString();
	
	DecoderParams_t DecoderParams(Params);
	
	if ( DecoderName == FakeDecoder_t::Name )
	{
		return PopMondegreen::DecoderInstanceManager.Alloc<FakeDecoder_t>(DecoderParams);
	}
	
	if ( DecoderName == MicrosoftCogninitiveDecoder_t::Name )
	{
		return PopMondegreen::DecoderInstanceManager.Alloc<MicrosoftCogninitiveDecoder_t>(DecoderParams);
	}
	/*
	if ( DecoderName == WhisperDecoder_t::Name )
	{
		return PopMondegreen::DecoderInstanceManager.Alloc<WhisperDecoder_t>(DecoderParams);
	}*/
	
	throw std::runtime_error("Don't know how to make Decoder");
}


void PopMondegreen::ReadFile(std::string_view Filename,std::function<void(std::span<uint8_t>,bool Eof)> OnChunk)
{
	if ( Filename == "test:LanaLovesTheLlama.wav" )
	{
		std::span File( LanaLovesTheLama );
		OnChunk(File,true);
		return;
	}
	
	throw std::runtime_error("todo ReadFile");
}

std::vector<uint8_t> PopMondegreen::ReadFile(std::string_view Filename)
{
	std::vector<uint8_t> FileContents;
	__volatile bool GotEof = false;
	auto OnChunk = [&](std::span<uint8_t> Chunk,bool Eof)
	{
		std::copy( Chunk.begin(), Chunk.end(), std::back_inserter(FileContents) );
		GotEof = Eof;
	};
	
	while ( !GotEof )
	{
		ReadFile( Filename, OnChunk );
	}
	return FileContents;
}

void PopMondegreen_PushData(int32_t Instance,AudioDataView_t& Data)
{
	try
	{
		auto pInstance = PopMondegreen::DecoderInstanceManager.GetInstance(Instance);
		pInstance->PushData(Data);
	}
	catch(std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}


__export void PopMondegreen_PushData(int32_t Instance,uint32_t TimestampMs,float* SampleData,int SampleCount,int ChannelCount,int SampleHz,const char* SampleMeta)
{
	try
	{
		if ( SampleData == nullptr && SampleCount != 0 )
			throw std::runtime_error("Null sample data, but count != 0");
		
		//	nothing to do
		//	gr: we allow pushing nothing for fake, which just needs a timecode
		//if ( SampleCount == 0 )
		//	return;
		
		AudioDataView_t DataView;
		DataView.mChannelCount = ChannelCount;
		DataView.mSampleRate = SampleHz;
		DataView.mTime = Timecode_t(TimestampMs);
		DataView.mSamples = std::span( SampleData, SampleCount );
		PopMondegreen_PushData( Instance, DataView );
	}
	catch(std::exception& e)
	{
		std::cerr << __FUNCTION__ << " exception; " << e.what() << std::endl;
	}
}

__export void PopMondegreen_PushEndOfStream(int32_t Instance)
{
	try
	{
		auto pInstance = PopMondegreen::DecoderInstanceManager.GetInstance(Instance);
		pInstance->PushEndOfStream();
	}
	catch(std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}

json11::Json::object OutputDataToJson(OutputData_t& OutputData)
{
	json11::Json::object Output;
	
	Output["StartTime"] = static_cast<int>(OutputData.mStartTime.mMilliseconds);
	Output["EndTime"] = static_cast<int>(OutputData.mEndTime.mMilliseconds);
	Output["Data"] = OutputData.mData;
	
	return Output;
}


__export void PopMondegreen_PopData(int32_t Instance,char* JsonBuffer,int JsonBufferSize)
{
	auto WriteJson = [&](json11::Json::object& Object)
	{
		if ( !JsonBuffer || JsonBufferSize==0 )
			return;

		auto Json = json11::Json(Object).dump();
		Soy::StringToBuffer( Json.c_str(), JsonBuffer, JsonBufferSize );
	};

	try
	{
		auto pInstance = PopMondegreen::DecoderInstanceManager.GetInstance(Instance);

		auto Output = pInstance->PopData();
		auto OutputJson = OutputDataToJson(Output);
		WriteJson( OutputJson );
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		json11::Json::object Output;
		Output["Error"] = std::string(e.what());
		WriteJson(Output);
	}
}


