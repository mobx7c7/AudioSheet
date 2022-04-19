#include "ofApp.h"
#include "ofEventUtils.h"
#include "ofSystemUtils.h"
#include "SheetCanvasView.h"
#include "StreamUtils.h"
#include "SoundPlayer\AVSoundPlayer.h"
#include "SoundPlayer\AVSoundPlayerView.h"

void ofApp::setup()
{
	ofLogToConsole();
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetVerticalSync(true);
	ofSetEscapeQuitsApp(false);
	ofSetWindowTitle("AudioSheet");

	if (true)
	{
		m_SoundPlayer.reset(new AVSoundPlayer());
		m_SoundPlayer->setup();
		auto soundPlayerView = new AVSoundPlayerView(m_SoundPlayer.get());
		soundPlayerView->setup();
		m_SoundPlayerView = soundPlayerView;
	}
	if (false)
	{
		m_DeviceImageReader = std::make_unique<DeviceImageReader>();
		m_DeviceImageReader->setup();
		m_DeviceImageReader->acquire();
		m_SheetCanvas = std::make_unique<SheetCanvas>(this);
	}

	if(m_SheetCanvas)
	{
		//m_SheetCanvasView = new SheetCanvasView(m_SheetCanvas.get());
		//m_SheetCanvasView->setBackgroundColor(ofColor::lightSteelBlue);
		//m_SheetCanvasView->setForegroundColor(ofColor::black);

		m_Ui.PaperPanel = new ofxDatGui(ofxDatGuiAnchor::TOP_LEFT);
		m_Ui.PaperPanel->addHeader("Page settings", false);
		m_Ui.PaperPanel->addFRM();

		m_Ui.SheetDPI = m_Ui.PaperPanel->addTextInput("DPI");
		m_Ui.SheetDPI->setEnabled(false);
		m_Ui.SheetDPI->setEnabled(false);

		m_Ui.SheetDimension = m_Ui.PaperPanel->addTextInput("Dimension");
		m_Ui.SheetDimension->setEnabled(false);
		m_Ui.SheetDimension->setTextUpperCase(false);

		m_Ui.SheetResolution = m_Ui.PaperPanel->addTextInput("Resolution");
		m_Ui.SheetResolution->setEnabled(false);
		m_Ui.SheetResolution->setTextUpperCase(false);

		if (m_Ui.SheetDPI)
		{
			std::stringstream ss;
			ss << StreamUtils::Metrics::Value<uint32_t>(m_SheetCanvas->getDPI());
			m_Ui.SheetDPI->setText(ss.str());
		}
		if (m_Ui.SheetDimension)
		{
			std::stringstream ss;
			ss << StreamUtils::Metrics::Dimension(m_SheetCanvas->getDimension(), "mm");
			m_Ui.SheetDimension->setText(ss.str());
		}
		if (m_Ui.SheetResolution)
		{
			std::stringstream ss;
			ss << StreamUtils::Metrics::Dimension(m_SheetCanvas->getResolution(), "px");
			m_Ui.SheetResolution->setText(ss.str());
		}
	}
	//////////////////////////////////////////////////////////////

	if(m_SoundPlayerView)
	{
		m_Ui.SoundPanel = new ofxDatGui(ofxDatGuiAnchor::TOP_LEFT);
		m_Ui.SoundPanel->onSliderEvent(this, &ofApp::onSliderEvent);
		m_Ui.SoundPanel->onToggleEvent(this, &ofApp::onToggleEvent);
		m_Ui.SoundPanel->onButtonEvent(this, &ofApp::onButtonEvent);
		m_Ui.SoundPanel->addHeader("Sound panel", false);
		
		m_Ui.SoundPanel->addLabel("Player details");
		{
			m_Ui.PlayerFile = m_Ui.SoundPanel->addTextInput("File");
			m_Ui.PlayerFile->setEnabled(false);
			m_Ui.PlayerFile->setTextUpperCase(false);

			m_Ui.PlayerFormat = m_Ui.SoundPanel->addTextInput("Format");
			m_Ui.PlayerFormat->setEnabled(false);
			m_Ui.PlayerFormat->setTextUpperCase(false);

			m_Ui.PlayerTotalTime = m_Ui.SoundPanel->addTextInput("Total time");
			m_Ui.PlayerTotalTime->setEnabled(false);
			m_Ui.PlayerTotalTime->setTextUpperCase(false);

			m_Ui.PlayerPlayedTime = m_Ui.SoundPanel->addTextInput("Played time");
			m_Ui.PlayerPlayedTime->setEnabled(false);
			m_Ui.PlayerPlayedTime->setTextUpperCase(false);

			m_Ui.PlayerFilledTime = m_Ui.SoundPanel->addTextInput("Filled time");
			m_Ui.PlayerFilledTime->setEnabled(false);
			m_Ui.PlayerFilledTime->setTextUpperCase(false);

			m_Ui.PlayerBufferTime = m_Ui.SoundPanel->addTextInput("Buffer time");
			m_Ui.PlayerBufferTime->setEnabled(false);
			m_Ui.PlayerBufferTime->setTextUpperCase(false);

			m_Ui.PlayerBufferSize = m_Ui.SoundPanel->addTextInput("Buffer size");
			m_Ui.PlayerBufferSize->setEnabled(false);
			m_Ui.PlayerBufferSize->setTextUpperCase(false);
		}
		m_Ui.SoundPanel->addBreak();
		{
			auto folder = m_Ui.SoundPanel->addFolder("FIFO Switches");

			m_Ui.SoundBufferFillEnabled = folder->addToggle("Sound Fill", true);
			m_Ui.SoundBufferReadEnabled = folder->addToggle("Sound Read", true);
		}
		{
			auto folder = m_Ui.SoundPanel->addFolder("FIFO Available");

			m_Ui.SoundBufferRead = folder->addSlider("Sound", 0, 1);
			m_Ui.SoundBufferRead->setEnabled(false);
			m_Ui.SoundBufferRead->setPrecision(0);

			m_Ui.ImageBufferRead = folder->addSlider("Image", 0, 1);
			m_Ui.ImageBufferRead->setEnabled(false);
			m_Ui.ImageBufferRead->setPrecision(0);
		}
		m_Ui.SoundPanel->addBreak();
		{
			m_Ui.SoundPosition = m_Ui.SoundPanel->addSlider("Position", 0, 1);
			m_Ui.SoundPosition->setEnabled(false);
			m_Ui.SoundPosition->setPrecision(1);
		}
		m_Ui.SoundPanel->addBreak();
		{
			m_Ui.PlayerOpen = m_Ui.SoundPanel->addButton("Open file");
			m_Ui.PlayerClose = m_Ui.SoundPanel->addButton("Close file");
		}
	}
}

