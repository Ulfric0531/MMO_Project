#include "pch.h"

LPFN_ACCEPTEX		JhNet_AcceptEx;
LPFN_CONNECTEX		JhNet_ConnectEx;
LPFN_DISCONNECTEX	JhNet_DisconnectEx;

JhNet* JhNet_MakeNetManager(WCHAR* serverIp, WORD serverPort, DWORD threadCount, NetMode netMode)
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	JhNet* jhNet = (JhNet*)malloc(sizeof(JhNet));
	jhNet->_netMode = netMode;

	int bindRetval;
	int listenRetval;
	int lingerOptRetval;
	int reuseAddrOptRetval;
	int errCode;

	switch (netMode)
	{
	case NETMODE_SERVER:
		jhNet->_acceptDisconnectIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 1);
		memset(&jhNet->_serverAddr, 0, sizeof(jhNet->_serverAddr));
		jhNet->_serverAddr.sin_family = AF_INET;
		jhNet->_serverAddr.sin_port = htons(serverPort);
		InetPton(AF_INET, L"0.0.0.0", (SOCKADDR*)(&jhNet->_serverAddr.sin_addr));
		jhNet->_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

		bindRetval = bind(jhNet->_socket, (SOCKADDR*)(&jhNet->_serverAddr), sizeof(jhNet->_serverAddr));
		if (bindRetval == SOCKET_ERROR)
		{
			errCode = WSAGetLastError();
			wprintf(L"Listen Socket bind() error : %d", errCode);
		}

		listenRetval = listen(jhNet->_socket, SOMAXCONN);
		if (listenRetval == SOCKET_ERROR)
		{
			errCode = WSAGetLastError();
			wprintf(L"Listen Socket listen() error : %d", errCode);
		}

		struct linger lingerOption;
		lingerOption.l_linger = 0;
		lingerOption.l_onoff = 0;
		lingerOptRetval = setsockopt(jhNet->_socket, SOL_SOCKET, SO_LINGER, (const char*)(&lingerOption), sizeof(lingerOption));
		if (lingerOptRetval == SOCKET_ERROR)
		{
			errCode = WSAGetLastError();
			wprintf(L"Listen Socket Linger setsockopt() error : %d", errCode);
		}

		BOOL reuseAddrOption = TRUE;
		reuseAddrOptRetval = setsockopt(jhNet->_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)(&reuseAddrOption), sizeof(BOOL));
		if (reuseAddrOptRetval == SOCKET_ERROR)
		{
			errCode = WSAGetLastError();
			wprintf(L"Listen Socket Reuse Address setsockopt() error : %d", errCode);
		}
		break;

	case NETMODE_CLIENT:
		OverlappedEx_Init(&jhNet->_connectOverlapped, IOTYPE_CONNECT, 0);
		memset(&jhNet->_serverAddr, 0, sizeof(jhNet->_serverAddr));
		jhNet->_serverAddr.sin_family = AF_INET;
		jhNet->_serverAddr.sin_port = htons(serverPort);
		InetPton(AF_INET, serverIp, &jhNet->_serverAddr.sin_addr);
		break;
	default:
		return NULL;
	}
	jhNet->_mainIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, threadCount);

	return jhNet;
	
}

BOOL JhNet_AddPeer(JhNet* netManager, Peer* peer)
{
	BOOL retVal = FALSE;
	AcquireSRWLockExclusive(&netManager->_peerListLock);
	for (DWORD i = 0; i < netManager->_maxPeerCount; ++i)
	{
		if (netManager->_peerList[i] == NULL)
		{
			netManager->_peerList[i] = peer;
			peer->_index = i;
			peer->_id = g_peerId;
			retVal = TRUE;
			++g_peerId;
			++netManager->_currentPeerCount;
			goto exit;
		}
	}
exit:
	ReleaseSRWLockExclusive(&netManager->_peerListLock);
	return retVal;
}

