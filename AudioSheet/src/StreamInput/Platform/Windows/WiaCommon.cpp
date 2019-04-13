#include "WiaCommon.h"
#include <ofLog.h>
#include <codecvt>
#include <wchar.h>

void ReportError(LPCTSTR pszMessage, HRESULT hr)
{
	if (S_OK != hr)
	{
		ofLog(OF_LOG_ERROR, "%ws: HRESULT: 0x%08X", pszMessage, hr);
	}
	else
	{
		ofLog(OF_LOG_ERROR, "%ws", pszMessage);
	}
}

HRESULT ReadPropertyBSTR(IWiaPropertyStorage* pWiaPropertyStorage, PROPID propid, BSTR* pbstr)
{
	if ((!pWiaPropertyStorage) || (!pbstr))
	{
		HRESULT hr = E_INVALIDARG;
		ReportError(TEXT("Invalid arguments passed to ReadPropertyBSTR()"), hr);
		return hr;
	}
	//initialize out variables
	*pbstr = NULL;

	// Declare PROPSPECs and PROPVARIANTs, and initialize them.
	PROPSPEC PropSpec[1] = { 0 };
	PROPVARIANT PropVar[1];
	PropVariantInit(PropVar);

	PropSpec[0].ulKind = PRSPEC_PROPID;
	PropSpec[0].propid = propid;

	HRESULT hr = pWiaPropertyStorage->ReadMultiple(1, PropSpec, PropVar);
	if (S_OK == hr)
	{
		if (PropVar[0].vt == VT_BSTR)
		{
			*pbstr = SysAllocString(PropVar[0].bstrVal);
			if (!(*pbstr))
			{
				hr = E_OUTOFMEMORY;
				ReportError(TEXT("Failed to allocate memory for BSTR in function ReadPropertyBSTR()"), hr);
			}
		}
		else
		{
			ReportError(TEXT("Trying to read a property which doesn't return a BSTR"));
		}
	}
	else
	{
		ReportError(TEXT("Error calling pWiaPropertyStorage->ReadMultiple() in ReadPropertyBSTR()"), hr);
	}

	//Free PropVar array
	PropVariantClear(PropVar);
	return hr;
}

HRESULT ReadPropertyLong(IWiaItem2* pWiaItem2, PROPID propid, LONG* lVal)
{
	if (!pWiaItem2)
	{
		HRESULT hr = E_INVALIDARG;
		ReportError(TEXT("\nInvalid arguments passed to GetBrightnessContrast()"), hr);
		return hr;
	}
	//initialize out variables
	*lVal = 0;

	//Get the IWiaPropertyStorage interface for IWiaItem2 
	IWiaPropertyStorage* pWiaPropertyStorage = NULL;
	HRESULT hr = pWiaItem2->QueryInterface(IID_IWiaPropertyStorage, (void**)&pWiaPropertyStorage);
	if (SUCCEEDED(hr))
	{
		// Declare PROPSPECs and PROPVARIANTs, and initialize them.
		PROPSPEC PropSpec[1] = { 0 };
		PROPVARIANT PropVar[1];
		PropVariantInit(PropVar);

		PropSpec[0].ulKind = PRSPEC_PROPID;
		PropSpec[0].propid = propid;
		hr = pWiaPropertyStorage->ReadMultiple(1, PropSpec, PropVar);
		if (S_OK == hr)
		{
			if (PropVar[0].vt == VT_I4)
			{
				*lVal = PropVar[0].lVal;
			}
			else
			{
				ReportError(TEXT("Trying to read a property which doesn't return a LONG"));
			}
		}
		else
		{
			ReportError(TEXT("Error calling pWiaPropertyStorage->ReadMultiple() in ReadPropertyLong()"), hr);
		}

		//Free PropVar array
		PropVariantClear(PropVar);

		//Release pWiaPropertyStorage
		pWiaPropertyStorage->Release();
		pWiaPropertyStorage = NULL;
	}
	else
	{
		ReportError(TEXT("pWiaItem2->QueryInterface failed on IID_IWiaPropertyStorage"), hr);
	}

	return hr;
}

