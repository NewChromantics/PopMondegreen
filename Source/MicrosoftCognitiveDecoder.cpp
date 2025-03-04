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


//	https://github.com/Azure-Samples/cognitive-services-speech-sdk/blob/master/quickstart/cpp/macos/from-microphone/helloworld.cpp
MicrosoftCogninitiveDecoder_t::MicrosoftCogninitiveDecoder_t(DecoderParams_t Params) :
	Decoder_t(Params)
{
	//mscog::setlocale(LC_ALL, "");
	
	// Creates an instance of a speech config with specified subscription key and service region.
	// Replace with your own subscription key and service region (e.g., "westus").
	auto SpeechConfig = mscog::SpeechConfig::FromSubscription( Params.mApiKey, Params.mApiRegion );
	
	// Creates a speech recognizer
	auto AudioConfig = mscog::Audio::AudioConfig::FromDefaultMicrophoneInput();
	//static std::shared_ptr<AudioConfig> FromStreamInput(std::shared_ptr<AudioInputStream> stream)
	mRecogniser = mscog::SpeechRecognizer::FromConfig(SpeechConfig, AudioConfig);
	//auto recognizer = mscog::SpeechRecognizer::FromConfig(config);
	
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

MicrosoftCogninitiveDecoder_t::~MicrosoftCogninitiveDecoder_t()
{
	//	got to wait for work to finish
	StopRecogniser();
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
	if ( Result.Reason != mscog::ResultReason::RecognizedSpeech)
	{
		return;
	}
	
	auto& Text = Result.Text;
	auto TimeOffset100Nanos = Result.Offset();
	auto Duration100Nanos = Result.Duration();
	auto TimeOffsetMs = Nano100sToMilliseconds(TimeOffset100Nanos);
	auto DurationMs = Nano100sToMilliseconds(Duration100Nanos);
	
	OutputData_t Output;
	Output.mStartTime = TimeOffsetMs;
	Output.mEndTime = TimeOffsetMs.mMilliseconds + DurationMs.mMilliseconds;
	Output.mData = Text;
	
	this->OnOutputData(Output);
}

void MicrosoftCogninitiveDecoder_t::PushData(AudioDataView_t Data)
{
}

void MicrosoftCogninitiveDecoder_t::StopRecogniser()
{
	mRecogniser->StopContinuousRecognitionAsync();
	mRecogniser.reset();
	
	//	will this block?
	//	block for the end
	mRecogniseFuture.get();
}


void MicrosoftCogninitiveDecoder_t::PushEndOfStream()
{
	StopRecogniser();
}
