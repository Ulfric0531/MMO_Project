#pragma once

typedef struct EmergencyBuffer EmergencyBuffer;
typedef struct Peer Peer;
typedef enum IoType IoType;

typedef struct OverlappedEx
{
	OVERLAPPED			_overlapped;
	IoType				_ioType;
} OverlappedEx;

typedef struct RecvOverlap
{
	OverlappedEx _overlappedEx;
} RecvOverlap;

typedef struct SendOverlap
{
	OverlappedEx		_overlappedEx;
	EmergencyBuffer*	_onFlightList;
} SendOverlap;

typedef struct DisconnectOverlap
{
	OverlappedEx _overlappedEx;
} DisconnectOverlap;

typedef struct AcceptOverlap
{
	OverlappedEx	_overlappedEx;
	BYTE			_addr[(sizeof(SOCKADDR_IN) + 16) * 2];
	SOCKET			_clientSock;
	Peer*			_peer;
} AcceptOverlap;

typedef struct ConnectOverlap
{
	OverlappedEx	_overlappedEx;
	const SOCKADDR	_serverAddr;
	SOCKET			_clientSock;
} ConnectOverlap;

void		OverlappedEx_Init(OverlappedEx* out_OverlappedEx, IoType ioType, SOCKET socket, Peer* peer);
void		OverlappedEx_Release(OverlappedEx* out_OverlappedEx);
void		OverlappedEx_Clear(OverlappedEx* out_OverlappedEx);
