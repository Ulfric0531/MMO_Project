#include "pch.h"



void Session_Init(Session* session, SOCKET socket, SOCKADDR_IN addr, DWORD64 id, DWORD index)
{
	InitializeSRWLock(&(session->_sendBufferLock));
	session->_socket = socket;
	session->_addr = addr;
	session->_id = id;
	session->_index = index;
	RingBuffer_Init(&(session->_recvBuffer), DEFAULT_RECVBUFF_SIZE);
	RingBuffer_Init(&(session->_sendBuffer), DEFAULT_SENDBUFF_SIZE);
	OverlappedEx_Init(&(session->_sendOverlap), IOTYPE_SEND);
	OverlappedEx_Init(&(session->_recvOverlap), IOTYPE_RECV);
	OverlappedEx_Init(&(session->_disconnectOverlap), IOTYPE_DISCONNECT);
	session->_refCount = 0;
	session->_isConnected = 1;
	session->_onSend = 0;

	session->_emergencyBufferCount = 0;
	session->_emergencyBufferHead = NULL;
	session->_emergencyBufferTail = NULL;
	InitializeSRWLock(&(session->_emergencyBufferLock));
}

void Session_Release(Session* session)
{
}

void Session_IncreaseRefCount(Session* session)
{
	InterlockedIncrement(&(session->_refCount));
}

void Session_DecreaseRefCount(Session* session)
{
	if (InterlockedDecrement(&(session->_refCount)) == 0)
	{
		// TODO : NetManager ReleaseSession
	}
}

void Session_RecvPost(Session* session)
{
	if (InterlockedCompareExchangeNoFence(&(session->_isConnected), 0, 0) == 0)
	{
		return;
	}

	Session_IncreaseRefCount(session);

	int errCode = 0;
	int retVal = 0;
	DWORD numOfBytes = 0;
	DWORD flags = 0;

	OverlappedEx_Clear(&(session->_recvOverlap));

	LONG front = RingBuffer_GetFrontIndex(&session->_recvBuffer);
	LONG rear = RingBuffer_GetRearIndex(&session->_recvBuffer);
	DWORD directEnqueueSize = RingBuffer_DirectEnqueueSize(&session->_recvBuffer, front, rear);
	DWORD remainSize = RingBuffer_GetFreeSize(&session->_recvBuffer, front, rear) - directEnqueueSize;
	WSABUF wsabuf[2];
	wsabuf[0].buf = session->_recvBuffer._buffer + rear;
	wsabuf[0].len = directEnqueueSize;
	wsabuf[1].buf = session->_recvBuffer._buffer;
	wsabuf[1].len = remainSize;

	retVal = WSARecv(session->_socket, wsabuf, 2, &numOfBytes, &flags, (LPOVERLAPPED)(&session->_recvOverlap), NULL);
	if (retVal == SOCKET_ERROR)
	{
		errCode = WSAGetLastError();
		if (errCode != WSA_IO_PENDING)
		{
			Session_DisconnectPost(session);

			Session_DecreaseRefCount(session);
		}
	}
}

void Session_TrySendPost(Session* session, BYTE* buffer, DWORD size)
{
	BOOL enqueueResult;
	EmergencyBuffer* sendBuffer;

	if (InterlockedCompareExchangeNoFence(&session->_isConnected, 0, 0) == 0)
	{
		return;
	}

	AcquireSRWLockExclusive(&session->_sendBufferLock);
	enqueueResult = RingBuffer_Enqueue(&session->_sendBuffer, buffer, size);
	ReleaseSRWLockExclusive(&session->_sendBufferLock);

	if (enqueueResult == FALSE)
	{
		sendBuffer = PoolAlloc(sizeof(EmergencyBuffer));
		sendBuffer->_size = size;
		sendBuffer->_buffer = PoolAlloc(sizeof(size));
		memcpy_s(sendBuffer->_buffer, size, buffer, size);

		AcquireSRWLockExclusive(&session->_emergencyBufferLock);
		if (session->_emergencyBufferHead == NULL)
		{
			session->_emergencyBufferHead = sendBuffer;
		}
		else
		{
			session->_emergencyBufferTail->_next = sendBuffer;
		}
		session->_emergencyBufferTail = sendBuffer;
		ReleaseSRWLockExclusive(&session->_emergencyBufferLock);
	}

	if (InterlockedCompareExchangeNoFence(&session->_onSend, 1, 0) == 1)
	{
		return;
	}

	Session_SendPost(session);
	return;
}

void Session_SendPost(Session* session)
{
	if (InterlockedCompareExchangeNoFence(&session->_isConnected, 0, 0) == 0)
	{
		return;
	}

	Session_IncreaseRefCount(session);

	int erroCode;
	int retVal;
	DWORD idx;
	DWORD directEnqueueSize;
	DWORD remainSize;
	DWORD numOfBytes;
	DWORD flags;
	DWORD count = 0;
	OverlappedEx_Clear(&session->_sendOverlap);


}

void Session_DisconnectPost(Session* session)
{
}

void Session_RecvComplete(Session* session, DWORD completedBytes, BOOL isIoCanceled)
{
}

void Session_SendComplate(Session* session, DWORD completedBytes, BOOL isIoCanceled)
{
}

void Session_DisconnectComplete(Session* session)
{
}

DWORD64 Session_GetId(Session* session)
{
	return 1;
}
