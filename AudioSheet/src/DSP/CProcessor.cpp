#include "CProcessor.h"
#include <algorithm> // std::min

namespace DSP
{
	void CProcessor::configure(Context *context)
	{
		if(m_Initialized && m_Context != context)
		{
			m_InternalBuffer.clear();

			onUnitialize();

			m_Initialized = false;
		}

		m_Context = context;

		if(!m_Initialized && m_Context && m_Context->isInitialized())
		{
			m_InternalBuffer.resize(m_Context->getBufferSize());

			onInitialize();

			m_Initialized = true;
		}
	}
	void CProcessor::reconfigure()
	{
		configure(m_Context);
	}
	void CProcessor::pull(float * data, int size)
	{
		if (isEnabled())
		{
			std::copy_n(data, size, m_InternalBuffer.data());

			onProcess(m_InternalBuffer.data(), size);

			std::copy_n(m_InternalBuffer.data(), size, data);
		}
	}
	CProcessor::CProcessor()
		: m_Enabled(true)
		, m_Initialized(false)
		, m_Context(nullptr)
	{}
	void CProcessor::enable() { m_Enabled = true; }
	void CProcessor::disable() { m_Enabled = false; }
	void CProcessor::setEnabled(bool enabled) { m_Enabled = enabled; }
	bool CProcessor::isEnabled() { return m_Enabled; }
	bool CProcessor::isInitialized() { return m_Initialized; }
	Context * CProcessor::getContext() { return m_Context; }
}