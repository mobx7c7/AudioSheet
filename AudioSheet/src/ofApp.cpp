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
	ofLogToConsole();
	ofSetLogLevel(OF_LOG_NOTICE);

	ofSoundStreamSettings settings;
	settings.setApi(ofSoundDevice::MS_DS);
	settings.setOutListener(this);
	settings.numOutputChannels = 2;
	settings.sampleRate = 44100;
	settings.bufferSize = 512;
	settings.numBuffers = 4;
	soundStream.setup(settings);
	///////////////////////////////////////////////////////////////////////
	
	deviceImageReader = std::make_unique<DeviceImageReader>();
	deviceImageReader->setup();
	deviceImageReader->acquire();
	
	///////////////////////////////////////////////////////////////////////

	sheetCanvas = std::make_unique<SheetCanvas>(this);
	sheetCanvas->setup();

	sheetStream = std::make_unique<SheetStream>();
	sheetStream->setup();
	sheetStream->setFrameCallback(std::bind(&ofApp::sheetFrameOut, this, std::placeholders::_1));
	sheetStream->load(ofToDataPath("audio.flac"));
	while (!sheetStream->isFinished())
	{
		sheetStream->next();
	}
	sheetStream->close();
	audioFilled = true;

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
	deviceImageReader->draw(ofGetWindowRect());

	std::stringstream ss;
	ss
		<< "FPS        = " << ofGetFrameRate() << std::endl
		<< std::endl
		<< "DPI        = " << Metrics::Value<uint32_t>(sheetCanvas->getDPI()) << std::endl
		<< "Dimension  = " << Metrics::Dimension(sheetCanvas->getDimension(), "mm") << std::endl
		<< "Resolution = " << Metrics::Dimension(sheetCanvas->getResolution(), "px") << std::endl;

	ofDrawBitmapString(ss.str(), glm::vec2(0, 10));
}

void ofApp::sheetFrameOut(const SheetFrame& frame)
{
	uint8_t* data = frame.data;

	int s, c;

	for (s = 0; s < frame.samples; s++)
	{
		for (c = 0; c < frame.channels; c++)
		{
			data += frame.sampleSize;
			audioBuffer.push_back(*reinterpret_cast<uint16_t*>(data));
		}
	}
}

void ofApp::audioOut(ofSoundBuffer & buffer)
{
	if (audioOffset < audioBuffer.size() && audioFilled)
	{
		size_t approvedLength = std::min(buffer.getNumFrames() * buffer.getNumChannels(), audioBuffer.size() - audioOffset);
		const auto* audioOffsetPtr = audioBuffer.data() + audioOffset;
		buffer.copyFrom(audioOffsetPtr, buffer.getNumFrames(), buffer.getNumChannels(), buffer.getSampleRate());
		audioOffset += approvedLength;
	}
}

void ofApp::keyPressed(int key)
{
	switch (key)
	{
		case OF_KEY_F5:
			sheetCanvas->reload();
			needsUpdate = true;
			break;
		case 's':
			soundStream.start();
			break;
		case 'e':
			soundStream.stop();
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
