#pragma once

#define MAX_PEER_COUNT (10000)

typedef struct OverlappedEx OverlappedEx;
typedef struct RingBuffer RingBuffer;
typedef struct Peer Peer;
typedef struct EmergencyBuffer EmergencyBuffer;
typedef enum NetMode NetMode;

typedef struct JhNet
{
	SOCKET			_socket;
	SOCKADDR_IN		_serverAddr;
	HANDLE			_mainIocp;
	HANDLE			_acceptDisconnectIocp;
	Peer*			_peerList[MAX_PEER_COUNT];
	SRWLOCK			_peerListLock;
	DWORD			_maxPeerCount;
	volatile LONG	_currentPeerCount;
	NetMode			_netMode;
} JhNet;

JhNet*		JhNet_MakeNetManager(WCHAR* serverIp, WORD serverPort, DWORD threadCount, NetMode netMode);
BOOL		JhNet_AddPeer(JhNet* netManager, Peer* peer);
BOOL		JhNet_DeletePeer(JhNet* netManager, DWORD peerIndex, DWORD64 peerId);
void		JhNet_InitWinSockEx();

void		JhNet_RecvPost(Peer* peer);
void		JhNet_TrySendPost(Peer* peer, BYTE* buffer, DWORD size);
void		JhNet_SendPost(Peer* peer);
BOOL		JhNet_ConnectPost(JhNet* netManager, ConnectOverlap* reservedOverlap);
void		JhNet_DisconnectPost(Peer* peer);
void		JhNet_AcceptPost(JhNet* netManager, AcceptOverlap* reservedOverlap);
void		JhNet_RecvComplete(Peer* peer, DWORD completedBytes, BOOL isIoCanceled);
void		JhNet_SendComplete(Peer* peer, DWORD completedBytes, BOOL isIoCanceled);
void		JhNet_ConnectComplete(JhNet* netManager, ConnectOverlap* overlap, BOOL isIoCanceled);
void		JhNet_DisconnectComplete(JhNet* netManager, Peer* peer, BOOL isIoCanceled);
void		JhNet_AcceptComplete(JhNet* netManager, AcceptOverlap* overlap, BOOL isIoCanceled);

void		JhNet_DispatchIocp(HANDLE iocpHandle, JhNet* netManager);