#pragma once
#include <wia_lh.h>
#include "..\..\ScannerInput.h"
#include <string>
#include <vector>

#define FOLDER_TRANSFER TRUE
#define FILE_TRANSFER FALSE

class WiaScannerInput : public ScannerInput
{
public:
	struct Field
	{
		std::string name, value;
		Field(const std::string &name, const std::string &value) 
			: name(name)
			, value(value) {}
	};
	struct NextField : public Field
	{
		NextField(const std::string &name, const std::string &value) 
			: Field(name, value) 
		{}
		friend std::ostream& operator<<(std::ostream &os, const NextField &field);
	};
	struct LastField : public Field
	{
		LastField(const std::string &name, const std::string &value) 
			: Field(name, value) 
		{}
		friend std::ostream& operator<<(std::ostream &os, const LastField &field);
	};
	struct DeviceInfo
	{
		ULONG ulNumProps;
		BSTR id, name, desc;
		friend std::ostream& operator<<(std::ostream &os, const DeviceInfo &info);
	};
private:
	std::vector<DeviceInfo> availableDevices;
	IWiaDevMgr2* pWiaDevMgr;
	IWiaItem2 *pWiaRootItem2; // device
private:
	void HandleEventsFromTransferCb(const StreamEventBase& ev);
	HRESULT CreateWiaDeviceManager(IWiaDevMgr2 **ppWiaDevMgr);
	HRESULT CreateWiaDevice(IWiaDevMgr2 *pWiaDevMgr, BSTR bstrDeviceID, IWiaItem2 **ppWiaDevice);
	HRESULT EnumerateDevices(IWiaDevMgr2 *pWiaDevMgr);
	HRESULT EnumerateCapabilities(IWiaItem2 *pWiaItem);
	HRESULT EnumerateItems(IWiaItem2 *pIWiaItem2Root);
	HRESULT ReadDeviceInfo(IWiaPropertyStorage *pWiaPropertyStorage, DeviceInfo* availableDevices);
	HRESULT TransferItem(IWiaItem2 *pIWiaItem2);
	HRESULT DownloadItem(IWiaItem2* pWiaItem2, BOOL bFolderTransfer);
	void refresh();
public:
	WiaScannerInput();
	~WiaScannerInput();
	void open(int deviceIndex);
	void close();
	void read();
};
