#pragma once
#include <functional>

enum StreamStatus
{
	Idle,
	Running,
	EndOfStream,
	Finished
};

class StreamEventBase
{
public:
	StreamStatus status;
};

typedef std::function<void(const StreamEventBase&)> StreamEventCb;