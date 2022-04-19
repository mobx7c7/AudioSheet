#pragma once
#include "AVSoundDecoder.h"
#include "AVSoundResampler.h"
#include <vector>


class AVSoundReader;

class AVSoundReaderExc : public AVSoundException
{
	AVSoundReader* m_Reader;

public:
	AVSoundReaderExc(AVSoundReader *reader, const char* msg, int err = 0);
	AVSoundReader* getReader();
};

class AVSoundReader
{
	// FIXME: Atribuir valor padronizado
	const int RESAMPLER_BUFSIZE = 16384;

	AVSoundDecoder2						m_Decoder;
	AVSoundResampler					m_Resampler;
	std::vector<std::vector<uint8_t>>	m_SwapBuffers;
	std::vector<uint8_t*>				m_SwapPlanes;
	bool								m_Initialized;
	
	void reset();

public:
	AVSoundReader();
	virtual ~AVSoundReader();
	void setup();
	void load(std::string filePath);
	void seek(double secs, bool relative = true);
	bool next();
	int read(uint8_t **data, int size, int planes);
	void close();

	AVSoundDecoder2* getDecoder();
	uint64_t getChannelLayout();
	AVSampleFormat getSampleFormat();
	int getNumChannels();
	int getNumSamples();
	int getNumPlanes();
	int getSampleRate();
	int getSampleSize();

	int getFrameSize(); // bytes, buffer size?
	int getAvailableSize(); // bytes, buffer size
	int getAvailableSamples();

	int64_t getDuration();
	int64_t getPosition();

	bool isFixedPoint();
	bool isFloatingPoint();
	bool isSigned();
	bool isUnsigned();
	bool isPlanar();
	bool isLoaded();
	bool isFinished();
};