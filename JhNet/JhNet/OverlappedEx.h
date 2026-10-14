#pragma once
//#include "Enum.h"

typedef struct EmergencyBuffer EmergencyBuffer;
typedef enum IoType IoType;

typedef struct OverlappedEx
{
	IoType				_ioType;
	EmergencyBuffer*	_onFlightList;
} OverlappedEx;


void		OverlappedEx_Init(OverlappedEx* out_OverlappedEx, IoType ioType);
void		OverlappedEx_Release(OverlappedEx* out_OverlappedEx);
void		OverlappedEx_Clear(OverlappedEx* out_OverlappedEx);
