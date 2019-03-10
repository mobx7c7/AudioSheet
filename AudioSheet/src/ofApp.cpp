#include "ofApp.h"
#include "ofEventUtils.h"

namespace Metrics
{
	struct Dimension
	{
		glm::vec2 value;
		std::string postfix;
		Dimension(const glm::vec2& value, std::string postfix = "") : value(value), postfix(postfix) {}
		friend std::ostream& operator<<(std::ostream& os, const Dimension& d)
		{
			return os << d.value.x << "x" << d.value.y << d.postfix;
		}
	};

	template<typename T, typename = std::enable_if<std::is_pod<T>::value>::type>
	struct Value
	{
		T value;
		std::string unit;
		Value(const T& value, std::string postfix = "") : value(value), unit(postfix) {}
		friend std::ostream& operator<<(std::ostream& os, const Value& u)
		{
			return os << u.value << u.unit;
		}
	};
}

void ofApp::setup()
{
	sheetCanvas = std::make_unique<SheetCanvas>(this);
	sheetCanvas->setup();

	needsUpdate = true;
}

void ofApp::update()
{
	if (needsUpdate)
	{
		sheetCanvas->update();
		needsUpdate = false;
	}
}

void ofApp::draw()
{
	ofClear(ofColor::slateGray);
	ofViewport(ofGetWindowRect());

	sheetCanvas->draw(ofGetWindowRect());

	std::stringstream ss;
	ss
		<< "FPS        = " << ofGetFrameRate() << std::endl
		<< std::endl
		<< "DPI        = " << Metrics::Value<uint32_t>(sheetCanvas->getDPI()) << std::endl
		<< "Dimension  = " << Metrics::Dimension(sheetCanvas->getDimension(), "mm") << std::endl
		<< "Resolution = " << Metrics::Dimension(sheetCanvas->getResolution(), "px") << std::endl;

	ofDrawBitmapString(ss.str(), glm::vec2(0, 10));
}

void ofApp::keyPressed(int key)
{
	switch (key)
	{
		case OF_KEY_F5:
			sheetCanvas->reload();
			needsUpdate = true;
			break;
	}
}

void ofApp::keyReleased(int key)
{

}

void ofApp::mouseMoved(int x, int y)
{

}

void ofApp::mouseDragged(int x, int y, int button)
{

}

void ofApp::mousePressed(int x, int y, int button)
{

}

void ofApp::mouseReleased(int x, int y, int button)
{

}

void ofApp::mouseEntered(int x, int y)
{

}

void ofApp::mouseExited(int x, int y)
{

}

void ofApp::windowResized(int w, int h)
{
	
}

void ofApp::gotMessage(ofMessage msg)
{

}

void ofApp::dragEvent(ofDragInfo dragInfo)
{

}