BOOL JhNet_DeletePeer(JhNet* netManager, DWORD peerIndex, DWORD64 peerId)
{
	BOOL retVal = FALSE;
	AcquireSRWLockExclusive(&netManager->_peerListLock);
	Peer* peer = netManager->_peerList[peerIndex];
	if (peer == NULL || peer->_id != peerId)
	{
		goto exit;
	}
	netManager->_peerList[peerIndex] = NULL;
	--netManager->_currentPeerCount;
	retVal = TRUE;
exit:
	ReleaseSRWLockExclusive(&netManager->_peerListLock);
	if (retVal == TRUE)
	{
		PoolFree(peer);
	}
	return retVal;
}

void JhNet_InitWinSockEx()
{
	GUID connectExID = WSAID_CONNECTEX;
	GUID disconnectExID = WSAID_DISCONNECTEX;
	GUID acceptExID = WSAID_ACCEPTEX;
	DWORD bytes;
	SOCKET dummySock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	WSAIoctl(dummySock, SIO_GET_EXTENSION_FUNCTION_POINTER, &connectExID, sizeof(connectExID), &JhNet_ConnectEx, sizeof(JhNet_ConnectEx), &bytes, NULL, NULL);
	WSAIoctl(dummySock, SIO_GET_EXTENSION_FUNCTION_POINTER, &disconnectExID, sizeof(disconnectExID), &JhNet_DisconnectEx, sizeof(JhNet_DisconnectEx), &bytes, NULL, NULL);
	WSAIoctl(dummySock, SIO_GET_EXTENSION_FUNCTION_POINTER, &acceptExID, sizeof(acceptExID), &JhNet_AcceptEx, sizeof(JhNet_AcceptEx), &bytes, NULL, NULL);
}

void JhNet_RecvPost(Peer* peer)
{
	if (InterlockedCompareExchangeNoFence(&peer->_isConnected, 0, 0) == 0)
	{
		return;
	}

	int errCode = 0;
	int retVal = 0;
	DWORD numOfBytes = 0;
	DWORD flags = 0;

	OverlappedEx_Clear((OverlappedEx*)&peer->_recvOverlap);

	LONG front = RingBuffer_GetFrontIndex(&peer->_recvBuffer);
	LONG rear = RingBuffer_GetRearIndex(&peer->_recvBuffer);
	DWORD directEnqueueSize = RingBuffer_DirectEnqueueSize(&peer->_recvBuffer, front, rear);
	DWORD remainSize = RingBuffer_GetFreeSize(&peer->_recvBuffer, front, rear) - directEnqueueSize;
	WSABUF wsabuf[2];
	wsabuf[0].buf = peer->_recvBuffer._buffer + rear;
	wsabuf[0].len = directEnqueueSize;
	wsabuf[1].buf = peer->_recvBuffer._buffer;
	wsabuf[1].len = remainSize;

	retVal = WSARecv(peer->_socket, wsabuf, 2, &numOfBytes, &flags, (LPOVERLAPPED)&peer->_recvOverlap, NULL);
	if (retVal == SOCKET_ERROR)
	{
		errCode = WSAGetLastError();
		if (errCode != WSA_IO_PENDING)
		{
			JhNet_DisconnectPost(peer);

			Peer_DecRefCount(peer);
		}
	}
}

void JhNet_TrySendPost(Peer* peer, BYTE* buffer, DWORD size)
{
	BOOL enqueueResult;
	EmergencyBuffer* sendBuffer;

	if (InterlockedCompareExchangeNoFence(&peer->_isConnected, 0, 0) == 0)
	{
		return;
	}

	AcquireSRWLockExclusive(&peer->_sendBufferLock);
	enqueueResult = RingBuffer_Enqueue(&peer->_sendBuffer, buffer, size);
	ReleaseSRWLockExclusive(&peer->_sendBufferLock);

	if (enqueueResult == FALSE)
	{
		sendBuffer = PoolAlloc(sizeof(EmergencyBuffer));
		sendBuffer->_size = size;
		sendBuffer->_buffer = PoolAlloc(sizeof(size));
		memcpy_s(sendBuffer->_buffer, size, buffer, size);

		AcquireSRWLockExclusive(&peer->_emergencyBufferLock);
		if (peer->_emergencyBufferHead == NULL)
		{
			peer->_emergencyBufferHead = sendBuffer;
		}
		else
		{
			peer->_emergencyBufferTail->_next = sendBuffer;
		}
		peer->_emergencyBufferTail = sendBuffer;
		ReleaseSRWLockExclusive(&peer->_emergencyBufferLock);
	}

	if (InterlockedCompareExchangeNoFence(&peer->_onSend, 1, 0) == 1)
	{
		return;
	}

	JhNet_SendPost(peer);
	return;
}

