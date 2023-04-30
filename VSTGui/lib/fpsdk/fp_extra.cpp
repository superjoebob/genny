#ifdef _USE_AFX
//#include "stdafx.h"
#endif
#include "fp_extra.h"
#include <math.h>

int MinOf(int a, int b)
{
    if (a < b)
        return a;
    return b;
}

/*
// mul & shift by 16
int MulShift16(int a, int b)
{
    int result;
    _asm {
		MOV   EDX,b
		MOV   EAX,a
        IMUL  EDX
        SHRD  EAX,EDX,16
        MOV   result,EAX
    }
    return result;
}
*/


float InvLogVol(float Value, float MaxValue)
{
	return logf(Value+1)/logf(MaxValue+1);
}

float VolumeToVelocity(float Volume)
{
	return InvLogVol(Volume*10, 261*10/127);
}

float VolumeToMIDIVelocity(float Volume)
{
	return InvLogVol(Volume*10, 261*10/127) * 127;
}