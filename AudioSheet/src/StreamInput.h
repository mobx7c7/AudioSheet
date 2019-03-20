#pragma once

class StreamInput
{
public:
	virtual ~StreamInput() = default;
	virtual void open(int deviceIndex = 0) = 0;
	virtual void close() = 0;
	virtual void read() = 0;
};
