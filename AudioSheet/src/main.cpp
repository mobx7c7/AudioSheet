#include "ofMain.h"
#include "ofApp.h"

int main()
{
	ofGLFWWindowSettings settings;
	settings.setGLVersion(3, 3);
	settings.decorated = true;
	settings.resizable = true;

	settings.setSize(1280, 720);
	auto mainWnd = ofCreateWindow(settings);
	auto mainApp = std::make_shared<ofApp>();

	ofRunApp(mainWnd, mainApp);
	ofRunMainLoop();
}
