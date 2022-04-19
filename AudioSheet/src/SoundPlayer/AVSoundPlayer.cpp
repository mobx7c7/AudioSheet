#include "AVSoundPlayer.h"
#include <ofEvents.h> // ofAddListener
#include <sstream>
#include <mutex>

void AVSoundPlayer::fillBuffer()
{
	std::lock_guard<std::mutex> lock(m_Mutex);

	while (m_DeviceFifo.getAvailableWrite() > m_Reader.getNumSamples() && !m_Reader.isFinished())
	{
		if (m_Reader.next())
		{
			auto frameReadSizeTarget = m_Reader.getFrameSize();

			for (int p = 0; p < m_SwapPlanes.size(); p++)
			{
				auto &buffer = m_SwapBuffers.at(p);
				buffer.resize(frameReadSizeTarget);
				m_SwapPlanes.at(p) = buffer.data();
			}

			auto frameReadSizeResult = m_Reader.read(m_SwapPlanes.data(), frameReadSizeTarget, m_SwapPlanes.size());

			if (frameReadSizeResult > 0)
			{
				auto position = m_SamplesFilled.load(std::memory_order_relaxed);
				position += frameReadSizeResult / getSampleSize() / getNumChannels();
				m_SamplesFilled.store(position, std::memory_order_release);

				SoundFrameEvent e;
				e.data = reinterpret_cast<BaseSampleType*>(*m_SwapPlanes.data());
				e.size = frameReadSizeResult / m_Reader.getSampleSize();

				ofNotifyEvent(m_Events.fill, e, this);

				m_DeviceFifo.write(e.data, e.size);
			}
		}

		if (m_Reader.isFinished())
		{
			ofNotifyEvent(m_Events.finish, this);
		}
	}
}

void AVSoundPlayer::audioOut(ofSoundBuffer & outBuffer)
{
	if (m_Reader.isLoaded())
	{
		if (m_BufferWriteEnabled)
		{
			// TODO: Asynchronous write
			fillBuffer();
		}

		if (m_BufferReadEnabled && m_DeviceFifo.getAvailableRead())
		{
			auto fifoReadSizeTarget = std::min(m_DeviceSwap.size(), m_DeviceFifo.getAvailableRead());

			if (m_DeviceFifo.read(m_DeviceSwap.data(), fifoReadSizeTarget))
			{
				auto position = m_SamplesPlayed.load(std::memory_order_relaxed);
				position += fifoReadSizeTarget / getNumChannels();
				m_SamplesPlayed.store(position, std::memory_order_release);

				outBuffer.copyFrom(m_DeviceSwap, m_Reader.getNumChannels(), m_Reader.getSampleRate());
			}
		}
	}
}

AVSoundPlayer::AVSoundPlayer()
	: m_AutoPlayEnabled(true)
	, m_BufferReadEnabled(true)
	, m_BufferWriteEnabled(true)
	, m_BufferSecs(10)
	, m_Initialized(false)
	, m_SamplesPlayed(0)
{}

AVSoundPlayer::~AVSoundPlayer()
{

}

void AVSoundPlayer::setup()
{
	if (!m_Initialized)
	{
		m_Reader.setup();
		m_Initialized = true;
	}
}

void AVSoundPlayer::load(boost::filesystem::path path)
{
	unload();

	// Note: Function 'load' throws exception.
	m_Reader.load(path.string());

	ofSoundStreamSettings settings;
	settings.setApi(ofSoundDevice::MS_DS);
	settings.setOutListener(this);
	settings.numOutputChannels = 2;
	settings.sampleRate = 44100;
	settings.bufferSize = 512;
	settings.numBuffers = 4;

	m_DeviceSwap.resize(settings.bufferSize*settings.numOutputChannels);
	m_DeviceFifo.resize(m_BufferSecs*settings.sampleRate*settings.numOutputChannels);

	m_SwapBuffers.resize(m_Reader.getNumPlanes());
	m_SwapPlanes.resize(m_Reader.getNumPlanes());

	m_SamplesFilled = 0;
	m_SamplesPlayed = 0;
	setBufferReadEnabled(m_AutoPlayEnabled);

	// Don't place after 'm_Stream.setup'.
	// The program may crash calling SoundPlayerView::write
	// inside a audio callback.
	ofNotifyEvent(m_Events.load, this); 

	// Note: Function 'startStream' is called on 'setup'
	m_Stream.setup(settings); 
}