HRESULT ReadPropertyLong(IWiaPropertyStorage * pWiaPropertyStorage, const PROPSPEC * pPropSpec, LONG * plResult)
{
	PROPVARIANT PropVariant;

	HRESULT hr = pWiaPropertyStorage->ReadMultiple(1,pPropSpec,&PropVariant);

	// Generally, the return value should be checked against S_FALSE.
	// If ReadMultiple returns S_FALSE, it means the property name or ID
	// had valid syntax, but it didn't exist in this property set, so
	// no properties were retrieved, and each PROPVARIANT structure is set 
	// to VT_EMPTY. But the following switch statement will handle this case
	// and return E_FAIL. So the caller of ReadPropertyLong does not need
	// to check for S_FALSE explicitly.

	if (SUCCEEDED(hr))
	{
		hr = S_OK;

		switch (PropVariant.vt)
		{
			case VT_I1:
				*plResult = (LONG)PropVariant.cVal;
				break;
			case VT_UI1:
				*plResult = (LONG)PropVariant.bVal;
				break;
			case VT_I2:
				*plResult = (LONG)PropVariant.iVal;
				break;
			case VT_UI2:
				*plResult = (LONG)PropVariant.uiVal;
				break;
			case VT_I4:
				*plResult = (LONG)PropVariant.lVal;
				break;
			case VT_UI4:
				*plResult = (LONG)PropVariant.ulVal;
				break;
			case VT_INT:
				*plResult = (LONG)PropVariant.intVal;
				break;
			case VT_UINT:
				*plResult = (LONG)PropVariant.uintVal;
				break;
			case VT_R4:
				*plResult = (LONG)(PropVariant.fltVal + 0.5);
				break;
			case VT_R8:
				*plResult = (LONG)(PropVariant.dblVal + 0.5);
				break;
			default:
				hr = E_FAIL;
				break;
		}
	}

	PropVariantClear(&PropVariant);

	return hr;
}

HRESULT ReadPropertyGuid(IWiaItem2* pWiaItem2, PROPID propid, GUID* pguid_val)
{
	if (!pWiaItem2)
	{
		HRESULT hr = E_INVALIDARG;
		ReportError(TEXT("Invalid argument passed to ReadPropertyGuid()"), hr);
		return hr;
	}
	//initialize out variables
	*pguid_val = GUID_NULL;

	//Get the IWiaPropertyStorage interface for IWiaItem2 
	IWiaPropertyStorage* pWiaPropertyStorage = NULL;
	HRESULT hr = pWiaItem2->QueryInterface(IID_IWiaPropertyStorage, (void**)&pWiaPropertyStorage);
	if (SUCCEEDED(hr))
	{
		// Declare PROPSPECs and PROPVARIANTs, and initialize them.
		PROPSPEC PropSpec[1] = { 0 };
		PROPVARIANT PropVar[1];
		PropVariantInit(PropVar);

		PropSpec[0].ulKind = PRSPEC_PROPID;
		PropSpec[0].propid = propid;
		hr = pWiaPropertyStorage->ReadMultiple(1, PropSpec, PropVar);
		if (S_OK == hr)
		{
			if (PropVar[0].vt == VT_CLSID)
			{
				memcpy(pguid_val, PropVar[0].puuid, sizeof(GUID));
			}
			else
			{
				ReportError(TEXT("Trying to read a property which doesn't return a Guid"));
			}

		}
		else
		{
			ReportError(TEXT("Error calling pWiaPropertyStorage->ReadMultiple() in ReadPropertyGuid()"), hr);
		}

		//Free PropVar array
		PropVariantClear(PropVar);

		//Release pWiaPropertyStorage 
		pWiaPropertyStorage->Release();
		pWiaPropertyStorage = NULL;
	}
	else
	{
		ReportError(TEXT("pWiaItem2->QueryInterface failed on IID_IWiaPropertyStorage"), hr);
	}

	return hr;
}

