#include "AVFIFO.h"

namespace AVPP
{
	AudioFifo::AudioFifo(AVSampleFormat fmt, int  channels, int nb_samples)
		: m_Fifo(av_audio_fifo_alloc(fmt, channels, nb_samples))
	{}
	AudioFifo::~AudioFifo()
	{
		av_audio_fifo_free(m_Fifo);
	}
	int AudioFifo::write(void** data, int nb_samples)
	{
		return av_audio_fifo_write(m_Fifo, data, nb_samples);
	}
	int AudioFifo::peek(void** data, int nb_samples)
	{
		return av_audio_fifo_peek(m_Fifo, data, nb_samples);
	}
	int AudioFifo::peekAt(void **data, int nb_samples, int offset)
	{
		return av_audio_fifo_peek_at(m_Fifo, data, nb_samples, offset);
	}
	int AudioFifo::read(void** data, int nb_samples)
	{
		return av_audio_fifo_read(m_Fifo, data, nb_samples);
	}
	int AudioFifo::drain(int nb_samples)
	{
		return av_audio_fifo_drain(m_Fifo, nb_samples);
	}
	void AudioFifo::reset()
	{
		av_audio_fifo_reset(m_Fifo);
	}
	int AudioFifo::getSize()
	{
		return av_audio_fifo_size(m_Fifo);
	}
	int AudioFifo::getSpace()
	{
		return av_audio_fifo_space(m_Fifo);
	}
}
