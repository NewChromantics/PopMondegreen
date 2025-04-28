#pragma once

#include "Decoder.hpp"

class whisper_context;



class WhisperDecoder_t : public Decoder_t
{
public:
	static constexpr auto	Name = "Whisper";
public:
	WhisperDecoder_t(DecoderParams_t Params);
	
	virtual std::string	GetName() override	{	return Name;	}
	
	virtual void		PushData(AudioDataView_t<float> Data) override;
	virtual void		PushEndOfStream() override;

private:
	void				CreateContext(std::string_view ModelUrl);
	bool				ThreadIteration();

	std::mutex			mContextLock;	//	this stops us trying to run two inferences at once
	whisper_context*	mContext = nullptr;
	
	std::vector<uint8_t>	mModelData;
	std::mutex				mDataLock;
	AudioDataView_t<float>	mPendingSamples;
};

/*
class WhisperDecoder_t : public AudioTranscriber_t
{
public:
	WhisperDecoder_t(std::string_view ModelUrl);
	~WhisperDecoder_t();
	
	//	probably will need meta to go with this
	void				PushSamples(const AudioData_t& AudioData);
	void				PopText(std::vector<std::string>& Text,bool& TranscriptionFinished);
	std::string			GetError();

private:
	bool				ThreadIteration();
	
	void				ProcessSamples(const AudioData_t& AudioData);

	void				OnModelDownloaded(Download_t& Download);
	void				OnError(std::string_view Error);
	void				CreateContext();
	
	std::mutex			mContextLock;	//	this stops us trying to run two inferences at once
	whisper_context*	mContext = nullptr;
	
	std::shared_ptr<Iterator_t>	mThread;
	
	std::recursive_mutex	mDataLock;
	std::string				mError;
	std::vector<uint8_t>	mModelData;
	AudioData_t				mPendingSamples;
	std::vector<std::string>	mOutputText;	//	text ready to be popped

	//	blocking
	std::shared_ptr<Download_t>	mModelDownload;
};
*/