void JhNet_SendPost(Peer* peer)
{
	if (InterlockedCompareExchangeNoFence(&peer->_isConnected, 0, 0) == 0)
	{
		return;
	}

	Peer_IncRefCount(peer);

	int errCode;
	int retVal;
	DWORD idx = 0;
	DWORD directDequeueSize;
	DWORD remainSize;
	DWORD numOfBytes;
	DWORD flags;
	DWORD count = 0;
	OverlappedEx_Clear((OverlappedEx*)&peer->_sendOverlap);

	AcquireSRWLockExclusive(&peer->_emergencyBufferLock);
	if (&peer->_emergencyBufferHead != NULL)
	{
		count = peer->_emergencyBufferCount;

		peer->_sendOverlap._onFlightList = peer->_emergencyBufferHead;
		peer->_emergencyBufferHead = NULL;
		peer->_emergencyBufferTail = NULL;
		peer->_emergencyBufferCount = 0;
	}
	ReleaseSRWLockExclusive(&peer->_emergencyBufferLock);

	WSABUF* wsaBuf = (WSABUF*)(PoolAlloc(sizeof(WSABUF) * count + 2));
	EmergencyBuffer* onFlightBuffer = peer->_sendOverlap._onFlightList;
	while (onFlightBuffer != NULL)
	{
		wsaBuf[idx].buf = onFlightBuffer->_buffer;
		wsaBuf[idx].len = onFlightBuffer->_size;
		onFlightBuffer = onFlightBuffer->_next;
		++idx;
	}

	LONG front = RingBuffer_GetFrontIndex(&peer->_sendBuffer);
	LONG rear = RingBuffer_GetRearIndex(&peer->_sendBuffer);
	directDequeueSize = RingBuffer_DirectDequeueSize(&peer->_sendBuffer, front, rear);
	remainSize = RingBuffer_GetCurrentSize(&peer->_sendBuffer, front, rear) - directDequeueSize;

	wsaBuf[idx].buf = peer->_sendBuffer._buffer + front;
	wsaBuf[idx].len = directDequeueSize;
	wsaBuf[idx + 1].buf = peer->_sendBuffer._buffer;
	wsaBuf[idx + 1].len = remainSize;

	retVal = WSASend(peer->_socket, wsaBuf, count + 2, &numOfBytes, &flags, (LPOVERLAPPED)(&peer->_sendOverlap), NULL);
	PoolFree(wsaBuf);

	if (retVal == SOCKET_ERROR)
	{
		errCode = WSAGetLastError();
		if (errCode != WSA_IO_PENDING)
		{
			JhNet_DisconnectPost(peer);

			Peer_DecRefCount(peer);
		}
	}
}

void JhNet_ConnectPost(Peer* peer)
{
	if ()
}

void JhNet_DisconnectPost(Peer* peer)
{
	if (InterlockedCompareExchangeNoFence(&peer->_isConnected, 0, 1) != 1)
	{
		return;
	}
	Peer_IncRefCount(peer);
	int errCode;
	int retVal;
	DWORD numOfBytes;
	DWORD flags = 0;

	retVal = JhNet_DisconnectEx(peer->_socket, &peer->_disconnectOverlap, flags, 0);
	if (retVal == SOCKET_ERROR)
	{
		errCode = WSAGetLastError();
		if (errCode != WSA_IO_PENDING)
		{
			closesocket(peer->_socket);
			Peer_DecRefCount(peer);
		}
	}
}

void JhNet_AcceptPost(JhNet* netManager)
{
	int retVal;
	int errCode;
	DWORD bytesReceived;
	SOCKET clientSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	AcceptOverlap* acceptOverlapped = PoolAlloc(sizeof(AcceptOverlap));
	OverlappedEx_Init(acceptOverlapped, IOTYPE_ACCEPT, clientSock);
	OverlappedEx_Clear((OverlappedEx*)acceptOverlapped);

	retVal = JhNet_AcceptEx(netManager->_socket, clientSock, acceptOverlapped->_addr, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &bytesReceived, (OVERLAPPED*)acceptOverlapped);
	if (retVal == FALSE)
	{
		errCode = WSAGetLastError();
		if (errCode != WSA_IO_PENDING)
		{
			printf("AcceptEx False! Error Code : %d\n", errCode);
			JhNet_AcceptPost(netManager);
		}
	}


}

