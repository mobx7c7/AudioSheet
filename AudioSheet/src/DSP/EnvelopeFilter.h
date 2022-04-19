#pragma once
#include "CProcessor.h"
#include "SampleRate.h"

namespace DSP
{
	// Based from: 
	// Envelope follower with different attack and release @ www.musicdsp.org
	class EnvelopeFollower
	{
		SampleRate m_SampleRate;
		double m_AttackTime, m_AttackCoeff;
		double m_ReleaseTime, m_ReleaseCoeff;
		float m_LastEnvelope;

	public:
		EnvelopeFollower();
		void	initialize(int sampleRate);
		void	setAttackTime(double attack);
		double	getAttackTime();
		void	setReleaseTime(double release);
		double	getReleaseTime();
		void	process(float *data, int size, int stride = 1);
	};

	class EnvelopeChannel
	{
		EnvelopeFollower m_EnvelopeFollower;

	public:
		void initialize(int sampleRate);
		void process(float *data, int size, int stride = 1);
	};

	class EnvelopeFilter : public CProcessor
	{
		using EnvelopeChannels = std::vector<EnvelopeChannel>;

		SampleRate m_SampleRate;
		EnvelopeChannels m_Channels;
		int m_SizePerChannel;

	protected:
		void onInitialize() override;
		void onProcess(float *data, int size) override;
	};
}