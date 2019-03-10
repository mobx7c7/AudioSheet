#pragma once
#include <ofGraphicsBaseTypes.h>
#include <ofFbo.h>
#include "ClockPattern.h"

class SheetCanvas : public ofBaseDraws
{
private:
	const static glm::vec2
		XMASK,
		YMASK;
	const static float
		UNIT_INCHES;
private:
	ofBaseApp*
		baseApp;
	std::unique_ptr<ClockPattern>
		clockPattern;
	ofFbo
		renderFbo;
	mutable ofRectangle
		sheetRect;
	glm::vec2
		dimension,
		resolution;
	int
		dotsPerInch;
public:
	SheetCanvas(ofBaseApp* baseApp);
	~SheetCanvas();
public:
	const ofFbo& getFbo() { return renderFbo; } 
	const glm::vec2& getDimension() { return dimension; }
	const glm::vec2& getResolution() { return resolution; }
	const int& getDPI() { return dotsPerInch; }
public:
	void setup();
	void reload();
	void update();
	void draw(const ofRectangle & rect) const override;
	void draw(float x, float y, float w, float h) const override;
	float getHeight() const override;
	float getWidth() const override;
};

