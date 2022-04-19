#pragma once

extern "C" {
#include <libavutil/samplefmt.h>
}
#include <ofLog.h>


enum class SampleFormat
{
	None, 
	U8,
	U8P,
	S16,
	S16P,
	S32,
	S32P,
	S64,
	S64P,
	FLT,
	FLTP,
	DBL,
	DBLP
};

ofLogLevel getLogLevel(int level);

bool isSampleFormatFixedPoint(AVSampleFormat format);
bool isSampleFormatFloatingPoint(AVSampleFormat format);
bool isSampleFormatSigned(AVSampleFormat format);
bool isSampleFormatUnsigned(AVSampleFormat format);
bool isSampleFormatPlanar(AVSampleFormat format);

void printBuildConfig();
void printEncoders();
void printDecoders();
void printMuxers();
void printDemuxers();