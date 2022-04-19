#pragma once
#include <string>
#include <exception>

class AVSoundException : public std::exception
{
	int m_Err;
	std::string m_Msg;

public:
	AVSoundException(const char* msg, int err = 0);
	char const* what() const override;
	int getError();
};