void AVSoundPlayer::unload()
{
	if (isLoaded())
	{
		m_Stream.stop();
		m_Reader.close();
		m_SwapBuffers.clear();
		m_SwapPlanes.clear();
		m_DeviceFifo.clear(); // FIXME: mutex
		ofNotifyEvent(m_Events.unload, this);
	}
}

void AVSoundPlayer::play()
{
	if (isLoaded())
	{
		setBufferReadEnabled(true);
		ofNotifyEvent(m_Events.play, this);
	}
}

void AVSoundPlayer::pause()
{
	if (isLoaded())
	{
		setBufferReadEnabled(false);
		ofNotifyEvent(m_Events.pause, this);
	}
}

void AVSoundPlayer::stop()
{
	if (isLoaded())
	{
		setBufferReadEnabled(false);
		m_Reader.seek(0, false);
		ofNotifyEvent(m_Events.stop, this);
	}
}

void AVSoundPlayer::seek(double secs, bool relative)
{
	if (isLoaded())
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		m_Reader.seek(secs, relative);
		m_DeviceFifo.clear();  // FIXME: mutex
		m_SamplesFilled = 0;
		m_SamplesPlayed = 0;
		ofNotifyEvent(m_Events.seek, this);
	}
}

void AVSoundPlayer::setAutoPlayEnabled(bool enabled) { m_AutoPlayEnabled = enabled; }

void AVSoundPlayer::setBufferReadEnabled(bool enabled) { m_BufferReadEnabled = enabled; }

void AVSoundPlayer::setBufferFillEnabled(bool enabled) { m_BufferWriteEnabled = enabled; }

double AVSoundPlayer::getBufferSecs() { return m_BufferSecs; }

size_t AVSoundPlayer::getBufferSize() { return m_DeviceFifo.getSize(); }

uint64_t AVSoundPlayer::getChannelLayout() { return m_Reader.getChannelLayout(); }

AVSampleFormat AVSoundPlayer::getSampleFormat() { return m_Reader.getSampleFormat(); }

int AVSoundPlayer::getNumChannels() { return m_Reader.getNumChannels(); }

int AVSoundPlayer::getNumPlanes() { return m_Reader.getNumPlanes(); }

int AVSoundPlayer::getSampleRate() { return m_Reader.getSampleRate(); }

int AVSoundPlayer::getSampleSize() { return m_Reader.getSampleSize(); }

bool AVSoundPlayer::isAutoPlayEnabled() { return m_AutoPlayEnabled; }

bool AVSoundPlayer::isBufferReadEnabled() { return m_BufferReadEnabled; }

bool AVSoundPlayer::isBufferWriteEnabled() { return m_BufferWriteEnabled; }

bool AVSoundPlayer::isFixedPoint() { return m_Reader.isFixedPoint(); }

bool AVSoundPlayer::isFloatingPoint() { return m_Reader.isFloatingPoint(); }

bool AVSoundPlayer::isSigned() { return m_Reader.isSigned(); }

bool AVSoundPlayer::isUnsigned() { return m_Reader.isUnsigned(); }

bool AVSoundPlayer::isPlanar() { return m_Reader.isPlanar(); }

bool AVSoundPlayer::isLoaded() { return m_Reader.isLoaded(); }

bool AVSoundPlayer::isPlaying() { return m_BufferReadEnabled; }

bool AVSoundPlayer::isFinished() { return m_Reader.isFinished(); }

std::string AVSoundPlayer::getFormatInfo()
{
	std::stringstream ss;

	auto decoder = m_Reader.getDecoder();

	if (decoder->isLoaded())
	{
		ss << decoder->getFormatName();
		ss << ", " << decoder->getNumChannels() << "ch";
		ss << ", " << decoder->getSampleRate() << "Hz";
		ss << ", " << (decoder->getSampleSize() << 3) << "-bit";
		ss << " ";

		if (decoder->isFixedPoint())
		{
			ss << (decoder->isUnsigned() ? "unsigned" : "signed") << " int";
		}
		else
		{
			ss << "float";
		}
	}
	else
	{
		ss << "unloaded";
	}

	return ss.str();
}