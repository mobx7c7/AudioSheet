#include "View.h"
#include <ofEventUtils.h>
#include <ofGraphics.h>

View::View()
	: m_ViewRect(glm::vec2(), glm::vec2(100))
	, m_Visible(true)
	, m_Name("View")
{
	registerEvents();

	m_Style.bFill = false;
	m_Style.color = ofColor::white;
	m_Style.bgColor = ofColor::black;
	m_Style.lineWidth = 4.0f;
	m_Style.blendingMode = ofBlendMode::OF_BLENDMODE_DISABLED;
	//m_Style.drawBitmapMode = ofDrawBitmapMode::OF_BITMAPMODE_MODEL;
	m_Style.drawBitmapMode = ofDrawBitmapMode::OF_BITMAPMODE_VIEWPORT;
}
View::~View()
{
	unregisterEvents();
}
void View::onUpdate(ofEventArgs &e)
{
	update();
}
void View::onDraw(ofEventArgs &e)
{
	if (!m_Visible) return;

	auto viewRectZeroPos = ofRectangle(glm::vec2(), m_ViewRect.width, m_ViewRect.height);

	ofPushMatrix();
	{
		ofPushView();
		{
			if (m_RenderFbo.isAllocated())
			{
				m_RenderFbo.begin();
				{
					ofClear(m_Style.bgColor);
					draw();
				}
				m_RenderFbo.end();

				ofViewport(m_ViewRect);
				ofSetupScreen();
				m_RenderFbo.draw(viewRectZeroPos);
			}
			else
			{
				ofViewport(m_ViewRect);
				ofSetupScreen();
				ofPushStyle();
				{
					ofSetColor(m_Style.bgColor);
					ofDrawRectangle(viewRectZeroPos);
				}
				ofPopStyle();
				draw();
			}

			ofPushStyle();
			{
				static const glm::vec2 fontOffset(0, 10);

				ofSetStyle(m_Style);
				ofDrawBitmapString(m_Name, fontOffset + glm::vec2(4));
				ofDrawRectangle(viewRectZeroPos);
			}
			ofPopStyle();
		}
		ofPopView();
	}
	ofPopMatrix();
}
void View::onExit(ofEventArgs &e)
{
	exit();
	delete this;
}
void View::registerEvents()
{
	ofAddListener(ofEvents().update, this, &View::onUpdate, OF_EVENT_ORDER_BEFORE_APP);
	ofAddListener(ofEvents().draw, this, &View::onDraw, OF_EVENT_ORDER_AFTER_APP);
	ofAddListener(ofEvents().exit, this, &View::onExit);
	//ofAddListener(ofEvents().windowResized, this, &View::onWindowResized);
}
void View::unregisterEvents()
{
	ofRemoveListener(ofEvents().update, this, &View::onUpdate, OF_EVENT_ORDER_BEFORE_APP);
	ofRemoveListener(ofEvents().draw, this, &View::onDraw, OF_EVENT_ORDER_AFTER_APP);
	ofRemoveListener(ofEvents().exit, this, &View::onExit);
	//ofRemoveListener(ofEvents().windowResized, this, &View::onWindowResized);
}
void View::setName(std::string name)
{
	m_Name = name;
}
std::string View::getName()
{
	return m_Name;
}
void View::setPosition(glm::vec2 position)
{
	m_ViewRect.setPosition(position.x, position.y);
}
glm::vec2 View::getPosition()
{
	return glm::vec2(m_ViewRect.x, m_ViewRect.y);
}
void View::setSize(glm::vec2 size)
{
	if (!(size.x == m_ViewRect.getWidth() && size.y == m_ViewRect.getHeight()))
	{
		m_ViewRect.setSize(size.x, size.y);
		viewResized();
	}
}
glm::vec2 View::getSize()
{
	return glm::vec2(m_ViewRect.width, m_ViewRect.height);
}
void View::setRect(ofRectangle rect)
{
	if (m_ViewRect != rect)
	{
		m_ViewRect = rect;
		viewResized();
	}
}
ofRectangle View::getRect()
{
	return m_ViewRect;
}
void View::setFbo(ofFbo fbo)
{
	m_RenderFbo = fbo;
}
ofFbo View::getFbo()
{
	return m_RenderFbo;
}
void View::setBackgroundColor(ofColor bgColor)
{
	m_Style.bgColor = bgColor;
}
ofColor View::getBackgroundColor()
{
	return m_Style.bgColor;
}
void View::setForegroundColor(ofColor fgColor)
{
	m_Style.color = fgColor;
}
ofColor View::getForegroundColor()
{
	return m_Style.color;
}
void View::setVisible(bool visible)
{
	m_Visible = visible;
}
bool View::isVisible()
{
	return m_Visible;
}
