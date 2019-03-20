#include "CWiaTransferCallback.h"
#include <ofLog.h>

CWiaTransferCallback::CWiaTransferCallback()
{
	m_cRef = 1;  // initializing it to 1 so that when object is created, we can call Release() on it. 
	m_lPageCount = 0;
	m_bFeederTransfer = FALSE;
	m_bstrFileExtension = NULL;
	m_bstrDirectoryName = NULL;
	memset(m_szFileName, 0, sizeof(m_szFileName));
}

CWiaTransferCallback::~CWiaTransferCallback()
{
	if (m_bstrDirectoryName)
	{
		SysFreeString(m_bstrDirectoryName);
		m_bstrDirectoryName = NULL;
	}

	if (m_bstrFileExtension)
	{
		SysFreeString(m_bstrFileExtension);
		m_bstrFileExtension = NULL;
	}

}

HRESULT CWiaTransferCallback::InitializeCallback(TCHAR* bstrDirectoryName, BSTR bstrExt, BOOL bFeederTransfer)
{
	HRESULT hr = S_OK;
	m_bFeederTransfer = bFeederTransfer;

	if (bstrDirectoryName)
	{
		m_bstrDirectoryName = SysAllocString(bstrDirectoryName);
		if (!m_bstrDirectoryName)
		{
			hr = E_OUTOFMEMORY;
			ReportError(TEXT("Failed to allocate memory for BSTR directory name"), hr);
			return hr;
		}
	}
	else
	{
		ReportError(TEXT("No directory name was given"));
		return E_INVALIDARG;
	}

	if (bstrExt)
	{
		m_bstrFileExtension = bstrExt;
	}

	return hr;
}

HRESULT CALLBACK CWiaTransferCallback::QueryInterface(REFIID riid, void **ppvObject)
{
	// Validate arguments
	if (NULL == ppvObject)
	{
		HRESULT hr = E_INVALIDARG;
		ReportError(TEXT("Invalid argument passed to QueryInterface()"), hr);
		return hr;
	}

	if (IsEqualIID(riid, IID_IUnknown))
	{
		*ppvObject = static_cast<IUnknown*>(this);
	}
	else if (IsEqualIID(riid, IID_IWiaTransferCallback))
	{
		*ppvObject = static_cast<IWiaTransferCallback*>(this);
	}
	else
	{
		*ppvObject = NULL;
		return (E_NOINTERFACE);
	}

	reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();
	return S_OK;
}

ULONG CALLBACK CWiaTransferCallback::AddRef()
{
	return InterlockedIncrement((long*)&m_cRef);
}

ULONG CALLBACK CWiaTransferCallback::Release()
{
	LONG cRef = InterlockedDecrement((long*)&m_cRef);
	if (0 == cRef)
	{
		delete this;
	}
	return cRef;
}

HRESULT STDMETHODCALLTYPE CWiaTransferCallback::TransferCallback(LONG lFlags, WiaTransferParams* pWiaTransferParams)
{
	HRESULT hr = S_OK;

	if (pWiaTransferParams == NULL)
	{
		hr = E_INVALIDARG;
		ReportError(TEXT("TransferCallback() was called with invalid args"), hr);
		return hr;
	}
	
	switch (pWiaTransferParams->lMessage)
	{
		case WIA_TRANSFER_MSG_STATUS:
			{
				ofLog(OF_LOG_NOTICE, "WIA_TRANSFER_MSG_STATUS - %ld%% complete", pWiaTransferParams->lPercentComplete);
			}
			break;
		case WIA_TRANSFER_MSG_END_OF_STREAM:
			{
				ofLog(OF_LOG_NOTICE, "WIA_TRANSFER_MSG_END_OF_STREAM");
			}
			break;
		case WIA_TRANSFER_MSG_END_OF_TRANSFER:
			{
				ofLog(OF_LOG_NOTICE, "WIA_TRANSFER_MSG_END_OF_TRANSFER");
				ofLog(OF_LOG_NOTICE, "Image Transferred to file %ws", m_szFileName);
			}
			break;
		default:
			break;
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE CWiaTransferCallback::GetNextStream(LONG lFlags, BSTR bstrItemName, BSTR bstrFullItemName, IStream **ppDestination)
{
	ofLog(OF_LOG_NOTICE, "GetNextStream");
	HRESULT hr = S_OK;

	if ((!ppDestination) || (!bstrItemName) || (!m_bstrDirectoryName))
	{
		hr = E_INVALIDARG;
		ReportError(TEXT("GetNextStream() was called with invalid parameters"), hr);
		return hr;
	}

	*ppDestination = NULL;

	if (m_bstrFileExtension)
	{
		//For feeder transfer, append the page count to the filename
		if (m_bFeederTransfer)
		{
			StringCchPrintf(m_szFileName, ARRAYSIZE(m_szFileName), TEXT("%ws\\%ws_page%d.%ws"), m_bstrDirectoryName, bstrItemName, ++m_lPageCount, m_bstrFileExtension);
		}
		else
		{
			StringCchPrintf(m_szFileName, ARRAYSIZE(m_szFileName), TEXT("%ws\\%ws.%ws"), m_bstrDirectoryName, bstrItemName, m_bstrFileExtension);
		}
	}

	else
	{
		// Dont append extension if m_bstrFileExtension = NULL.
		StringCchPrintf(m_szFileName, ARRAYSIZE(m_szFileName), TEXT("%ws\\%ws"), m_bstrDirectoryName, bstrItemName);
	}

	hr = SHCreateStreamOnFile(m_szFileName, STGM_CREATE | STGM_READWRITE, ppDestination);
	if (SUCCEEDED(hr))
	{
		// We're not going to keep the Stream around, so don't AddRef.
		// The caller will release the stream when done.
	}
	else
	{
		ofLog(OF_LOG_NOTICE, "Failed to Create a Stream on File %ws", m_szFileName);
		*ppDestination = NULL;
	}
	return hr;
}