#include "pch.h"

Peer* Peer_Make(SOCKET socket, SOCKADDR_IN addr, DWORD64 id, DWORD index)
{
	Peer* peer = (Peer*)malloc(sizeof(Peer));
	InitializeSRWLock(&peer->_sendBufferLock);
	peer->_socket = socket;
	peer->_addr = addr;
	peer->_id = id;
	peer->_index = index;
	RingBuffer_Init(&peer->_recvBuffer, DEFAULT_RECVBUFF_SIZE);
	RingBuffer_Init(&peer->_sendBuffer, DEFAULT_SENDBUFF_SIZE);
	OverlappedEx_Init(&peer->_sendOverlap, IOTYPE_SEND);
	OverlappedEx_Init(&peer->_recvOverlap, IOTYPE_RECV);
	OverlappedEx_Init(&peer->_disconnectOverlap, IOTYPE_DISCONNECT);
	peer->_refCount = 1;  // for WSARecv
	peer->_isConnected = 1;
	peer->_onSend = 0;

	peer->_emergencyBufferCount = 0;
	peer->_emergencyBufferHead = NULL;
	peer->_emergencyBufferTail = NULL;
	InitializeSRWLock(&peer->_emergencyBufferLock);

	return peer;
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
