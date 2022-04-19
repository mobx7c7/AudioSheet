#pragma once
#include <vector>
#include <memory>
#include <ofSoundStream.h>
#include <ofSoundBuffer.h>
#include "AVSoundReader.h"
#include "..\SoundPlayer.h"
#include "..\RingBuffer.h"

class AVSoundPlayer : public SoundPlayer
{
	friend class ofSoundStreamSettings; // Acesso ao método 'audioOut'
	friend class AVSoundPlayerView;

	using BaseSampleType = float;

	ofSoundStream						m_Stream;
	AVSoundReader						m_Reader;
	std::vector<std::vector<uint8_t>>	m_SwapBuffers;
	std::vector<uint8_t*>				m_SwapPlanes;
	std::mutex							m_Mutex;
	std::vector<BaseSampleType>			m_DeviceSwap;
	RingBufferT<BaseSampleType>			m_DeviceFifo;
	double								m_BufferSecs;

	bool								m_Initialized;
	bool								m_AutoPlayEnabled;
	bool								m_BufferReadEnabled;
	bool								m_BufferWriteEnabled;

	std::atomic<int64_t>				m_SamplesFilled;
	std::atomic<int64_t>				m_SamplesPlayed;

	Events								m_Events;

	void fillBuffer();
	void audioOut(ofSoundBuffer &buffer);

public:
	AVSoundPlayer();
	virtual ~AVSoundPlayer();
	void setup();
	void load(boost::filesystem::path);
	void unload();
	void play();
	void pause();
	void stop();
	void seek(double secs, bool relative = true);
	void setAutoPlayEnabled(bool);
	void setBufferFillEnabled(bool);
	void setBufferReadEnabled(bool);
	uint64_t getChannelLayout();
	AVSampleFormat getSampleFormat();
	int	getNumChannels();
	//int getNumSamples() { return m_Reader.getNumSamples(); }
	int	getNumPlanes();
	int getSampleRate();
	int getSampleSize();
	//int getFrameSize() { return m_Reader.getFrameSize(); }
	double getBufferSecs();
	size_t getBufferSize();
	size_t getBufferFillAvailable() { return m_DeviceFifo.getAvailableWrite(); }
	size_t getBufferReadAvailable() { return m_DeviceFifo.getAvailableRead(); }
	int64_t getDuration() { return m_Reader.getDuration(); }
	int64_t getPosition() { return m_Reader.getPosition(); }
	int64_t getNumSamplesFilled() { return m_SamplesFilled; }
	int64_t getNumSamplesPlayed() { return m_SamplesPlayed; }
	double getNumSecondsFilled() { return static_cast<double>(m_SamplesFilled) / m_Reader.getSampleRate(); }
	double getNumSecondsPlayed() { return static_cast<double>(m_SamplesPlayed) / m_Reader.getSampleRate(); }
	std::string getFormatInfo();
	Events& getEvents() { return m_Events; }
	bool isAutoPlayEnabled();
	bool isBufferReadEnabled();
	bool isBufferWriteEnabled();
	bool isFixedPoint();
	bool isFloatingPoint();
	bool isSigned();
	bool isUnsigned();
	bool isPlanar();
	bool isLoaded();
	bool isPlaying(); // mesmo que isBufferReadEnabled()
	bool isFinished();
};