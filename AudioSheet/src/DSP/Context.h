#pragma once
#include "Processor.h"

namespace DSP
{
	class Context
	{
	public:
		virtual ~Context() = default;
		virtual void initialize(int numChannels, int sampleRate, int bufferSize) = 0;
		virtual bool insert(Processor *p) = 0;
		virtual bool remove(Processor *p) = 0;
		virtual void removeAll() = 0;
		virtual void pull(float *data, int size) = 0;
		virtual int getNumChannels() = 0;
		virtual int getSampleRate() = 0;
		virtual int getBufferSize() = 0;
		virtual bool isInitialized() = 0;
	};
}