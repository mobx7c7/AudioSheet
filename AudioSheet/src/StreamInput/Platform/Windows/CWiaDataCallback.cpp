#include "CWiaDataCallback.h"
#include "BitmapUtil.h"
#include <ofLog.h>

std::ostream& operator<<(std::ostream& os, const PWIA_DATA_CALLBACK_HEADER &pHeader)
{
	return os << "PWIA_DATA_CALLBACK_HEADER("
		<< "Size=" << pHeader->lSize << ","
		<< "BufferSize=" << pHeader->lBufferSize << ","
		<< "PageCount=" << pHeader->lPageCount << ")";
}

std::ostream& operator<<(std::ostream& os, const PBITMAPFILEHEADER &pHeader)
{
	char* id = (char*)&pHeader->bfType;

	return os << "BITMAPFILEHEADER("
		<< "Type=" << id[0] << id[1] << ","
		<< "Size=" << pHeader->bfSize << ","
		<< "Reserved1=" << pHeader->bfReserved1 << ","
		<< "Reserved2=" << pHeader->bfReserved2 << ","
		<< "OffBits=" << pHeader->bfOffBits << ")";
}

std::ostream& operator<<(std::ostream& os, const PBITMAPINFOHEADER &pHeader)
{
	return os << "BITMAPINFOHEADER("
		<< "Size=" << pHeader->biSize << ","
		<< "Width=" << pHeader->biWidth << ","
		<< "Height=" << pHeader->biHeight << ","
		<< "Planes=" << pHeader->biPlanes << ","
		<< "BitCount=" << pHeader->biBitCount << ","
		<< "Compression=" << pHeader->biCompression << ","
		<< "SizeImage=" << pHeader->biSizeImage << ","
		<< "XPelsPerMeter=" << pHeader->biXPelsPerMeter << ","
		<< "YPelsPerMeter=" << pHeader->biYPelsPerMeter << ","
		<< "ClrUsed=" << pHeader->biClrUsed << ","
		<< "ClrImportant=" << pHeader->biClrImportant << ")";
}

CWiaDataCallback::CWiaDataCallback(StreamEventCb fnEventCallback)
{
	m_cRef = 0;
	m_bBMP = FALSE;
	m_fnEventCallback = fnEventCallback;
}

HRESULT __stdcall CWiaDataCallback::QueryInterface(REFIID riid, void ** ppvObject)
{
	if (ppvObject == NULL)
	{
		return E_POINTER;
	}

	if (riid == IID_IUnknown)
	{
		*ppvObject = (IUnknown*)this;
	}
	else if (riid == IID_IWiaDataCallback)
	{
		*ppvObject = (IWiaDataCallback*)this;
	}
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();

	return S_OK;
}

ULONG __stdcall CWiaDataCallback::AddRef(void)
{
	return InterlockedIncrement(&m_cRef);
}

ULONG __stdcall CWiaDataCallback::Release(void)
{
	LONG cRef = InterlockedDecrement(&m_cRef);

	if (cRef == 0)
	{
		delete this;
	}

	return cRef;
}

HRESULT CWiaDataCallback::HandleStatus(LONG lStatus, LONG lPercentComplete)
{
	switch (lStatus)
	{
		case IT_STATUS_TRANSFER_FROM_DEVICE:
			// "Reading data from the device (%d%% complete)"
			break;
		case IT_STATUS_PROCESSING_DATA:
			// "Processing data (%d%% complete)"
			break;
		case IT_STATUS_TRANSFER_TO_CLIENT:
			// "Transferring data (%d%% complete)"
			break;
		default:
			return E_INVALIDARG;
	}

	return S_OK;
}

HRESULT CWiaDataCallback::HandleHeader(PBYTE pbBuffer)
{
	HRESULT hr = S_OK;

	PWIA_DATA_CALLBACK_HEADER pHeader = (PWIA_DATA_CALLBACK_HEADER)pbBuffer;

	// Determine if this is a BMP transfer

	m_bBMP = pHeader->guidFormatID == WiaImgFmt_MEMORYBMP || pHeader->guidFormatID == WiaImgFmt_BMP;

	return hr;
}

HRESULT CWiaDataCallback::HandleData(LONG lStatus, LONG lPercentComplete, LONG lOffset, LONG lLength, PBYTE pbBuffer)
{
	// Invoke the callback function

	ScannerInputEvent inputEvent;
	std::memset(&inputEvent, 0, sizeof(inputEvent));
	inputEvent.status = StreamStatus::Running;
	inputEvent.data = reinterpret_cast<char*>(pbBuffer);
	inputEvent.size = static_cast<size_t>(lLength);
	inputEvent.offset = static_cast<int>(lOffset);
	inputEvent.page = static_cast<int>(m_lCount);
	inputEvent.progress = static_cast<int>(lPercentComplete);
	m_fnEventCallback(inputEvent);

	return S_OK;
}

HRESULT CWiaDataCallback::HandleNewPage()
{
	HRESULT hr  S_OK;

	ScannerInputEvent inputEvent;
	std::memset(&inputEvent, 0, sizeof(inputEvent));
	inputEvent.status = StreamStatus::Finished;
	inputEvent.page = static_cast<int>(m_lCount);
	inputEvent.progress = 100;
	m_fnEventCallback(inputEvent);

	m_lCount++;

	return hr;
}

HRESULT __stdcall CWiaDataCallback::BandedDataCallback(
	LONG lMessage,
	LONG lStatus,
	LONG lPercentComplete,
	LONG lOffset,
	LONG lLength,
	LONG lReserved,
	LONG lResLength,
	PBYTE pbBuffer)
{
	HRESULT hr = S_OK;

	switch (lMessage)
	{
		case IT_MSG_DATA_HEADER:
			hr = HandleHeader(pbBuffer);
			break;
		case IT_MSG_DATA:
			hr = HandleData(lStatus, lPercentComplete, lOffset, lLength, pbBuffer);
			break;
		case IT_MSG_STATUS:
			hr = HandleStatus(lStatus, lPercentComplete);
			break;
		case IT_MSG_TERMINATION:
		case IT_MSG_NEW_PAGE:
			hr = HandleNewPage();
			break;
	}

	return hr;
}
