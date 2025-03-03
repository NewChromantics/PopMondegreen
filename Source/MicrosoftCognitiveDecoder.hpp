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
	virtual void		PushData(AudioDataView_t Data) override;
	virtual void		PushEndOfStream() override;

private:
	void				StopRecogniser();
	
	
	std::future<void>	mRecogniseFuture;
	std::shared_ptr<Microsoft::CognitiveServices::Speech::SpeechRecognizer>	mRecogniser;
};
