#include "MicrosoftCognitiveDecoder.hpp"
#include <MicrosoftCognitiveServicesSpeech/speechapi_cxx.h>
#include <future>

namespace mscog = Microsoft::CognitiveServices::Speech;




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
	
	
	//	async recogniser callbacks
	mRecogniser->Recognizing.Connect([this](const mscog::SpeechRecognitionEventArgs& e)
										  {
		std::cout << "Recognizing:" << e.Result->Text << std::endl;
	});
	
	mRecogniser->Recognized.Connect([this](const mscog::SpeechRecognitionEventArgs& e)
										 {
		if (e.Result->Reason == mscog::ResultReason::RecognizedSpeech)
		{
			std::cout << "RECOGNIZED: Text=" << e.Result->Text 
			<< " (text could not be translated)" << std::endl;
		}
		else if (e.Result->Reason == mscog::ResultReason::NoMatch)
		{
			std::cout << "NOMATCH: Speech could not be recognized." << std::endl;
		}
	});
	
	mRecogniser->Canceled.Connect([this](const mscog::SpeechRecognitionCanceledEventArgs& e)
									   {
		std::cout << "CANCELED: Reason=" << (int)e.Reason << std::endl;
		if (e.Reason == mscog::CancellationReason::Error)
		{
			std::cout << "CANCELED: ErrorCode=" << (int)e.ErrorCode << "\n"
			<< "CANCELED: ErrorDetails=" << e.ErrorDetails << "\n"
			<< "CANCELED: Did you set the speech resource key and region values?" << std::endl;
			
			//	promise resolve
			//mRecogniseFuture.set_value(); // Notify to stop recognition.
		}
	});
	
	mRecogniser->SessionStopped.Connect([this](const mscog::SessionEventArgs& e)
											 {
		std::cout << "Session stopped.";
		
		//	promise resolve
		//mRecogniseFuture.set_value(); // Notify to stop recognition.
	});
}

MicrosoftCogninitiveDecoder_t::~MicrosoftCogninitiveDecoder_t()
{
	//	got to wait for work to finish
	StopRecogniser();
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
