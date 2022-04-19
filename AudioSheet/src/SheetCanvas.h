#pragma once
#include <ofFbo.h>
#include "ClockPattern.h"

class SheetCanvas
{
private:
	const static float
		UNIT_INCHES;
private:
	ofBaseApp*
		m_App;
	std::unique_ptr<ClockPattern>
		m_ClockPattern;
	ofFbo
		m_RenderFbo;
	glm::vec2
		m_Dimension,
		m_Resolution;
	int
		m_DotsPerInch;
	bool
		m_NeedsUpdate;

public:
	SheetCanvas(ofBaseApp* baseApp);
	~SheetCanvas();

public:
	void setDimension(glm::vec2);
	void setDotsPerInch(int);
	const ofFbo& getFbo();
	const glm::vec2& getDimension();
	const glm::vec2& getResolution();
	const int& getDPI();
	void update();
	void reload();
};

