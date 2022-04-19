#include "SheetCanvas.h"
#include <ofGraphics.h>
#include <ofEventUtils.h>

const float SheetCanvas::UNIT_INCHES = 25.4f;

SheetCanvas::SheetCanvas(ofBaseApp* app)
	: m_App(app)
	, m_DotsPerInch(100)
	, m_Dimension(297, 210)
{
	reload();
}

SheetCanvas::~SheetCanvas()
{}

void SheetCanvas::setDimension(glm::vec2 dimension)
{
	m_Dimension = dimension;
	reload();
}

void SheetCanvas::setDotsPerInch(int dotsPerInch)
{
	m_DotsPerInch = dotsPerInch;
	reload();
}

const ofFbo & SheetCanvas::getFbo() { return m_RenderFbo; }

const glm::vec2 & SheetCanvas::getDimension() { return m_Dimension; }

const glm::vec2 & SheetCanvas::getResolution() { return m_Resolution; }

const int & SheetCanvas::getDPI() { return m_DotsPerInch; }

void SheetCanvas::reload()
{
	m_Resolution = glm::floor((m_Dimension / UNIT_INCHES) * (float)m_DotsPerInch);

	if (!m_RenderFbo.isAllocated() ||
		!(m_Resolution.x == m_RenderFbo.getWidth() && m_Resolution.y == m_RenderFbo.getHeight()))
	{
		ofFboSettings fboSettings;
		fboSettings.width = m_Resolution.x;
		fboSettings.height = m_Resolution.y;
		fboSettings.internalformat = GL_RGBA32F;
		fboSettings.numColorbuffers = 1;

		m_RenderFbo.allocate(fboSettings);	
	}

	if (!m_ClockPattern)
		m_ClockPattern = std::make_unique<ClockPattern>(m_App);

	m_ClockPattern->setup();

	m_NeedsUpdate = true;
}

void SheetCanvas::update()
{
	if (m_ClockPattern)
	{
		if (m_NeedsUpdate)
		{
			m_RenderFbo.begin();
			ofPushMatrix();
			ofClear(ofColor::lightGray);
			{
				m_ClockPattern->draw(ofRectangle(glm::vec2(), m_Resolution));
			}
			ofPopMatrix();
			m_RenderFbo.end();

			m_NeedsUpdate = false;
		}
	}
}