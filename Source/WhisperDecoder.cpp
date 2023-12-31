#include "WhisperDecoder.hpp"
#include <fstream>
#include <sstream>
#include <thread>
#include "Whisper/whisper.h"
#include <iostream>

void DownloadFileTask(std::string_view Filename,size_t LengthFromBack,std::function<void(std::span<uint8_t>)> OnChunk)
{
	std::vector<char> Buffer;
	auto BufferSizeMb = 10;
	Buffer.resize( 1024 * 1024 * BufferSizeMb );

	//	gr: windows doesnt support string_view param, so need terminated version
	std::string Filename0(Filename);
	std::ifstream File(Filename0.c_str(), std::ios::binary);
	//File.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
	if ( !File )
	{
		std::stringstream Error;
		Error << "Failed to open file " << Filename;
		throw std::runtime_error(Error.str());
	}
	
	if ( !File.is_open() )
	{
		std::stringstream Error;
		Error << "Failed to open file " << Filename;
		throw std::runtime_error(Error.str());
	}

	//	seek to send if we only want the tail
	if ( LengthFromBack > 0 )
	{
		File.seekg( 0, std::ios::end );
		auto FileLength = File.tellg();
		auto LengthFromBackAsPos = static_cast<decltype(FileLength)>(LengthFromBack);
		auto SeekTo = std::max<decltype(FileLength)>( 0ll, FileLength - LengthFromBackAsPos );
		File.seekg( SeekTo, std::ios::beg );
		if ( File.fail() )
			//Platform::ThrowLastError("Read file; seek to end failed");
			throw std::runtime_error("Read file; seek to end failed");
	}

	auto mAborted = false;
	while ( !mAborted )
	{
		//auto Available = File.in_avail();
		//	read chunk
		//	gr: readsome not implemented on mac? always returns 0
		//auto BytesRead = File.readsome( Buffer.data(), Buffer.size() );
		//auto AltBytesRead = File.gcount();
		File.read( Buffer.data(), Buffer.size() );
		auto BytesRead = File.gcount();
		if ( BytesRead > 0 )
		{
			auto* BufferBytes = reinterpret_cast<uint8_t*>( Buffer.data() );
			auto Chunk = std::span<uint8_t>( BufferBytes, BytesRead );
			OnChunk( Chunk );
		}

		//	check EOF first as fail() includes the EOF bit
		if ( File.eof() )
			break;

		if ( !File.good() )
			//Platform::ThrowLastError("Read file not good");
			throw std::runtime_error("Read file not good");

		if ( File.fail() )
			//Platform::ThrowLastError("Read file fail");
			throw std::runtime_error("Read file fail");

		//	pause if none read, thread needs to breath?
		if ( BytesRead == 0 )
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	if ( mAborted )
		throw std::runtime_error("File download aborted");

}


std::vector<uint8_t> DownloadFileBlocking(std::string_view Url)
{
	std::vector<uint8_t> FileContents;
	
	auto OnChunk = [&](std::span<uint8_t> Chunk)
	{
		std::copy( Chunk.begin(), Chunk.end(), std::back_inserter(FileContents) );
	};
	DownloadFileTask( Url, 0, OnChunk );

	return FileContents;
}



WhisperDecoder_t::WhisperDecoder_t(ListenerParams_t Params) :
	Listener_t		( Params )
{
	//	load model
	mModelData = DownloadFileBlocking( Params.mModelUrl );
	CreateContext();
}

void WhisperDecoder_t::CreateContext()
{
	std::scoped_lock Lock(mContextLock,mDataLock);
	mContext = whisper_init_from_buffer( mModelData.data(), mModelData.size() );
	if ( !mContext )
		throw std::runtime_error("Failed to create whisper context");
	
	std::cerr << "Created whsiper context" << std::endl;
}

bool WhisperDecoder_t::ThreadIteration()
{
	//	waiting for context...
	if ( !mContext )
		return true;
	
	//	process next chunk of data
	std::scoped_lock Lock(mDataLock);
	ProcessSamples( mPendingSamples );
	
	return true;
}

void WhisperDecoder_t::PushSamples(const AudioData_t& AudioData)
{
	//	need to do something here to build up samples
	std::scoped_lock DataLock(mDataLock);
	mPendingSamples = AudioData;
}

void WhisperDecoder_t::ProcessSamples(const AudioData_t& AudioData)
{
	if ( AudioData.mSampleRate != WHISPER_SAMPLE_RATE )
	{
		std::stringstream Error;
		Error << "Whisper requires audio sample rate " << WHISPER_SAMPLE_RATE << " but data is " << AudioData.mSampleRate;
		throw std::runtime_error(Error.str());
	}
	
	std::scoped_lock Lock(mContextLock);
	if ( !mContext )
		throw std::runtime_error("Not yet loaded whisper context");

	//	https://github.com/ggerganov/whisper.cpp/blob/master/examples/stream/stream.cpp#L91
	auto AudioContextSize = 0;
	auto Speedup = false;	//	speed up audio by x2 (reduced accuracy
	whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);

	wparams.print_progress   = true;
	wparams.print_special    = true;
	wparams.print_realtime   = true;
	wparams.print_timestamps = true;
	wparams.translate        = false;
	wparams.no_context       = false;
	wparams.single_segment   = false;
	wparams.max_tokens       = 0;
	wparams.language         = "en";
	wparams.n_threads        = 1;

	wparams.audio_ctx        = AudioContextSize;
	wparams.speed_up         = Speedup;

	// disable temperature fallback
	wparams.temperature_inc  = -1.0f;

	auto StartTime = EventTime_t::Now();
	
	//	this is to try and help audio between segments, i think
	//wparams.prompt_tokens    = params.no_context ? nullptr : prompt_tokens.data();
	//wparams.prompt_n_tokens  = params.no_context ? 0       : prompt_tokens.size();

	{
		std::span<const float> Samples( AudioData.mSamples.data(), AudioData.mSamples.size() );
		auto Error = whisper_full( mContext, wparams, Samples.data(), Samples.size() );
		if ( Error != 0 )
		{
			std::stringstream ErrorString;
			ErrorString << "Whisper process error: " << Error;
			throw std::runtime_error(ErrorString.str());
		}
	}
	
	//	get text out
	std::scoped_lock DataLock(mDataLock);
	auto SegmentCount = whisper_full_n_segments( mContext );
	for ( auto s=0;	s<SegmentCount;	s++ )
	{
		auto Text = whisper_full_get_segment_text( mContext, s );
		mOutputText.push_back( Text );
	}
	
	auto FinishTime = EventTime_t::Now();
	auto ProcessDurationMs = FinishTime.mTimeMs - StartTime.mTimeMs;
}
