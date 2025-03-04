#include "ShazamDecoder.hpp"
#import "ShazamKit/ShazamKit.h"
#include <iostream>



class ShazamSession_t
{
public:
	ShazamSession_t();
	~ShazamSession_t();
	
	void			PushData(AudioDataView_t<int16_t> Data);

	SHSession*		mSession = nullptr;
};



@interface ShazamDelegate_t : NSObject<SHSessionDelegate>
@end

@implementation ShazamDelegate_t
{
	ShazamSession_t*	mParent;
	//std::shared_mutex	mParentLock;
}
- (instancetype)init:(ShazamSession_t* _Nonnull)Parent
{
	self = [super init];
	self->mParent = Parent;
	return self;
}

@end




ShazamSession_t::ShazamSession_t()
{
	mSession = [[SHSession alloc] init];
	auto Delegate = [[ShazamDelegate_t alloc] init];
	mSession.delegate = Delegate;
}

ShazamSession_t::~ShazamSession_t()
{
}

void ShazamSession_t::PushData(AudioDataView_t<int16_t> Samples)
{
	AVAudioChannelCount ChannelCount = Samples.mChannelCount;
	double SampleRate = Samples.mSamplesPerSecond;
	AVAudioCommonFormat DataFormat = AVAudioPCMFormatFloat32;
	bool Interleaved = true;
	auto* Format = [[AVAudioFormat alloc] initWithCommonFormat:DataFormat sampleRate:SampleRate channels:ChannelCount interleaved:Interleaved];

	AVAudioFrameCount FrameCount = Samples.mSamples.size();
	AVAudioPCMBuffer* Buffer = [[AVAudioPCMBuffer alloc] initWithPCMFormat:Format frameCapacity:FrameCount];

	//	gr: interleaved, so how many buffers?
	Buffer.floatChannelData[0][0] = 0.f;
	
	//	gr: fix this
	AVAudioTime* Time = [[AVAudioTime alloc]initWithHostTime:0];
	[mSession matchStreamingBuffer:Buffer atTime:Time];
}


ShazamDecoder_t::ShazamDecoder_t(DecoderParams_t Params) :
	Decoder_t		( Params )
{
	mSession.reset( new ShazamSession_t );
}

void ShazamDecoder_t::PushData(AudioDataView_t<int16_t> Data)
{
	mSession->PushData( Data );
}

/*

class MatchingHelper: NSObject {
  private var session: SHSession?
  private let audioEngine = AVAudioEngine()

  private var matchHandler: ((SHMatchedMediaItem?, Error?) -> Void)?

  init(matchHandler handler: ((SHMatchedMediaItem?, Error?) -> Void)?) {
	matchHandler = handler
  }
}
*/
