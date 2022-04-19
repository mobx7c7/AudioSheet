#include "AverageFilter.h"

namespace DSP
{
	void AverageChannel::initialize(AverageFilter *filter)
	{
		m_Filter = filter;
		m_LastAvg = 0.0f;
		m_NextAvg = 0.0f;
	}
	void AverageChannel::process(float * data, int size, int stride)
	{
		for (int i = 0; i < size; i++)
		{
			m_NextAvg += *data;

			if (++m_Count > m_Filter->getMaxCount())
			{
				m_LastAvg = m_NextAvg / m_Filter->getMaxCount();
				m_NextAvg = 0.0f;
				m_Count = 0;
			}

			*data = m_LastAvg;

			data += stride;
		}
	}
	void AverageFilter::onInitialize()
	{
		auto context = getContext();
		auto numChannels = context->getNumChannels();
		auto bufferSize = context->getBufferSize();

		m_Channels.resize(numChannels);

		m_SizePerChannel = bufferSize / numChannels;

		for (auto &channel : m_Channels)
			channel.initialize(this);
	}
	void AverageFilter::onProcess(float * data, int size)
	{
		for (int i = 0; i < m_Channels.size(); i++)
			m_Channels.at(i).process(data++, m_SizePerChannel, m_Channels.size());
	}
	void AverageFilter::onReset()
	{
		for (int i = 0; i < m_Channels.size(); i++)
			m_Channels.at(i).initialize(this); // lazy...
	}
	AverageFilter::AverageFilter()
		: m_SizePerChannel(0)
		, m_MaxCount(0)
	{}
}