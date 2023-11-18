#include "FakeListener.hpp"


FakeListener_t::FakeListener_t(ListenerParams_t Params) :
	Listener_t		( Params )
{
}

void FakeListener_t::PushData(AudioDataView_t Data)
{
	//	output dummy data to say stuff has arrived
}
