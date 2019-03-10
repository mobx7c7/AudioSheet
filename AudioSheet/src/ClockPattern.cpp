#include "ClockPattern.h"
#include "ofApp.h"

ClockPattern::ClockPattern(ofBaseApp* baseApp)
	: baseApp(baseApp)
{}

ClockPattern::~ClockPattern()
{}

void ClockPattern::setup()
{
	shader.load("shaders/pattern");
}

void ClockPattern::draw(float x, float y, float w, float h) const
{
	ofRectangle rect(x, y, w, h);

	if (plane.getWidth() != w || plane.getHeight() != h)
	{
		plane.set(w, h);
		plane.mapTexCoords(1, 1, 0, 0);
	}

	if (shader.isLoaded())
	{
		shader.begin();
		shader.setUniform2f("window.pos", glm::vec2(x, y));
		shader.setUniform2f("window.siz", glm::vec2(w, h));
		plane.setPosition(rect.getCenter());
		plane.draw();
		shader.end();
	}
}

void ClockPattern::draw(const ofRectangle & rect) const
{
	draw(rect.x, rect.y, rect.width, rect.height);
}

float ClockPattern::getHeight() const
{
	return 0.0f;
}

float ClockPattern::getWidth() const
{
	return 0.0f;
}
