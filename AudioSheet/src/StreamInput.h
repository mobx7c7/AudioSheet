#pragma once
#include "StreamEvent.h"

class StreamInput
{
public:
	virtual ~StreamInput() = default;
	virtual void setCallback(StreamEventCb cb) = 0;
	virtual void open(int deviceIndex) = 0;
	virtual void close() = 0;
	virtual void read() = 0;
	//virtual void asyncRead() = 0;
};
