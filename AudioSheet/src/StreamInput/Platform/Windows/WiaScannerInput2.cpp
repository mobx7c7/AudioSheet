#include "WiaScannerInput2.h"
#include "CWiaDataCallback.h"
#include "CComPtrArray.h"
#include <ofLog.h>
#include <codecvt>

LONG ConvertImageType(ScannerImageType type)
{
	switch (type)
	{
		case ScannerImageType::Text:
			return WIA_INTENT_IMAGE_TYPE_TEXT;
		case ScannerImageType::Grayscale:
			return WIA_INTENT_IMAGE_TYPE_GRAYSCALE;
		case ScannerImageType::Color:
			return WIA_INTENT_IMAGE_TYPE_COLOR;
		default:
			return 0;
	}
}

LONG WiaScannerInput2::GetPropertyLong(PROPID propid)
{
	switch (propid)
	{
		case WIA_IPS_XRES:
			return getResolutionX();
		case WIA_IPS_YRES:
			return getResolutionY();
		case WIA_IPS_XPOS:
			return getPositionX();
		case WIA_IPS_YPOS:
			return getPositionY();
		case WIA_IPS_XEXTENT:
			return getExtentX();
		case WIA_IPS_YEXTENT:
			return getExtentY();
		case WIA_IPS_CUR_INTENT:
			return ConvertImageType(getImageType());
		default:
			return 0u;
	}
}

HRESULT WiaScannerInput2::CreateDevice(int deviceIndex)
{
	HRESULT hr = S_OK;

	BSTR bstrDeviceID = devices[deviceIndex];

	if (bstrDeviceID == NULL)
	{
		hr = E_INVALIDARG;
		ReportError(TEXT("Invalid device ID"), hr);
		return hr;
	}

	pWiaDevice = NULL;

	hr = pWiaDevMgr->CreateDevice(bstrDeviceID, &pWiaDevice);

	if (SUCCEEDED(hr))
	{
		selectedDevice = deviceIndex;
	}

	return hr;
}

HRESULT WiaScannerInput2::ListDevices()
{
	HRESULT hr = S_OK;

	if (NULL == pWiaDevMgr)
	{
		hr = E_INVALIDARG;
		ReportError(TEXT("Device manager not initialized"), hr);
		return hr;
	}

	// Get a device enumerator interface
	IEnumWIA_DEV_INFO *pWiaEnumDevInfo = NULL;
	hr = pWiaDevMgr->EnumDeviceInfo(WIA_DEVINFO_ENUM_LOCAL, &pWiaEnumDevInfo);

	if (SUCCEEDED(hr))
	{
		// Reset the device enumerator to the beginning of the list
		hr = pWiaEnumDevInfo->Reset();

		if (SUCCEEDED(hr))
		{
			while (S_OK == hr)
			{
				// Get the next device's property storage interface pointer
				IWiaPropertyStorage *pWiaPropertyStorage = NULL;
				hr = pWiaEnumDevInfo->Next(1, &pWiaPropertyStorage, NULL);
				
				if (hr == S_OK)
				{
					hr = AddDevice(pWiaPropertyStorage);

					if (FAILED(hr))
					{
						ReportError(TEXT("Error adding device"), hr);
					}

					pWiaPropertyStorage->Release();
					pWiaPropertyStorage = NULL;
				}
				else if (FAILED(hr))
				{
					ReportError(TEXT("Error calling IEnumWIA_DEV_INFO::Next()"), hr);
				}
			}

			if (S_FALSE == hr)
			{
				hr = S_OK;
			}
		}
		else
		{
			ReportError(TEXT("Error calling IEnumWIA_DEV_INFO::Reset()"), hr);
		}

		pWiaEnumDevInfo->Release();
		pWiaEnumDevInfo = NULL;
	}
	else
	{
		ReportError(TEXT("Error calling IWiaDevMgr::EnumDeviceInfo"), hr);
	}

	return hr;
}

HRESULT WiaScannerInput2::ListChildren(IWiaItem* pItemRoot, LONG* lCount, IWiaItem*** ppiWiaItem)
{
	IEnumWiaItem *pEnumWiaItem = NULL;

	HRESULT hr = pItemRoot->EnumChildItems(&pEnumWiaItem);

	if (SUCCEEDED(hr))
	{
		ULONG ulCount;
		hr = pEnumWiaItem->GetCount(&ulCount);

		if (SUCCEEDED(hr))
		{
			*lCount = ulCount;
			*ppiWiaItem = (IWiaItem**)CoTaskMemAlloc(ulCount * sizeof(IWiaItem*));

			if (*ppiWiaItem == NULL)
			{
				return E_OUTOFMEMORY;
			}

			IWiaItem** pItemChildNext = ppiWiaItem[0];

			while (S_OK == hr)
			{
				IWiaItem* pItemChild = NULL;
				hr = pEnumWiaItem->Next(1, &pItemChild, NULL);

				if (S_OK == hr)
				{
					IWiaPropertyStorage *pWiaPropertyStorage = NULL;
					HRESULT hr = pItemChild->QueryInterface(IID_IWiaPropertyStorage, (void**)&pWiaPropertyStorage);

					if (SUCCEEDED(hr))
					{
						BSTR bstrDevName = NULL;
						ReadPropertyBSTR(pWiaPropertyStorage, WIA_IPA_FULL_ITEM_NAME, &bstrDevName);
						
						ofLog(OF_LOG_NOTICE, "Item Name: %ws", bstrDevName);

						pWiaPropertyStorage->Release();
						pWiaPropertyStorage = NULL;
					}

					*pItemChildNext++ = pItemChild;
				}
			}
			ofLogNotice() << std::endl;
		}

		if (S_FALSE == hr)
		{
			hr = S_OK;
		}

		pEnumWiaItem->Release();
		pEnumWiaItem = NULL;
	}

	return hr;
}

