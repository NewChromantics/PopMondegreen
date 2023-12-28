#pragma once


#include "PopMondegreen.hpp"
#include <string>
#include "PopJson/PopJson.hpp"
#include "Timecode.hpp"



class AudioDataView_t
{
public:
	int					mSampleRate = 0;
	int					mChannelCount = 0;
	std::span<float>	mSamples;
	
	//	whilst not part of audio data... it goes along with it!
	Timecode_t			mTime;
};


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
	
	//	for whisper
	std::string		mModelUrl;
};




class Decoder_t
{
public:
	Decoder_t(DecoderParams_t Params);
	
	virtual std::string	GetName()=0;
	
	virtual void		PushData(AudioDataView_t Data)=0;
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
