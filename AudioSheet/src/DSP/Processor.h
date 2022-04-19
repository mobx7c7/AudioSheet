#pragma once

namespace DSP
{
	class Processor
	{
	public:
		virtual void enable() = 0;
		virtual void disable() = 0;
		virtual void setEnabled(bool) = 0;
		virtual bool isEnabled() = 0;
	};
}
