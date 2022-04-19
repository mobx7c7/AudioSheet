#include "AVSoundResampler.h"

extern "C" {
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
}

#include "AVUtils.hpp"
#include <algorithm>

AVSoundResamplerExc::AVSoundResamplerExc(AVSoundResampler * resampler, const char * msg, int err)
	: AVSoundException(msg, err)
	, m_Resampler(resampler)
{}

AVSoundResampler * AVSoundResamplerExc::getResampler() { return m_Resampler; }

AVSoundResampler::AVSoundResampler()
: AVSoundResampler(0, AV_SAMPLE_FMT_NONE, 0)
{}

AVSoundResampler::AVSoundResampler(int sample_rate, AVSampleFormat sample_fmt, uint64_t ch_layout)
: dst_ch_layout(ch_layout)
, dst_sample_fmt(sample_fmt)
, dst_sample_rate(sample_rate)
, dst_data(nullptr)
, swr_ctx(nullptr)
{
	reset();
}

AVSoundResampler::~AVSoundResampler()
{
	reset();
}

void AVSoundResampler::reset()
{
	if (dst_data)
	{
		for (int p = 0; p < getNumPlanes(); p++)
			av_freep(&dst_data[p]);

		av_freep(&dst_data);
	}

	if (swr_ctx)
	{
		swr_free(&swr_ctx);
	}

	src_ch_layout		= 0;
	src_sample_fmt		= AV_SAMPLE_FMT_NONE;
	src_sample_rate		= 0;
	src_nb_channels		= 0;
	src_nb_samples_max	= 0;
	dst_sample_size		= 0;
	dst_nb_channels		= 0;
	dst_nb_samples_max	= 0;
	dst_linesize		= 0;
	dst_conv_samples	= 0;
	dst_conv_bufsize	= 0;
}

void AVSoundResampler::init(int nb_samples, int sample_rate, AVSampleFormat sample_fmt, uint64_t ch_layout)
{
	try
	{
		if (dst_sample_rate == 0 || dst_sample_fmt == AV_SAMPLE_FMT_NONE || dst_ch_layout == 0)
		{
			throw AVSoundResamplerExc(this, "Invalid output configuration");
		}

		if (sample_rate == 0 || sample_fmt == AV_SAMPLE_FMT_NONE || ch_layout == 0)
		{
			throw AVSoundResamplerExc(this, "Invalid input configuration");
		}

		if (!swr_ctx && !(swr_ctx = swr_alloc()))
		{
			throw AVSoundResamplerExc(this, "Could not allocate resampler context");
		}

		// FFmpeg Resampler Documentation
		// https://ffmpeg.org/ffmpeg-resampler.html

		av_opt_set_int(
			swr_ctx, "in_channel_layout", ch_layout, 0);
		av_opt_set_int(
			swr_ctx, "in_sample_rate", sample_rate, 0);
		av_opt_set_sample_fmt(
			swr_ctx, "in_sample_fmt", sample_fmt, 0);

		av_opt_set_int(
			swr_ctx, "out_channel_layout", dst_ch_layout, 0);
		av_opt_set_int(
			swr_ctx, "out_sample_rate", dst_sample_rate, 0);
		av_opt_set_sample_fmt(
			swr_ctx, "out_sample_fmt", dst_sample_fmt, 0);

		int ret = swr_init(swr_ctx);

		if (ret < 0)
		{
			throw AVSoundResamplerExc(this, "Failed to initialize the resampling context");
		}

		src_ch_layout		= ch_layout;
		src_sample_fmt		= sample_fmt;
		src_sample_rate		= sample_rate;
		src_nb_channels		= av_get_channel_layout_nb_channels(ch_layout);
		src_nb_samples_max	= nb_samples;

		dst_sample_size		= av_get_bytes_per_sample(dst_sample_fmt);
		dst_nb_channels		= av_get_channel_layout_nb_channels(dst_ch_layout);
		dst_nb_samples_max	= av_rescale_rnd(src_nb_samples_max, dst_sample_rate, src_sample_rate, AV_ROUND_UP);

		ret = av_samples_alloc_array_and_samples(
			&dst_data, &dst_linesize, dst_nb_channels, dst_nb_samples_max, dst_sample_fmt, 0);

		if (ret < 0)
		{
			throw AVSoundResamplerExc(this, "Could not allocate buffer");
		}
	}
	catch (AVSoundException &e)
	{
		reset();

		throw e;
	}
}

