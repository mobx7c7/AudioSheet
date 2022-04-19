#pragma once
#include "ofMain.h"
#include "SheetCanvas.h"
#include "DeviceImageReader.h"
#include "SoundPlayer.h"
#include "SoundPlayerView.h"
#include "ofxDatGui.h"
#include "View.h"
#include <mutex>

class ofApp : public ofBaseApp
{
	struct UI
	{
		ofxDatGui
			*PaperPanel,
			*SoundPanel;
		ofxDatGuiTextInput
			*SheetDPI,
			*SheetDimension,
			*SheetResolution,
			*PlayerFile,
			*PlayerFormat,
			*PlayerTotalTime,
			*PlayerPlayedTime,
			*PlayerFilledTime,
			*PlayerBufferTime,
			*PlayerBufferSize;
		ofxDatGuiSlider
			*SoundPosition,
			*SoundBufferRead,
			*ImageBufferRead;
		ofxDatGuiButton
			*PlayerOpen,
			*PlayerClose;
		ofxDatGuiToggle
			*SoundBufferFillEnabled,
			*SoundBufferReadEnabled;
	}m_Ui;
	
	View
		*m_SheetCanvasView,
		*m_SoundPlayerView;

	std::unique_ptr<SheetCanvas>
		m_SheetCanvas;

	std::unique_ptr<DeviceImageReader>
		m_DeviceImageReader;

	std::unique_ptr<SoundPlayer>
		m_SoundPlayer;

	void onSliderEvent(ofxDatGuiSliderEvent e);
	void onToggleEvent(ofxDatGuiToggleEvent e);
	void onButtonEvent(ofxDatGuiButtonEvent e);

	void resetFields();
	void openSoundFileDialog();
	void closeSoundFile();

public:
	void setup();
	void update();
	void draw();
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
