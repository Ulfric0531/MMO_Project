#include "pch.h"

void OverlappedEx_Init(OverlappedEx* out_OverlappedEx, IoType ioType)
{
	memset(out_OverlappedEx, 0, sizeof(OVERLAPPED));
	out_OverlappedEx->_ioType = ioType;
	out_OverlappedEx->_onFlightList = NULL;
}

void OverlappedEx_Release(OverlappedEx* out_OverlappedEx)
{
}

void OverlappedEx_Clear(OverlappedEx* out_OverlappedEx)
{
	// OVERLAPPED
	OVERLAPPED* overlapped = out_OverlappedEx;
	overlapped->hEvent = 0;
	overlapped->Offset = 0;
	overlapped->OffsetHigh = 0;
	overlapped->Internal = 0;
	overlapped->InternalHigh = 0;
}

