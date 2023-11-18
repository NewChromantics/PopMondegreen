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



TEST(PopMondegreen, CreateInstance)
{
	auto* Params = "{\"Name\":\"Fake\"}";
	std::array<char,1000> ErrorBuffer;
	auto Instance = PopMondegreen_CreateInstance(Params, ErrorBuffer.data(), ErrorBuffer.size() );
	PopMondegreen_FreeInstance( Instance );
	
	std::string Error(ErrorBuffer.data());
	
	EXPECT_EQ( Error.empty(), true ) << "Create instance error " << Error;
	
}


TEST(PopMondegreen, CreateWhisperInstance)
{
	auto* Params = "{\"Name\":\"Whisper\"}";
	std::array<char,1000> ErrorBuffer;
	auto Instance = PopMondegreen_CreateInstance(Params, ErrorBuffer.data(), ErrorBuffer.size() );
	
	//	push data in
	//	make sure data comes out
	
	PopMondegreen_FreeInstance( Instance );
	
	std::string Error(ErrorBuffer.data());
	
	EXPECT_EQ( Error.empty(), true ) << "Create instance error " << Error;
	
}

