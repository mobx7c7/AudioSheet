#pragma once

//Sources:
//https://www.gamedev.net/forums/topic/624876-how-to-read-an-audio-file-with-ffmpeg-in-c/

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/fifo.h>
}

#include <functional>
#include <string>

struct SheetFrame
{
	uint8_t*
		data;
	size_t
		size;
	int
		samples,
		sampleRate,
		sampleSize,
		channels;
	friend std::ostream& operator<<(std::ostream&, const SheetFrame&);
};

class SheetStream
{
public:
	typedef std::function<void(const std::string&)> 
		ErrorCallback;
	typedef std::function<void(const SheetFrame&)> 
		FrameCallback;
	typedef std::function<void()> 
		FinishCallback;
private:
	AVFrame
		*frame;
	AVStream
		*stream;
	AVCodecContext
		*codecContext;
	AVFormatContext
		*formatContext;
	ErrorCallback
		errorCallback;
	FrameCallback
		frameCallback;
	FinishCallback
		finishCallback;
	bool
		setupDone = false;
private:
	void printConfiguration();
	void printDecoders();
	void printFormatInfo();
	void printFrameInfo(const AVFrame* frame);
private:
	void reset();
	void cleanup();
	void readPackets();
	void decodePacket(AVPacket packet);
public:
	SheetStream();
	~SheetStream();
	void setup();
	void setErrorCallback(ErrorCallback cb);
	void setFrameCallback(FrameCallback cb);
	void setFinishCallback(FinishCallback cb);
	void load(std::string filePath);
	void next();
	//int getSampleRate() { return codecContext->sample_rate; }
	//int getChannels() { return codecContext->channels; }
};