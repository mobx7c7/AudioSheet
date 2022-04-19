#pragma once
#include "..\..\ScannerInput.h"
#include <Wia.h>
#include <Sti.h>
#include <atlbase.h>
#include <string>
#include <vector>
#include "WiaCommon.h"

class WiaScannerInput2 : public ScannerInput
{
private:
	int selectedDevice = 0;
	std::vector<BSTR> devices;
	IWiaDevMgr* pWiaDevMgr;
	IWiaItem* pWiaDevice;

private:
	LONG GetPropertyLong(PROPID);
	HRESULT CreateDevice(int deviceIndex);
	HRESULT ListDevices();
	HRESULT ListChildren(IWiaItem* pItemRoot, LONG* lCount, IWiaItem*** ppiWiaItem);
	HRESULT AddDevice(IWiaPropertyStorage *pStorage);
	HRESULT FillProperties(IWiaPropertyStorage*);
	HRESULT DownloadImage(LONG lDeviceType, LONG lFlags, LONG lIntent, GUID *pguidFormat);

public:
	WiaScannerInput2();
	~WiaScannerInput2();
	void open(int deviceIndex); // rename to selectDevice
	void setup();
	void refresh();
	void close();
	void read();
};