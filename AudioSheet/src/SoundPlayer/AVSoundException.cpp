#include "AVSoundException.h"
#include "AVUtils.hpp"
#include <array>

AVSoundException::AVSoundException(const char* msg, int err)
	: m_Msg(msg)
	, m_Err(err)
{
	std::array<char, AV_ERROR_MAX_STRING_SIZE> errbuf;

	if (av_strerror(err, errbuf.data(), errbuf.size()) == 0)
	{
		m_Msg += ": " + std::string(errbuf.data());
	}
}
char const* AVSoundException::what() const
{
	return m_Msg.c_str();
}

int AVSoundException::getError()
{
	return m_Err;
}
