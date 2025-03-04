#include "Microphone_Apple.hpp"
#import "AVFAudio/AVFAudio.h"
#include <iostream>
#include <span>
#include <sstream>
#include <TargetConditionals.h>	//	#if TARGET_OS_IPHONE etc




AudioSource_t::AudioSource_t(std::function<void(AudioDataView_t<float>)> OnSamples) :
	mOnSamples	( OnSamples )
{
}

void AudioSource_t::OnSamples(AudioDataView_t<float> Data)
{
	mOnSamples( Data );
}




class AvfAudioSource
{
public:
	AvfAudioSource(std::function<void(AudioDataView_t<float>)> OnSamples);
	
	AVAudioEngine*	mAudioEngine = nullptr;
	bool			mWaitingForPermission = false;
	bool			mGotPermission = false;
	std::string		mOnError;
};



AvfAudioSource::AvfAudioSource(std::function<void(AudioDataView_t<float>)> OnSamples)
{
	//AVAudioFormat* format = [[AVAudioFormat alloc] init
	AVAudioChannelCount Channels = 1;
	double SampleRate = 44000;
	auto* format = [[AVAudioFormat alloc] initStandardFormatWithSampleRate:SampleRate channels:Channels];
	
	AVAudioEngine* engine = [[AVAudioEngine alloc] init];
	AVAudioNodeBus AudioBus = 0;
	AVAudioFrameCount BufferSize = SampleRate;
	
	auto OnSample = ^(AVAudioPCMBuffer * _Nonnull buffer, AVAudioTime * _Nonnull when)
	{
		if ( !buffer )
		{
			std::cerr << "Got null PCM buffer" << std::endl;
			return;
		}
		
		auto FrameCount = buffer.frameLength;
		std::cerr << "Got PCM buffer FrameCount=" << FrameCount << std::endl;
		
		AudioDataView_t<float> Data;
		Data.mSamplesPerSecond = buffer.format.sampleRate * 1000.0;
		Data.mChannelCount = buffer.format.channelCount;
		
		auto SampleCount = buffer.frameLength * buffer.format.channelCount;
		auto ChannelsSampleData = buffer.floatChannelData;
		auto* Channel0Data = ChannelsSampleData[0];
		Data.mSamples = std::span( Channel0Data, SampleCount );
		
		OnSamples( Data );
	};
	[engine.inputNode installTapOnBus:AudioBus bufferSize:BufferSize format:format block:OnSample];

	auto OnPermission = ^()
	{
		mGotPermission = true;
		std::cerr << "Got Permissions response" << std::endl;
		
		NSError* StartupError = nil;
		auto Result = [engine startAndReturnError:&StartupError];
		if ( !Result || StartupError )
		{
			std::stringstream Error;
			Error << "AudioEngine Start failed; ";
			if ( StartupError )
				Error << [StartupError localizedDescription];
			else
				Error << "Unknown error";
			throw std::runtime_error(Error.str());
		}
	};
	
	
	//	macos just starts.
	//	ios requires permission
#if TARGET_OS_IPHONE
	

	auto SharedInstance = [AVAudioSession sharedInsance];
	[SharedInstance setCategory(record)];
	mWaitingForPermission = true;
	[SharedInstance requestRecordPermission(OnPermission)];
	
	
#elif TARGET_OS_MAC
	
	OnPermission();
	
#else
#error unknown target
#endif

}



Microphone_t::Microphone_t(std::function<void(AudioDataView_t<float>)> OnSamples) :
	AudioSource_t	( OnSamples )
{
	auto OnAvfSamples = [this](AudioDataView_t<float> Audio)
	{
		this->OnSamples( Audio );
	};
	mAvfSource.reset( new AvfAudioSource(OnAvfSamples) );
}


