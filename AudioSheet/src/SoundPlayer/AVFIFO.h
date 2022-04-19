#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/audio_fifo.h>
}

namespace AVPP
{
	class AudioFifo
	{
	private:
		AVAudioFifo* m_Fifo;
	public:
		AudioFifo(AVSampleFormat fmt, int  channels, int nb_samples);
		~AudioFifo();
		int write(void** data, int nb_samples);
		int peek(void** data, int nb_samples);
		int peekAt(void **data, int nb_samples, int offset);
		int read(void** data, int nb_samples);
		int drain(int nb_samples);
		void reset();
		int getSize();
		int getSpace();
	};
};
