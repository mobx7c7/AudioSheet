#pragma once
#include "WiaCommon.h"

#define MAX_TEMP_PATH 1024
#define MAX_FILENAME_LENGTH	1024

class CWiaTransferCallback : public IWiaTransferCallback
{
private:

	ULONG m_cRef;								// for reference counting
	long  m_lPageCount;							// page counting for feeder item
	BSTR  m_bstrFileExtension;					// file extension to be appended to the download file 
	BOOL  m_bFeederTransfer;					// flag indicating whether download is from feeder item
	BSTR  m_bstrDirectoryName;					// download directory 
	TCHAR m_szFileName[MAX_FILENAME_LENGTH];	// download file

public:

	CWiaTransferCallback();
	virtual ~CWiaTransferCallback();

	HRESULT InitializeCallback(TCHAR* bstrDirectoryName, BSTR bstrExt, BOOL bFeederTransfer);

	// IUnknown functions
	HRESULT CALLBACK QueryInterface(REFIID riid, void **ppvObject);
	ULONG CALLBACK AddRef();
	ULONG CALLBACK Release();

	// IWiaTransferCallback functions
	HRESULT STDMETHODCALLTYPE TransferCallback(LONG lFlags, WiaTransferParams  *pWiaTransferParams);
	HRESULT STDMETHODCALLTYPE GetNextStream(LONG lFlags, BSTR bstrItemName, BSTR bstrFullItemName, IStream **ppDestination);

};