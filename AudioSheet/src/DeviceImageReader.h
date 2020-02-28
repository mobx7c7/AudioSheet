#pragma once
#include <ofGraphicsBaseTypes.h>
#include <ofFbo.h>
#include <ofImage.h>
#include <memory>
#include <vector>
#include "StreamInput.h"

class DeviceImageReader : public ofBaseDraws
{
private:
	typedef std::vector<uint8_t> ReceiveBuffer;
	std::shared_ptr<StreamInput> scannerInput;
	ReceiveBuffer receiveBuffer;
	ofImage imageBuffer;
	ofFbo magFbo;
private:
	void onDeviceEvent(const StreamEventBase& eventBase);
	void unpackPixelData(PBITMAPINFOHEADER pHeader, int stride, ReceiveBuffer &buf, ReceiveBuffer::iterator beg);
	void readPixelData(PBITMAPINFOHEADER pHeader, std::vector<uint8_t> &obuf);
	void allocateTextureFromBMP();
public:
	void setup();
	void acquire();
	void draw(const ofRectangle & rect) const;
	void draw(float x, float y, float w, float h) const override;
	float getHeight() const override;
	float getWidth() const override;
};
