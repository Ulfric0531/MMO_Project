#pragma once

typedef struct OverlappedEx OverlappedEx;
typedef struct RingBuffer RingBuffer;

typedef struct EmergencyBuffer
{
	BYTE*				buffer;
	DWORD				size;
	EmergencyBuffer*	next;
} EmergencyBuffer;

typedef struct Session
{
	SRWLOCK			_sendBufferLock;
	DWORD64			_id;
	DWORD			_index;
	SOCKET			_socket;
	SOCKADDR_IN		_addr;
	RingBuffer		_recvBuffer;
	RingBuffer		_sendBuffer;
	OverlappedEx	_recvOverlap;
	OverlappedEx	_sendOverlap;
	OverlappedEx	_disconnectOverlap;
	volatile LONG	_refCount;
	volatile LONG	_isConnected;
	volatile LONG	_onSend;
	

	DWORD				_emergencyBufferCount;
	EmergencyBuffer*	_emergencyBufferHead;
	EmergencyBuffer*	_emergencyBufferTail;
	SRWLOCK				_emergencyBufferLock;
} Session;

void Session_Init(Session* session, SOCKET socket, SOCKADDR_IN addr, DWORD64 id, DWORD index);
void Session_Release(Session* session);

void Session_IncreaseRefCount(Session* session);
void Session_DecreaseRefCount(Session* session);

void Session_RecvPost(Session* session);
void Session_TrySendPost(Session* session, BYTE* buffer, DWORD size);
void Session_SendPost(Session* session);
void Session_DisconnectPost(Session* session);
void Session_RecvComplete(Session* session, DWORD completedBytes, BOOL isIoCanceled);
void Session_SendComplate(Session* session, DWORD completedBytes, BOOL isIoCanceled);
void Session_DisconnectComplete(Session* session);

DWORD64 Session_GetId(Session* session);

