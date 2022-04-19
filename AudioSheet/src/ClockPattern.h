#pragma once
#include <ofBaseApp.h>
#include <ofGraphicsBaseTypes.h>
#include <ofShader.h>
#include <of3dPrimitives.h>
#include <ofTexture.h>

class ClockPattern : public ofBaseDraws
{
private:
	ofBaseApp* baseApp;
	mutable ofShader shader;
	mutable ofPlanePrimitive plane;
	mutable ofTexture noiseTexture;
	int m_Tracks;
	int m_Bandguard;
	glm::vec2 m_MarginX;
	glm::vec2 m_MarginY;
public:
	ClockPattern(ofBaseApp* baseApp);
	~ClockPattern();
	void setup();
	void draw(float x, float y, float w, float h) const override;
	void draw(const ofRectangle & rect) const override;
	float getHeight() const override;
	float getWidth() const override;
};