void JhNet_RecvComplete(Peer* peer, DWORD completedBytes, BOOL isIoCanceled)
{
	if (isIoCanceled == TRUE || completedBytes == 0)
	{
		JhNet_DisconnectPost(peer);
	}
	if (InterlockedCompareExchangeNoFence(&peer->_isConnected, 0, 0) == 0)
	{
		Peer_DecRefCount(peer);
	}

	// 컨텐츠 서버의 콜백함수 호출
	// OnRecv();

	JhNet_RecvPost(peer);
}

void JhNet_SendComplete(Peer* peer, DWORD completedBytes, BOOL isIoCanceled)
{
	if (isIoCanceled == TRUE || completedBytes == 0)
	{
		JhNet_DisconnectPost(peer);
	}
	if (InterlockedCompareExchangeNoFence(&peer->_isConnected, 0, 0) == 0)
	{
		Peer_DecRefCount(peer);
	}
	RingBuffer_MoveFront(&peer->_sendBuffer, completedBytes);
	LONG front = RingBuffer_GetFrontIndex(&peer->_sendBuffer);
	LONG rear = RingBuffer_GetRearIndex(&peer->_sendBuffer);
	DWORD remainSize = RingBuffer_GetCurrentSize(&peer->_sendBuffer, front, rear);

	if (remainSize > 0 || peer->_sendOverlap._onFlightList != NULL)
	{
		JhNet_SendPost(peer);
		return;
	}
	
	InterlockedExchange(&peer->_onSend, 0);
	Peer_DecRefCount(peer);
}

void JhNet_ConnectComplete(Peer* peer)
{
}

void JhNet_DisconnectComplete(Peer* peer)
{
	closesocket(peer->_socket);
	// 컨텐츠 서버의 콜백함수 호출
	// OnDisconnect();
	Peer_DecRefCount(peer);
}

void JhNet_AcceptComplete()
{
}

void JhNet_DispatchIocp(HANDLE iocpHandle)
{
	DWORD errCode;
	BOOL retVal;
	DWORD completedBytes = 0;
	Peer* peer = NULL;
	OverlappedEx* ioEvent = NULL;

	retVal = GetQueuedCompletionStatus(iocpHandle, &completedBytes, (PULONG_PTR)&peer, (LPOVERLAPPED*)&ioEvent, INFINITE);
	if (retVal == FALSE)
	{
		errCode = GetLastError();
		if (ioEvent == NULL)
		{
			wprintf(L"Overlapped is NULL in GQCS return : %d\n", errCode);
			// TODO : 서버 종료
			return;
		}
		else
		{
			if (errCode != ERROR_OPERATION_ABORTED)
			{
				JhNet_DisconnectPost(peer);
				return;
			}

			switch (ioEvent->_ioType)
			{
			case IOTYPE_RECV:
				JhNet_RecvComplete(peer, completedBytes, TRUE);
				break;
			case IOTYPE_SEND:
				JhNet_SendComplete(peer, completedBytes, TRUE);
				break;
			case IOTYPE_ACCEPT:
				break;
			case IOTYPE_DISCONNECT:
				JhNet_DisconnectComplete(peer);
				break;
			case IOTYPE_CONNECT:
				JhNet_ConnectComplete(peer);
				break;
			default:
				break;
			}
		}
	}
	else
	{
		switch (ioEvent->_ioType)
		{
		case IOTYPE_RECV:
			JhNet_RecvComplete(peer, completedBytes, FALSE);
			break;
		case IOTYPE_SEND:
			JhNet_SendComplete(peer, completedBytes, FALSE);
			break;
		case IOTYPE_ACCEPT:
			break;
		case IOTYPE_DISCONNECT:
			JhNet_DisconnectComplete(peer);
			break;
		case IOTYPE_CONNECT:
			JhNet_ConnectComplete(peer);
			break;
		default:
			break;
		}
	}
}
