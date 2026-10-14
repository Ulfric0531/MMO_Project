#include "pch.h"



void Session_Init(Session* session, SOCKET socket, SOCKADDR_IN addr, DWORD64 id)
{
	session->_socket = socket;
	session->_addr = addr;
	session->_id = id;
	RingBuffer_Init(&(session->_recvBuffer), DEFAULT_RECVBUFF_SIZE);
	RingBuffer_Init(&(session->_sendBuffer), DEFAULT_SENDBUFF_SIZE);
}

void Session_Release(Session* session)
{
}

void Session_IncreaseRefCount(Session* session)
{
}

void Session_DecreaseRefCount(Session* session)
{
}

void Session_RecvPost(Session* session)
{
}

void Session_TrySendPost(Session* session, BYTE* buffer, DWORD size)
{
}

void Session_SendPost(Session* session)
{
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
