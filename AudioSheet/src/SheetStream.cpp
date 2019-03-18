#include "SheetStream.h"
#include <ofLog.h>
#include <ofUtils.h>

std::ostream& operator<<(std::ostream& os, const SheetFrame& frame)
{
	return os
		<< "size=" << frame.size
		<< ", samples=" << frame.samples
		<< ", channels=" << frame.channels
		<< ", sampleRate=" << frame.sampleRate
		<< ", sampleSize=" << frame.sampleSize;
}

ofLogLevel getLogLevel(int level)
{
	if (level > AV_LOG_INFO)
		//AV_LOG_VERBOSE
		//AV_LOG_DEBUG
		//AV_LOG_TRACE
		return ofLogLevel::OF_LOG_VERBOSE;
	else if (level > AV_LOG_WARNING)
		//AV_LOG_INFO
		return ofLogLevel::OF_LOG_NOTICE;
	else if (level > AV_LOG_ERROR)
		//AV_LOG_WARNING
		return ofLogLevel::OF_LOG_WARNING;
	else if (level > AV_LOG_FATAL)
		//AV_LOG_ERROR
		return ofLogLevel::OF_LOG_ERROR;
	else if (level > AV_LOG_PANIC)
		//AV_LOG_FATAL
		return ofLogLevel::OF_LOG_FATAL_ERROR;
	else if (level > AV_LOG_QUIET)
		//AV_LOG_PANIC
		return ofLogLevel::OF_LOG_FATAL_ERROR;
	else
		//AV_LOG_QUIET
		return ofLogLevel::OF_LOG_SILENT;
}

std::string getErrorString(int code)
{
	std::vector<char> err(AV_ERROR_MAX_STRING_SIZE);
	if (av_strerror(code, err.data(), err.size()) == 0)
	{
		return err.data();
	}
	return "";
}

void avLogOutput(void *ptr, int level, const char *fmt, va_list vargs)
{
	ofLog(getLogLevel(level), fmt, vargs);
}

SheetStream::SheetStream()
{}

SheetStream::~SheetStream()
{}

void SheetStream::printConfiguration()
{
	std::istringstream iss(avcodec_configuration());
	std::vector<std::string> result(
		std::istream_iterator<std::string>{iss},
		std::istream_iterator<std::string>());

	ofLogVerbose() << "[AVCODEC CONFIGURATION]";
	std::for_each(result.begin(), result.end(), [&](std::string &param)
	{
		ofLogVerbose() << param;
	});
}

void SheetStream::printDecoders()
{
	ofLogVerbose() << "[AVAILABLE DECODERS]";

	void* i = nullptr;

	const AVCodec *codec;

	while ((codec = av_codec_iterate(&i)))
	{
		if (av_codec_is_decoder(codec) && codec->type == AVMEDIA_TYPE_AUDIO)
		{
			ofLogVerbose() << "- " << codec->name;
		}
	}
}

void SheetStream::printFormatInfo()
{
	if (ofGetLogLevel() > OF_LOG_NOTICE)
		return;

	ofLogNotice() << "[FORMAT INFO]";
	ofLogNotice() << "- Codec: " << avcodec_get_name(codecContext->codec_id);
	ofLogNotice() << "- Channels: " << codecContext->channels << "ch";
	ofLogNotice() << "- Sample rate: " << codecContext->sample_rate << "Hz";
	ofLogNotice() << "- Sample format: " << av_get_sample_fmt_name(codecContext->sample_fmt);
}

void SheetStream::printFrameInfo(const AVFrame* frame)
{
	if (ofGetLogLevel() > OF_LOG_NOTICE)
		return;

	ofLogNotice() << "[FRAME INFO]";
	ofLogNotice() << "- Samples: " << frame->nb_samples;
	ofLogNotice() << "- Channels: " << codecContext->channels;
	ofLogNotice() << "- Format: " << av_get_sample_fmt_name(codecContext->sample_fmt);
	ofLogNotice() << "- Bytes/sample: " << av_get_bytes_per_sample(codecContext->sample_fmt);
	ofLogNotice() << "- Is planar?: " << av_sample_fmt_is_planar(codecContext->sample_fmt);
}

void SheetStream::cleanup()
{
	if (frame)
		av_free(frame);
	if (codecContext)
		avcodec_free_context(&codecContext);
	if (formatContext)
		avformat_close_input(&formatContext);

	reset();
}

void SheetStream::reset()
{
	frame = nullptr;
	stream = nullptr;
	codecContext = nullptr;
	formatContext = nullptr;
	finished = false;
}

