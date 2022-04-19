#pragma once
#include "CProcessor.h"

namespace DSP
{
	class AverageFilter;

	class AverageChannel
	{
		AverageFilter* m_Filter;
		float m_LastAvg;
		float m_NextAvg;
		int m_Count;

	public:
		void initialize(AverageFilter *filter);
		void process(float *data, int size, int stride = 1);
	};

	class AverageFilter : public CProcessor
	{
		using AverageChannels = std::vector<AverageChannel>;

		AverageChannels m_Channels;
		int m_SizePerChannel;
		int m_MaxCount;

	protected:
		void onInitialize() override;
		void onProcess(float *data, int size) override;
		void onReset() override;

	public:
		AverageFilter();
		void setMaxCount(int maxCount) { m_MaxCount = maxCount; }
		int getMaxCount() { return m_MaxCount; }
	};
}