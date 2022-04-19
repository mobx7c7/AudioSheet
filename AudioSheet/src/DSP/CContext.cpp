#include "CContext.h"
#include "CProcessor.h"

namespace DSP
{
	CContext::CContext()
		: m_Initialized(false)
	{

	}
	CContext::~CContext()
	{
		removeAll();
	}
	void CContext::initialize(int numChannels, int sampleRate, int bufferSize)
	{
		m_Initialized = false;
		m_NumChannels = numChannels;
		m_SampleRate = sampleRate;
		m_BufferSize = bufferSize;
		m_InternalBuffer.resize(bufferSize);
		m_Initialized = true;
		m_ReconfigureEvent.notify(this);
	}
	bool CContext::insert(Processor *p)
	{
		assert(p != nullptr);

		auto processor = static_cast<CProcessor*>(p);

		auto found = std::find_if(m_Processors.begin(), m_Processors.end(), findElement(processor));
		
		if (found == m_Processors.end())
		{
			// initialize will be called inside configure
			processor->configure(this);

			ofAddListener(m_ReconfigureEvent, processor, &CProcessor::reconfigure);

			m_Processors.emplace_back(processor);

			return true;
		}

		return false;
	}
	bool CContext::remove(Processor *p)
	{
		assert(p != nullptr);

		auto processor = static_cast<CProcessor*>(p);

		auto found = std::find_if(m_Processors.begin(), m_Processors.end(), findElement(processor));

		if (found != m_Processors.end())
		{
			// unitialize will be called inside configure
			processor->configure(nullptr);

			ofRemoveListener(m_ReconfigureEvent, processor, &CProcessor::reconfigure);

			m_Processors.erase(found);

			return true;
		}

		return false;
	}
	void CContext::removeAll()
	{
		auto next = m_Processors.begin();
		
		while (next != m_Processors.end())
		{
			auto processor = static_cast<CProcessor*>(next->get());

			processor->configure(nullptr);

			ofRemoveListener(m_ReconfigureEvent, processor, &CProcessor::reconfigure);

			next = m_Processors.erase(next);
		}
	}
	void CContext::pull(float *data, int size)
	{
		size = std::min(size, (int)m_InternalBuffer.size());

		std::copy_n(data, size, m_InternalBuffer.data());

		for (int i = 0; i < m_Processors.size(); i++)
		{
			auto processor = static_cast<CProcessor*>(m_Processors.at(i).get());

			processor->pull(m_InternalBuffer.data(), size);
		}

		std::copy_n(m_InternalBuffer.data(), size, data);
	}

	int CContext::getNumChannels() { return m_NumChannels; }
	int CContext::getSampleRate() { return m_SampleRate; }
	int CContext::getBufferSize() { return m_BufferSize; }
}
