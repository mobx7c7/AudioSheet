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

CWiaDataCallback::CWiaDataCallback(StreamEventCb fnEventCallback, PVOID pProgressCallbackParam, LONG * plCount, IStream *** pppStream)
{
	m_cRef = 0;
	m_bBMP = FALSE;
	m_nHeaderSize = 0;
	m_nDataSize = 0;
	m_fnEventCallback = fnEventCallback;
	m_pProgressCallbackParam = pProgressCallbackParam;
	m_plCount = plCount;
	m_pppStream = pppStream;
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

HRESULT CWiaDataCallback::ReAllocBuffer(ULONG nSize)
{
	HRESULT hr;

	// If m_pStream is not initialized yet, create a new stream object

	if (m_pStream == 0)
	{
		hr = CreateStreamOnHGlobal(0, TRUE, &m_pStream);

		if (FAILED(hr))
		{
			return hr;
		}
	}

	// Next, set the size of the stream object

	ULARGE_INTEGER liSize = { nSize };

	hr = m_pStream->SetSize(liSize);

	if (FAILED(hr))
	{
		return hr;
	}

	m_nDataSize = nSize;

	return S_OK;
}

HRESULT CWiaDataCallback::CopyToBuffer(ULONG nOffset, LPCVOID pBuffer, ULONG nSize)
{
	HRESULT hr;

	// First move the stream pointer to the data offset

	LARGE_INTEGER liOffset = { nOffset };

	hr = m_pStream->Seek(liOffset, STREAM_SEEK_SET, 0);

	if (FAILED(hr))
	{
		return hr;
	}

	// Next, write the new data to the stream

	hr = m_pStream->Write(pBuffer, nSize, 0);

	if (FAILED(hr))
	{
		return hr;
	}

	return S_OK;
}

HRESULT CWiaDataCallback::StoreBuffer()
{
	// Increase the successfully transferred buffers array size

	IStream **ppStream = (IStream **)CoTaskMemRealloc(*m_pppStream,(*m_plCount + 1) * sizeof(IStream*));

	if (ppStream == NULL)
	{
		return E_OUTOFMEMORY;
	}

	*m_pppStream = ppStream;

	// Rewind the current buffer

	LARGE_INTEGER liZero = { 0 };

	m_pStream->Seek(liZero, STREAM_SEEK_SET, 0);

	// Store the current buffer as the last successfully transferred buffer

	(*m_pppStream)[*m_plCount] = m_pStream;
	(*m_pppStream)[*m_plCount]->AddRef();

	*m_plCount += 1;

	// Reset the current buffer

	m_pStream.Release();

	m_nDataSize = 0;

	return S_OK;
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

	//ofLogNotice() << pHeader;
	
	// Determine if this is a BMP transfer

	m_bBMP = pHeader->guidFormatID == WiaImgFmt_MEMORYBMP || pHeader->guidFormatID == WiaImgFmt_BMP;

	// For WiaImgFmt_MEMORYBMP transfers, WIA does not send a BITMAPFILEHEADER before the data.
	// In this program, we desire all BMP files to contain a BITMAPFILEHEADER, so add it manually

	m_nHeaderSize = pHeader->guidFormatID == WiaImgFmt_MEMORYBMP ? sizeof(BITMAPFILEHEADER) : 0;

	// Allocate memory for the image if the size is given in the header

	if (pHeader != NULL && pHeader->lBufferSize != 0)
	{
		hr = ReAllocBuffer(m_nHeaderSize + pHeader->lBufferSize);
	}

	return hr;
}

HRESULT CWiaDataCallback::HandleData(LONG lStatus, LONG lPercentComplete, LONG lOffset, LONG lLength, PBYTE pbBuffer)
{
	// Invoke the callback function

	HRESULT hr = HandleStatus(lStatus, lPercentComplete);

	if (FAILED(hr) || hr == S_FALSE)
	{
		return hr;
	}

	// If the buffer is not allocated yet and this is the first block, 
	// and the transferred image is in BMP format, allocate the buffer
	// according to the size information in the bitmap header

	if (m_pStream == NULL && lOffset == 0 && m_bBMP)
	{
		LONG nBufferSize = BitmapUtil::GetBitmapSize(pbBuffer);

		if (nBufferSize != 0)
		{
			hr = ReAllocBuffer(m_nHeaderSize + nBufferSize);

			if (FAILED(hr))
			{
				return hr;
			}
		}
	}

	// If the transfer goes past the buffer, try to expand it

	if (m_nHeaderSize + lOffset + lLength > m_nDataSize)
	{
		hr = ReAllocBuffer(m_nHeaderSize + lOffset + lLength);

		if (FAILED(hr))
		{
			return hr;
		}
	}

	// copy the transfer buffer

	hr = CopyToBuffer(m_nHeaderSize + lOffset, pbBuffer, lLength);

	if (FAILED(hr))
	{
		return hr;
	}

	ScannerInputEvent inputEvent;
	std::memset(&inputEvent, 0, sizeof(inputEvent));
	inputEvent.status	= StreamStatus::Running;
	inputEvent.data		= reinterpret_cast<char*>(pbBuffer);
	inputEvent.size		= static_cast<size_t>(lLength);
	inputEvent.offset	= static_cast<int>(lOffset);
	inputEvent.page		= static_cast<int>(*m_plCount);
	inputEvent.progress	= static_cast<int>(lPercentComplete);
	m_fnEventCallback(inputEvent);

	return hr;
}

HRESULT CWiaDataCallback::HandleNewPage()
{
	HRESULT hr  S_OK;

	if (m_pStream != NULL)
	{
		// For BMP files, we should validate the the image header
		// So, obtain the memory buffer from the stream

		if (m_bBMP)
		{
			// Since the stream is created using CreateStreamOnHGlobal,
			// we can get the memory buffer with GetHGlobalFromStream.

			HGLOBAL hBuffer;

			hr = GetHGlobalFromStream(m_pStream, &hBuffer);

			if (FAILED(hr))
			{
				return hr;
			}

			PBITMAPFILEHEADER pFileHeader = (PBITMAPFILEHEADER)GlobalLock(hBuffer);
			PBITMAPINFOHEADER pInfoHeader = (PBITMAPINFOHEADER)(pFileHeader + 1);

			if (pFileHeader == NULL)
			{
				return HRESULT_FROM_WIN32(GetLastError());
			}

			// Some scroll-fed scanners may return 0 as the bitmap height
			// In this case, calculate the image height and modify the header

			BitmapUtil::FixBitmapHeight(pInfoHeader, m_nDataSize, TRUE);

			// For WiaImgFmt_MEMORYBMP transfers, the WIA service does not 
			// include a BITMAPFILEHEADER preceeding the bitmap data. 
			// In this case, fill in the BITMAPFILEHEADER structure.

			if (m_nHeaderSize != 0)
			{
				BitmapUtil::FillBitmapFileHeader(pInfoHeader, pFileHeader);
			}

			//ofLogNotice() << pFileHeader;
			//ofLogNotice() << pInfoHeader;

			GlobalUnlock(hBuffer);
		}

		ScannerInputEvent inputEvent;
		std::memset(&inputEvent, 0, sizeof(inputEvent));
		inputEvent.status	= StreamStatus::Finished;
		inputEvent.page		= static_cast<int>(*m_plCount);
		inputEvent.progress = 100;
		m_fnEventCallback(inputEvent);

		// Store this buffer in the successfully transferred images array

		hr = StoreBuffer();

		if (FAILED(hr))
		{
			return hr;
		}
	}

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

	if (FAILED(hr) || hr == S_FALSE)
	{
		//TODO: Mensagem
	}

	return hr;
}
