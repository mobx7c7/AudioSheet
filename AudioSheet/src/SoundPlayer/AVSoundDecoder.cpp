#include "AVSoundDecoder.h"
#include "AVUtils.hpp"

/*
AVSoundDecoder::AVSoundDecoder()
	: m_Initialized(false)
{
	reset();
}

AVSoundDecoder::~AVSoundDecoder()
{
	cleanup();
}

void AVSoundDecoder::printBuildConfig()
{
	ofLogVerbose() << "[AVCODEC BUILD CONFIG]";

	using iterator_type = std::istream_iterator<std::string>;

	std::istringstream iss(avcodec_configuration());
	std::vector<std::string> result(iterator_type{iss}, iterator_type());

	std::for_each(result.begin(), result.end(), [&](std::string &param)
	{
		ofLogVerbose() << param;
	});
}

void AVSoundDecoder::printDecoders()
{
	ofLogVerbose() << "[AVCODEC DECODERS]";

	void* i = nullptr;

	const AVCodec *codec;

	while ((codec = av_codec_iterate(&i)))
	{
		if (av_codec_is_decoder(codec) && codec->type == AVMEDIA_TYPE_AUDIO)
		{
			ofLog(ofLogLevel::OF_LOG_VERBOSE, "%s", codec->name);
		}
	}
}

void AVSoundDecoder::printMetadata()
{
	ofLogVerbose() << "[STREAM METADATA]";

	AVDictionaryEntry *tag = nullptr;

	while ((tag = av_dict_get(m_FormatContext->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
	{
		ofLog(ofLogLevel::OF_LOG_VERBOSE, "%s=%s", tag->key, tag->value);
	}
}

void AVSoundDecoder::cleanup()
{
	if (m_Frame)
		av_frame_free(&m_Frame);

	if (m_CodecContext)
		avcodec_free_context(&m_CodecContext);

	if (m_FormatContext)
		avformat_close_input(&m_FormatContext);

	reset();
}

void AVSoundDecoder::reset()
{
	m_CodecContext = nullptr;
	m_FormatContext = nullptr;
	m_Stream = nullptr;
	m_Frame = nullptr;
	m_StrReadFrmRet = 0;
	m_DecSendPktRet = 0;
	m_DecRecvFrmRet = 0;
	m_Resampler = AVSoundResampler(44100, AV_SAMPLE_FMT_FLT, AV_CH_LAYOUT_STEREO);
}

void AVSoundDecoder::readPacket()
{
	if (m_StrReadFrmRet == AVERROR_EOF)
	{
		return;
	}

	av_init_packet(&m_Packet);

	// 1. Retrieve packet from stream
	if ((m_StrReadFrmRet = av_read_frame(m_FormatContext, &m_Packet)) == 0)
	{
		if (m_Packet.stream_index == m_Stream->index)
		{
			// 2. Decode current packet
			if ((m_DecSendPktRet = avcodec_send_packet(m_CodecContext, &m_Packet)) != 0)
			{
				throw AVSoundDecoderExc(this, "Error sending packet for decoding", m_DecSendPktRet);
			}
		}
		// Free on every av_read_frame() to avoid memory leak
		av_packet_unref(&m_Packet);
	}
	else if (m_StrReadFrmRet == AVERROR_EOF)
	{
		// Flush remaining frames
		if (m_CodecContext->codec->capabilities & AV_CODEC_CAP_DELAY)
		{
			av_init_packet(&m_Packet);

			m_DecSendPktRet = avcodec_send_packet(m_CodecContext, &m_Packet);
		}
	}
	else
	{
		throw AVSoundDecoderExc(this, "Error while reading packets", m_StrReadFrmRet);
	}
}

void AVSoundDecoder::setup()
{
	if (!m_Initialized)
	{
		printBuildConfig();
		printDecoders();
		//av_log_set_callback(avLogOutput);
		av_log_set_level(AV_LOG_TRACE);
		m_Initialized = true;
	}
}

void AVSoundDecoder::load(std::string filepath)
{
	if (!m_Initialized)
	{
		throw AVSoundDecoderExc(this, "Decoder not initialized");
	}
	if (!isLoaded())
	{
		reset();

		AVCodec* decoder;
		int ret, streamIndex;

		if (!(m_FormatContext = avformat_alloc_context()))
		{
			throw AVSoundDecoderExc(this, "Could not create format context");
		}

		if ((ret = avformat_open_input(&m_FormatContext, filepath.c_str(), nullptr, nullptr)) < 0)
		{
			throw AVSoundDecoderExc(this, "Could not open input file", ret);
		}

		if ((ret = avformat_find_stream_info(m_FormatContext, nullptr)) < 0)
		{
			throw AVSoundDecoderExc(this, "Could not find stream info", ret);
		}

		if ((streamIndex = av_find_best_stream(m_FormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, &decoder, 0)) < 0)
		{
			throw AVSoundDecoderExc(this, "Could not find any audio stream");
		}

		m_Stream = m_FormatContext->streams[streamIndex];

		m_CodecContext = avcodec_alloc_context3(decoder);

		avcodec_parameters_to_context(m_CodecContext, m_Stream->codecpar);

		if ((ret = avcodec_open2(m_CodecContext, m_CodecContext->codec, nullptr)) < 0)
		{
			throw AVSoundDecoderExc(this, "Could not open decoder context", ret);
		}

		if (!(m_Frame = av_frame_alloc()))
		{
			throw AVSoundDecoderExc(this, "Could not allocate frame");
		}

		//FIXME: Quebra na impressão dos metadados
		printMetadata();



		const int RESAMPLER_BUFSIZE = 16384; // FIXME: Atribuir valor padronizado
		m_Resampler.init(RESAMPLER_BUFSIZE, getSampleRate(), getSampleFormat(), getChannelLayout());
	}
}

void AVSoundDecoder::seek(double secs)
{
	AVStream* stream = m_Stream; // mesmo que m_FormatContext->streams[0]

	if (stream)
	{
		int ret;

		auto target_ts = static_cast<int64_t>(secs * stream->time_base.den / stream->time_base.num);

		target_ts += stream->cur_dts; // relative

		int flags = AVSEEK_FLAG_ANY;

		if (ret = av_seek_frame(m_FormatContext, stream->index, target_ts, flags))
		{
			throw AVSoundDecoderExc(this, "Error while seeking frame");
		}

		avcodec_flush_buffers(m_CodecContext);
	}
}

bool AVSoundDecoder::next()
{
	if (m_StrReadFrmRet == AVERROR_EOF)
	{
		return false;
	}

	// 3. Retrieve frame from last decoded packet
	if ((m_DecRecvFrmRet = avcodec_receive_frame(m_CodecContext, m_Frame)) == AVERROR(EAGAIN))
	{
		readPacket();

		return next();
	}

	m_Resampler.write(const_cast<const uint8_t**>(m_Frame->data), getFrameSamples());

	return true;
}

int AVSoundDecoder::read(uint8_t** data, int size, int planes)
{
	if (m_DecRecvFrmRet == 0 && m_Frame->nb_samples > 0)
	{
		//planes = isPlanar() ? std::min(planes, m_Frame->channels) : 1;

		//// For audio, only linesize[0] may be set.
		//// For planar audio, each channel plane must be the same size.
		
		//int sizeNeeded = std::min(size, m_Frame->linesize[0]);

		//for (int x = 0; x < planes; x++)
		//{
		//	std::copy_n(m_Frame->data[x], sizeNeeded, data[x]);
		//	//m_Frame->data[x] += sizeUsed;
		//}

		////m_Frame->linesize[0] -= sizeUsed;

		//return sizeNeeded;

		return m_Resampler.read(data, size, planes);
	}

	return 0;
}

void AVSoundDecoder::unload()
{
	if (m_FormatContext)
		cleanup();
}

AVSampleFormat AVSoundDecoder::getSampleFormat()
{ 
	return m_CodecContext->sample_fmt; 
}

uint64_t AVSoundDecoder::getChannelLayout() 
{ 
	return m_CodecContext->channel_layout;
}

int AVSoundDecoder::getNumChannels()
{
	return m_CodecContext->channels;
}

int AVSoundDecoder::getNumPlanes()
{ 
	return isPlanar() ? getNumChannels() : 1; 
}

int AVSoundDecoder::getSampleRate()
{
	return m_CodecContext->sample_rate;
}

int AVSoundDecoder::getSampleSize()
{
	return av_get_bytes_per_sample(m_CodecContext->sample_fmt);
}

int AVSoundDecoder::getFrameSize()
{
	return m_DecRecvFrmRet == 0 && m_Frame ? m_Frame->linesize[0] : 0;
}

int AVSoundDecoder::getFrameSamples()
{
	return m_DecRecvFrmRet == 0 && m_Frame ? m_Frame->nb_samples : 0;
}

const char* AVSoundDecoder::getCodecName()
{
	return avcodec_get_name(m_CodecContext != nullptr ? m_CodecContext->codec_id : AV_CODEC_ID_NONE);
}

int64_t AVSoundDecoder::getDuration()
{
	return m_Stream ? m_Stream->duration : 0;
}

int64_t AVSoundDecoder::getPosition()
{
	return m_Frame ? m_Frame->pkt_dts : 0;
}

bool AVSoundDecoder::isFixedPoint()
{
	if (!m_CodecContext) return false;
	return isSampleFormatFixedPoint(m_CodecContext->sample_fmt);
}

bool AVSoundDecoder::isFloatingPoint()
{
	if (!m_CodecContext) return false;
	return isSampleFormatFloatingPoint(m_CodecContext->sample_fmt);
}

bool AVSoundDecoder::isSigned()
{
	if (!m_CodecContext) return false;
	return isSampleFormatSigned(m_CodecContext->sample_fmt);
}

bool AVSoundDecoder::isUnsigned()
{
	if (!m_CodecContext) return false;
	return isSampleFormatUnsigned(m_CodecContext->sample_fmt);
}

bool AVSoundDecoder::isPlanar()
{
	if (m_CodecContext) return false;
	return isSampleFormatPlanar(m_CodecContext->sample_fmt);
}

bool AVSoundDecoder::isLoaded()
{
	return m_CodecContext && m_FormatContext && m_Resampler.isInitialized();
}

bool AVSoundDecoder::isFinished()
{
	return m_StrReadFrmRet == AVERROR_EOF;
}

uint64_t AVSoundDecoder::getOutChannelFormat() { return m_Resampler.getChannelLayout(); }

AVSampleFormat AVSoundDecoder::getOutSampleFormat() { return m_Resampler.getSampleFormat(); }

int AVSoundDecoder::getOutNumChannels() { return m_Resampler.getNumChannels(); }

int AVSoundDecoder::getOutNumSamples() { return m_Resampler.getNumSamples(); }

int AVSoundDecoder::getOutNumPlanes() { return m_Resampler.getNumPlanes(); }

int AVSoundDecoder::getOutSampleRate() { return m_Resampler.getSampleRate(); }

int AVSoundDecoder::getOutSampleSize() { return m_Resampler.getSampleSize(); }

int AVSoundDecoder::getOutFrameSize() { return m_Resampler.getFrameSize(); }

*/















