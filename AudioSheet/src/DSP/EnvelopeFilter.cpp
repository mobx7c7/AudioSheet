#include "EnvelopeFilter.h"
#include <algorithm>
#include <cmath>
#include <float.h> // From Windows SDK
#include <limits>

namespace DSP
{
	EnvelopeFollower::EnvelopeFollower()
		: m_AttackTime(0)
		, m_AttackCoeff(0)
		, m_ReleaseTime(0)
		, m_ReleaseCoeff(0)
		, m_LastEnvelope(0)
	{}

	void EnvelopeFollower::initialize(int sampleRate)
	{
		m_SampleRate.set(static_cast<double>(sampleRate), 1000.0); // ms
		m_LastEnvelope = 0.0f;
	}

	void EnvelopeFollower::setAttackTime(double attack)
	{
		m_AttackTime = attack;
		m_AttackCoeff = exp(log(0.01) / m_SampleRate.timeToFreq(attack));
	}

	double EnvelopeFollower::getAttackTime()
	{
		return m_AttackTime;
	}

	void EnvelopeFollower::setReleaseTime(double release)
	{
		m_ReleaseTime = release;
		m_ReleaseCoeff = exp(log(0.01) / m_SampleRate.timeToFreq(release));
	}

	double EnvelopeFollower::getReleaseTime()
	{
		return m_ReleaseTime;
	}

	void EnvelopeFollower::process(float *data, int size, int stride)
	{
		float thisSample, thisEnvelope, deltaEnvelope;

		thisEnvelope = m_LastEnvelope;

		for (int i = 0; i < size; i++)
		{
			thisSample = fabs(*data);

			deltaEnvelope = thisEnvelope - thisSample;

			if (thisSample > thisEnvelope)
			{
				thisEnvelope = static_cast<float>(m_AttackCoeff * deltaEnvelope + thisSample);
			}
			else
			{
				thisEnvelope = static_cast<float>(m_ReleaseCoeff * deltaEnvelope + thisSample);
			}

			thisEnvelope = std::min(std::max(thisEnvelope, 0.0f), 1.0f);

			*data = thisEnvelope;

			data += stride;
		}

		m_LastEnvelope = thisEnvelope;
	}

	void EnvelopeChannel::initialize(int sampleRate)
	{
		m_EnvelopeFollower.initialize(sampleRate);
		m_EnvelopeFollower.setAttackTime(1);
		m_EnvelopeFollower.setReleaseTime(1000.0);
	}
	void EnvelopeChannel::process(float *data, int size, int stride)
	{
		m_EnvelopeFollower.process(data, size, stride);
	}

	void EnvelopeFilter::onInitialize()
	{
		auto context = getContext();
		auto numChannels = context->getNumChannels();
		auto sampleRate = context->getSampleRate();
		auto bufferSize = context->getBufferSize();

		m_SampleRate.set(bufferSize);
		m_Channels.resize(numChannels);

		m_SizePerChannel = bufferSize / numChannels;

		for (auto &channel : m_Channels)
			channel.initialize(sampleRate);
	}
	void EnvelopeFilter::onProcess(float *data, int size)
	{
		for (int i = 0; i < m_Channels.size(); i++)
			m_Channels.at(i).process(data++, m_SizePerChannel, m_Channels.size());
	}
}