void ofApp::resetFields()
{
	m_Ui.PlayerFile->setText("");
	m_Ui.PlayerFormat->setText("");
	m_Ui.PlayerFilledTime->setText("");
	m_Ui.PlayerPlayedTime->setText("");
	m_Ui.PlayerBufferSize->setText("");
	m_Ui.PlayerBufferTime->setText("");
	m_Ui.SoundPosition->setMax(1);
	m_Ui.SoundPosition->setValue(0);
	m_Ui.SoundBufferRead->setMax(1);
	m_Ui.SoundBufferRead->setValue(0);
	m_Ui.ImageBufferRead->setMax(1);
	m_Ui.ImageBufferRead->setValue(0);
}

void ofApp::openSoundFileDialog()
{
	auto result = ofSystemLoadDialog("Load sound");

	if (result.bSuccess)
	{
		boost::filesystem::path path(result.getPath());

		try
		{
			m_SoundPlayer->load(path);

			m_Ui.PlayerFile->setText(path.filename().string());
			m_Ui.PlayerFormat->setText(m_SoundPlayer->getFormatInfo());
			{
				//std::stringstream ss;
				//ss << StreamUtils::Metrics::Value<int>(m_SoundPlayer->getDuration(), "ms");
				//m_Ui.PlayerBufferTime->setText(ss.str());
			}
			{
				std::stringstream ss;
				ss << StreamUtils::Metrics::Value<int>(m_SoundPlayer->getBufferSize()*m_SoundPlayer->getSampleSize(), "bytes");
				m_Ui.PlayerBufferSize->setText(ss.str());
			}
			{
				std::stringstream ss;
				ss << StreamUtils::Metrics::Value<int>(m_SoundPlayer->getBufferSecs() * 1000, "ms");
				m_Ui.PlayerBufferTime->setText(ss.str());
			}
			auto soundPlayerView = static_cast<AVSoundPlayerView*>(m_SoundPlayerView);
			m_Ui.SoundPosition->setMax(static_cast<float>(m_SoundPlayer->getDuration())/m_SoundPlayer->getSampleRate());
			m_Ui.SoundBufferRead->setMax(m_SoundPlayer->getBufferSize());
			m_Ui.ImageBufferRead->setMax(soundPlayerView->getBufferSize());

			// Será disparado por AvSoundPlayerView 
			// ao fazer preenchimento de buffer.
			//m_SoundPlayer->play(); 
		}
		catch (const AVSoundException &e)
		{
			ofSystemAlertDialog(e.what());
			resetFields();
		}
	}
}

