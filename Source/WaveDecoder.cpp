#include "WaveDecoder.hpp"
#include "FileReader.hpp"
#include <iostream>



AudioDataView_t<float> Audio16ToFloat(AudioDataView_t<int16_t>& Audio16,std::vector<float>& Floats)
{
	Floats.reserve( Audio16.mSamples.size() );
	
	for ( auto i=0;	i<Audio16.mSamples.size();	i++ )
	{
		auto Value16 = Audio16.mSamples[i];
		auto Valuef = static_cast<float>(Value16) / static_cast<float>( std::numeric_limits<int16_t>::max() );
		Floats.push_back(Valuef);
	}
	
	AudioDataView_t<float> AudioDataFloat;
	AudioDataFloat.mSamples = Floats;
	AudioDataFloat.mSamplesPerSecond = Audio16.mSamplesPerSecond;
	AudioDataFloat.mChannelCount = Audio16.mChannelCount;
	AudioDataFloat.mTime = Audio16.mTime;

	return AudioDataFloat;
}

AudioDataView_t<int16_t> AudioFloatTo16(AudioDataView_t<float>& AudioFloats,std::vector<int16_t>& Values16)
{
	Values16.reserve( AudioFloats.mSamples.size() );
	
	for ( auto i=0;	i<AudioFloats.mSamples.size();	i++ )
	{
		auto Valuef = AudioFloats.mSamples[i];
		auto Value16 = Valuef * static_cast<float>( std::numeric_limits<int16_t>::max() );
		Values16.push_back(Valuef);
	}
	
	AudioDataView_t<int16_t> AudioData16;
	AudioData16.mSamples = Values16;
	AudioData16.mSamplesPerSecond = AudioFloats.mSamplesPerSecond;
	AudioData16.mChannelCount = AudioFloats.mChannelCount;
	AudioData16.mTime = AudioFloats.mTime;
	
	return AudioData16;
}


AudioDataView_t<float> AudioFloatResample(AudioDataView_t<float>& Audio,std::vector<float>& Storage)
{
	//	if reducing samples, we can drop (hacky, but works!)
	throw std::runtime_error("todo; Resample wave");
}



bool IsFourccAscii(uint8_t v)
{
	//return isprint(v);
	//	space to tilde
	//	https://www.asciitable.com/
	if ( v >= 32 && v <= 126 )
		return true;
	
	return false;
}


template <typename T>
T SwapEndian(T Value)
{
	//	MSVCC
	//	unsigned long _byteswap_ulong(unsigned long value);
	//	GCC
	//uint32_t __builtin_bswap32 (uint32_t x)
	static_assert(std::is_pod<T>::value, "SwapEndian only for POD types");
	auto* Start = reinterpret_cast<uint8_t*>(&Value);
	auto* End = Start + sizeof(T);
	std::reverse(Start, End);
	return Value;
}

uint32_t Platform_SwapEndian(uint32_t Value)
{
	return ::SwapEndian(Value);
}

void GetFourccString(std::stringstream& String,uint32_t Fourcc,bool Reversed)
{
	Fourcc = Reversed ? Platform_SwapEndian(Fourcc) : Fourcc;
	
	uint8_t a = (Fourcc>>0) & 0xff;
	uint8_t b = (Fourcc>>8) & 0xff;
	uint8_t c = (Fourcc>>16) & 0xff;
	uint8_t d = (Fourcc>>24) & 0xff;
	std::array<uint8_t,4> abcd = { a,b,c,d };
	
	for ( int i=0;	i<abcd.size();	i++ )
	{
		auto Value = abcd[i];
		if ( IsFourccAscii(Value) )
		{
			String << static_cast<char>(Value);
		}
		else
		{
			String << " 0x" << std::hex << Value << " " << std::dec;
		}
	}
}

void WriteHex(std::ostream& String,uint8_t Value)
{
	String << "0x";
	
	auto a = (Value >> 4) & 0xf;
	auto b = (Value >> 0) & 0xf;
	
	if ( a < 10 )
		String << a;
	else
		String << ('a'+(a-10));
	
	if ( b < 10 )
		String << b;
	else
		String << ('a'+(b-10));
	
}

