#pragma once

#define DEFAULT_RECVBUFF_SIZE (4000)
#define DEFAULT_SENDBUFF_SIZE (16000)

typedef enum
{
	IOTYPE_RECV,
	IOTYPE_SEND,
	IOTYPE_ACCEPT,
	IOTYPE_DISCONNECT
} IoType;

#include "Allocator.h"

#include <stdlib.h>
#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <process.h>
#pragma comment(lib, "ws2_32.lib")

#include "OverlappedEx.h"
#include "Iocp.h"
#include "RingBuffer.h"
#include "Session.h"

#include <stdio.h>

/*
extern LPFN_ACCEPTEX		AcceptEx;
extern LPFN_CONNECTEX		ConnectEx;
extern LPFN_DISCONNECTEX	DisconnectEx;
*/

