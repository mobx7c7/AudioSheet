#pragma once
#include "ofMain.h"
#include "SheetCanvas.h"
#include "SheetStream.h"
#include "DeviceImageReader.h"
#include <ofSoundStream.h>
#include <ofSoundBuffer.h>
#include <mutex>

class ofApp : public ofBaseApp
{
private:
	std::unique_ptr<SheetCanvas>
		sheetCanvas;
	std::unique_ptr<SheetStream>
		sheetStream;
	std::unique_ptr<DeviceImageReader>
		deviceImageReader;
	ofSoundStream
		soundStream;
	std::vector<short>
		audioBuffer;
	std::mutex
		audioMutex;
	int
		audioOffset = 0;
	bool
		needsUpdate,
		audioFilled;
public:
	void setup();
	void update();
	void draw();
	void sheetFrameOut(const SheetFrame& frame);
	void audioOut(ofSoundBuffer & buffer);
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
};
