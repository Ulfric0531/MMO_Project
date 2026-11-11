#include "pch.h"

void OverlappedEx_Init(OverlappedEx* out_OverlappedEx, IoType ioType, SOCKET socket)
{
	memset(out_OverlappedEx, 0, sizeof(OVERLAPPED));
	out_OverlappedEx->_ioType = ioType;

	switch (ioType)
	{
	case IOTYPE_RECV:
		break;
	case IOTYPE_SEND:
	{
		SendOverlap* sendOverlap = out_OverlappedEx;
		sendOverlap->_onFlightList = NULL;
		break;
	}
	case IOTYPE_ACCEPT:
	{
		AcceptOverlap* acceptOverlap = out_OverlappedEx;
		acceptOverlap->_clientSock = socket;
	}
		break;
	case IOTYPE_DISCONNECT:
		break;
	case IOTYPE_CONNECT:
		break;
	}
	
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

