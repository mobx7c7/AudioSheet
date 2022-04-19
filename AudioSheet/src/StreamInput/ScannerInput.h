#pragma once
#include "..\StreamInput.h"
#include "ScannerInputEvent.h"
//#include <ofRectangle.h> // erro: conflito com 'min' gdiplustypes.h

enum class ScannerImageType
{
	Text,
	Grayscale,
	Color,
};

class ScannerInput : public StreamInput
{
	StreamEventCb m_Callback;
	ScannerImageType m_ImageType;
	int m_ResolutionX;
	int m_ResolutionY;
	int m_PositionX;
	int m_PositionY;
	int m_ExtentX;
	int m_ExtentY;
	//ofRectangle m_Area;

protected:
	void dispachEvent(const StreamEventBase&);
	ScannerInput();

public:
	ScannerInput(const ScannerInput&) = delete;
	ScannerInput& operator=(const ScannerInput&) = delete;
	virtual ~ScannerInput() = default;
	void setCallback(StreamEventCb);
	void setType(ScannerImageType);
	ScannerImageType getImageType();
	void setResolution(int x, int y);
	void setResolutionX(int x);
	void setResolutionY(int y);
	int getResolutionX();
	int getResolutionY();

	//void setArea(ofRectangle);
	//ofRectangle getArea();

	void setPosition(int x, int y);
	void setPositionX(int x);
	void setPositionY(int y);
	int getPositionX();
	int getPositionY();
	void setExtent(int x, int y);
	void setExtentX(int x);
	void setExtentY(int y);
	int getExtentX();
	int getExtentY();

	//void setBrightness(int);
	//void setContrast(int);
	//void setInverted(bool);
	//void setMirroed(bool);
	//void setRotation(int);
	//void setThreshold(int);
};