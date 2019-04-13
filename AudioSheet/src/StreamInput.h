#pragma once
#include "StreamEvent.h"

class StreamInput
{
protected:
	StreamEventCb eventCb;
public:
	void setEventCallback(StreamEventCb cb){eventCb = cb;}
	virtual ~StreamInput() = default;
	virtual void open(int deviceIndex = 0) = 0;
	virtual void close() = 0;
	virtual void read() = 0;
};
