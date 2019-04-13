#pragma once
#include <ofGraphicsBaseTypes.h>
#include <ofTexture.h>
#include <memory>
#include <vector>
#include "StreamInput.h"

class DeviceImageReader : public ofBaseDraws
{
private:
	std::shared_ptr<StreamInput>
		scannerInput;
	std::vector<uint8_t>
		receiveBuffer;
	ofTexture
		imageTexture;
private:
	void onDeviceEvent(const StreamEventBase& eventBase);
	void allocateTextureFromBMP();
public:
	void setup();
	void acquire();
	void draw(const ofRectangle & rect) const;
	void draw(float x, float y, float w, float h) const override;
	float getHeight() const override;
	float getWidth() const override;
};