HRESULT ReadPropertyGuid(IWiaPropertyStorage * pWiaPropertyStorage, const PROPSPEC * pPropSpec, GUID * pguidResult)
{
	PROPVARIANT PropVariant;

	HRESULT hr = pWiaPropertyStorage->ReadMultiple(1,pPropSpec,&PropVariant);

	// Generally, the return value should be checked against S_FALSE.
	// If ReadMultiple returns S_FALSE, it means the property name or ID
	// had valid syntax, but it didn't exist in this property set, so
	// no properties were retrieved, and each PROPVARIANT structure is set 
	// to VT_EMPTY. But the following switch statement will handle this case
	// and return E_FAIL. So the caller of ReadPropertyGuid does not need
	// to check for S_FALSE explicitly.

	if (SUCCEEDED(hr))
	{
		hr = S_OK;
		switch (PropVariant.vt)
		{
			case VT_CLSID:
				*pguidResult = *PropVariant.puuid;
				break;
			case VT_BSTR:
				hr = CLSIDFromString(PropVariant.bstrVal, pguidResult);
				break;
			case VT_LPWSTR:
				hr = CLSIDFromString(PropVariant.pwszVal, pguidResult);
				break;
			case VT_LPSTR:
			{
				WCHAR wszGuid[MAX_GUID_STRING_LEN];
				size_t *pConvertedChars = NULL;
				mbstowcs_s(pConvertedChars, wszGuid, COUNTOF(wszGuid) - 1, PropVariant.pszVal, MAX_GUID_STRING_LEN);
				wszGuid[MAX_GUID_STRING_LEN - 1] = L'\0';
				hr = CLSIDFromString(wszGuid, pguidResult);
				break;
			}
			default:
				hr = E_FAIL;
				break;
		}
	}

	PropVariantClear(&PropVariant);

	return hr;
}

HRESULT WritePropertyGuid(IWiaPropertyStorage* pWiaPropertyStorage, PROPID propid, GUID guid)
{
	if (!pWiaPropertyStorage)
	{
		HRESULT hr = E_INVALIDARG;
		ReportError(TEXT("Invalid argument passed to WritePropertyGuid()"), hr);
		return hr;
	}

	// Declare PROPSPECs and PROPVARIANTs, and initialize them.
	PROPSPEC PropSpec[1] = { 0 };
	PROPVARIANT PropVar[1];
	PropVariantInit(PropVar);

	PropSpec[0].ulKind = PRSPEC_PROPID;
	PropSpec[0].propid = propid;

	//Fill values in Propvar which are to be written 
	PropVar[0].vt = VT_CLSID;
	PropVar[0].puuid = &guid;

	HRESULT hr = pWiaPropertyStorage->WriteMultiple(1, PropSpec, PropVar, WIA_IPA_FIRST);

	if (FAILED(hr))
	{
		ReportError(TEXT("pWiaPropertyStorage->WriteMultiple() failed in WritePropertyGuid()"), hr);
	}
	return hr;
}

HRESULT WritePropertyLong(IWiaPropertyStorage* pWiaPropertyStorage, PROPID propid, LONG lVal)
{
	if (!pWiaPropertyStorage)
	{
		HRESULT hr = E_INVALIDARG;
		ReportError(TEXT("Invalid argument passed to WritePropertyLong()"), hr);
		return hr;
	}

	// Declare PROPSPECs and PROPVARIANTs, and initialize them.
	PROPSPEC PropSpec[1] = { 0 };
	PROPVARIANT PropVar[1];
	PropVariantInit(PropVar);

	//Fill values in Propvar which are to be written 
	PropSpec[0].ulKind = PRSPEC_PROPID;
	PropSpec[0].propid = propid;
	PropVar[0].vt = VT_I4;
	PropVar[0].lVal = lVal;

	HRESULT hr = pWiaPropertyStorage->WriteMultiple(1, PropSpec, PropVar, WIA_IPA_FIRST);
	if (FAILED(hr))
	{
		ReportError(TEXT("pWiaPropertyStorage->WriteMultiple() failed in WritePropertyLong()"), hr);
	}
	return hr;
}


