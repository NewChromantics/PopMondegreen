#pragma once

#include "Decoder.hpp"

class whisper_context;



class WhisperDecoder_t : public Decoder_t
{
public:
	static constexpr auto	Name = "Whisper";
public:
	WhisperDecoder_t(DecoderParams_t Params);
	
	virtual std::string	GetName() override	{	return Name;	}
	
	virtual void		PushAudioData(AudioDataView_t<float> Data) override;
	virtual void		PushEndOfStream() override;

private:
	void				CreateContext(std::string_view ModelUrl);
	bool				ThreadIteration();

	std::mutex			mContextLock;	//	this stops us trying to run two inferences at once
	whisper_context*	mContext = nullptr;
};
