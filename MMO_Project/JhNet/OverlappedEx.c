#include "pch.h"

void OverlappedEx_Init(OverlappedEx* out_OverlappedEx, IoType ioType, SOCKET socket, Peer* peer)
{
	ZeroMemory(out_OverlappedEx, sizeof(OVERLAPPED));
	out_OverlappedEx->_ioType = ioType;

	switch (ioType)
	{
	case IOTYPE_RECV:
		break;
	case IOTYPE_SEND:
	{
		((SendOverlap*)out_OverlappedEx)->_onFlightList = NULL;
		break;
	}
	case IOTYPE_ACCEPT:
	{
		((AcceptOverlap*)out_OverlappedEx)->_clientSock = socket;
		((AcceptOverlap*)out_OverlappedEx)->_peer = peer;
		break;
	}	
	case IOTYPE_DISCONNECT:
		break;
	case IOTYPE_CONNECT:
		((ConnectOverlap*)out_OverlappedEx)->_clientSock = socket;
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

