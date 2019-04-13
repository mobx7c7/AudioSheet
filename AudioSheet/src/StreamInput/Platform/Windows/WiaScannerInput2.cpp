#include "WiaScannerInput2.h"
#include "CWiaDataCallback.h"
#include "CComPtrArray.h"
#include <ofLog.h>
#include <codecvt>

void WiaScannerInput2::RedirectEvent(const StreamEventBase &ev)
{
	if (eventCb)
		eventCb(ev);
}

HRESULT WiaScannerInput2::GetImage(
	HWND				hWndParent,
	LONG                 lDeviceType,
	LONG                 lFlags,
	LONG                 lIntent,
	IWiaDevMgr          *pSuppliedWiaDevMgr,
	IWiaItem            *pSuppliedItemRoot,
	GUID                *pguidFormat,
	LONG                *plCount,
	IStream             ***pppStream
)
{
	HRESULT hr;

	// Validate and initialize output parameters

	if (plCount == NULL || pppStream == NULL)
	{
		return E_POINTER;
	}

	*plCount = 0;
	*pppStream = NULL;

	// Initialize the local root item variable with the supplied value.
	// If no value is supplied, display the device selection common dialog.

	CComPtr<IWiaItem> pItemRoot = pSuppliedItemRoot;

	if (pItemRoot == NULL)
	{
		// Initialize the device manager pointer with the supplied value
		// If no value is supplied, connect to the local device manager

		CComPtr<IWiaDevMgr> pWiaDevMgr = pSuppliedWiaDevMgr;

		if (pWiaDevMgr == NULL)
		{
			hr = pWiaDevMgr.CoCreateInstance(CLSID_WiaDevMgr);

			if (FAILED(hr))
			{
				return hr;
			}
		}

		// Display the device selection common dialog

		hr = pWiaDevMgr->SelectDeviceDlg(hWndParent, lDeviceType, lFlags, 0, &pItemRoot);

		if (FAILED(hr) || hr == S_FALSE)
		{
			return hr;
		}
	}

	// Display the image selection common dialog 

	CComPtrArray<IWiaItem> ppIWiaItem;

	hr = pItemRoot->DeviceDlg(hWndParent, lFlags, lIntent, &ppIWiaItem.Count(), &ppIWiaItem);

	if (FAILED(hr) || hr == S_FALSE)
	{
		return hr;
	}

	// For ADF scanners, the common dialog explicitly sets the page count to one.
	// So in order to transfer multiple images, set the page count to ALL_PAGES
	// if the WIA_DEVICE_DIALOG_SINGLE_IMAGE flag is not specified, 

	if (!(lFlags & WIA_DEVICE_DIALOG_SINGLE_IMAGE))
	{
		// Get the property storage interface pointer for the root item

		CComQIPtr<IWiaPropertyStorage> pWiaRootPropertyStorage(pItemRoot);

		if (pWiaRootPropertyStorage == NULL)
		{
			return E_NOINTERFACE;
		}

		// Determine if the selected device is a scanner or not

		PROPSPEC specDevType;
		specDevType.ulKind = PRSPEC_PROPID;
		specDevType.propid = WIA_DIP_DEV_TYPE;

		LONG nDevType;

		hr = ReadPropertyLong(pWiaRootPropertyStorage, &specDevType, &nDevType);

		if (SUCCEEDED(hr) && (GET_STIDEVICE_TYPE(nDevType) == StiDeviceTypeScanner))
		{
			// Determine if the document feeder is selected or not

			PROPSPEC specDocumentHandlingSelect;
			specDocumentHandlingSelect.ulKind = PRSPEC_PROPID;
			specDocumentHandlingSelect.propid = WIA_DPS_DOCUMENT_HANDLING_SELECT;

			LONG nDocumentHandlingSelect;

			hr = ReadPropertyLong(pWiaRootPropertyStorage, &specDocumentHandlingSelect, &nDocumentHandlingSelect);

			if (SUCCEEDED(hr) && (nDocumentHandlingSelect & FEEDER))
			{
				PROPSPEC specPages;
				specPages.ulKind = PRSPEC_PROPID;
				specPages.propid = WIA_DPS_PAGES;

				PROPVARIANT varPages;
				varPages.vt = VT_I4;
				varPages.lVal = ALL_PAGES;

				pWiaRootPropertyStorage->WriteMultiple(1, &specPages, &varPages, WIA_DPS_FIRST);

				PropVariantClear(&varPages);
			}
		}
	}



	// Create the data callback interface
	auto eventFunc = std::bind(&WiaScannerInput2::RedirectEvent, this, std::placeholders::_1);
	CComPtr<CWiaDataCallback> pDataCallback = new CWiaDataCallback(eventFunc, this, plCount, pppStream);

	if (pDataCallback == NULL)
	{
		return E_OUTOFMEMORY;
	}



	// Start the transfer of the selected items

	for (int i = 0; i < ppIWiaItem.Count(); ++i)
	{
		// Get the interface pointers

		CComQIPtr<IWiaPropertyStorage> pWiaPropertyStorage(ppIWiaItem[i]);

		if (pWiaPropertyStorage == NULL)
		{
			return E_NOINTERFACE;
		}

		CComQIPtr<IWiaDataTransfer> pIWiaDataTransfer(ppIWiaItem[i]);

		if (pIWiaDataTransfer == NULL)
		{
			return E_NOINTERFACE;
		}

		// Set the transfer type

		PROPSPEC specTymed;
		specTymed.ulKind = PRSPEC_PROPID;
		specTymed.propid = WIA_IPA_TYMED;

		PROPVARIANT varTymed;
		varTymed.vt = VT_I4;
		varTymed.lVal = TYMED_CALLBACK;

		hr = pWiaPropertyStorage->WriteMultiple(1, &specTymed, &varTymed, WIA_IPA_FIRST);

		PropVariantClear(&varTymed);

		if (FAILED(hr))
		{
			return hr;
		}

		// If there is no transfer format specified, use the device default

		GUID guidFormat = GUID_NULL;

		if (pguidFormat == NULL)
		{
			pguidFormat = &guidFormat;
		}

		if (*pguidFormat == GUID_NULL)
		{
			PROPSPEC specPreferredFormat;
			specPreferredFormat.ulKind = PRSPEC_PROPID;
			specPreferredFormat.propid = WIA_IPA_PREFERRED_FORMAT;

			hr = ReadPropertyGuid(pWiaPropertyStorage, &specPreferredFormat, pguidFormat);

			if (FAILED(hr))
			{
				return hr;
			}
		}

		// Set the transfer format

		PROPSPEC specFormat;
		specFormat.ulKind = PRSPEC_PROPID;
		specFormat.propid = WIA_IPA_FORMAT;

		PROPVARIANT varFormat;
		varFormat.vt = VT_CLSID;
		varFormat.puuid = (CLSID *)CoTaskMemAlloc(sizeof(CLSID));

		if (varFormat.puuid == NULL)
		{
			return E_OUTOFMEMORY;
		}

		*varFormat.puuid = *pguidFormat;

		hr = pWiaPropertyStorage->WriteMultiple(1, &specFormat, &varFormat, WIA_IPA_FIRST);
		PropVariantClear(&varFormat);

		if (FAILED(hr))
		{
			return hr;
		}

		// Read the transfer buffer size from the device, default to 64K

		PROPSPEC specBufferSize;
		specBufferSize.ulKind = PRSPEC_PROPID;
		specBufferSize.propid = WIA_IPA_BUFFER_SIZE;

		LONG nBufferSize;
		hr = ReadPropertyLong(pWiaPropertyStorage, &specBufferSize, &nBufferSize);

		if (FAILED(hr))
		{
			nBufferSize = 64 * 1024;
		}

		// Choose double buffered transfer for better performance

		WIA_DATA_TRANSFER_INFO wiaDataTrannsferInfo = { 0 };
		wiaDataTrannsferInfo.ulSize = sizeof(WIA_DATA_TRANSFER_INFO);
		wiaDataTrannsferInfo.ulBufferSize = 2 * nBufferSize;
		wiaDataTrannsferInfo.bDoubleBuffer = TRUE;

		// Start the transfer

		hr = pIWiaDataTransfer->idtGetBandedData(&wiaDataTrannsferInfo, pDataCallback);

		if (FAILED(hr) || hr == S_FALSE)
		{
			return hr;
		}
	}

	return S_OK;
}

WiaScannerInput2::WiaScannerInput2()
{}

WiaScannerInput2::~WiaScannerInput2()
{}

void WiaScannerInput2::open(int deviceIndex)
{}

void WiaScannerInput2::close()
{}

void WiaScannerInput2::read()
{
	HWND hWndParent = GetActiveWindow();
	//HWND hWndParent = WindowFromDC(wglGetCurrentDC());

	HRESULT hr;

	CComPtrArray<IStream> ppStream;

	hr = GetImage(hWndParent,
				  StiDeviceTypeDefault,
				  0,
				  WIA_INTENT_NONE,
				  NULL,
				  NULL,
				  NULL,
				  &ppStream.Count(),
				  &ppStream);

	if (FAILED(hr))
	{
		ofLogError() << "Error downloading image";
	}
}
