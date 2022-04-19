#pragma once
extern "C" {
#include <libavutil/samplefmt.h>
}
#include <string>
#include <boost/filesystem/path.hpp>
#include <ofEvent.h>

struct SoundFrameEvent
{
	const float* data;
	size_t size;
};

class SoundPlayer
{
public:
	struct Events
	{
		ofEvent<SoundFrameEvent> fill;
		ofEvent<void> load;
		ofEvent<void> unload;
		ofEvent<void> play;
		ofEvent<void> pause;
		ofEvent<void> stop;
		ofEvent<void> seek;
		ofEvent<void> finish;
	};
	enum class State
	{
		Unitialized,
		Unloaded,
		Stopped,
		Playing,
		Paused,
	};
	virtual ~SoundPlayer() = default;
	virtual	void setup() = 0;
	virtual	void load(boost::filesystem::path) = 0;
	virtual	void unload() = 0;
	virtual	void play() = 0;
	virtual	void pause() = 0;
	virtual	void stop() = 0;
	virtual	void seek(double secs, bool relative = true) = 0;
	virtual void setAutoPlayEnabled(bool) = 0;
	virtual	void setBufferFillEnabled(bool) = 0;
	virtual	void setBufferReadEnabled(bool) = 0;
	virtual	uint64_t getChannelLayout() = 0;
	virtual	AVSampleFormat getSampleFormat() = 0;
	virtual	int getNumChannels() = 0;
	//virtual int getNumSamples() = 0;
	virtual	int getNumPlanes() = 0;
	virtual	int getSampleRate() = 0;
	virtual	int getSampleSize() = 0;
	//virtual int getFrameSize() = 0;
	virtual	double getBufferSecs() = 0;
	virtual	size_t getBufferSize() = 0;
	virtual	size_t getBufferFillAvailable() = 0;
	virtual	size_t getBufferReadAvailable() = 0;
	virtual	int64_t getDuration() = 0;
	virtual	int64_t getPosition() = 0;
	virtual int64_t getNumSamplesFilled() = 0;
	virtual int64_t getNumSamplesPlayed() = 0;
	virtual double getNumSecondsFilled() = 0;
	virtual double getNumSecondsPlayed() = 0;
	virtual	std::string getFormatInfo() = 0;
	virtual Events& getEvents() = 0;
	virtual bool isAutoPlayEnabled() = 0;
	virtual	bool isBufferReadEnabled() = 0;
	virtual	bool isBufferWriteEnabled() = 0;
	virtual	bool isFixedPoint() = 0;
	virtual	bool isFloatingPoint() = 0;
	virtual	bool isSigned() = 0;
	virtual	bool isUnsigned() = 0;
	virtual	bool isPlanar() = 0;
	virtual	bool isLoaded() = 0;
	virtual bool isPlaying() = 0;
	virtual	bool isFinished() = 0;
};
