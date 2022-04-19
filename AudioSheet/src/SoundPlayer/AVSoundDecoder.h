#pragma once
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
#include "AVSoundException.h"

/*
#include "AVSoundResampler.h"

class AVSoundDecoder
{
	AVCodecContext*		m_CodecContext;
	AVFormatContext*	m_FormatContext;
	AVStream*			m_Stream;
	AVPacket			m_Packet;
	AVFrame*			m_Frame;
	int					m_StrReadFrmRet, m_DecSendPktRet, m_DecRecvFrmRet;
	bool				m_Initialized;
	AVSoundResampler	m_Resampler;

	void printBuildConfig();
	void printDecoders();
	void printMetadata();
	void reset();
	void cleanup();
	void readPacket();

public:
	AVSoundDecoder();
	virtual ~AVSoundDecoder();

	void			setup();
	void			load(std::string filePath);
	
	void			seek(double secs);
	bool			next();
	int				read(uint8_t** data, int size, int planes);
	void			unload();

	AVSampleFormat	getSampleFormat();
	uint64_t		getChannelLayout();
	int				getNumChannels();
	//int				getNumSamples(); // total samples?
	int				getNumPlanes();
	int				getSampleRate();
	int				getSampleSize();

	int				getFrameSize();
	int				getFrameSamples(); // available samples?

	const char*		getCodecName();
	int64_t			getDuration();
	int64_t			getPosition();

	bool			isFixedPoint();
	bool			isFloatingPoint();
	bool			isSigned();
	bool			isUnsigned();
	bool			isPlanar();
	bool			isLoaded();
	bool			isFinished();

	uint64_t		getOutChannelFormat();
	AVSampleFormat	getOutSampleFormat();
	int				getOutNumChannels();
	int				getOutNumSamples();
	int				getOutNumPlanes();
	int				getOutSampleRate();
	int				getOutSampleSize();
	int				getOutFrameSize();

	//int64				decoded_audio_time() { return decoded_audio_time_; }
	//int64				decoded_audio_duration() { return decoded_audio_duration_; }
	//int64				decoded_video_time() { return decoded_video_time_; }
	//int64				decoded_video_duration() { return decoded_video_duration_; }

	//AVStream*			av_audio_stream(){return av_format_context_->streams[audio_stream_index_];}
	//AVStream*			av_video_stream(){return av_format_context_->streams[video_stream_index_];}
	//AVCodecContext*	av_audio_context(){return av_audio_stream()->codec;}
	//AVCodecContext*	av_video_context(){return av_video_stream()->codec;}
};
*/

class AVSoundDecoder2;

class AVSoundDecoderExc : public AVSoundException
{
	AVSoundDecoder2* m_Decoder;

public:
	AVSoundDecoderExc(AVSoundDecoder2 *decoder, const char* msg, int err = 0);
	AVSoundDecoder2* getDecoder();
};

class AVSoundDecoderLoadExc : public AVSoundDecoderExc
{
public:
	AVSoundDecoderLoadExc(AVSoundDecoder2 *decoder, const char* msg, int err = 0);
};

class AVSoundDecoder2
{
	AVCodecContext*		m_CodecContext;
	AVFormatContext*	m_FormatContext;
	AVStream*			m_Stream;
	AVPacket			m_Packet;
	AVFrame*			m_Frame;
	int					m_StrReadFrmRet, m_DecSendPktRet, m_DecRecvFrmRet;
	bool				m_Initialized;

	void printMetadata();
	void reset();
	void cleanup();
	void readPacket();

public:
	AVSoundDecoder2();
	virtual ~AVSoundDecoder2();

	void			setup();
	void			load(std::string filePath);

	void			seek(double secs, bool relative = true);
	bool			next();
	int				read(uint8_t** data, int size, int planes);
	void			close();

	const char*		getFormatName();

	AVSampleFormat	getSampleFormat();
	uint64_t		getChannelLayout();
	int				getNumChannels();
	//int				getNumSamples(); // total samples?
	int				getNumPlanes();
	int				getSampleRate();
	int				getSampleSize();
	int				getFrameSize();		// getNumFrameBytes
	int				getFrameSamples();	// getNumFrameSamples, available samples?
	
	int64_t			getDuration();
	int64_t			getPosition();

	bool			isFixedPoint();
	bool			isFloatingPoint();
	bool			isSigned();
	bool			isUnsigned();
	bool			isPlanar();
	bool			isLoaded();
	bool			isFinished();
};
