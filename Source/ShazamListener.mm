#include "ShazamListener.hpp"
#import "ShazamKit/ShazamKit.h"
#include <iostream>



class ShazamSession_t
{
public:
	ShazamSession_t();
	~ShazamSession_t();
	
	void			PushData(AudioDataView_t Data);

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

void ShazamSession_t::PushData(AudioDataView_t Samples)
{
	AVAudioChannelCount Channels = Samples.mChannelCount;
	double SampleRate = Samples.mSampleRate;
	auto* Format = [[AVAudioFormat alloc] initStandardFormatWithSampleRate:SampleRate channels:Channels];
	AVAudioFrameCount FrameCount = Samples.mSamples.size();
	AVAudioPCMBuffer* Buffer = [[AVAudioPCMBuffer alloc] initWithPCMFormat:Format frameCapacity:FrameCount];
	
	//	gr: fix this
	AVAudioTime* Time = [[AVAudioTime alloc]initWithHostTime:0];
	[mSession matchStreamingBuffer:Buffer atTime:Time];
}


ShazamListener_t::ShazamListener_t(ListenerParams_t Params) :
	Listener_t		( Params )
{
	mSession.reset( new ShazamSession_t );
}

void ShazamListener_t::PushData(AudioDataView_t Data)
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
