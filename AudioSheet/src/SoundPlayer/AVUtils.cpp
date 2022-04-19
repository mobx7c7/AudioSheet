#include "AVUtils.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

ofLogLevel getLogLevel(int level)
{
	//AV_LOG_VERBOSE, AV_LOG_DEBUG, AV_LOG_TRACE
	if (level > AV_LOG_INFO)
		return ofLogLevel::OF_LOG_VERBOSE;
	//AV_LOG_INFO
	if (level > AV_LOG_WARNING)
		return ofLogLevel::OF_LOG_NOTICE;
	//AV_LOG_WARNING
	if (level > AV_LOG_ERROR)
		return ofLogLevel::OF_LOG_WARNING;
	//AV_LOG_ERROR
	if (level > AV_LOG_FATAL)
		return ofLogLevel::OF_LOG_ERROR;
	//AV_LOG_FATAL
	if (level > AV_LOG_PANIC)
		return ofLogLevel::OF_LOG_FATAL_ERROR;
	//AV_LOG_PANIC
	if (level > AV_LOG_QUIET)
		return ofLogLevel::OF_LOG_FATAL_ERROR;

	//AV_LOG_QUIET
	return ofLogLevel::OF_LOG_SILENT;
}

bool isSampleFormatFixedPoint(AVSampleFormat format)
{
	switch (format)
	{
		case AV_SAMPLE_FMT_U8:
		case AV_SAMPLE_FMT_U8P:
		case AV_SAMPLE_FMT_S16:
		case AV_SAMPLE_FMT_S16P:
		case AV_SAMPLE_FMT_S32:
		case AV_SAMPLE_FMT_S32P:
		case AV_SAMPLE_FMT_S64:
		case AV_SAMPLE_FMT_S64P:
			return true;
		default:
			return false;
	}
}

bool isSampleFormatFloatingPoint(AVSampleFormat format)
{
	switch (format)
	{
		case AV_SAMPLE_FMT_FLT:
		case AV_SAMPLE_FMT_FLTP:
		case AV_SAMPLE_FMT_DBL:
		case AV_SAMPLE_FMT_DBLP:
			return true;
		default:
			return false;
	}
}

bool isSampleFormatSigned(AVSampleFormat format)
{
	switch (format)
	{
		case AV_SAMPLE_FMT_S16:
		case AV_SAMPLE_FMT_S16P:
		case AV_SAMPLE_FMT_S32:
		case AV_SAMPLE_FMT_S32P:
		case AV_SAMPLE_FMT_S64:
		case AV_SAMPLE_FMT_S64P:
		case AV_SAMPLE_FMT_FLT:
		case AV_SAMPLE_FMT_FLTP:
		case AV_SAMPLE_FMT_DBL:
		case AV_SAMPLE_FMT_DBLP:
			return true;
		default:
			return false;
	}
}

bool isSampleFormatUnsigned(AVSampleFormat format)
{
	switch (format)
	{
		case AV_SAMPLE_FMT_U8:
		case AV_SAMPLE_FMT_U8P:
			return true;
		default:
			return false;
	}
}

bool isSampleFormatPlanar(AVSampleFormat format)
{
	return av_sample_fmt_is_planar(format) == 1;
}


void printBuildConfig()
{
	ofLogVerbose() << "[AVCODEC CONFIG]";

	using iterator_type = std::istream_iterator<std::string>;

	std::istringstream iss(avcodec_configuration());
	std::vector<std::string> result(iterator_type{ iss }, iterator_type());

	std::for_each(result.begin(), result.end(), [&](std::string &param)
	{
		ofLog(ofLogLevel::OF_LOG_VERBOSE, "  %s", param.c_str());
	});
}

void printEncoders()
{
	ofLogVerbose() << "[AVCODEC ENCODERS]";

	void* i = nullptr;

	const AVCodec *codec;

	while ((codec = av_codec_iterate(&i)))
	{
		if (av_codec_is_encoder(codec) && codec->type == AVMEDIA_TYPE_AUDIO)
		{
			ofLog(ofLogLevel::OF_LOG_VERBOSE, "  %s", codec->name);
		}
	}
}

void printDecoders()
{
	ofLogVerbose() << "[AVCODEC DECODERS]";

	void* i = nullptr;

	const AVCodec *codec;

	while ((codec = av_codec_iterate(&i)))
	{
		if (av_codec_is_decoder(codec) && codec->type == AVMEDIA_TYPE_AUDIO)
		{
			ofLog(ofLogLevel::OF_LOG_VERBOSE, "  %s", codec->name);
		}
	}
}

void printMuxers()
{
	ofLogVerbose() << "[AVCODEC DECODERS]";

	void* i = nullptr;

	const AVOutputFormat *muxer;

	while ((muxer = av_muxer_iterate(&i)))
	{
		ofLog(ofLogLevel::OF_LOG_VERBOSE, "  %s", muxer->name);
	}
}

void printDemuxers()
{
	ofLogVerbose() << "[AVCODEC DEMUXERS]";

	void* i = nullptr;

	const AVInputFormat *demuxer;

	while ((demuxer = av_demuxer_iterate(&i)))
	{
		ofLog(ofLogLevel::OF_LOG_VERBOSE, "  %s", demuxer->name);
	}
}
