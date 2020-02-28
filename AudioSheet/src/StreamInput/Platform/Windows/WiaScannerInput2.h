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
	std::vector<BSTR> devices;
	IWiaDevMgr* pWiaDevMgr;
	IWiaItem* pWiaItem;
	DWORD readThreadId;
private:
	//DWORD WINAPI GetImageThread(LPVOID lpParam);
	void RedirectEvent(const StreamEventBase&);
	HRESULT CreateDeviceManager(IWiaDevMgr** ppWiaDevMgr);
	HRESULT CreateDevice(IWiaDevMgr* pWiaDevMgr, BSTR bstrDeviceID, IWiaItem** ppWiaDevice);
	HRESULT EnumerateDevices(IWiaDevMgr *pWiaDevMgr);
	HRESULT EnumerateChildren(IWiaItem* pItemRoot, LONG* lCount, IWiaItem*** ppiWiaItem);
	HRESULT AddDevice(IWiaPropertyStorage *pStorage);
	HRESULT GetImage(
		HWND hWndParent,
		LONG lDeviceType,
		LONG lFlags,
		LONG lIntent,
		IWiaDevMgr *pSuppliedWiaDevMgr,
		IWiaItem *pSuppliedItemRoot,
		GUID *pguidFormat);
public:
	WiaScannerInput2();
	~WiaScannerInput2();
	void open(int deviceIndex);
	void close();
	void read();
};