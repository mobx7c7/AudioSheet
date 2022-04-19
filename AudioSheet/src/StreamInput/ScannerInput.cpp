#include "ScannerInput.h"

ScannerInput::ScannerInput()
	: m_ImageType(ScannerImageType::Color)
	, m_ResolutionX(200)
	, m_ResolutionY(200)
	, m_PositionX(-1)
	, m_PositionY(-1)
	, m_ExtentX(-1)
	, m_ExtentY(-1)
{}
void ScannerInput::dispachEvent(const StreamEventBase &e)
{
	if (m_Callback) m_Callback(e);
}
void ScannerInput::setCallback(StreamEventCb cb)
{
	m_Callback = cb;
}
void ScannerInput::setType(ScannerImageType imageType)
{
	m_ImageType = imageType;
}
ScannerImageType ScannerInput::getImageType()
{
	return m_ImageType;
}
void ScannerInput::setResolution(int x, int y)
{
	m_ResolutionX = x; m_ResolutionY = y;
}
void ScannerInput::setResolutionX(int x)
{
	m_ResolutionX = x;
}
void ScannerInput::setResolutionY(int y)
{
	m_ResolutionY = y;
}
int ScannerInput::getResolutionX()
{
	return m_ResolutionX;
}
int ScannerInput::getResolutionY()
{
	return m_ResolutionY;
}
void ScannerInput::setPosition(int x, int y)
{
	m_PositionX = x; m_PositionY = y;
}
void ScannerInput::setPositionX(int x)
{
	m_PositionX = x;
}
void ScannerInput::setPositionY(int y)
{
	m_PositionY = y;
}
int ScannerInput::getPositionX()
{
	return m_PositionX;
}
int ScannerInput::getPositionY()
{
	return m_PositionY;
}
void ScannerInput::setExtent(int x, int y)
{
	m_ExtentX = x; m_ExtentY = y;
}
void ScannerInput::setExtentX(int x)
{
	m_PositionX = x;
}
void ScannerInput::setExtentY(int y)
{
	m_PositionY = y;
}
int ScannerInput::getExtentX()
{
	return m_ExtentX;
}
int ScannerInput::getExtentY()
{
	return m_ExtentY;
}