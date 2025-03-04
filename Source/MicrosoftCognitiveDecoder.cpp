#include "MicrosoftCognitiveDecoder.hpp"
#include <MicrosoftCognitiveServicesSpeech/speechapi_cxx.h>
#include <future>

namespace mscog = Microsoft::CognitiveServices::Speech;


Timecode_t Nano100sToMilliseconds(uint64_t Nanos100)
{
	//	1ms = 1000000 nanos
	auto Nano100ToMs = (1000000/100);
	auto Millisecs = Nanos100 / Nano100ToMs;
	return Timecode_t(Millisecs);
}


class PcmInputStream : public Microsoft::CognitiveServices::Speech::Audio::AudioInputStream
{
};


//	https://github.com/Azure-Samples/cognitive-services-speech-sdk/blob/master/quickstart/cpp/macos/from-microphone/helloworld.cpp
MicrosoftCogninitiveDecoder_t::MicrosoftCogninitiveDecoder_t(DecoderParams_t Params) :
	Decoder_t(Params)
{
	//mscog::setlocale(LC_ALL, "");
	
	// Creates an instance of a speech config with specified subscription key and service region.
	// Replace with your own subscription key and service region (e.g., "westus").
	mSpeechConfig = mscog::SpeechConfig::FromSubscription( Params.mApiKey, Params.mApiRegion );
	
	//	can we verify this config?
	
	//	cannot create recogniser until we know the format
	if ( Params.mUseApiMicrophone )
	{
		CreateRecogniserWithMicrophone();
	}
}
	

MicrosoftCogninitiveDecoder_t::~MicrosoftCogninitiveDecoder_t()
{
	//	got to wait for work to finish
	StopRecogniser();
}


void MicrosoftCogninitiveDecoder_t::CreateRecogniser(std::shared_ptr<mscog::Audio::AudioConfig> AudioInputConfig)
{
	if ( mRecogniser )
		throw std::runtime_error("Already have recogniser");
	
	mRecogniser = mscog::SpeechRecognizer::FromConfig(mSpeechConfig, AudioInputConfig);
	
	mRecogniseFuture = mRecogniser->StartContinuousRecognitionAsync();
	/*
	 // Performs recognition. RecognizeOnceAsync() returns when the first utterance has been recognized,
	 // so it is suitable only for single shot recognition like command or query. For long-running
	 // recognition, use StartContinuousRecognitionAsync() instead.
	 mSession = std::make_shared<MscogSession_t>();
	 auto& Session = *mSession;
	 auto future = recognizer->RecognizeOnceAsync();
	 Session.mRecognitionFuture = future;
	 */
	
	
	//	in-progress match
	auto OnSpeechRecognising = [this](const mscog::SpeechRecognitionEventArgs& e)
	{
		this->OnSpeechRecognised(e);
	};
	
	//	completed match - or sentance?	
	auto OnSpeechRecognised = [this](const mscog::SpeechRecognitionEventArgs& e)
	{
		this->OnSpeechRecognised(e);
	};
	
	auto OnCancelled = [this](const mscog::SpeechRecognitionCanceledEventArgs& e)
	{
		this->OnRecogniseCancelled(e);
	};
	
	auto OnStopped = [this](const mscog::SessionEventArgs& e)
	{
		std::cout << "Session stopped.";
		//	promise resolve
		//mRecogniseFuture.set_value(); // Notify to stop recognition.
	};
	
	mRecogniser->Recognizing.Connect(OnSpeechRecognising);
	mRecogniser->Recognized.Connect(OnSpeechRecognised);
	mRecogniser->Canceled.Connect(OnCancelled);
	mRecogniser->SessionStopped.Connect(OnStopped);
}


void MicrosoftCogninitiveDecoder_t::CreateRecogniserWithMicrophone()
{
	auto AudioConfig = mscog::Audio::AudioConfig::FromDefaultMicrophoneInput();
	CreateRecogniser(AudioConfig);
	
}

void MicrosoftCogninitiveDecoder_t::CreateRecogniser(AudioDataView_t<float> Format)
{
	throw std::runtime_error("todo: create float recogniser");
}