void ofApp::closeSoundFile()
{
	if (m_SoundPlayer->isLoaded())
	{
		m_SoundPlayer->unload();
		resetFields();
	}
}

void ofApp::onSliderEvent(ofxDatGuiSliderEvent e)
{

}

void ofApp::onToggleEvent(ofxDatGuiToggleEvent e)
{
	if (e.target == m_Ui.SoundBufferReadEnabled)
	{
		m_SoundPlayer->setBufferReadEnabled(e.target->getChecked());
	}
	if (e.target == m_Ui.SoundBufferFillEnabled)
	{
		m_SoundPlayer->setBufferFillEnabled(e.target->getChecked());
	}
}

void ofApp::onButtonEvent(ofxDatGuiButtonEvent e)
{
	if (e.target == m_Ui.PlayerOpen)
	{
		openSoundFileDialog();
	}
	if (e.target == m_Ui.PlayerClose)
	{
		closeSoundFile();
	}
}

void ofApp::update()
{
	if (m_SoundPlayer && m_SoundPlayer->isLoaded())
	{
		auto duration = m_SoundPlayer->getDuration();

		auto soundPlayerView = static_cast<SoundPlayerView*>(m_SoundPlayerView);

		if (m_Ui.SoundPosition)
			m_Ui.SoundPosition->setValue(m_SoundPlayer->getNumSecondsPlayed());
		
		if (m_Ui.SoundBufferRead)
			m_Ui.SoundBufferRead->setValue(m_SoundPlayer->getBufferReadAvailable());

		if (m_Ui.ImageBufferRead)
			m_Ui.ImageBufferRead->setValue(soundPlayerView->getBufferReadAvailable());

		if (m_Ui.PlayerFilledTime || m_Ui.PlayerPlayedTime)
		{
			auto timeToVec = [](double timeInSecs) -> glm::ivec4
			{
				auto dtime = glm::dvec4(timeInSecs);
				dtime /= glm::dvec4(1440, 60, 1, 0.001);

				auto itime = glm::ivec4(dtime);
				itime %= glm::ivec4(24, 60, 60, 1000);

				return itime;
			};
			
			const char* timePrintFormat = "%02d:%02d:%02d:%03d";
			
			std::array<char, 13> buf;

			if(m_Ui.PlayerFilledTime)
			{
				auto time = timeToVec(m_SoundPlayer->getNumSecondsFilled());
				snprintf(buf.data(), buf.size(), timePrintFormat, time.x, time.y, time.z, time.w);
				m_Ui.PlayerFilledTime->setText(buf.data());
			}

			if(m_Ui.PlayerPlayedTime)
			{
				auto time = timeToVec(m_SoundPlayer->getNumSecondsPlayed());
				snprintf(buf.data(), buf.size(), timePrintFormat, time.x, time.y, time.z, time.w);
				m_Ui.PlayerPlayedTime->setText(buf.data());
			}
		}
	}

	auto windowRect = ofGetWindowRect();

	if (m_Ui.SoundPanel)
	{
		windowRect.translateX(m_Ui.SoundPanel->getWidth());
		windowRect.width -= m_Ui.SoundPanel->getWidth();
	}

	if (m_SheetCanvasView)
		m_SheetCanvasView->setRect(windowRect);

	if (m_SoundPlayerView)
		m_SoundPlayerView->setRect(windowRect);
}

void ofApp::draw()
{
	auto windowRect = ofGetWindowRect();
	auto bgColor = ofColor::lightSlateGray;
	auto fgColor = ofColor::lightSteelBlue;

	ofClear(bgColor);
	ofViewport(windowRect);
	ofSetupScreen();

	if (m_DeviceImageReader)
		m_DeviceImageReader->draw(windowRect);
}

void ofApp::keyPressed(int key)
{
	const double seekSecsStep = 1.0;

	auto soundPlayerView = static_cast<AVSoundPlayerView*>(m_SoundPlayerView);

	switch (key)
	{
		case OF_KEY_F1:
			m_SoundPlayer->play();
			break;
		case OF_KEY_F2:
			m_SoundPlayer->pause();
			break;
		case OF_KEY_F3:
			m_SoundPlayer->seek(-seekSecsStep);
			break;
		case OF_KEY_F4:
			m_SoundPlayer->seek(seekSecsStep);
			break;
		case OF_KEY_F5:
			//m_SheetCanvas->reload();
			soundPlayerView->reload();
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
