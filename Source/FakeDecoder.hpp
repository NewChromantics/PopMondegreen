#pragma once

#include "Decoder.hpp"

class FakeDecoder_t : public Decoder_t
{
public:
	static constexpr auto	Name = "Fake";
public:
	FakeDecoder_t(DecoderParams_t Params);
	
	virtual std::string	GetName() override	{	return Name;	}
	virtual void		PushData(AudioDataView_t Data) override;
};
