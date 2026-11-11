#include "pch.h"

void Peer_Init(Peer* out_Peer, SOCKET socket, SOCKADDR_IN addr, DWORD64 id, DWORD index)
{
	InitializeSRWLock(&out_Peer->_sendBufferLock);
	out_Peer->_socket = socket;
	out_Peer->_addr = addr;
	out_Peer->_id = id;
	out_Peer->_index = index;
	RingBuffer_Init(&out_Peer->_recvBuffer, DEFAULT_RECVBUFF_SIZE);
	RingBuffer_Init(&out_Peer->_sendBuffer, DEFAULT_SENDBUFF_SIZE);
	OverlappedEx_Init(&out_Peer->_sendOverlap, IOTYPE_SEND);
	OverlappedEx_Init(&out_Peer->_recvOverlap, IOTYPE_RECV);
	OverlappedEx_Init(&out_Peer->_disconnectOverlap, IOTYPE_DISCONNECT);
	out_Peer->_refCount = 1;  // for WSARecv
	out_Peer->_isConnected = 1;
	out_Peer->_onSend = 0;

	out_Peer->_emergencyBufferCount = 0;
	out_Peer->_emergencyBufferHead = NULL;
	out_Peer->_emergencyBufferTail = NULL;
	InitializeSRWLock(&out_Peer->_emergencyBufferLock);
}

void Peer_Release(Peer* peer)
{
}

void Peer_IncRefCount(Peer* peer)
{
	InterlockedIncrement(&peer->_refCount);
}

void Peer_DecRefCount(Peer* peer)
{
	if (InterlockedDecrement(&peer->_refCount) == 0)
	{
		// TODO : NetManager ReleaseSession
	}
}
