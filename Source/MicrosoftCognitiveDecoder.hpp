#pragma once

#include "Decoder.hpp"
#include <MicrosoftCognitiveServicesSpeech/speechapi_cxx.h>
#include <future>

class MscogSession_t;


class MicrosoftCogninitiveDecoder_t : public Decoder_t
{
public:
	static constexpr auto	Name = "MicrosoftCognitive";
public:
	MicrosoftCogninitiveDecoder_t(DecoderParams_t Params);
	virtual ~MicrosoftCogninitiveDecoder_t();
	
	virtual std::string	GetName() override	{	return Name;	}
	virtual void		PushAudioData(AudioDataView_t<int16_t> Data) override;
	virtual void		PushEndOfStream() override;

private:
	void				CreateRecogniserWithMicrophone();
	void				CreateRecogniser(AudioDataView_t<float> Format);
	void				CreateRecogniser(AudioDataView_t<int16_t> Format);
	void				CreateRecogniser(std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::AudioConfig> AudioInputConfig);
	
	void				StopRecogniser();
	void				OnSpeechRecognised(const Microsoft::CognitiveServices::Speech::SpeechRecognitionEventArgs& Event);
	void				OnRecogniseCancelled(const Microsoft::CognitiveServices::Speech::SpeechRecognitionCanceledEventArgs& Event);
	
	Timecode_t			mFirstInputTime;
	std::shared_ptr<Microsoft::CognitiveServices::Speech::Audio::PushAudioInputStream> mInputStream;
	
	std::future<void>	mRecogniseFuture;
	std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechRecognizer>	mRecogniser;

	std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechConfig>	mSpeechConfig;

};
