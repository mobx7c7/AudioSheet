#pragma once
#include <ofColor.h>
#include <ofRectangle.h>
#include <ofEvents.h>
#include <ofFbo.h>

class View
{
	ofRectangle m_ViewRect;
	ofStyle m_Style;
	ofFbo m_RenderFbo;
	std::string m_Name;
	bool m_Visible;

	//Methods
	void onUpdate(ofEventArgs &e);
	void onDraw(ofEventArgs &e);
	void onExit(ofEventArgs &e);
	//Functions
	void registerEvents();
	void unregisterEvents();

protected:
	//virtual void setup() {}
	virtual void update() {}
	virtual void draw() {}
	virtual void exit() {}
	virtual void viewResized() {}
	View();

public:
	virtual ~View();
	void setName(std::string);
	std::string getName();
	void setPosition(glm::vec2);
	glm::vec2 getPosition();
	void setSize(glm::vec2);
	glm::vec2 getSize();
	void setRect(ofRectangle);
	ofRectangle getRect();
	void setFbo(ofFbo);
	ofFbo getFbo();
	void setBackgroundColor(ofColor);
	ofColor getBackgroundColor();
	void setForegroundColor(ofColor);
	ofColor getForegroundColor();
	void setVisible(bool);
	bool isVisible();
};
