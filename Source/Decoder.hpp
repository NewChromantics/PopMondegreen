#pragma once


#include "PopMondegreen.hpp"
#include <string>
#include "PopJson/PopJson.hpp"
#include "Timecode.hpp"
#include "AudioDataView.hpp"



class OutputData_t
{
public:
	Timecode_t		mStartTime;
	Timecode_t		mEndTime;
	std::string		mData;
	
	std::string		mError;		//	is an error rather than data

	PopMondegreen::EventTime_t	mOutputTime;
};



class DecoderParams_t
{
public:
	DecoderParams_t(PopJson::Json_t& Params);
	
	bool			mUseApiMicrophone = false;	//	for any API with built in mic support
	
	//	for whisper
	std::string		mModelUrl;
	
	//	for microsoft conginitive services/azure
	std::string		mApiKey;
	std::string		mApiRegion;
};




class Decoder_t
{
public:
	Decoder_t(DecoderParams_t Params);
	
	virtual std::string	GetName()=0;
	
	//	gr: find a nice way to detect if a processor isn't implemented?
	//		pass functors to constructor and auto fill auto conversion?
	virtual void		PushData(AudioDataView_t<int16_t> Data);
	virtual void		PushData(AudioDataView_t<float> Data);
	virtual void		PushEndOfStream()=0;
	OutputData_t		PopData();

protected:
	void				OnOutputData(OutputData_t Data);
	void				OnError(std::string_view Error);
	
	DecoderParams_t		mParams;
	
private:
	std::mutex					mOutputLock;
	std::vector<OutputData_t>	mOutputs;
};
