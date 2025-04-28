#include "WhisperDecoder.hpp"
#include <fstream>
#include <sstream>
#include <thread>
#include "whisper/whisper.h"
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
		Error << "Failed to open file \"" << Filename << "\"";
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



WhisperDecoder_t::WhisperDecoder_t(DecoderParams_t Params) :
	Decoder_t		( Params )
{
	if ( Params.mModelUrl.empty() )
		throw std::runtime_error("Whisper missing model url");
	
	CreateContext(Params.mModelUrl);
}

void WhisperDecoder_t::CreateContext(std::string_view ModelUrl)
{
	std::scoped_lock Lock(mContextLock);

	//	load model
	//mModelData = DownloadFileBlocking( ModelUrl );
	//mContext = whisper_init_from_buffer( mModelData.data(), mModelData.size() );

	//	make a custom loader to read data instantly
	std::string Filename0(ModelUrl);
	std::ifstream File(Filename0.c_str(), std::ios::binary);
	if ( !File.is_open() )
	{
		std::stringstream Error;
		Error << "Failed to open file " << Filename0;
		throw std::runtime_error(Error.str());
	}
	File.seekg( 0, std::ios::beg );
	if ( File.fail() )
		throw std::runtime_error("Read file; seek to end failed");
	
	auto Read = [](void * ctx, void * output, size_t read_size)
	{
		auto& File = *reinterpret_cast<std::ifstream*>(ctx);
		auto* output8 = reinterpret_cast<char*>(output);
		File.read( output8, read_size );
		auto BytesRead =  File.gcount();
		return static_cast<size_t>(BytesRead);
	};
	
	auto IsEof = [](void* ctx)
	{
		auto& File = *reinterpret_cast<std::ifstream*>(ctx);
		return File.eof();
	};
	
	auto Close = [](void* ctx)
	{
		//auto& File = *reinterpret_cast<std::ifstream*>(ctx);
	};
	
	//	read directly from filesystem
	whisper_model_loader Loader;
	Loader.context = &File;
	Loader.read = Read;
	Loader.eof = IsEof;
	Loader.close = Close;
	mContext = whisper_init( &Loader );
	
	
	
	if ( !mContext )
		throw std::runtime_error("Failed to create whisper context");
	
	std::cerr << "Created whsiper context" << std::endl;
}
/*
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
*/

void WhisperDecoder_t::PushEndOfStream()
{
	throw std::runtime_error("todo: WhisperDecoder_t::PushEndOfStream");
}

void WhisperDecoder_t::PushData(AudioDataView_t<float> AudioData)
{
	std::vector<float> ResampledData;
	if ( AudioData.mSamplesPerSecond != WHISPER_SAMPLE_RATE )
	{
		AudioData.Resample( WHISPER_SAMPLE_RATE, ResampledData );
		/*
		std::stringstream Error;
		Error << "Whisper requires audio sample rate " << WHISPER_SAMPLE_RATE << " but data is " << AudioData.mSamplesPerSecond;
		throw std::runtime_error(Error.str());
		*/
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
	//wparams.speed_up         = Speedup;

	// disable temperature fallback
	wparams.temperature_inc  = -1.0f;

	auto StartTime = PopMondegreen::EventTime_t::Now();
	
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
	auto Timings = whisper_get_timings( mContext );

	auto SegmentCount = whisper_full_n_segments( mContext );
	for ( auto s=0;	s<SegmentCount;	s++ )
	{
		auto Text = whisper_full_get_segment_text( mContext, s );
		OutputData_t Output;
		Output.mData = Text;
		Output.mOutputTime = PopMondegreen::EventTime_t::Now();
		OnOutputData( Output );
	}
	
	//auto FinishTime = EventTime_t::Now();
	//auto ProcessDurationMs = FinishTime.mTimeMs - StartTime.mTimeMs;
}
