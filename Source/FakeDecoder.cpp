#include "FakeDecoder.hpp"


FakeDecoder_t::FakeDecoder_t(DecoderParams_t Params) :
Decoder_t		( Params )
{
}

void FakeDecoder_t::PushData(AudioDataView_t Data)
{
	//	output dummy data to say stuff has arrived
}
