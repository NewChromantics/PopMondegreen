#include "FakeDecoder.hpp"


FakeDecoder_t::FakeDecoder_t(DecoderParams_t Params) :
	Decoder_t		( Params )
{
}

FakeDecoder_t::~FakeDecoder_t()
{
	Stop();
	if ( mOutputGeneratorThread.joinable() )
	{
		mOutputGeneratorThread.join();
		mOutputGeneratorThread = {};
	}
}
	

void FakeDecoder_t::PushAudioData(AudioDataView_t<int16_t> Data)
{
	//	output dummy data to say stuff has arrived
	Start( Data.mTime );
}

void FakeDecoder_t::PushEndOfStream()
{
	Stop();
}

void FakeDecoder_t::Stop()
{
	mGeneratingOutput = false;
}


void FakeDecoder_t::Start(Timecode_t FirstTimecode)
{
	//	already running
	if ( mOutputGeneratorThread.joinable() )
		return;
	
	mGeneratingOutput = true;
	mFirstTimecode = FirstTimecode;
	
	//	start thread
	mOutputGeneratorThread = std::thread( [this](){	OutputGeneratorThread();	} );
}

void FakeDecoder_t::OutputGeneratorThread()
{
	try
	{
		while ( mGeneratingOutput )
		{
			OutputGeneratorIteration();
			std::this_thread::sleep_for( std::chrono::milliseconds(500) );
		}
	}
	catch(std::exception& e)
	{
		OnError( e.what() );
	}
}



void FakeDecoder_t::OutputGeneratorIteration()
{
	if ( !mFirstTimecode.IsValid() )
		return;
	
	using namespace std::string_view_literals;
	constexpr std::array Words = {"I"sv,"Love"sv,"Rock"sv,"and"sv,"Roll"sv};
	
	auto NewWord = Words[ mOutputCounter % Words.size() ];
	mOutputCounter++;
	
	OutputData_t Output;
	Output.mData = NewWord;
	Output.mStartTime = mFirstTimecode.mMilliseconds + (mOutputCounter * 500 );
	Output.mEndTime = Output.mStartTime.mMilliseconds + 500;
	
	OnOutputData( Output );
}
