#include <iostream>
#include <sstream>
#include "PopMondegreen.h"

#include "gtest/gtest.h"
#if defined(TARGET_OSX)
#if GTEST_HAS_FILE_SYSTEM
#error This build will error in sandbox mac apps
#endif
#endif

#include "PopJson/PopJson.hpp"
#include "Decoder.hpp"
#include "WaveDecoder.hpp"
#include <thread>

void PopMondegreen_Run(PopJson::ViewBase_t& Options)
{
	auto Listener = Options.GetValue("Listener").GetString();
	auto Decoder = Options.GetValue("Decoder").GetString();
	
}


void PopMondegreen_Run(std::string_view SourceName,std::string_view DecoderName)
{
	//auto Listener
	
}


int main()
{
	std::cout << "PopMondegreen verison " << PopMondegreen_GetVersionThousand() << std::endl;

	// Must be called prior to running any tests
	testing::InitGoogleTest();
	
	std::string_view GTestFilter = "";
	//std::string_view GTestFilter = "**DecodeFile**";
	if ( !GTestFilter.empty() )
	{
		using namespace testing;
		GTEST_FLAG(filter) = std::string(GTestFilter);
	}
	
	static bool BreakOnTestError = false;
	if ( BreakOnTestError )
		GTEST_FLAG_SET(break_on_failure,true);
	
	const auto ReturnCode = RUN_ALL_TESTS();
	
	if ( ReturnCode != 0 )
		return ReturnCode;
	
	std::cerr << "Integration tests succeeded!" << std::endl;
	return 0;
}


class PopH264_General_Tests : public testing::TestWithParam<bool>
{
protected:
};


TEST(PopMondegreen, GetVersion )
{
	auto VersionThousand = PopMondegreen_GetVersionThousand();

	auto VersionMajor = VersionThousand / 1000 / 1000;
	auto VersionMinor = (VersionThousand / 1000) % 1000;
	auto VersionPatch = VersionThousand % 1000;

	std::cout << "PopMondegreen verison " << VersionMajor << "." << VersionMinor << "." << VersionPatch << std::endl;
	EXPECT_EQ( VersionMajor, VERSION_MAJOR );
	EXPECT_EQ( VersionMinor, VERSION_MINOR );
	EXPECT_EQ( VersionPatch, VERSION_PATCH );
	EXPECT_LT( VersionMajor, 1000 );
	EXPECT_LT( VersionMinor, 1000 );
	EXPECT_LT( VersionPatch, 1000 );
}



TEST(PopMondegreen, CreateDefaultInstance)
{
	auto* Params = "{}";
	std::array<char,1000> ErrorBuffer;
	auto Instance = PopMondegreen_CreateInstance(Params, ErrorBuffer.data(), ErrorBuffer.size() );
	PopMondegreen_FreeInstance( Instance );
	
	std::string Error(ErrorBuffer.data());
	
	EXPECT_EQ( Error.empty(), true ) << "Create instance error " << Error;
	
}

TEST(PopMondegreen, CreateFakeInstance)
{
	auto* Params = "{\"Name\":\"Fake\"}";
	std::array<char,1000> ErrorBuffer;
	auto Instance = PopMondegreen_CreateInstance(Params, ErrorBuffer.data(), ErrorBuffer.size() );
	
	PopMondegreen_PushData( Instance, 1000, nullptr, 0, 0, 0, nullptr );
	
	//	this should keep popping data
	for ( int it=0;	it<20;	it++ )
	{
		std::array<char,1000> OutputJsonBuffer;
		PopMondegreen_PopData( Instance, OutputJsonBuffer.data(), OutputJsonBuffer.size() );
		std::string_view OutputJson( OutputJsonBuffer.data(), std::strlen(OutputJsonBuffer.data()) );
		PopJson::Json_t Output(OutputJson);
		std::cerr << "Output json; " << OutputJson << std::endl;
		std::this_thread::sleep_for( std::chrono::milliseconds(500) );
	}
	
	PopMondegreen_PushEndOfStream( Instance );
	PopMondegreen_FreeInstance( Instance );
	
	std::string Error(ErrorBuffer.data());
	
	EXPECT_EQ( Error.empty(), true ) << "Create instance error " << Error;
	
}



TEST(PopMondegreen, CreateWhisperInstance)
{
	//	create audio decoder
	auto* DecoderParams = "{\"Name\":\"Whisper\"}";
	std::array<char,1000> ErrorBuffer;
	auto Decoder = PopMondegreen_CreateInstance( DecoderParams, ErrorBuffer.data(), ErrorBuffer.size() );
	{
		std::string Error(ErrorBuffer.data());
		EXPECT_EQ( Error.empty(), true ) << "Create instance error " << Error;
	}
	
	auto OnWaveData = [&](AudioDataView_t Data,bool Eof)
	{
		PopMondegreen_PushData( Decoder, Data );
		if ( Eof )
			PopMondegreen_PushEndOfStream(Decoder);
	};

	//	create wav decoder
	WaveDecoder_t WaveDecoder(OnWaveData);
	
	auto OnFileData = [&](std::span<uint8_t> Chunk,bool Eof)
	{
		WaveDecoder.PushData( Chunk );
		if ( Eof )
			WaveDecoder.PushEndOfData();
	};
	
	//	load wav file
	{
		auto Wav = PopMondegreen::ReadFile("test:LanaLovesTheLlama.wav");
		OnFileData( Wav, true );
	}
	
	//	read from decoder
	std::string DecoderError;
	while ( DecoderError.empty() )
	{
		std::vector<char> JsonBuffer;
		JsonBuffer.resize( 1024 * 1024 * 1 );
		PopMondegreen_PopData( Decoder, JsonBuffer.data(), JsonBuffer.size() );
		std::string_view Json( JsonBuffer.data(), std::strlen(JsonBuffer.data()) );
		
		PopJson::Json_t Data(Json);
		auto Error = Data.GetValue("Error").GetString();
		if ( !Error.empty() )
			DecoderError = Error;
		
		throw std::runtime_error( std::string("handle json; ") + std::string(Json) );
	}
	
	PopMondegreen_FreeInstance( Decoder );
	
}

