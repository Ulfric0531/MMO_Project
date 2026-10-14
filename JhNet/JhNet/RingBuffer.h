#pragma once

typedef struct RingBuffer
{
	BYTE*			_buffer;
	volatile LONG	_front;
	volatile LONG	_rear;
	DWORD			_maxBufferSize;
} RingBuffer;

void	RingBuffer_Init(RingBuffer* ringBuffer, DWORD bufferSize);
void	RingBuffer_Release(RingBuffer* ringBuffer);

void	RingBuffer_ClearBuffer(RingBuffer* ringBuffer);
DWORD	RingBuffer_GetCurrentSize(RingBuffer* ringBuffer, LONG front, LONG rear);
DWORD	RingBuffer_GetFreeSize(RingBuffer* ringBuffer, LONG front, LONG rear);
BOOL	RingBuffer_Enqueue(RingBuffer* ringBuffer, BYTE* elementSrc, DWORD elementSize);
BOOL	RingBuffer_Dequeue(RingBuffer* ringBuffer, BYTE* elementDst, DWORD elementSize);
BOOL	RingBuffer_Peek(RingBuffer* ringBuffer, BYTE* elementDst, DWORD elementSize);
BOOL	RingBuffer_MoveFront(RingBuffer* ringBuffer, DWORD elementSize);
BOOL	RingBuffer_MoveRear(RingBuffer* ringBuffer, DWORD elementSize);
DWORD	RingBuffer_DirectEnqueueSize(RingBuffer* ringBuffer, LONG front, LONG rear);
DWORD	RingBuffer_DirectDequeueSize(RingBuffer* ringBuffer, LONG front, LONG rear);