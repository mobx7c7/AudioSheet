#pragma once
#include <Wia.h>
#include <atlbase.h>
#include "WiaCommon.h"
#include "CComPtrArray.h"
#include "..\..\ScannerInput.h"

class CWiaDataCallback : public IWiaDataCallback
{
private:

	LONG m_cRef;
	BOOL m_bBMP;
	LONG m_nHeaderSize;
	LONG m_nDataSize;
	CComPtr<IStream>  m_pStream;
	StreamEventCb  m_fnEventCallback;
	PVOID m_pProgressCallbackParam;
	LONG *m_plCount;
	IStream ***m_pppStream;

private:

	HRESULT ReAllocBuffer(ULONG nBufferSize);
	HRESULT CopyToBuffer(ULONG nOffset, LPCVOID pBuffer, ULONG nSize);
	HRESULT StoreBuffer();

private:

	HRESULT HandleStatus(LONG lStatus, LONG lPercentComplete);
	HRESULT HandleHeader(PBYTE pbBuffer);
	HRESULT HandleData(LONG lStatus, LONG lPercentComplete, LONG lOffset, LONG lLength, PBYTE pbBuffer);
	HRESULT HandleNewPage();

public:

	CWiaDataCallback(
		StreamEventCb		pfnProgressCallback,
		PVOID                pProgressCallbackParam,
		LONG                *plCount,
		IStream             ***pppStream
	);

	HRESULT __stdcall
		QueryInterface(
			REFIID riid, 
			void ** ppvObject) override;

	ULONG __stdcall
		AddRef(void) override;

	ULONG __stdcall
		Release(void) override;

	HRESULT __stdcall 
		BandedDataCallback(
			LONG lMessage, 
			LONG lStatus, 
			LONG lPercentComplete, 
			LONG lOffset, 
			LONG lLength, 
			LONG lReserved,
			LONG lResLength,
			PBYTE pbBuffer) override;

};