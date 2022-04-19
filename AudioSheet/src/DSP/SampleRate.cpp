#include "SampleRate.h"

void SampleRate::set(double frequency, double timeUnit)
{
	m_Frequency = frequency;
	m_TimeUnit = timeUnit;
	m_TimeBase = timeUnit / frequency;
}

double SampleRate::getFrequency()
{
	return m_Frequency;
}

double SampleRate::getTimeUnit()
{
	return m_TimeUnit;
}

double SampleRate::getTimeBase()
{
	return m_TimeBase;
}

double SampleRate::timeBaseOf(double time, double newTimeUnit)
{
	if (time > 0)
		time = m_TimeBase * (m_TimeUnit / time);
	else
		time = 1.0f;

	return newTimeUnit * time;
}

double SampleRate::timeBaseOf(double time)
{
	return timeBaseOf(time, m_TimeUnit);
}
