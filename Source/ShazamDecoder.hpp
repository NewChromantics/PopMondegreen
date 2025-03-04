#pragma once

#include "Decoder.hpp"


class ShazamSession_t;


class ShazamDecoder_t : public Decoder_t
{
public:
	static constexpr auto	Name = "Shazam";
public:
	ShazamDecoder_t(DecoderParams_t Params);
	
	virtual std::string	GetName() override	{	return Name;	}
	virtual void		PushData(AudioDataView_t<int16_t> Data) override;

private:
	std::shared_ptr<ShazamSession_t>	mSession;	//	obj-c container
};
