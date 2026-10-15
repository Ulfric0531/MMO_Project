#include "pch.h"

void RingBuffer_Init(RingBuffer* ringBuffer, DWORD bufferSize)
{
	ringBuffer->_front = 0;
	ringBuffer->_rear = 0;
	ringBuffer->_maxBufferSize = bufferSize + 1;
	ringBuffer->_buffer = (BYTE*)malloc(bufferSize + 1); // TODO
}

void RingBuffer_Release(RingBuffer* ringBuffer)
{
	// TODO
}

void RingBuffer_ClearBuffer(RingBuffer* ringBuffer)
{
	ringBuffer->_front = 0;
	ringBuffer->_rear = 0;
}

DWORD RingBuffer_GetCurrentSize(RingBuffer* ringBuffer, LONG front, LONG rear)
{
	DWORD maxBufferSize = ringBuffer->_maxBufferSize;

	if (front <= rear)
	{
		return rear - front;
	}
	return maxBufferSize - front + rear;
}

DWORD RingBuffer_GetFreeSize(RingBuffer* ringBuffer, LONG front, LONG rear)
{
	DWORD maxBufferSize = ringBuffer->_maxBufferSize;

	if (front <= rear)
	{
		return (maxBufferSize - 1) - (rear - front);
	}
	return front - rear - 1;
}

BOOL RingBuffer_Enqueue(RingBuffer* ringBuffer, BYTE* elementSrc, DWORD elementSize)
{
	LONG front = InterlockedCompareExchange(&(ringBuffer->_front), 0, 0);
	LONG rear = InterlockedCompareExchange(&(ringBuffer->_rear), 0, 0);
	DWORD maxBufferSize = ringBuffer->_maxBufferSize;
	BYTE* buffer = ringBuffer->_buffer;

	if (RingBuffer_GetFreeSize(ringBuffer, front, rear) < elementSize)
	{
		return FALSE;
	}

	if (RingBuffer_DirectEnqueueSize(ringBuffer, front, rear) >= elementSize)
	{
		memcpy(buffer + rear, elementSrc, elementSize);
		InterlockedExchange(&(ringBuffer->_rear), (rear + elementSize) % maxBufferSize);
		return TRUE;
	}

	DWORD directEnqueueSize = RingBuffer_DirectEnqueueSize(ringBuffer, front, rear);
	memcpy(buffer + rear, elementSrc, directEnqueueSize);
	memcpy(buffer, elementSrc + directEnqueueSize, elementSize - directEnqueueSize);
	InterlockedExchange(&(ringBuffer->_rear), (rear + elementSize) % maxBufferSize);
	return TRUE;
}

BOOL RingBuffer_Dequeue(RingBuffer* ringBuffer, BYTE* elementDst, DWORD elementSize)
{
	LONG front = InterlockedCompareExchange(&(ringBuffer->_front), 0, 0);
	LONG rear = InterlockedCompareExchange(&(ringBuffer->_rear), 0, 0);
	DWORD maxBufferSize = ringBuffer->_maxBufferSize;
	BYTE* buffer = ringBuffer->_buffer;

	if (elementSize > RingBuffer_GetCurrentSize(ringBuffer, front, rear))
	{
		return FALSE;
	}

	if (RingBuffer_DirectDequeueSize(ringBuffer, front, rear) >= elementSize)
	{
		memcpy(elementDst, buffer + front, elementSize);
		InterlockedExchange(&(ringBuffer->_front), (front + elementSize) % maxBufferSize);
		return TRUE;
	}

	DWORD directDequeueSize = RingBuffer_DirectDequeueSize(ringBuffer, front, rear);
	memcpy(elementDst, buffer + front, directDequeueSize);
	memcpy(elementDst + directDequeueSize, buffer, elementSize - directDequeueSize);
	InterlockedExchange(&(ringBuffer->_front), (front + elementSize) % maxBufferSize);
	return TRUE;
}

BOOL RingBuffer_Peek(RingBuffer* ringBuffer, BYTE* elementDst, DWORD elementSize)
{
	LONG front = InterlockedCompareExchange(&(ringBuffer->_front), 0, 0);
	LONG rear = InterlockedCompareExchange(&(ringBuffer->_rear), 0, 0);
	DWORD maxBufferSize = ringBuffer->_maxBufferSize;
	BYTE* buffer = ringBuffer->_buffer;

	if (elementSize > RingBuffer_GetCurrentSize(ringBuffer, front, rear))
	{
		return FALSE;
	}

	if (RingBuffer_DirectDequeueSize(ringBuffer, front, rear) >= elementSize)
	{
		memcpy(elementDst, buffer + front, elementSize);
		return TRUE;
	}

	DWORD directDequeueSize = RingBuffer_DirectDequeueSize(ringBuffer, front, rear);

	memcpy(elementDst, buffer + front, directDequeueSize);
	front = 0;
	memcpy(elementDst + directDequeueSize, buffer + front, elementSize - directDequeueSize);
	return TRUE;
}

BOOL RingBuffer_MoveFront(RingBuffer* ringBuffer, DWORD elementSize)
{
	LONG front = InterlockedCompareExchange(&(ringBuffer->_front), 0, 0);
	LONG rear = InterlockedCompareExchange(&(ringBuffer->_rear), 0, 0);
	DWORD maxBufferSize = ringBuffer->_maxBufferSize;

	if (RingBuffer_GetCurrentSize(ringBuffer, front, rear) < elementSize)
	{
		return FALSE;
	}
	InterlockedExchange(&(ringBuffer->_front), (front + elementSize) % maxBufferSize);

	return TRUE;
}

BOOL RingBuffer_MoveRear(RingBuffer* ringBuffer, DWORD elementSize)
{
	LONG front = InterlockedCompareExchange(&(ringBuffer->_front), 0, 0);
	LONG rear = InterlockedCompareExchange(&(ringBuffer->_rear), 0, 0);
	DWORD maxBufferSize = ringBuffer->_maxBufferSize;

	if (RingBuffer_GetFreeSize(ringBuffer, front, rear) < elementSize)
	{
		return FALSE;
	}

	InterlockedExchange(&(ringBuffer->_rear), (rear + elementSize) % maxBufferSize);

	return TRUE;
}

DWORD RingBuffer_DirectEnqueueSize(RingBuffer* ringBuffer, LONG front, LONG rear)
{
	DWORD maxBufferSize = ringBuffer->_maxBufferSize;

	if (front <= rear)
	{
		return maxBufferSize - rear;
	}
	
	return front - rear - 1;
}

DWORD RingBuffer_DirectDequeueSize(RingBuffer* ringBuffer, LONG front, LONG rear)
{
	DWORD maxBufferSize = ringBuffer->_maxBufferSize;

	if (front <= rear)
	{
		return RingBuffer_GetCurrentSize(ringBuffer, front, rear);
	}

	return maxBufferSize - front;
}

LONG RingBuffer_GetFrontIndex(RingBuffer* ringBuffer)
{
	return InterlockedCompareExchange(&(ringBuffer->_front), 0, 0);
}

LONG RingBuffer_GetRearIndex(RingBuffer* ringBuffer)
{
	return InterlockedCompareExchange(&(ringBuffer->_rear), 0, 0);
}
