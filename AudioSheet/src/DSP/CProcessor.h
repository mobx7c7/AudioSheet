#pragma once
#include "Context.h"
#include "Processor.h"
#include <vector>

namespace DSP
{
	class CProcessor : public Processor
	{	 
		friend class CContext;

		std::vector<float> m_InternalBuffer;
		bool m_Enabled;
		bool m_Initialized;
		Context* m_Context;

		void configure(Context* context);
		void reconfigure();
		void pull(float *data, int size);

	protected:
		//data: interleaved samples in 'n' channels
		virtual void onProcess(float *data, int size) {}
		virtual void onInitialize() {}
		virtual void onUnitialize() {}
		virtual void onReset() {}

	public:
		CProcessor();
		void reset() { onReset(); }
		void enable();
		void disable();
		void setEnabled(bool enabled);
		bool isEnabled();
		bool isInitialized();
		Context* getContext();
	};
}