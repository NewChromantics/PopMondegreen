#include "Listener.hpp"


ListenerParams_t::ListenerParams_t(std::string_view Json)
{
}

Listener_t::Listener_t(ListenerParams_t Params) :
	mParams		( Params )
{
}
