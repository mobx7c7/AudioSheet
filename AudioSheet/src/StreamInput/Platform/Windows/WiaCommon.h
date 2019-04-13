#pragma once
#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include <tchar.h>
#include <wia.h>
#include <oleauto.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <vector>

#ifndef MAX_GUID_STRING_LEN
#define MAX_GUID_STRING_LEN 39
#endif //MAX_GUID_STRING_LEN
#define COUNTOF(x) ( sizeof(x) / sizeof(*x) )


// Helper function to display an error message and an optional HRESULT
void ReportError(LPCTSTR pszMessage, HRESULT hr = S_OK);

// Reads item property which returns GUID like WIA_IPA_FORMAT, WIA_IPA_ITEM_CATEGORY etc.
HRESULT ReadPropertyGuid(IWiaItem2* pWiaItem2, PROPID propid, GUID* guid);

HRESULT ReadPropertyGuid(IWiaPropertyStorage *pWiaPropertyStorage, const PROPSPEC *pPropSpec, GUID *pguidResult);

// Reads item property which returns BSTR like WIA_DIP_DEV_ID, WIA_DIP_DEV_NAME etc.
HRESULT ReadPropertyBSTR(IWiaPropertyStorage* pWiaPropertyStorage, PROPID propid, BSTR* pbstr);

// Reads item property which returns LONG 
HRESULT ReadPropertyLong(IWiaItem2* pWiaItem2, PROPID propid, LONG* lVal);

HRESULT ReadPropertyLong(IWiaPropertyStorage *pWiaPropertyStorage, const PROPSPEC *pPropSpec, LONG *plResult);

// Writes item property which takes GUID like WIA_IPA_FORMAT, WIA_IPA_ITEM_CATEGORY etc.
HRESULT WritePropertyGuid(IWiaPropertyStorage* pWiaPropertyStorage, PROPID propid, GUID guid);

// Writes item property which takes LONG like WIA_IPA_PAGES etc.
HRESULT WritePropertyLong(IWiaPropertyStorage* pWiaPropertyStorage, PROPID propid, LONG lVal);

// Reads such WIA device properties as Device ID, Device name and Device descripion 
// and also it returns the device ID.
HRESULT ReadWiaPropsAndGetDeviceID(IWiaPropertyStorage *pWiaPropertyStorage, BSTR* pbstrDeviceID);

// Prints the item name 
HRESULT PrintItemName(IWiaItem2 *pIWiaItem2);

void FindtemTypeNameList(LONG lItemType, std::vector<std::string> &result);

void FindItemCategoryNameList(GUID lItemCategory, std::vector<std::string> &result);

void PrintItemTypes(LONG lItemType);

void PrintItemCategory(GUID lItemCategory);