HRESULT WiaScannerInput2::AddDevice(IWiaPropertyStorage *pWiaPropertyStorage)
{
	BSTR deviceId;

	HRESULT hr = ReadPropertyBSTR(pWiaPropertyStorage, WIA_DIP_DEV_ID, &deviceId);

	if (SUCCEEDED(hr))
	{
		devices.push_back(deviceId);
		PrintProperties(pWiaPropertyStorage, { WIA_DIP_DEV_NAME, WIA_DIP_DEV_DESC });
	}

	return hr;
}




HRESULT WiaScannerInput2::FillProperties(IWiaPropertyStorage* pWiaPropertyStorage)
{
	HRESULT hr = S_OK;

	std::vector<PROPID> properties
	{
		WIA_IPS_XRES,
		WIA_IPS_YRES,
		//WIA_IPS_XPOS,
		//WIA_IPS_YPOS,
		//WIA_IPS_XEXTENT,
		//WIA_IPS_YEXTENT,
		WIA_IPS_CUR_INTENT,
	};

	for (auto propid : properties)
	{
		hr = WritePropertyLong(pWiaPropertyStorage, propid, GetPropertyLong(propid));

		if (FAILED(hr))
		{
			//ReportError(TEXT("Error writing property"), hr);
			break;
		}
	}

	return hr;
}