void MicrosoftCogninitiveDecoder_t::CreateRecogniser(AudioDataView_t<int16_t> Format)
{
	if ( mRecogniser )
		return;
	
	auto SampleBits = 16;
	auto StreamFormat = mscog::Audio::AudioStreamFormat::GetWaveFormatPCM( Format.mSamplesPerSecond, SampleBits, Format.mChannelCount );
	mInputStream = mscog::Audio::PushAudioInputStream::Create(StreamFormat);
	auto AudioConfig = mscog::Audio::AudioConfig::FromStreamInput(mInputStream);
	
	CreateRecogniser(AudioConfig);
	
}


void MicrosoftCogninitiveDecoder_t::OnRecogniseCancelled(const mscog::SpeechRecognitionCanceledEventArgs& Event)
{
	std::cerr << "CANCELED: Reason=" << static_cast<int>(Event.Reason) << std::endl;
	
	if (Event.Reason == mscog::CancellationReason::Error)
	{
		std::stringstream Error;
		Error << "Recognise ErrorCode=" << static_cast<int>(Event.ErrorCode) << "; " << Event.ErrorDetails;
		
		this->OnError( Error.str() );

		//	promise resolve
		//mRecogniseFuture.set_value(); // Notify to stop recognition.
	}
}

void MicrosoftCogninitiveDecoder_t::OnSpeechRecognised(const mscog::SpeechRecognitionEventArgs& Event)
{
	auto pResult = Event.Result;
	if ( !pResult )
		return;
	auto& Result = *pResult;
	
	//	detect non errr results
	switch ( Result.Reason )
	{
		case mscog::ResultReason::RecognizingSpeech:
		case mscog::ResultReason::RecognizedSpeech:
		case mscog::ResultReason::RecognizingIntent:
		case mscog::ResultReason::RecognizingKeyword:
		case mscog::ResultReason::RecognizedKeyword:
			break;
			
		default:
			return;
	}
	
	auto& Text = Result.Text;
	auto TimeOffset100Nanos = Result.Offset();
	auto Duration100Nanos = Result.Duration();
	auto TimeOffsetMs = Nano100sToMilliseconds(TimeOffset100Nanos);
	auto DurationMs = Nano100sToMilliseconds(Duration100Nanos);
	
	//	align to user's input time
	TimeOffsetMs.mMilliseconds += this->mFirstInputTime.mMilliseconds;
	
	OutputData_t Output;
	Output.mStartTime = TimeOffsetMs;
	Output.mEndTime = TimeOffsetMs.mMilliseconds + DurationMs.mMilliseconds;
	Output.mData = Text;
	
	this->OnOutputData(Output);
}

void MicrosoftCogninitiveDecoder_t::PushData(AudioDataView_t<int16_t> Data)
{
	//	todo: verify we're using 16bit data
	if ( !mRecogniser )
		CreateRecogniser(Data);

	if ( !mInputStream )
		throw std::runtime_error("PushData to decoder with no input stream");
	
	if ( !mFirstInputTime.IsValid() )
	{
		if ( !Data.mTime.IsValid() )
			throw std::runtime_error("Input time must be valid");
		
		mFirstInputTime = Data.mTime;
	}
	
	//	gr: docs call this "size", do not say if this is length or bytes
	auto Size = Data.mSamples.size_bytes();
	auto Data8 = reinterpret_cast<uint8_t*>( Data.mSamples.data() );
	//	API copies this data (say the docs)
	mInputStream->Write( Data8, Size );
}

void MicrosoftCogninitiveDecoder_t::StopRecogniser()
{
	if ( mInputStream )
	{
		mInputStream->Close();
		mInputStream.reset();
	}
	
	//	gr: this future references something in the recogniser so dont access after freeing
	if ( mRecogniser )
	{
		//	will this block?
		//	block for the end
		mRecogniseFuture.get();
	}
	
	if ( mRecogniser )
	{
		mRecogniser->StopContinuousRecognitionAsync();
		mRecogniser.reset();
	}
	
}


void MicrosoftCogninitiveDecoder_t::PushEndOfStream()
{
	StopRecogniser();
	
	if ( mInputStream )
	{
		mInputStream->Close();
		mInputStream.reset();
	}

}