HRESULT ReadWiaPropsAndGetDeviceID(IWiaPropertyStorage *pWiaPropertyStorage, BSTR* pbstrDeviceID)
{
	// Validate arguments
	if ((NULL == pWiaPropertyStorage) || (NULL == pbstrDeviceID))
	{
		HRESULT hr = E_INVALIDARG;
		ReportError(TEXT("Invalid argument passed to ReadWiaPropsAndGetDeviceID()"), hr);
		return hr;
	}

	//Initialize out variables
	*pbstrDeviceID = NULL;

	BSTR  bstrdeviceName = NULL, bstrdeviceDesc = NULL;

	//Read device ID
	HRESULT hr = ReadPropertyBSTR(pWiaPropertyStorage, WIA_DIP_DEV_ID, pbstrDeviceID);
	if (FAILED(hr))
	{
		ReportError(TEXT("ReadPropertyBSTR() failed for WIA_DIP_DEV_ID in ReadWiaPropsAndGetDeviceID()"), hr);
		return hr;
	}

	//Read device name
	hr = ReadPropertyBSTR(pWiaPropertyStorage, WIA_DIP_DEV_NAME, &bstrdeviceName);
	if (FAILED(hr))
	{
		ReportError(TEXT("ReadPropertyBSTR() failed for WIA_DIP_DEV_NAME in ReadWiaPropsAndGetDeviceID()"), hr);
		return hr;
	}

	//Read device description
	hr = ReadPropertyBSTR(pWiaPropertyStorage, WIA_DIP_DEV_DESC, &bstrdeviceDesc);
	if (FAILED(hr))
	{
		ReportError(TEXT("ReadPropertyBSTR() failed for WIA_DIP_DEV_DESC in ReadWiaPropsAndGetDeviceID()"), hr);
		return hr;
	}

	ofLog(OF_LOG_NOTICE, "WIA_DIP_DEV_ID: %ws", *pbstrDeviceID);
	ofLog(OF_LOG_NOTICE, "WIA_DIP_DEV_NAME: %ws", bstrdeviceName);
	ofLog(OF_LOG_NOTICE, "WIA_DIP_DEV_DESC: %ws", bstrdeviceDesc);

	return S_OK;
}


HRESULT PrintItemName(IWiaItem2 *pIWiaItem2)
{
	// Validate arguments
	if (NULL == pIWiaItem2)
	{
		HRESULT hr = E_INVALIDARG;
		ReportError(TEXT("Invalid parameters passed"), hr);
		return hr;
	}

	// Get the IWiaPropertyStorage interface
	IWiaPropertyStorage *pWiaPropertyStorage = NULL;
	HRESULT hr = pIWiaItem2->QueryInterface(IID_IWiaPropertyStorage, (void**)&pWiaPropertyStorage);
	if (SUCCEEDED(hr))
	{

		BSTR bstrDevName = NULL;
		ReadPropertyBSTR(pWiaPropertyStorage, WIA_IPA_FULL_ITEM_NAME, &bstrDevName);
		//ofLog(OF_LOG_NOTICE, "");
		ofLog(OF_LOG_NOTICE, "Item Name: %ws", bstrDevName);
		//ofLog(OF_LOG_NOTICE, "");

		// Release the IWiaPropertyStorage interface
		pWiaPropertyStorage->Release();
		pWiaPropertyStorage = NULL;
	}
	else
	{
		ReportError(TEXT("QueryInterface() failed on IID_IWiaPropertyStorage"), hr);
	}

	// Return the result of reading the properties
	return hr;
}

