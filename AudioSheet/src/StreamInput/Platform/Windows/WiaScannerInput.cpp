#include "WiaScannerInput.h"
#include "CWiaTransferCallback.h"
#include <ofLog.h>
#include <codecvt>

std::ostream& operator<<(std::ostream &os, const WiaScannerInput::NextField &field)
{
	return os << field.name << "=" << field.value << ", ";
}

std::ostream& operator<<(std::ostream &os, const WiaScannerInput::LastField &field)
{
	return os << field.name << "=" << field.value;
}

std::ostream& operator<<(std::ostream &os, const WiaScannerInput::DeviceInfo &info)
{
	using convert_type = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type, wchar_t> converter;

	return os
		<< "Device("
		//<< WiaScannerInput::NextField("id", converter.to_bytes(info.id))
		<< WiaScannerInput::NextField("name", converter.to_bytes(info.name))
		<< WiaScannerInput::LastField("description", converter.to_bytes(info.desc))
		<< ")";
}

WiaScannerInput::WiaScannerInput()
	: pWiaDevMgr(NULL)
	, pWiaRootItem2(NULL)
{
	HRESULT hr = CreateWiaDeviceManager(&pWiaDevMgr);
	if (FAILED(hr))
	{
		ReportError(TEXT("Can't create WiaDeviceManager"), hr);
	}
}

WiaScannerInput::~WiaScannerInput()
{
	if (pWiaDevMgr)
	{
		pWiaDevMgr->Release();
		pWiaDevMgr = NULL;
		availableDevices.clear();
	}
}

HRESULT WiaScannerInput::CreateWiaDeviceManager(IWiaDevMgr2 ** ppWiaDevMgr)
{
	if (NULL == ppWiaDevMgr)
	{
		return E_INVALIDARG;
	}

	*ppWiaDevMgr = NULL;

	// Create an instance of the device manager
	return CoCreateInstance(CLSID_WiaDevMgr2, NULL, CLSCTX_LOCAL_SERVER, IID_IWiaDevMgr2, (void**)ppWiaDevMgr);
}

HRESULT WiaScannerInput::CreateWiaDevice(IWiaDevMgr2 * pWiaDevMgr, BSTR bstrDeviceID, IWiaItem2 ** ppWiaDevice)
{
	if (NULL == pWiaDevMgr || NULL == bstrDeviceID || NULL == ppWiaDevice)
	{
		return E_INVALIDARG;
	}

	*ppWiaDevice = NULL;

	// Create the WIA Device
	return pWiaDevMgr->CreateDevice(0, bstrDeviceID, ppWiaDevice);
}

