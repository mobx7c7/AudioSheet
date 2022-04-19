#pragma once
#include "Context.h"
#include <memory>
#include <vector>
#include <ofEvent.h>
#include <ofEvents.h>

namespace DSP
{
	class CContext : public Context
	{
		friend class Processor;
		
		using ProcessorElem = std::unique_ptr<Processor>;

		struct findElement
		{
			Processor *p;
			findElement(Processor *p) : p(p){}
			bool operator()(ProcessorElem &e){return p == e.get();}
		};

		std::vector<float> m_InternalBuffer;
		std::vector<ProcessorElem> m_Processors;
		ofEvent<void> m_ReconfigureEvent;
		int m_NumChannels;
		int m_SampleRate;
		int m_BufferSize;
		bool m_Initialized;

	public:
		CContext();
		virtual ~CContext();
		void initialize(int numChannels, int sampleRate, int bufferSize);
		bool insert(Processor *p);
		bool remove(Processor *p);
		void removeAll();
		void pull(float *data, int size);
		int getNumChannels();
		int getSampleRate();
		int getBufferSize();
		bool isInitialized() { return m_Initialized; }
	};
}