void FindtemTypeNameList(LONG lItemType, std::vector<std::string> &result)
{
	if (lItemType & WiaItemTypeFree)
		result.push_back("WiaItemTypeFree");
	if (lItemType & WiaItemTypeImage)
		result.push_back("WiaItemTypeImage");
	if (lItemType & WiaItemTypeFile)
		result.push_back("WiaItemTypeFile");
	if (lItemType & WiaItemTypeFolder)
		result.push_back("WiaItemTypeFolder");
	if (lItemType & WiaItemTypeRoot)
		result.push_back("WiaItemTypeRoot");
	if (lItemType & WiaItemTypeAnalyze)
		result.push_back("WiaItemTypeAnalyze");
	if (lItemType & WiaItemTypeAudio)
		result.push_back("WiaItemTypeAudio");
	if (lItemType & WiaItemTypeDevice)
		result.push_back("WiaItemTypeDevice");
	if (lItemType & WiaItemTypeDeleted)
		result.push_back("WiaItemTypeDeleted");
	if (lItemType & WiaItemTypeDisconnected)
		result.push_back("WiaItemTypeDisconnected");
	if (lItemType & WiaItemTypeHPanorama)
		result.push_back("WiaItemTypeHPanorama");
	if (lItemType & WiaItemTypeVPanorama)
		result.push_back("WiaItemTypeVPanorama");
	if (lItemType & WiaItemTypeBurst)
		result.push_back("WiaItemTypeBurst");
	if (lItemType & WiaItemTypeStorage)
		result.push_back("WiaItemTypeStorage");
	if (lItemType & WiaItemTypeTransfer)
		result.push_back("WiaItemTypeTransfer");
	if (lItemType & WiaItemTypeGenerated)
		result.push_back("WiaItemTypeGenerated");
	if (lItemType & WiaItemTypeHasAttachments)
		result.push_back("WiaItemTypeHasAttachments");
	if (lItemType & WiaItemTypeVideo)
		result.push_back("WiaItemTypeVideo");
}

void FindItemCategoryNameList(GUID lItemCategory, std::vector<std::string> &result)
{
	if (lItemCategory == WIA_CATEGORY_FINISHED_FILE)
		result.push_back("WIA_CATEGORY_FINISHED_FILE");
	if (lItemCategory == WIA_CATEGORY_FLATBED)
		result.push_back("WIA_CATEGORY_FLATBED");
	if (lItemCategory == WIA_CATEGORY_FEEDER)
		result.push_back("WIA_CATEGORY_FEEDER");
	if (lItemCategory == WIA_CATEGORY_FILM)
		result.push_back("WIA_CATEGORY_FILM");
	if (lItemCategory == WIA_CATEGORY_ROOT)
		result.push_back("WIA_CATEGORY_ROOT");
	if (lItemCategory == WIA_CATEGORY_FOLDER)
		result.push_back("WIA_CATEGORY_FOLDER");
	if (lItemCategory == WIA_CATEGORY_FEEDER_FRONT)
		result.push_back("WIA_CATEGORY_FEEDER_FRONT");
	if (lItemCategory == WIA_CATEGORY_FEEDER_BACK)
		result.push_back("WIA_CATEGORY_FEEDER_BACK");
	if (lItemCategory == WIA_CATEGORY_AUTO)
		result.push_back("WIA_CATEGORY_AUTO");
	if (lItemCategory == WIA_CATEGORY_IMPRINTER)
		result.push_back("WIA_CATEGORY_IMPRINTER");
	if (lItemCategory == WIA_CATEGORY_ENDORSER)
		result.push_back("WIA_CATEGORY_ENDORSER");
	if (lItemCategory == WIA_CATEGORY_BARCODE_READER)
		result.push_back("WIA_CATEGORY_BARCODE_READER");
	if (lItemCategory == WIA_CATEGORY_PATCH_CODE_READER)
		result.push_back("WIA_CATEGORY_PATCH_CODE_READER");
	if (lItemCategory == WIA_CATEGORY_MICR_READER)
		result.push_back("WIA_CATEGORY_MICR_READER");
}

void PrintItemTypes(LONG lItemType)
{
	std::vector<std::string> result;

	FindtemTypeNameList(lItemType, result);

	for (auto& name : result)
	{
		ofLogNotice() << "ItemType=" << name;
	}
}

void PrintItemCategory(GUID lItemCategory)
{
	std::vector<std::string> result;

	FindItemCategoryNameList(lItemCategory, result);

	for (auto& name : result)
	{
		ofLogNotice() << "ItemCategory=" << name;
	}
}