HRESULT WiaScannerInput2::DownloadImage(LONG lDeviceType, LONG lFlags, LONG lIntent, GUID *pguidFormat)
{
	HRESULT hr;

	/////////////////////////////////////////////////////////////////////////
	// Define image without image selection dialog
	/////////////////////////////////////////////////////////////////////////
	CComPtrArray<IWiaItem> ppIWiaItem;

	hr = ListChildren(this->pWiaDevice, &ppIWiaItem.Count(), &ppIWiaItem);
	if (FAILED(hr)) return hr;
	
	CComQIPtr<IWiaPropertyStorage> pWiaPropertyStorage(ppIWiaItem[0]);
	hr = FillProperties(pWiaPropertyStorage);
	if (FAILED(hr)) return hr;

	// For ADF scanners, the common dialog explicitly sets the page count to one.
	// So in order to transfer multiple images, set the page count to ALL_PAGES
	// if the WIA_DEVICE_DIALOG_SINGLE_IMAGE flag is not specified, 
	//
	// ADF: Automatic Document Feeder

	if (!(lFlags & WIA_DEVICE_DIALOG_SINGLE_IMAGE))
	{
		// Get the property storage interface pointer for the root item

		CComQIPtr<IWiaPropertyStorage> pWiaRootPropertyStorage(this->pWiaDevice);

		if (pWiaRootPropertyStorage == NULL) return E_NOINTERFACE;

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

	/////////////////////////////////////////////////////////////////////////
	// Create the data callback interface
	/////////////////////////////////////////////////////////////////////////

	auto eventFunc = std::bind(&WiaScannerInput2::dispachEvent, this, std::placeholders::_1);

	CComPtr<CWiaDataCallback> pDataCallback = new CWiaDataCallback(eventFunc);

	if (pDataCallback == NULL) return E_OUTOFMEMORY;

	/////////////////////////////////////////////////////////////////////////
	// Start the transfer of the selected items
	/////////////////////////////////////////////////////////////////////////

	for (int i = 0; i < ppIWiaItem.Count(); ++i)
	{
		// Get the interface pointers

		CComQIPtr<IWiaPropertyStorage> pWiaPropertyStorage(ppIWiaItem[i]);
		if (pWiaPropertyStorage == NULL) return E_NOINTERFACE;

		CComQIPtr<IWiaDataTransfer> pIWiaDataTransfer(ppIWiaItem[i]);
		if (pIWiaDataTransfer == NULL) return E_NOINTERFACE;

		// Set the transfer type

		PROPSPEC specTymed;
		specTymed.ulKind = PRSPEC_PROPID;
		specTymed.propid = WIA_IPA_TYMED;

		PROPVARIANT varTymed;
		varTymed.vt = VT_I4;
		varTymed.lVal = TYMED_CALLBACK;

		hr = pWiaPropertyStorage->WriteMultiple(1, &specTymed, &varTymed, WIA_IPA_FIRST);

		PropVariantClear(&varTymed);

		if (FAILED(hr)) return hr;

		// If there is no transfer format specified, use the device default

		GUID guidFormat = GUID_NULL;

		if (pguidFormat == NULL) pguidFormat = &guidFormat;

		if (*pguidFormat == GUID_NULL)
		{
			PROPSPEC specPreferredFormat;
			specPreferredFormat.ulKind = PRSPEC_PROPID;
			specPreferredFormat.propid = WIA_IPA_PREFERRED_FORMAT;

			hr = ReadPropertyGuid(pWiaPropertyStorage, &specPreferredFormat, pguidFormat);

			if (FAILED(hr)) return hr;
		}

		// Set the transfer format

		PROPSPEC specFormat;
		specFormat.ulKind = PRSPEC_PROPID;
		specFormat.propid = WIA_IPA_FORMAT;

		PROPVARIANT varFormat;
		varFormat.vt = VT_CLSID;
		varFormat.puuid = (CLSID *)CoTaskMemAlloc(sizeof(CLSID));

		if (varFormat.puuid == NULL) return E_OUTOFMEMORY;

		*varFormat.puuid = *pguidFormat;

		hr = pWiaPropertyStorage->WriteMultiple(1, &specFormat, &varFormat, WIA_IPA_FIRST);
		
		PropVariantClear(&varFormat);

		if (FAILED(hr)) return hr;

		// Read the transfer buffer size from the device, default to 64K

		PROPSPEC specBufferSize;
		specBufferSize.ulKind = PRSPEC_PROPID;
		specBufferSize.propid = WIA_IPA_BUFFER_SIZE;

		LONG nBufferSize;
		hr = ReadPropertyLong(pWiaPropertyStorage, &specBufferSize, &nBufferSize);

		if (FAILED(hr)) nBufferSize = 64 * 1024;

		// Choose double buffered transfer for better performance

		WIA_DATA_TRANSFER_INFO wiaDataTransferInfo = { 0 };
		wiaDataTransferInfo.ulSize = sizeof(WIA_DATA_TRANSFER_INFO);
		wiaDataTransferInfo.ulBufferSize = 2 * nBufferSize;
		wiaDataTransferInfo.bDoubleBuffer = TRUE;

		// Start the transfer

		hr = pIWiaDataTransfer->idtGetBandedData(&wiaDataTransferInfo, pDataCallback);

		if (FAILED(hr) || hr == S_FALSE) return hr;
	}

	return S_OK;
}



WiaScannerInput2::WiaScannerInput2()
	: selectedDevice(-1)
	, pWiaDevMgr(NULL)
{

}

WiaScannerInput2::~WiaScannerInput2()
{
	if (pWiaDevMgr != NULL)
	{
		pWiaDevMgr->Release();
	}
}

void WiaScannerInput2::setup()
{
	if (pWiaDevMgr == NULL)
	{
		HRESULT hr = CoCreateInstance(CLSID_WiaDevMgr, NULL, CLSCTX_LOCAL_SERVER, IID_IWiaDevMgr, (void**)&pWiaDevMgr);

		if (SUCCEEDED(hr))
		{
			refresh();
		}
		else
		{
			ReportError(TEXT("Error creating device manager"), hr);
		}
	}
}

void WiaScannerInput2::refresh()
{
	HRESULT hr = ListDevices();

	if (FAILED(hr))
	{
		ReportError(TEXT("Error listing devices"), hr);
	}
	else if(devices.empty())
	{
		ofLogError() << "No devices available";
	}
	else
	{
		//TODO: print available devices
	}
}

void WiaScannerInput2::open(int deviceIndex)
{
	if (devices.empty())
	{
		ofLogError() << "No devices available";
	}
	else if(deviceIndex < 0 || deviceIndex >= devices.size())
	{
		ofLogError() << "Device not found";
	}
	else if (deviceIndex == selectedDevice)
	{
		ofLogError() << "Device is already opened";
	}
	else
	{
		HRESULT hr = CreateDevice(deviceIndex);

		if (SUCCEEDED(hr))
		{
			ofLogNotice() << "Device selected";
			//TODO: Print selected device
		}
		else
		{
			ReportError(TEXT("Error creating device"), hr);
		}
	}
}

void WiaScannerInput2::close()
{
	if (pWiaDevice != NULL)
	{
		selectedDevice = -1;
		pWiaDevice->Release();
		pWiaDevice = NULL;
	}
}

void WiaScannerInput2::read()
{
	if (pWiaDevice == NULL)
	{
		ofLogError() << "Device not selected";
	}
	else
	{
		HRESULT hr = DownloadImage(StiDeviceTypeScanner, 0, WIA_INTENT_NONE, NULL);

		if (FAILED(hr))
		{
			ReportError(TEXT("Error downloading image"), hr);
		}
	}
}
