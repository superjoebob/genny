#ifndef FP_DEF_H
#define FP_DEF_H

#include "math.h"
#ifndef __APPLE__
#include "mmsystem.h"
#endif

#define WaveT_Bits 14                  // 14 bits for the length of the wavetable
#define WaveT_Size (1 << WaveT_Bits)   // length of the wavetable
#define WaveT_Shift (32-WaveT_Bits)    // shift for full DWORD conversion
#define WaveT_Step 1 << WaveT_Shift    // speed for 1 sample in the wavetable
#define WaveT_PMask (0xFFFFFFFF >> WaveT_Shift)  // mask to limit the position to the range of the wavetable
#define WaveT_FMask (0xFFFFFFFF >> WaveT_Bits)   // mask to get the frac part of the position



#define MIDIMsg_PortMask 0xFFFFFF;
#define MIDIMsg_Null     0xFFFFFFFF;

// see REC_FromMIDI
const int FromMIDI_Max = 1 << 30;
const int FromMIDI_Half = FromMIDI_Max >> 1;
const float FromMIDI_Div = 1.0f / FromMIDI_Max;



// published wavetables
typedef float TWaveT[WaveT_Size];
typedef TWaveT *PWaveT;

// interlaced stereo 32Bit float buffer
typedef float TWAV32FS[2];
typedef TWAV32FS *PWAV32FS;
typedef float TWAV32FM;
typedef TWAV32FM *PWAV32FM;


// MIDI out message structure (3 bytes standard MIDI message + port)
typedef struct 
{
    unsigned char Status;
    unsigned char Data1;
    unsigned char Data2;
    unsigned char Port;
} TMIDIOutMsg, *PMIDIOutMsg;

// extended wav format
typedef struct
{
#ifndef __APPLE__
    WAVEFORMATEX WaveFormatEx;
#endif
    union
    {
        struct
        {
            unsigned short wValidBitsPerSample;   // bits of precision
            unsigned long dwChannelMask;          // which channels are present in stream
#ifndef __APPLE__
            GUID SubFormat;
#endif
        } stream;
        unsigned short wSamplesPerBlock;          // valid if wBitsPerSample==0
        unsigned short wReserved;                 // if neither applies, set to zero
    };
} TWaveFormatExtensible, *PWaveFormatExtensible;


// Bar:Step:Tick
typedef struct
{
    int Bar;
    int Step;
    int Tick;
} TSongTime, *PSongTime;

// time sig info (easily converted to standard x/x time sig, but more powerful)
typedef struct
{
    int StepsPerBar;
    int StepsPerBeat;
    int PPQ;
} TTimeSigInfo, *PTimeSigInfo;


#endif   // FP_DEF_H