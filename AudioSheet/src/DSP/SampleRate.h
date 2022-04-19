#pragma once

class SampleRate
{
	double m_Frequency;
	double m_TimeBase;
	double m_TimeUnit;

public:
	void set(double frequency, double timeUnit = 1.0); // timeBase default: secs

	double getFrequency();
	double getTimeUnit();
	double getTimeBase();
	// The argument 'time' must be in 'timeUnit' domain
	double timeBaseOf(double time, double newTimeUnit);
	double timeBaseOf(double time);

	double timeToFreq(double time)
	{
		return m_Frequency * (time / m_TimeUnit);
	}
};
