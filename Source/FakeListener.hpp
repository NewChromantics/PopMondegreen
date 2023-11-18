#pragma once

#include "Listener.hpp"

class FakeListener_t : public Listener_t
{
public:
	static constexpr auto	Name = "Fake";
public:
	FakeListener_t(ListenerParams_t Params);
	
	virtual std::string	GetName() override	{	return Name;	}
	virtual void		PushData(AudioDataView_t Data) override;
};
