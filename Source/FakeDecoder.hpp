#pragma once

#include "Decoder.hpp"
#include <thread>

class FakeDecoder_t : public Decoder_t
{
public:
	static constexpr auto	Name = "Fake";
public:
	FakeDecoder_t(DecoderParams_t Params);
	~FakeDecoder_t();
	
	virtual std::string	GetName() override	{	return Name;	}
	virtual void		PushAudioData(AudioDataView_t<int16_t> Data) override;
	virtual void		PushEndOfStream() override;
	
private:
	void				Start(Timecode_t FirstTimecode);
	void				Stop();
	void				OutputGeneratorThread();
	void				OutputGeneratorIteration();
	
	std::thread			mOutputGeneratorThread;
	bool				mGeneratingOutput = false;
	Timecode_t			mFirstTimecode;
	int					mOutputCounter = 0;
};
