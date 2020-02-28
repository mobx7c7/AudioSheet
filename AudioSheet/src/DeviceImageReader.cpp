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
		<< "{"
		<< "\tSize:" << header->biSize << std::endl
		<< "\tWidth:" << header->biWidth << std::endl
		<< "\tHeight:" << header->biHeight << std::endl
		<< "\tPlanes:" << header->biPlanes << std::endl
		<< "\tBitCount:" << header->biBitCount << std::endl
		<< "\tCompression:" << header->biCompression << std::endl
		<< "\tSizeImage:" << header->biSizeImage << std::endl
		<< "\tXPelsPerMeter:" << header->biXPelsPerMeter << std::endl
		<< "\tYPelsPerMeter:" << header->biYPelsPerMeter << std::endl
		<< "\tClrUsed:" << header->biClrUsed << std::endl
		<< "\tClrImportant:" << header->biClrImportant << std::endl
		<< "}";
}

void DeviceImageReader::setup()
{
	magFbo.allocate(256, 256, GL_RGBA);

	try
	{
		scannerInput = std::make_shared<WiaScannerInput2>();
		scannerInput->setCallback(std::bind(&DeviceImageReader::onDeviceEvent, this, std::placeholders::_1));
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
			std::copy_n(inputEvent.data, inputEvent.size, std::back_inserter(receiveBuffer));
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

void DeviceImageReader::unpackPixelData(PBITMAPINFOHEADER pHeader, int stride, ReceiveBuffer &buf, ReceiveBuffer::iterator beg)
{
	std::vector<uint8_t> unpacked(pHeader->biWidth);

	const int plen = 8 / pHeader->biBitCount;
	const int pval = 256 / pHeader->biBitCount - 1;
	const int mask = pHeader->biBitCount;
	int x, nextByte, nextShift;

	auto end = buf.end();

	while (beg < end)
	{
		for (x = 0; x < pHeader->biWidth; x++)
		{
			nextByte = x / plen;
			nextShift = 7 - (x % plen) * pHeader->biBitCount;
			unpacked[x] = ((beg[nextByte] >> nextShift) & mask) * pval;
		}
		std::copy(unpacked.begin(), unpacked.end(), std::back_inserter(buf));
		
		beg += stride;
	}
}

void DeviceImageReader::readPixelData(PBITMAPINFOHEADER pHeader, std::vector<uint8_t> &obuf)
{
	// Calculating Surface Stride
	// Source: BITMAPINFOHEADER structure page @ Windows API Reference.
	int stride = (((pHeader->biWidth * pHeader->biBitCount) + 31) & ~31) >> 3;
	//int bytesPerLine = pHeader->biBitCount < 8 ? stride : pHeader->biWidth * (pHeader->biBitCount / 8);

	// Color table: 2^n indices, 4 bytes per index
	int colorTableSize = pHeader->biClrUsed * 4;

	auto beg = receiveBuffer.begin();
	auto end = receiveBuffer.end();
	beg += sizeof(BITMAPINFOHEADER);
	beg += colorTableSize;

	if (pHeader->biBitCount < 8)
	{
		unpackPixelData(pHeader, stride, receiveBuffer, beg);
	}
	else
	{
		/*
		if (stride != bytesPerLine)
		{
			while (beg < end)
			{
				std::copy_n(beg, stride, std::back_inserter(obuf));
				beg += stride;
			}
		}
		else
		{
			std::copy(beg, end, std::back_inserter(obuf));
		}
		*/

		std::copy(beg, end, std::back_inserter(obuf));
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

		ofImageType imageType = ofImageType::OF_IMAGE_UNDEFINED;

		switch (pHeader->biBitCount)
		{
			case 1: // 1-bit
			case 8: // Grayscale
				imageType = OF_IMAGE_GRAYSCALE;
				break;
			case 24: // RGB24, RGB888
				imageType = OF_IMAGE_COLOR;
				break;
		}

		if (imageType != ofImageType::OF_IMAGE_UNDEFINED)
		{
			std::vector<uint8_t> pixelBuffer;
			readPixelData(pHeader, pixelBuffer);

			imageBuffer.allocate(width, height, imageType);
			imageBuffer.setFromPixels(pixelBuffer.data(), width, height, imageType, false);
			imageBuffer.setUseTexture(true);
		}
	}
}

void DeviceImageReader::draw(float x, float y, float w, float h) const
{
	draw(ofRectangle(x, y, w, h));
}

void DeviceImageReader::draw(const ofRectangle & rect) const
{
	if (imageBuffer.isAllocated())
	{
		glm::vec3 mousePosition(ofGetMouseX(), ofGetMouseY(), 0.0f);

		glm::vec3 mouseOffset(20, 20, 0);

		ofRectangle imageRect(rect.getPosition(), imageBuffer.getWidth(), imageBuffer.getHeight());

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
			imageBuffer.draw(imageMagRect);
		}
		magFbo.end();

		imageBuffer.draw(imageRect);
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