AVSoundDecoderExc::AVSoundDecoderExc(AVSoundDecoder2 * decoder, const char * msg, int err)
	: AVSoundException(msg, err)
	, m_Decoder(decoder)
{

}

AVSoundDecoder2 * AVSoundDecoderExc::getDecoder() { return m_Decoder; }

AVSoundDecoderLoadExc::AVSoundDecoderLoadExc(AVSoundDecoder2 * decoder, const char * msg, int err)
	: AVSoundDecoderExc(decoder, msg, err)
{
	decoder->close();
}

AVSoundDecoder2::AVSoundDecoder2()
	: m_Initialized(false)
{
	reset();
}

AVSoundDecoder2::~AVSoundDecoder2()
{
	cleanup();
}

void AVSoundDecoder2::printMetadata()
{
	if (isLoaded())
	{
		ofLogVerbose() << "[METADATA]";

		AVDictionaryEntry *tag = nullptr;

		while ((tag = av_dict_get(m_FormatContext->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
		{
			ofLog(ofLogLevel::OF_LOG_VERBOSE, "  %s=%s", tag->key, tag->value);
		}
	}
}

void AVSoundDecoder2::cleanup()
{
	if (m_Frame)
		av_frame_free(&m_Frame);

	if (m_CodecContext)
		avcodec_free_context(&m_CodecContext);

	if (m_FormatContext)
		avformat_close_input(&m_FormatContext);

	reset();
}

void AVSoundDecoder2::reset()
{
	m_CodecContext = nullptr;
	m_FormatContext = nullptr;
	m_Stream = nullptr;
	m_Frame = nullptr;
	m_StrReadFrmRet = 0;
	m_DecSendPktRet = 0;
	m_DecRecvFrmRet = 0;
}

void AVSoundDecoder2::readPacket()
{
	if (m_StrReadFrmRet == AVERROR_EOF)
	{
		return;
	}

	av_init_packet(&m_Packet);

	// 1. Retrieve packet from stream
	if ((m_StrReadFrmRet = av_read_frame(m_FormatContext, &m_Packet)) == 0)
	{
		if (m_Packet.stream_index == m_Stream->index)
		{
			// 2. Decode current packet
			if ((m_DecSendPktRet = avcodec_send_packet(m_CodecContext, &m_Packet)) != 0)
			{
				throw AVSoundDecoderExc(this, "Error sending packet for decoding", m_DecSendPktRet);
			}
		}
		// Free on every av_read_frame() to avoid memory leak
		av_packet_unref(&m_Packet);
	}
	else if (m_StrReadFrmRet == AVERROR_EOF)
	{
		// Flush remaining frames
		if (m_CodecContext->codec->capabilities & AV_CODEC_CAP_DELAY)
		{
			av_init_packet(&m_Packet);

			m_DecSendPktRet = avcodec_send_packet(m_CodecContext, &m_Packet);
		}
	}
	else
	{
		throw AVSoundDecoderExc(this, "Error while reading packets", m_StrReadFrmRet);
	}
}

void AVSoundDecoder2::setup()
{
	if (!m_Initialized)
	{
		//av_register_all(); //obsoleto desde v4.0 (só para lembrar)
		//printBuildConfig();
		//printDecoders();
		//printEncoders();
		//printMuxers();
		//printDemuxers();
		m_Initialized = true;
	}
}

void AVSoundDecoder2::load(std::string filepath)
{
	if (!m_Initialized)
	{
		throw AVSoundDecoderExc(this, "Not initialized");
	}
	if (!isLoaded())
	{
		reset();

		int ret;
		AVCodec* decoder;

		if (!(m_FormatContext = avformat_alloc_context()))
		{
			throw AVSoundDecoderLoadExc(this, "Could not create format context");
		}

		if ((ret = avformat_open_input(&m_FormatContext, filepath.c_str(), nullptr, nullptr)) < 0)
		{
			throw AVSoundDecoderLoadExc(this, "Could not open input file", ret);
		}

		if ((ret = avformat_find_stream_info(m_FormatContext, nullptr)) < 0)
		{
			throw AVSoundDecoderLoadExc(this, "Could not find stream info", ret);
		}

		if ((ret = av_find_best_stream(m_FormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, &decoder, 0)) >= 0)
		{
			m_Stream = m_FormatContext->streams[ret];
		}
		else
		{ 
			throw AVSoundDecoderLoadExc(this, "Could not find any audio stream", ret);
		}

		if ((m_CodecContext = avcodec_alloc_context3(decoder)))
		{
			avcodec_parameters_to_context(m_CodecContext, m_Stream->codecpar);
		}
		else
		{
			throw AVSoundDecoderLoadExc(this, "Could not create codec context");
		}

		if ((ret = avcodec_open2(m_CodecContext, m_CodecContext->codec, nullptr)) < 0)
		{
			throw AVSoundDecoderLoadExc(this, "Could not open decoder context", ret);
		}

		if (!(m_Frame = av_frame_alloc()))
		{
			throw AVSoundDecoderLoadExc(this, "Could not allocate frame");
		}
		
		printMetadata();
	}
}

void AVSoundDecoder2::seek(double secs, bool relative)
{
	if (!(m_Stream && m_Frame)) return;

	int ret = 0;

	auto target_ts = av_rescale_q(
		static_cast<int64_t>(secs * AV_TIME_BASE), 
		av_get_time_base_q(), // Soluciona problema do AV_TIME_BASE_Q em código C++
		m_Stream->time_base);
	
	if (relative)
		//target_ts += m_Stream->cur_dts; //Atualizado somente ao utilizar 'seek' e valor não é inicializado ao carregar.
		target_ts += m_Frame->pkt_dts;

	if ((ret = av_seek_frame(m_FormatContext, m_Stream->index, target_ts, AVSEEK_FLAG_ANY)) < 0)
	{
		throw AVSoundDecoderExc(this, "Error while seeking frame", ret);
	}

	avcodec_flush_buffers(m_CodecContext);
}

bool AVSoundDecoder2::next()
{
	if (m_StrReadFrmRet == AVERROR_EOF)
	{
		return false;
	}
	// 3. Retrieve frame from last decoded packet
	if ((m_DecRecvFrmRet = avcodec_receive_frame(m_CodecContext, m_Frame)) == AVERROR(EAGAIN))
	{
		readPacket();
		return next();
	}

	return true;
}

int AVSoundDecoder2::read(uint8_t** data, int size, int planes)
{
	if (m_DecRecvFrmRet == 0 && m_Frame->nb_samples > 0)
	{
		// For audio, only linesize[0] may be set.
		// For planar audio, each channel plane must be the same size.
		size = std::min(size, m_Frame->linesize[0]);

		planes = std::min(planes, getNumPlanes());

		for (int p = 0; p < planes; p++)
		{
			std::copy_n(m_Frame->data[p], size, data[p]);
		}

		return size;
	}

	return 0;
}

void AVSoundDecoder2::close()
{
	if (isLoaded()) cleanup();
}

const char* AVSoundDecoder2::getFormatName()
{
	//return avcodec_get_name(m_CodecContext ? m_CodecContext->codec_id : AV_CODEC_ID_NONE);
	return isLoaded() ? m_FormatContext->iformat->name : 0;
}

AVSampleFormat AVSoundDecoder2::getSampleFormat()
{
	return isLoaded() ? m_CodecContext->sample_fmt : AV_SAMPLE_FMT_NONE;
}

uint64_t AVSoundDecoder2::getChannelLayout()
{
	if (!isLoaded())
		return 0;
	
	if (m_CodecContext->channel_layout)
		return m_CodecContext->channel_layout;

	// The value 0 means that the channel layout is not known.
	return av_get_default_channel_layout(m_CodecContext->channels);
}

int AVSoundDecoder2::getNumChannels()
{
	return isLoaded() ? m_CodecContext->channels : 0;
}

int AVSoundDecoder2::getNumPlanes()
{
	return isPlanar() ? getNumChannels() : 1;
}

int AVSoundDecoder2::getSampleRate()
{
	return isLoaded() ? m_CodecContext->sample_rate : 0;
}

int AVSoundDecoder2::getSampleSize()
{
	return isLoaded() ? av_get_bytes_per_sample(m_CodecContext->sample_fmt) : 0;
}

int AVSoundDecoder2::getFrameSize()
{
	return m_DecRecvFrmRet == 0 && m_Frame ? m_Frame->linesize[0] : 0;
}

int AVSoundDecoder2::getFrameSamples()
{
	return m_DecRecvFrmRet == 0 && m_Frame ? m_Frame->nb_samples : 0;
}

int64_t AVSoundDecoder2::getDuration()
{
	if (!(m_DecRecvFrmRet == 0 && m_Stream)) 
		return 0;
	else
		return av_rescale_q(m_Stream->duration, m_Stream->time_base, m_CodecContext->time_base);
}

int64_t AVSoundDecoder2::getPosition()
{
	if (!(m_DecRecvFrmRet == 0 && m_Frame)) 
		return 0;
	else
		return av_rescale_q(m_Frame->pts, m_Stream->time_base, m_CodecContext->time_base);
}

bool AVSoundDecoder2::isFixedPoint()
{
	return isLoaded() ? isSampleFormatFixedPoint(m_CodecContext->sample_fmt) : false;
}

bool AVSoundDecoder2::isFloatingPoint()
{
	return isLoaded() ? isSampleFormatFloatingPoint(m_CodecContext->sample_fmt) : false;
}

bool AVSoundDecoder2::isSigned()
{
	return isLoaded() ? isSampleFormatSigned(m_CodecContext->sample_fmt) : false;
}

bool AVSoundDecoder2::isUnsigned()
{
	return isLoaded() ? isSampleFormatUnsigned(m_CodecContext->sample_fmt) : false;
}

bool AVSoundDecoder2::isPlanar()
{
	return isLoaded() ? isSampleFormatPlanar(m_CodecContext->sample_fmt) : false;
}

bool AVSoundDecoder2::isLoaded()
{
	return m_CodecContext && m_FormatContext;
}

bool AVSoundDecoder2::isFinished()
{
	return isLoaded() ? m_StrReadFrmRet == AVERROR_EOF : false;
}