std::string GetFourccsString(std::span<uint32_t> Fourccs,bool Reversed)
{
	auto IsAscii = [](uint8_t v)
	{
		return isprint(v);
		//if ( v >= 'a' && v <= 'z' )	return true;
		//if ( v >= 'A' && v <= 'Z' )	return true;
		//if ( v >= 'a' && v <= 'z' )	return true;
	};
	
	std::stringstream String;
	for ( auto Fourcc : Fourccs )
	{
		Fourcc = Reversed ? Platform_SwapEndian(Fourcc) : Fourcc;
		
		uint8_t a = (Fourcc>>0) & 0xff;
		uint8_t b = (Fourcc>>8) & 0xff;
		uint8_t c = (Fourcc>>16) & 0xff;
		uint8_t d = (Fourcc>>24) & 0xff;
		std::array<uint8_t,4> abcd = { a,b,c,d };
		
		for ( int i=0;	i<abcd.size();	i++ )
		{
			auto Value = abcd[i];
			if ( IsFourccAscii(Value) )
			{
				String << static_cast<char>(Value);
			}
			else
			{
				String << " ";
				WriteHex( String, Value );
			}
		}
	}
	return String.str();
}

std::string GetFourccString(uint32_t Fourcc,bool Reversed=false)
{
	//std::array<uint32_t,1> Fourccs = { Fourcc };
	//uint32_t _Fourccs[1]={Fourcc};
	//std::span<uint32_t> Fourccs( _Fourccs, 1 );
	std::span<uint32_t> Fourccs( &Fourcc, 1 );
	return GetFourccsString( Fourccs, Reversed );
}


#define FOURCC_32(a,b,c,d)	( a | (b<<8) | (c<<16) | (d<<24) )

class WavChunk_t
{
	public:
	uint32_t			mFourcc = 0;
	uint32_t			mSize = 0;
	std::span<uint8_t>	mData;
};


class ChunkReader_t : public FileReader_t
{
	public:
	using FileReader_t::FileReader_t;	//	inherit constructor
	//ChunkReader_t(std::span<uint8_t> AllData);
	
	WavChunk_t			ReadChunk();
	WavChunk_t			ReadChunk(uint32_t ExpectedFourcc);
};

class WavReader_t : public ChunkReader_t
{
	public:
	WavReader_t(std::span<uint8_t> AllData);
	
	void				ReadFormat(WavChunk_t Chunk);
	void				ReadData(WavChunk_t Chunk);
	
	uint16_t			mFormat = 0;	//	gr: what is this? integer vs float?
	uint16_t			mChannelCount = 0;
	uint32_t			mSampleRate = 0;
	uint32_t			mByteRate = 0;
	uint16_t			mBlockAlign = 0;
	uint16_t			mBitsPerSample = 0;
	std::span<uint8_t>	mSampleData;
};

WavReader_t::WavReader_t(std::span<uint8_t> Data) :
ChunkReader_t	( Data )
{
	//	riff chunk contains sub atoms
	auto RiffChunk = ReadChunk( FOURCC_32('R','I','F','F') );
	
	ChunkReader_t Riff(RiffChunk.mData);
	Riff.ReadFourcc(FOURCC_32('W','A','V','E'));
	auto WaveSubData = Riff.RemainingData();
	
	ChunkReader_t Wave(WaveSubData);
	
	while ( Wave.RemainingBytes() > 0 )
	{
		auto Chunk = Wave.ReadChunk();
		if ( Chunk.mFourcc == FOURCC_32('f','m','t',' ') )
		{
			ReadFormat( Chunk );
		}
		else if ( Chunk.mFourcc == FOURCC_32('d','a','t','a') )
		{
			ReadData( Chunk );
		}
		else
		{
			//	todo: save this as meta for output
			std::cerr << "Wav got unused chunk " << GetFourccString(Chunk.mFourcc) << std::endl;
		}
	}
}

WavChunk_t ChunkReader_t::ReadChunk()
{
	WavChunk_t Atom;
	Atom.mFourcc = Read32();
	Atom.mSize = Read32();
	//	gr: does this size include header?
	Atom.mData = ReadBytes(Atom.mSize);
	return Atom;
}

WavChunk_t ChunkReader_t::ReadChunk(uint32_t ExpectedFourcc)
{
	auto Chunk = ReadChunk();
	if ( Chunk.mFourcc != ExpectedFourcc )
	{
		std::stringstream Error;
		Error << "Wav chunk fourcc expected [" << GetFourccString(ExpectedFourcc) << "] found [" << GetFourccString(Chunk.mFourcc) << "]";
		throw std::runtime_error( Error.str() );
	}
	return Chunk;
}

void WavReader_t::ReadFormat(WavChunk_t Chunk)
{
	FileReader_t ChunkData( Chunk.mData );
	mFormat = ChunkData.Read16();
	mChannelCount = ChunkData.Read16();
	mSampleRate = ChunkData.Read32();
	mByteRate = ChunkData.Read32();
	mBlockAlign = ChunkData.Read16();
	mBitsPerSample = ChunkData.Read16();
}

