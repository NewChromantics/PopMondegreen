#pragma once

#include "Listener.hpp"


class ShazamSession_t;


class ShazamListener_t : public Listener_t
{
public:
	static constexpr auto	Name = "Shazam";
public:
	ShazamListener_t(ListenerParams_t Params);
	
	virtual std::string	GetName() override	{	return Name;	}
	virtual void		PushData(AudioDataView_t Data) override;

private:
	std::shared_ptr<ShazamSession_t>	mSession;	//	obj-c container
};
