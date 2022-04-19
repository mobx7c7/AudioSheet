#include "AVSoundReader.h"
#include "AVUtils.hpp"

void avLogOutput(void *ptr, int level, const char *fmt, va_list vargs)
{
	ofLog(getLogLevel(level), fmt, vargs);
}

AVSoundReaderExc::AVSoundReaderExc(AVSoundReader *reader, const char * msg, int err)
	: AVSoundException(msg, err)
	, m_Reader(reader)
{}

AVSoundReader* AVSoundReaderExc::getReader() { return m_Reader; }

AVSoundReader::AVSoundReader()
	: m_Initialized(false)
{
	reset();
}

AVSoundReader::~AVSoundReader()
{
	reset();
}

void AVSoundReader::reset()
{
	
}

void AVSoundReader::setup()
{
	if (!m_Initialized)
	{
		//av_log_set_callback(avLogOutput);
		//av_log_set_level(AV_LOG_TRACE);
		m_Decoder.setup();
		m_Initialized = true;
	}
}

void AVSoundReader::load(std::string filePath)
{
	if (!m_Initialized)
	{
		throw AVSoundReaderExc(this, "Not initialized");
	}

	if (!m_Decoder.isLoaded())
	{
		m_Decoder.load(filePath);

		if (m_Decoder.isLoaded())
		{
			m_Resampler = AVSoundResampler(
				44100, 
				AV_SAMPLE_FMT_FLT, 
				AV_CH_LAYOUT_STEREO);

			m_Resampler.init(
				RESAMPLER_BUFSIZE,
				m_Decoder.getSampleRate(),
				m_Decoder.getSampleFormat(),
				m_Decoder.getChannelLayout());

			int numPlanes = m_Decoder.getNumPlanes();
			m_SwapBuffers.resize(numPlanes);
			m_SwapPlanes.resize(numPlanes);
		}
	}
}

void AVSoundReader::seek(double secs, bool relative)
{
	m_Decoder.seek(secs, relative);
}

bool AVSoundReader::next()
{
	if (m_Decoder.next())
	{
		int frameSize = m_Decoder.getFrameSize();
		
		for (int p = 0; p < m_SwapPlanes.size(); p++)
		{
			auto &buffer = m_SwapBuffers.at(p);
			buffer.resize(frameSize);
			m_SwapPlanes.at(p) = buffer.data();
		}

		if (m_Decoder.read(m_SwapPlanes.data(), frameSize, m_SwapPlanes.size()) > 0)
		{
			return m_Resampler.write(const_cast<const uint8_t**>(m_SwapPlanes.data()), m_Decoder.getFrameSamples()) > 0;
		}
	}

	return false;
}

int AVSoundReader::read(uint8_t **data, int size, int planes)
{
	return m_Resampler.read(data, size, planes);
}

void AVSoundReader::close()
{
	m_Decoder.close();
}

AVSoundDecoder2* AVSoundReader::getDecoder()
{
	return &m_Decoder;
}

uint64_t AVSoundReader::getChannelLayout()
{
	return m_Resampler.getChannelLayout();
}

AVSampleFormat AVSoundReader::getSampleFormat()
{
	return m_Resampler.getSampleFormat();
}

int AVSoundReader::getNumChannels()
{
	return m_Resampler.getNumChannels();
}

int AVSoundReader::getNumSamples()
{
	return m_Resampler.getNumSamples();
}

int AVSoundReader::getNumPlanes()
{
	return m_Resampler.getNumPlanes();
}

int AVSoundReader::getSampleRate()
{
	return m_Resampler.getSampleRate();
}

int AVSoundReader::getSampleSize()
{
	return m_Resampler.getSampleSize();
}

int AVSoundReader::getFrameSize()
{
	return m_Resampler.getFrameSize();
}

int AVSoundReader::getAvailableSize()
{
	return m_Resampler.getAvailableSize();
}

int AVSoundReader::getAvailableSamples()
{
	return m_Resampler.getAvailableSamples();
}

int64_t AVSoundReader::getDuration()
{
	//return av_rescale(
	//	m_Decoder.getDuration(),
	//	m_Resampler.getSampleRate(),
	//	m_Decoder.getSampleRate());

	return m_Decoder.getDuration();
}

int64_t AVSoundReader::getPosition()
{
	//return av_rescale(
	//	m_Decoder.getPosition(),
	//	m_Resampler.getSampleRate(),
	//	m_Decoder.getSampleRate());
	
	return m_Decoder.getPosition();
}

bool AVSoundReader::isFixedPoint()
{
	return m_Resampler.isFixedPoint();
}

bool AVSoundReader::isFloatingPoint()
{
	return m_Resampler.isFloatingPoint();
}

bool AVSoundReader::isSigned()
{
	return m_Resampler.isSigned();
}

bool AVSoundReader::isUnsigned()
{
	return m_Resampler.isUnsigned();
}

bool AVSoundReader::isPlanar()
{
	return m_Resampler.isPlanar();
}

bool AVSoundReader::isLoaded()
{
	return m_Decoder.isLoaded() && m_Resampler.isInitialized();
}

bool AVSoundReader::isFinished()
{
	return isLoaded() && m_Decoder.isFinished();
}