void AVSoundResampler::dst_resize_buf(int new_size)
{
	for(int p=0; p<getNumPlanes(); p++)
		av_freep(&dst_data[p]);

	int ret = av_samples_alloc(
		dst_data, &dst_linesize, dst_nb_channels, new_size, dst_sample_fmt, 1);

	if (ret < 0)
	{
		throw AVSoundResamplerExc(this, "Could not allocate samples", ret);
	}

	dst_nb_samples_max = new_size;
}

int AVSoundResampler::write(const uint8_t** src_data, int src_nb_samples)
{
	if (!isInitialized())
	{
		throw AVSoundResamplerExc(this, "Resampler not initialized");
	}

	int delay = swr_get_delay(swr_ctx, src_sample_rate);

	int dst_nb_samples = av_rescale_rnd(
		delay + src_nb_samples, dst_sample_rate, src_sample_rate, AV_ROUND_UP);

	if (dst_nb_samples > dst_nb_samples_max)
	{
		dst_resize_buf(dst_nb_samples);
	}

	// @return number of samples output per channel, negative value on error
	dst_conv_samples = swr_convert(
		swr_ctx, dst_data, dst_nb_samples, src_data, src_nb_samples);

	if (dst_conv_samples < 0)
	{
		throw AVSoundResamplerExc(this, "Error while converting audio");
	}

	dst_conv_bufsize = av_samples_get_buffer_size(
		&dst_linesize, dst_nb_channels, dst_conv_samples, dst_sample_fmt, 1);

	if (dst_conv_bufsize < 0)
	{
		throw AVSoundResamplerExc(this, "Could not get buffer size");
	}

	return dst_conv_bufsize;
}

int AVSoundResampler::read(uint8_t **data, int size, int planes)
{
	size = std::min(size, dst_conv_bufsize);

	planes = std::min(planes, getNumPlanes());

	for (int p = 0; p < planes; p++)
	{
		std::copy_n(dst_data[p], size, data[p]);
	}

	return size;
}

uint64_t AVSoundResampler::getChannelLayout()
{
	return dst_ch_layout;
}

AVSampleFormat AVSoundResampler::getSampleFormat()
{
	return dst_sample_fmt;
}

int AVSoundResampler::getSampleRate()
{
	return dst_sample_rate;
}

int AVSoundResampler::getSampleSize()
{
	return dst_sample_size;
}

int AVSoundResampler::getNumSamples()
{
	return dst_nb_samples_max;
}

int AVSoundResampler::getNumChannels()
{
	return dst_nb_channels;
}

int AVSoundResampler::getNumPlanes()
{
	return isPlanar() ? getNumChannels() : 1;
}

int AVSoundResampler::getFrameSize()
{
	return dst_linesize;
}

int AVSoundResampler::getAvailableSize()
{
	return dst_conv_bufsize;
}

int AVSoundResampler::getAvailableSamples()
{
	return dst_conv_samples;
}

bool AVSoundResampler::isFixedPoint()
{
	return isSampleFormatFixedPoint(getSampleFormat());
}

bool AVSoundResampler::isFloatingPoint()
{
	return isSampleFormatFloatingPoint(getSampleFormat());
}

bool AVSoundResampler::isSigned()
{
	return isSampleFormatSigned(getSampleFormat());
}

bool AVSoundResampler::isUnsigned()
{
	return isSampleFormatUnsigned(getSampleFormat());
}

bool AVSoundResampler::isPlanar()
{
	return isSampleFormatPlanar(getSampleFormat());
}

bool AVSoundResampler::isInitialized() 
{ 
	return swr_ctx && swr_is_initialized(swr_ctx) && dst_data;
}