HRESULT WiaScannerInput::ListDevices(IWiaDevMgr2 * pWiaDevMgr2)
{
	if (NULL == pWiaDevMgr2)
	{
		HRESULT hr = E_INVALIDARG;
		ReportError(TEXT("Invalid argument passed to EnumerateWiaDevices()"), hr);
		return hr;
	}

	// Get a device enumerator interface
	IEnumWIA_DEV_INFO *pWiaEnumDevInfo = NULL;
	HRESULT hr = pWiaDevMgr2->EnumDeviceInfo(WIA_DEVINFO_ENUM_LOCAL, &pWiaEnumDevInfo);
	if (SUCCEEDED(hr))
	{
		// Reset the device enumerator to the beginning of the list
		hr = pWiaEnumDevInfo->Reset();
		if (SUCCEEDED(hr))
		{
			availableDevices.clear();

			while (S_OK == hr)
			{
				// Get the next device's property storage interface pointer
				IWiaPropertyStorage *pWiaPropertyStorage = NULL;
				hr = pWiaEnumDevInfo->Next(1, &pWiaPropertyStorage, NULL);
				if (hr == S_OK)
				{
					DeviceInfo info;
					HRESULT hr1 = ReadDeviceInfo(pWiaPropertyStorage, &info);
					if (SUCCEEDED(hr1))
					{
						availableDevices.push_back(info);
					}
					else
					{
						ReportError(TEXT("ReadDeviceInfo() failed in EnumerateWiaDevices()"), hr1);
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
		ReportError(TEXT("Error calling IWiaDevMgr2::EnumDeviceInfo"), hr);
	}
	return hr;
}

HRESULT WiaScannerInput::EnumerateCapabilities(IWiaItem2 * pWiaItem)
{
	if (NULL == pWiaItem)
	{
		return E_INVALIDARG;
	}

	IEnumWIA_DEV_CAPS *pWiaEnumDevCaps = NULL;
	HRESULT hr = pWiaItem->EnumDeviceCapabilities(WIA_DEVICE_COMMANDS, &pWiaEnumDevCaps);
	if (SUCCEEDED(hr))
	{
		WIA_DEV_CAP pWiaDevCap;
		while ((hr = pWiaEnumDevCaps->Next(1, &pWiaDevCap, NULL)) == S_OK)
		{
			ofLog(OF_LOG_NOTICE, "Capability(name=%ws(), description=%ws)", pWiaDevCap.bstrName, pWiaDevCap.bstrDescription);
		}

		if (S_FALSE == hr)
		{
			hr = S_OK;
		}

		pWiaEnumDevCaps->Release();
		pWiaEnumDevCaps = NULL;
	}

	return hr;
}

HRESULT WiaScannerInput::EnumerateItems(IWiaItem2 *pIWiaItem2Root)
{
	if (NULL == pIWiaItem2Root)
	{
		HRESULT hr = E_INVALIDARG;
		ReportError(TEXT("Invalid argument passed to EnumerateAndTransferItems()"), hr);
		return hr;
	}

	PrintItemName(pIWiaItem2Root);

	IEnumWiaItem2 *pEnumWiaItem2 = NULL;
	HRESULT hr = pIWiaItem2Root->EnumChildItems(0, &pEnumWiaItem2);
	if (SUCCEEDED(hr))
	{
		// We will loop until we get an error or pEnumWiaItem2->Next returns
		// S_FALSE to signal the end of the list.
		while (S_OK == hr)
		{
			// Get the next child item
			IWiaItem2 *pChildWiaItem2 = NULL;
			hr = pEnumWiaItem2->Next(1, &pChildWiaItem2, NULL);
			if (S_OK == hr)
			{
				PrintItemName(pChildWiaItem2);
				TransferItem(pChildWiaItem2);

				pChildWiaItem2->Release();
				pChildWiaItem2 = NULL;
			}
			else if (FAILED(hr))
			{
				ReportError(TEXT("Error calling pEnumWiaItem2->Next"), hr);
			}
		}

		if (S_FALSE == hr)
		{
			hr = S_OK;
		}

		pEnumWiaItem2->Release();
		pEnumWiaItem2 = NULL;
	}
	else
	{
		ReportError(TEXT("pIWiaItem2->EnumChildItems() failed in EnumerateItems"), hr);
	}
	return hr;
}

HRESULT WiaScannerInput::ReadDeviceInfo(IWiaPropertyStorage *pWiaPropertyStorage, DeviceInfo* availableDevices)
{
	HRESULT hr = S_OK;

	if ((NULL == pWiaPropertyStorage) || (NULL == availableDevices))
	{
		hr = E_INVALIDARG;
		ReportError(TEXT("Invalid argument passed to ReadDeviceInfo()"), hr);
		return hr;
	}

	//Read property count
	hr = pWiaPropertyStorage->GetCount(&availableDevices->ulNumProps);
	if (FAILED(hr))
	{
		ReportError(TEXT("ReadPropertyBSTR() failed for GetCount() in ReadDeviceInfo()"), hr);
		return hr;
	}

	//Read device ID
	hr = ReadPropertyBSTR(pWiaPropertyStorage, WIA_DIP_DEV_ID, &availableDevices->id);
	if (FAILED(hr))
	{
		ReportError(TEXT("ReadPropertyBSTR() failed for WIA_DIP_DEV_ID in ReadDeviceInfo()"), hr);
		return hr;
	}

	//Read device name
	hr = ReadPropertyBSTR(pWiaPropertyStorage, WIA_DIP_DEV_NAME, &availableDevices->name);
	if (FAILED(hr))
	{
		ReportError(TEXT("ReadPropertyBSTR() failed for WIA_DIP_DEV_NAME in ReadDeviceInfo()"), hr);
		return hr;
	}

	//Read device description
	hr = ReadPropertyBSTR(pWiaPropertyStorage, WIA_DIP_DEV_DESC, &availableDevices->desc);
	if (FAILED(hr))
	{
		ReportError(TEXT("ReadPropertyBSTR() failed for WIA_DIP_DEV_DESC in ReadDeviceInfo()"), hr);
		return hr;
	}

	return hr;
}

HRESULT WiaScannerInput::TransferItem(IWiaItem2 *pIWiaItem2)
{
	HRESULT hr = S_OK;

	if (NULL == pIWiaItem2)
	{
		hr = E_INVALIDARG;
		ofLog(OF_LOG_NOTICE, "Invalid parameters passed", hr);
		return hr;
	}

	LONG lItemType = 0;
	hr = pIWiaItem2->GetItemType(&lItemType);

	if (lItemType & WiaItemTypeTransfer)
	{
		if ((lItemType & WiaItemTypeFolder))
		{
			ofLog(OF_LOG_NOTICE, "Downloading folder");
			hr = DownloadItem(pIWiaItem2, FOLDER_TRANSFER);
		}
		if (lItemType & WiaItemTypeFile)
		{
			ofLog(OF_LOG_NOTICE, "Downloading file");
			hr = DownloadItem(pIWiaItem2, FILE_TRANSFER);
		}
	}

	return hr;
}

HRESULT WiaScannerInput::DownloadItem(IWiaItem2* pWiaItem2, BOOL bTransferFlag)
{
	HRESULT hr = S_OK;

	if (!pWiaItem2)
	{
		hr = E_INVALIDARG;
		ReportError(TEXT("Invalid argument passed to DownloadItem()"), hr);
		return hr;
	}
	// Get the IWiaTransfer interface
	IWiaTransfer *pWiaTransfer = NULL;
	hr = pWiaItem2->QueryInterface(IID_IWiaTransfer, (void**)&pWiaTransfer);
	if (SUCCEEDED(hr))
	{
		// Create our callback class
		CWiaTransferCallback *pWiaClassCallback = new CWiaTransferCallback(
			std::bind(&WiaScannerInput::dispachEvent, this, std::placeholders::_1));

		if (pWiaClassCallback)
		{
			// Get the IWiaTransferCallback interface from our callback class.
			IWiaTransferCallback *pWiaTransferCallback = NULL;
			hr = pWiaClassCallback->QueryInterface(IID_IWiaTransferCallback, (void**)&pWiaTransferCallback);
			if (SUCCEEDED(hr))
			{
				//Set the format for the item to BMP
				IWiaPropertyStorage* pWiaPropertyStorage = NULL;
				hr = pWiaItem2->QueryInterface(IID_IWiaPropertyStorage, (void**)&pWiaPropertyStorage);
				if (SUCCEEDED(hr))
				{
					//Find out item category
					GUID itemCategory = GUID_NULL;
					ReadPropertyGuid(pWiaItem2, WIA_IPA_ITEM_CATEGORY, &itemCategory);

					if ((!IsEqualIID(itemCategory, WIA_CATEGORY_FINISHED_FILE)) || (!IsEqualIID(itemCategory, WIA_CATEGORY_FOLDER)))
					{
						hr = WritePropertyGuid(pWiaPropertyStorage, WIA_IPA_FORMAT, WiaImgFmt_BMP);
					}

					if (FAILED(hr))
					{
						ReportError(TEXT("WritePropertyGuid() failed in DownloadItem().Format couldn't be set to BMP"), hr);
					}

					//Get the file extension 
					BSTR bstrFileExtension = NULL;
					if (!IsEqualIID(itemCategory, WIA_CATEGORY_FOLDER))
					{
						ReadPropertyBSTR(pWiaPropertyStorage, WIA_IPA_FILENAME_EXTENSION, &bstrFileExtension);
					}

					//Get the temporary folder path which is the directory where we will download the images
					TCHAR bufferTempPath[MAX_TEMP_PATH];
					GetTempPath(MAX_TEMP_PATH, bufferTempPath);


					//Find the item type 
					LONG lItemType = 0;
					hr = pWiaItem2->GetItemType(&lItemType);

					BOOL bFeederTransfer = FALSE;
					if (IsEqualGUID(itemCategory, WIA_CATEGORY_FEEDER))
					{
						//Set WIA_IPS_PAGES to ALL_PAGES will enable transfer of all pages in the document feeder (multi-page transfer).
						//If somebody wants to scan a specific number of pages say N, he should set WIA_IPS_PAGES to N. 
						WritePropertyLong(pWiaPropertyStorage, WIA_IPS_PAGES, ALL_PAGES);

						bFeederTransfer = TRUE;

					}
					else if (lItemType & WiaItemTypeStorage)
					{
						//We are setting file extension null for storage items since we are uploading files already with an extension.
						//If other storage items are present on the device, for them also we are assuming that they already have extension
						//embedded in their names
						bstrFileExtension = NULL;
					}

					pWiaClassCallback->InitializeCallback(bufferTempPath, bstrFileExtension, bFeederTransfer);

					//Now download based on whether its a folder item or a file item
					if (bTransferFlag == FOLDER_TRANSFER)
					{
						hr = pWiaTransfer->Download(WIA_TRANSFER_ACQUIRE_CHILDREN, pWiaTransferCallback);
						if (S_OK == hr)
						{
							ofLog(OF_LOG_NOTICE, "pWiaTransfer->Download() on folder item SUCCEEDED");
						}
						else if (S_FALSE == hr)
						{
							ReportError(TEXT("pWiaTransfer->Download() on folder item returned S_FALSE. Folder may not be having child items"), hr);
						}
						else if (FAILED(hr))
						{
							ReportError(TEXT("pWiaTransfer->Download() on folder item failed"), hr);
						}
					}
					else //FILE_TRANSFER
					{
						hr = pWiaTransfer->Download(0, pWiaTransferCallback);
						if (S_OK == hr)
						{
							ofLog(OF_LOG_NOTICE, "pWiaTransfer->Download() on file item SUCCEEDED");
						}
						else if (S_FALSE == hr)
						{
							ReportError(TEXT("pWiaTransfer->Download() on file item returned S_FALSE. File may be empty"), hr);
						}
						else if (FAILED(hr))
						{
							ReportError(TEXT("pWiaTransfer->Download() on file item failed"), hr);
						}

					}

					pWiaPropertyStorage->Release();
					pWiaPropertyStorage = NULL;
				}
				else
				{
					ReportError(TEXT("QueryInterface failed on IID_IWiaPropertyStorage"), hr);
				}

				pWiaTransferCallback->Release();
				pWiaTransferCallback = NULL;
			}
			else
			{
				ReportError(TEXT("pWiaClassCallback->QueryInterface failed on IID_IWiaTransferCallback"), hr);
			}

			pWiaClassCallback->Release();
			pWiaClassCallback = NULL;
		}
		else
		{
			ReportError(TEXT("Unable to create CWiaTransferCallback class instance"));
		}

		pWiaTransfer->Release();
		pWiaTransfer = NULL;
	}
	else
	{
		ReportError(TEXT("pIWiaItem2->QueryInterface failed on IID_IWiaTransfer"), hr);
	}
	return hr;
}

void WiaScannerInput::refresh()
{
	if (pWiaDevMgr)
	{
		HRESULT hr = ListDevices(pWiaDevMgr);
		if (SUCCEEDED(hr))
		{
			for (auto& deviceInfo : availableDevices)
			{
				ofLogNotice() << deviceInfo;
			}
		}
		else
		{
			ReportError(TEXT("Error calling EnumerateWiaDevices on open()"), hr);
		}
	}
}

void WiaScannerInput::open(int deviceIndex)
{
	if (pWiaDevMgr)
	{
		refresh();

		if (availableDevices.empty())
		{
			throw std::exception("No devices available");
		}
		if (pWiaRootItem2)
		{
			throw std::exception("Device currently opened needs to be closed first");
		}
		if (deviceIndex >= availableDevices.size())
		{
			throw std::exception("Device doesn't exist");
		}

		HRESULT hr = CreateWiaDevice(pWiaDevMgr, availableDevices[deviceIndex].id, &pWiaRootItem2);
		if (FAILED(hr))
		{
			ReportError(TEXT("Error calling IWiaDevMgr2::CreateDevice()"), hr);
		}
	}
}

void WiaScannerInput::close()
{
	if (pWiaDevMgr)
	{
		if (!pWiaRootItem2)
		{
			throw std::exception("No device opened");
		}

		pWiaRootItem2->Release();
		pWiaRootItem2 = NULL;
	}
}

void WiaScannerInput::read()
{
	if (pWiaDevMgr && pWiaRootItem2)
	{
		if (!pWiaRootItem2)
		{
			throw std::exception("No device opened");
		}

		HRESULT hr = EnumerateItems(pWiaRootItem2);
		if (FAILED(hr))
		{
			ReportError(TEXT("EnumerateAndTransferItems() failed in open()"), hr);
		}
	}
}


