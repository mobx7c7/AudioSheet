#pragma once
extern "C"{
#include <libswresample/swresample.h>
}
#include "AVSoundException.h"

class AVSoundResampler;

class AVSoundResamplerExc : public AVSoundException
{
	AVSoundResampler* m_Resampler;

public:
	AVSoundResamplerExc(AVSoundResampler *resampler, const char* msg, int err = 0);
	AVSoundResampler* getResampler();
};

class AVSoundResampler
{
	uint64_t		src_ch_layout;
	AVSampleFormat	src_sample_fmt;
	int				src_sample_rate;
	int				src_nb_channels;
	int				src_nb_samples_max;
	uint64_t		dst_ch_layout;
	AVSampleFormat	dst_sample_fmt;
	int				dst_sample_rate;
	int				dst_sample_size;
	int				dst_nb_channels;
	int				dst_nb_samples_max;
	int				dst_linesize;
	uint8_t**		dst_data; // planes
	int				dst_conv_samples; // available samples
	int				dst_conv_bufsize; // available buffer
	SwrContext*		swr_ctx;

	void dst_resize_buf(int new_size);

public:
	AVSoundResampler();
	AVSoundResampler(int sample_rate, AVSampleFormat sample_fmt, uint64_t ch_layout);
	~AVSoundResampler();
	void			reset();
	void			init(int nb_samples, int sample_rate, AVSampleFormat sample_fmt, uint64_t ch_layout);
	int				write(const uint8_t** data, int samples);
	int				read(uint8_t** data, int size, int planes);
	uint64_t		getChannelLayout();
	AVSampleFormat	getSampleFormat();
	int				getSampleRate();
	int				getSampleSize();
	int				getNumSamples();
	int				getNumChannels();
	int				getNumPlanes();
	int				getFrameSize();
	int				getAvailableSize();
	int				getAvailableSamples();
	bool			isFixedPoint();
	bool			isFloatingPoint();
	bool			isSigned();
	bool			isUnsigned();
	bool			isPlanar();
	bool			isInitialized();
};
