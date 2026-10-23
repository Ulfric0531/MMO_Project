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
	OverlappedEx	_acceptOverlapped;
	OverlappedEx	_connectOverlapped;
	HANDLE			_mainIocp;
	HANDLE			_acceptDisconnectIocp;
	Peer*			_peerList[MAX_PEER_COUNT];
	SRWLOCK			_peerListLock;
	DWORD			_maxPeerCount;
	DWORD			_currentPeerCount;
	NetMode			_netMode;
} JhNet;

JhNet*		JhNet_MakeNetManager(WCHAR* serverIp, WORD serverPort, DWORD threadCount, NetMode netMode);
DWORD64		JhNet_AddPeer(JhNet* netManager, Peer* peer);
void		JhNet_DeletePeer(JhNet* netManager, DWORD peerIndex, DWORD64 peerId);
void		JhNet_InitWinSockEx();
void		JhNet_RecvPost(Peer* peer);
void		JhNet_TrySendPost(Peer* peer, BYTE* buffer, DWORD size);
void		JhNet_SendPost(Peer* peer);
void		JhNet_ConnectPost(Peer* peer);
void		JhNet_DisconnectPost(Peer* peer);
void		JhNet_RecvComplete(Peer* peer, DWORD completedBytes, BOOL isIoCanceled);
void		JhNet_SendComplete(Peer* peer, DWORD completedBytes, BOOL isIoCanceled);
void		JhNet_ConnectComplete(Peer* peer);
void		JhNet_DisconnectComplete(Peer* peer);
void		JhNet_DispatchIocp(HANDLE iocpHandle);