#pragma once
#include "..\StreamEvent.h"

class ScannerInputEvent : public StreamEventBase
{
public:
	char* data;
	size_t size;
	int page;
	int progress;
	int offset;
};