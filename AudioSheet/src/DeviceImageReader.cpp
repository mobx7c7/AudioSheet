#include "DeviceImageReader.h"
#include <ofGraphics.h>
#include <ofLog.h>
#include <ofEvents.h>
#include "StreamInput/Platform/Windows/WiaScannerInput.h"
#include "StreamInput/Platform/Windows/WiaScannerInput2.h"

ofLog& operator<<(ofLog& logger, PBITMAPINFOHEADER header)
{
	return logger 
	<< "BITMAPINFOHEADER" << std::endl
	<< "Size:" << header->biSize << std::endl
	<< "Width:" << header->biWidth << std::endl
	<< "Height:" << header->biHeight << std::endl
	<< "Planes:" << header->biPlanes << std::endl
	<< "BitCount:" << header->biBitCount << std::endl
	<< "Compression:" << header->biCompression << std::endl
	<< "SizeImage:" << header->biSizeImage << std::endl
	<< "XPelsPerMeter:" << header->biXPelsPerMeter << std::endl
	<< "YPelsPerMeter:" << header->biYPelsPerMeter << std::endl
	<< "ClrUsed:" << header->biClrUsed << std::endl
	<< "ClrImportant:" << header->biClrImportant;
}

void DeviceImageReader::setup()
{
	magFbo.allocate(256, 256, GL_RGBA);
	try
	{
		scannerInput = std::make_shared<WiaScannerInput2>();
		scannerInput->setEventCallback(std::bind(&DeviceImageReader::onDeviceEvent, this, std::placeholders::_1));
	}
	catch (const std::exception& e)
	{
		ofLogError() << e.what();
	}
}

void DeviceImageReader::acquire()
{
	if (scannerInput)
	{
		try
		{
			scannerInput->open(0);
			scannerInput->read();
			scannerInput->close();
		}
		catch (const std::exception& e)
		{
			ofLogError() << e.what();
		}
	}
}

void DeviceImageReader::onDeviceEvent(const StreamEventBase& eventBase)
{
	auto& inputEvent = static_cast<const ScannerInputEvent&>(eventBase);

	ofLogNotice logger("DeviceImageReader");

	switch (inputEvent.status)
	{
		case StreamStatus::Running:
			std::copy(inputEvent.data, inputEvent.data + inputEvent.size, std::back_inserter(receiveBuffer));
			if (inputEvent.offset == 0 && receiveBuffer.size() >= sizeof(BITMAPINFOHEADER))
				logger << reinterpret_cast<PBITMAPINFOHEADER>(receiveBuffer.data());
			else
				logger << inputEvent.progress << "% complete";
			break;
		case StreamStatus::EndOfStream:
			logger << "Transfer EOF";
			break;
		case StreamStatus::Finished:
			logger << "Transfer finished";
			allocateTextureFromBMP();
			break;
		default:
			break;
	}
}

void DeviceImageReader::readPixelData(PBITMAPINFOHEADER pHeader, std::vector<uint8_t> &obuf)
{
	// Calculating Surface Stride
	// Source: BITMAPINFOHEADER structure page at Windows API Reference.
	int stride = (((pHeader->biWidth * pHeader->biBitCount) + 31) & ~31) >> 3;
	int bytesPerLine = pHeader->biWidth * pHeader->biBitCount / 8;

	// Color table: 2^n indices, 4 bytes per index
	int colorTableSize = pHeader->biClrUsed * 4;

	auto beg = receiveBuffer.begin();
	auto end = receiveBuffer.end();

	beg += sizeof(BITMAPINFOHEADER);
	beg += colorTableSize;

	if (stride == bytesPerLine)
	{
		std::copy(beg, end, std::back_inserter(obuf));
	}
	else
	{
		while (beg < end)
		{
			std::copy(beg, beg + bytesPerLine, std::back_inserter(obuf));
			beg += stride;
		}
	}
}

void DeviceImageReader::allocateTextureFromBMP()
{
	PBITMAPINFOHEADER pHeader = reinterpret_cast<PBITMAPINFOHEADER>(receiveBuffer.data());

	if (pHeader->biCompression == BI_RGB)
	{
		// If biHeight...
		// - positive: bitmap is a bottom - up DIB and its origin is the lower - left corner.
		// - negative: bitmap is a top - down DIB and its origin is the upper - left corner.
		// Source: BITMAPINFOHEADER structure page @ Windows API Reference.

		bool isTopDown = pHeader->biHeight < 0;
		int width = pHeader->biWidth;
		int height = isTopDown ? ~pHeader->biHeight : pHeader->biHeight;
		int format = 0, type = 0;

		switch (pHeader->biBitCount)
		{
			case 8: // Grayscale
				format = GL_RED;
				type = GL_UNSIGNED_BYTE;
				break;
			case 24: // RGB24, RGB888
				format = GL_BGR;
				type = GL_UNSIGNED_BYTE;
				break;
		}

		if (format && type)
		{
			std::vector<uint8_t> pixelBuffer;
			readPixelData(pHeader, pixelBuffer);

			imageTexture.allocate(width, height, GL_RGB, format, type);
			imageTexture.loadData(pixelBuffer.data(), width, height, format);
			imageTexture.setTextureMinMagFilter(GL_LINEAR, GL_NEAREST);
		}
	}
}

void DeviceImageReader::draw(float x, float y, float w, float h) const
{
	draw(ofRectangle(x, y, w, h));
}

void DeviceImageReader::draw(const ofRectangle & rect) const
{
	if (imageTexture.isAllocated())
	{
		glm::vec3 mousePosition(ofGetMouseX(), ofGetMouseY(),0.0f);

		glm::vec3 mouseOffset(20, 20, 0);

		ofRectangle imageRect(rect.getPosition(), imageTexture.getWidth(), imageTexture.getHeight());

		ofRectangle magDrawRect(mousePosition + mouseOffset, magFbo.getWidth(), magFbo.getHeight());

		float magScale = 8.0f;

		magFbo.begin(true);
		{
			ofClear(ofColor::black);
			auto imageMagRect = imageRect;

			ofRectangle magViewRect(glm::vec2(), magDrawRect.getWidth(), magDrawRect.getHeight());
			magViewRect.setPosition(-magViewRect.getCenter());

			imageMagRect.scale(magScale);
			imageMagRect.setPosition((-mousePosition * magScale) - magViewRect.getPosition());
			imageTexture.draw(imageMagRect);
		}
		magFbo.end();

		imageTexture.draw(imageRect);
		magFbo.draw(magDrawRect);

		ofPushStyle();
		ofStyle style;
		style.bFill = false;
		style.color = ofColor::black;
		ofSetStyle(style);
		ofDrawRectangle(magDrawRect);
		ofPopStyle();
	}
}

float DeviceImageReader::getHeight() const
{
	return 0.0f;
}

float DeviceImageReader::getWidth() const
{
	return 0.0f;
}
