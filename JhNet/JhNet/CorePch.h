#pragma once

#define DEFAULT_RECVBUFF_SIZE (4000)
#define DEFAULT_SENDBUFF_SIZE (16000)

typedef enum
{
	IOTYPE_RECV,
	IOTYPE_SEND,
	IOTYPE_ACCEPT,
	IOTYPE_DISCONNECT,
	IOTYPE_CONNECT
} IoType;

#include "Allocator.h"
#include "SpinLock.h"
#include <stdlib.h>
#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <process.h>
#pragma comment(lib, "ws2_32.lib")

#include "OverlappedEx.h"
#include "RingBuffer.h"
#include "Peer.h"
#include "JhNet.h"
#include "MemoryPool.h"
#include "MemoryManager.h"
#include "Allocator.h"

#include <stdio.h>

typedef enum NetMode
{
	NETMODE_CLIENT,
	NETMODE_SERVER
} NetMode;

extern LPFN_ACCEPTEX		JhNet_AcceptEx;
extern LPFN_CONNECTEX		JhNet_ConnectEx;
extern LPFN_DISCONNECTEX	JhNet_DisconnectEx;