void WavReader_t::ReadData(WavChunk_t Chunk)
{
	mSampleData = Chunk.mData;
}





WaveDecoder_t::WaveDecoder_t(std::function<void(AudioDataView_t<float>,bool Eof)> OnData) :
	mOnDataFloat	( OnData )
{
}

WaveDecoder_t::WaveDecoder_t(std::function<void(AudioDataView_t<int16_t>,bool Eof)> OnData) :
	mOnData16	( OnData )
{
}

void WaveDecoder_t::PushData(std::span<uint8_t> FileData)
{
	//	write lock
	std::unique_lock lock(mWavDataLock);
	std::copy( FileData.begin(), FileData.end(), std::back_inserter(mWavData) );
}

void WaveDecoder_t::PushEndOfData()
{
	//	read lock
	std::shared_lock Lock(mWavDataLock);
	DecodeWavFile( mWavData );
}


void WaveDecoder_t::OnOutputData(AudioDataView_t<float> Data)
{
	if ( mOnDataFloat )
	{
		mOnDataFloat( Data, false );
	}
	
	if ( mOnData16 )
	{
		//	convert to 16
		//throw std::runtime_error("todo: convert to 16 bit");
		std::cerr << "todo: convert to 16 bit" << std::endl;
	}
}

void WaveDecoder_t::OnOutputData(AudioDataView_t<int16_t> Data)
{
	if ( mOnDataFloat )
	{
		throw std::runtime_error("todo: convert to float");
	}
	
	if ( mOnData16 )
	{
		mOnData16( Data, false );
	}
}

void WaveDecoder_t::OnOutputEof()
{
	if ( mOnData16 )
		mOnData16( {}, true );
	
	if ( mOnDataFloat )
		mOnDataFloat( {}, true );
}



void WaveDecoder_t::DecodeWavFile(std::span<uint8_t> WavData)
{
	//	https://answers.unity.com/questions/737002/wav-byte-to-audioclip.html#
	//	.WAV to data
	WavReader_t Wav( WavData );
	FileReader_t SampleData(Wav.mSampleData);
	
	auto BytesPerSample = Wav.mBitsPerSample / 8;
	if ( BytesPerSample != 1 && BytesPerSample != 2 && BytesPerSample != 4 )
	{
		std::stringstream Error;
		Error << "Todo: handle wav sample data other than 8,16,32bit (BitsPerSample=" << Wav.mBitsPerSample << ")";
		throw std::runtime_error(Error.str());
	}
	
	auto SampleSize = BytesPerSample * Wav.mChannelCount;
	auto SampleCount = SampleData.size() / SampleSize;
	auto Overflow = SampleData.size() % SampleSize;
	if ( Overflow != 0 )
	{
		std::stringstream Error;
		Error << "Error; sample data x" << SampleData.size() << "bytes doesn't align to BytesPerSample(" << BytesPerSample << ") * ChannelCount(" << Wav.mChannelCount << ")";
		throw std::runtime_error(Error.str());
	}
	
	//	read all samples, deinterleaved to 2 arrays
	std::vector<float> SampleFloats;
	std::vector<int16_t> Sample16s;
	SampleFloats.reserve( SampleCount*Wav.mChannelCount );
	Sample16s.reserve( SampleCount*Wav.mChannelCount );
	
	for ( int s=0;	s<SampleCount;	s++ )
	{
		for ( int c=0;	c<Wav.mChannelCount;	c++ )
		{
			float Sample = 0;
			if ( BytesPerSample == 1 )
				Sample = SampleData.Read8AsFloat();
			if ( BytesPerSample == 2 )
				Sample = SampleData.Read16AsFloat();
			if ( BytesPerSample == 4 )
				Sample = SampleData.Read32AsFloat();
			
			int16_t Sample16 = Sample * 32767;//std::numeric_limits<int16_t>().max();
			SampleFloats.push_back(Sample);
			Sample16s.push_back(Sample16);
		}
	}
	
	AudioDataView_t<float> PcmDataf;
	PcmDataf.mTime = Timecode_t(0);
	PcmDataf.mChannelCount = Wav.mChannelCount;
	PcmDataf.mSamplesPerSecond = Wav.mSampleRate;
	PcmDataf.mSamples = SampleFloats; //std::span<float>(SampleFloats.data(),SampleFloats.size()) );
	OnOutputData(PcmDataf);

	AudioDataView_t<int16_t> PcmData16;
	PcmData16.mTime = Timecode_t(0);
	PcmData16.mChannelCount = Wav.mChannelCount;
	PcmData16.mSamplesPerSecond = Wav.mSampleRate;
	PcmData16.mSamples = Sample16s;
	OnOutputData(PcmData16);
}

