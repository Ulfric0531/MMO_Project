#pragma once

typedef struct EmergencyBuffer
{
	BYTE*				_buffer;
	DWORD				_size;
	EmergencyBuffer*	_next;
} EmergencyBuffer;

typedef struct Peer
{
	SRWLOCK				_sendBufferLock;
	DWORD64				_id;
	DWORD				_index;
	SOCKET				_socket;
	SOCKADDR_IN			_addr;
	RingBuffer			_recvBuffer;
	RingBuffer			_sendBuffer;
	RecvOverlap			_recvOverlap;
	SendOverlap			_sendOverlap;
	DisconnectOverlap	_disconnectOverlap;
	volatile LONG		_refCount;
	volatile LONG		_isConnected;
	volatile LONG		_onSend;


	DWORD				_emergencyBufferCount;
	EmergencyBuffer*	_emergencyBufferHead;
	EmergencyBuffer*	_emergencyBufferTail;
	SRWLOCK				_emergencyBufferLock;
} Peer;

void	Peer_Init(Peer* out_Peer, SOCKET socket, SOCKADDR_IN addr, DWORD64 id, DWORD index);
void	Peer_Release(Peer* peer);
void	Peer_IncRefCount(Peer* peer);
void	Peer_DecRefCount(Peer* peer);