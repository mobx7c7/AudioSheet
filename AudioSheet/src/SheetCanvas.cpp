#include "SheetCanvas.h"
#include <ofGraphics.h>
#include <ofEventUtils.h>

const float SheetCanvas::UNIT_INCHES = 25.4f;

SheetCanvas::SheetCanvas(ofBaseApp* baseApp)
	: baseApp(baseApp)
{}

SheetCanvas::~SheetCanvas()
{}

void SheetCanvas::setup()
{
	dotsPerInch = 600;
	dimension = glm::vec2(297, 210);
	resolution = glm::floor((dimension / UNIT_INCHES) * (float)dotsPerInch);

	ofFboSettings fboSettings;
	fboSettings.width = resolution.x;
	fboSettings.height = resolution.y;
	fboSettings.internalformat = GL_RGBA32F;
	fboSettings.numColorbuffers = 1;
	renderFbo.allocate(fboSettings);

	clockPattern = std::make_unique<ClockPattern>(baseApp);
	clockPattern->setup();
}

void SheetCanvas::reload()
{
	clockPattern->setup();
}

void SheetCanvas::update()
{
	auto fboRect = ofRectangle(glm::vec2(), resolution);

	ofPushMatrix();
	renderFbo.bind();
	ofClear(ofColor::lightGray);
	ofViewport(0, 0, renderFbo.getWidth(), renderFbo.getHeight(), false);
	{
		clockPattern->draw(fboRect);
	}
	renderFbo.unbind();
	ofPopMatrix();
}

void SheetCanvas::draw(float x, float y, float w, float h) const
{
	draw(ofRectangle(x,y,w,h));
}

void SheetCanvas::draw(const ofRectangle & rect) const
{
	const static glm::vec2 XMASK(1, 0);
	const static glm::vec2 YMASK(0, 1);

	auto sheetRect = ofRectangle(glm::vec2(), resolution);
	sheetRect.scale(rect.getHeight() / sheetRect.getHeight());
	sheetRect.setPosition((rect.getCenter() - sheetRect.getCenter()) * XMASK);

	renderFbo.getTexture().draw(sheetRect);
}

float SheetCanvas::getHeight() const
{
	return resolution.x;
}

float SheetCanvas::getWidth() const
{
	return resolution.y;
}
