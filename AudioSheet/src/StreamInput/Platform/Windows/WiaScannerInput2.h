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
	IWiaDevMgr2* pWiaDevMgr;
private:
	void RedirectEvent(const StreamEventBase&);
	HRESULT GetImage(
		HWND hWndParent,
		LONG lDeviceType,
		LONG lFlags,
		LONG lIntent,
		IWiaDevMgr *pSuppliedWiaDevMgr,
		IWiaItem *pSuppliedItemRoot,
		GUID *pguidFormat,
		LONG *plCount,
		IStream ***pppStream);
public:
	WiaScannerInput2();
	~WiaScannerInput2();
	void open(int deviceIndex);
	void close();
	void read();
};