void SheetStream::readNextFrame()
{
	int result = av_read_frame(formatContext, &readingPacket);

	switch (result)
	{
		case 0:
			if (readingPacket.stream_index == stream->index)
			{
				decodePacket(readingPacket);
			}
			// Free on every av_read_frame() to avoid memory leak
			av_packet_unref(&readingPacket);
			break;
		case AVERROR_EOF:
			if (codecContext->codec->capabilities & AV_CODEC_CAP_DELAY)
			{
				av_init_packet(&readingPacket);
				decodePacket(readingPacket);
			}
			finished = true;
			break;
		default:
			throw std::exception(("Error while reading packets: " + getErrorString(result)).c_str());
	}
}

void SheetStream::decodePacket(AVPacket packet)
{
	int result;

	if ((result = avcodec_send_packet(codecContext, &packet)) < 0)
	{
		throw std::exception("Error sending packet for decoding");
	}

	SheetFrame sheetFrame;

	while ((result = avcodec_receive_frame(codecContext, frame)) == 0)
	{
		std::stringstream ss;
		ss << std::fixed << std::setprecision(8);
		ss
			<< (frame->pkt_duration * av_q2d(stream->time_base)) << '\t'
			<< (frame->pkt_pts * av_q2d(stream->time_base)) << '\t'
			<< (frame->pkt_dts * av_q2d(stream->time_base)) << '\t'
			<< (frame->pkt_pos * av_q2d(stream->time_base)) << '\t';
		ofLogNotice() << ss.str();

		sheetFrame.data = frame->data[0];
		sheetFrame.size = frame->linesize[0];
		sheetFrame.samples = frame->nb_samples;
		sheetFrame.sampleRate = codecContext->sample_rate;
		sheetFrame.sampleSize = av_get_bytes_per_sample(codecContext->sample_fmt);
		sheetFrame.channels = codecContext->channels;

		if (frameCallback)
			frameCallback(sheetFrame);
	}
}

void SheetStream::setup()
{
	if (!setted)
	{
		printConfiguration();
		printDecoders();
		reset();
		av_log_set_callback(avLogOutput);
		setted = true;
	}
}

void SheetStream::setErrorCallback(ErrorCallback cb)
{
	errorCallback = cb;
}

void SheetStream::setFrameCallback(FrameCallback cb)
{
	frameCallback = cb;
}

void SheetStream::setFinishCallback(FinishCallback cb)
{
	finishCallback = cb;
}

void SheetStream::load(std::string filepath)
{
	if (!setted)
	{
		ofLogError() << "Setup wasn't made";
	}
	else if (formatContext)
	{
		return;
	}
	else
	{
		try
		{
			reset();

			AVCodec* decoder;
			int result, streamIndex;

			if (!(formatContext = avformat_alloc_context()))
			{
				throw std::exception("Could not create format context");
			}

			if ((result = avformat_open_input(&formatContext, filepath.c_str(), nullptr, nullptr)) < 0)
			{
				throw std::exception(("Error opening input file: " + getErrorString(result)).c_str());
			}

			if ((result = avformat_find_stream_info(formatContext, NULL)) < 0)
			{
				throw std::exception(("Error finding stream info: " + getErrorString(result)).c_str());
			}

			if ((streamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, &decoder, 0)) < 0)
			{
				throw std::exception("Could not find any audio stream");
			}

			stream = formatContext->streams[streamIndex];
			codecContext = avcodec_alloc_context3(decoder);
			avcodec_parameters_to_context(codecContext, stream->codecpar);

			if ((result = avcodec_open2(codecContext, codecContext->codec, NULL)) < 0)
			{
				throw std::exception(("Could not open decoder context: " + getErrorString(result)).c_str());
			}

			if (!(frame = av_frame_alloc()))
			{
				throw std::exception("Error allocating frame");
			}

			printFormatInfo();

			av_init_packet(&readingPacket);
		}
		catch (const std::exception& e)
		{
			if (errorCallback)
				errorCallback(std::string(e.what()));

			close();
		}
	}
}

void SheetStream::next()
{
	if (!finished)
	{
		
		try
		{
			ofLogVerbose() << "Reading frame";

			readNextFrame();

			if (finished && finishCallback)
				finishCallback();
		}
		catch (const std::exception& e)
		{
			if (errorCallback)
				errorCallback(std::string(e.what()));

			close();
		}
	}
}

void SheetStream::close()
{
	if (formatContext)
	{
		cleanup();
	}
}

bool SheetStream::isFinished()
{
	return finished;
}