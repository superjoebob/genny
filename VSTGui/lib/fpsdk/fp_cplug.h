/*

FL Studio generator/effect plugins SDK
C++ object extension
handy functions for easier implementation of a Fruity plugin in C++

(99-15) gol, reflex

*/

#pragma once

#include "fp_plugclass.h"
#include "fp_extra.h"

#ifdef __BORLANDC__
    #include <vcl.h>
#endif

#ifndef __BORLANDC__
extern HINSTANCE HInstance;
#endif


#define DefaultMenuID 1024



// audio renderer, typically used by a thread
class TAudioRenderer
{
private:
	int fSmpRate;
public:
	// filters
	double OmegaMul;
	float SmpRateScale, FilterMaxFreq;
          
	// ramping
	float Ramp_Res_Norm;     // source is -1..1 normalized
	float Ramp_Res_Min;      // source is -1..1 normalized, but quicker ramping, with minimum safety
	float Ramp_Res_FromInt;  // source is integer

	TAudioRenderer(TAudioRenderer *source = NULL);
	
	inline double SamplesToMS(double Time);
	inline double MSToSamples(double Time);

	int getSmpRate();
	void setSmpRate(int newValue);
};



class TCPPFruityPlug : public TFruityPlug {
private:
    #ifdef __BORLANDC__
    int PopupParam;
    void __fastcall MenuClick(TObject *Sender);
    #endif
    void CheckItem(HMENU Menu, int Item, int ParamNum, int Index);

protected:
	void SetInstanceHandle(HINSTANCE instance);

public:
	// some audio properties
    TAudioRenderer AudioRenderer;
	
	// temp buffer
	PWAV32FM ProcessTempBuffer;
	int MaxProcessLength;

    float PitchMul;
    TFruityPlugHost *PlugHost;
	int MaxPoly;
	char ProgPath[MAX_PATH];

    RTL_CRITICAL_SECTION ThreadSafeCS;

    // messages (to the plugin)
    virtual void _stdcall DestroyObject();
    virtual intptr_t _stdcall Dispatcher(intptr_t ID, intptr_t Index, intptr_t Value);
    virtual void _stdcall Idle_Public();
    virtual void _stdcall SaveRestoreState(IStream *Stream, BOOL Save);

    // names (see FPN_Param) (Name must be at least 256 chars long)
    virtual void _stdcall GetName(int Section, int Index, int Value, char *Name);

    // events
    virtual int _stdcall ProcessEvent(int EventID, int EventValue, int Flags);
    virtual int _stdcall ProcessParam(int Index, int Value, int RECFlags);

    // effect processing (source & dest can be the same)
    virtual void _stdcall Eff_Render(PWAV32FS SourceBuffer, PWAV32FS DestBuffer, int Length);
    // generator processing (can render less than length)
    virtual void _stdcall Gen_Render(PWAV32FS DestBuffer, int &Length);

    // voice handling
    virtual TVoiceHandle _stdcall TriggerVoice(PVoiceParams VoiceParams, intptr_t SetTag);
    virtual void _stdcall Voice_Release(TVoiceHandle Handle);
    virtual void _stdcall Voice_Kill(TVoiceHandle Handle);
    virtual int _stdcall Voice_ProcessEvent(TVoiceHandle Handle, intptr_t EventID, intptr_t EventValue, intptr_t Flags);
    // FPF_UseSampler only
    virtual int _stdcall Voice_Render(TVoiceHandle Handle, PWAV32FS DestBuffer, int &Length);

    // FPF_EventController
    virtual void _stdcall NewTick();

    virtual void _stdcall MIDITick();

	virtual void _stdcall MIDIIn(int &Msg);

	// buffered messages to itself (see PlugMsg_Delayed)
    virtual void _stdcall MsgIn(intptr_t Msg);
	virtual int _stdcall OutputVoice_ProcessEvent(TOutVoiceHandle Handle, intptr_t EventID, intptr_t EventValue, intptr_t Flags) { return 0; };
	virtual void _stdcall OutputVoice_Kill(TVoiceHandle Handle) {};


    // internal
    int GetStep_Cents(int Pitch);
	float GetStep_Cents(float Pitch);
    int GetStep_Freq(int Freq);
    void ProcessAllParams();
    void SkipRendering(void *SourceBuffer, void *DestBuffer, int Length);
    TCPPFruityPlug(intptr_t SetHostTag, TFruityPlugHost *SetPlugHost, HINSTANCE instance);
	bool AllocTempBuffer(int Length, int NumBuffers);


	// replaces PlugHost::LockMix_Shared
	inline void LockMix_Shared();
	inline void UnlockMix_Shared();


    // hints
    void ShowHintMsg(char *Msg);
    void ShowHintMsg_Direct(char *Msg);
    void ShowHintMsg_Percent(int Value, int Max);
    void ShowHintMsg_Pitch(int Value, int PitchType);
    void ShowHintMsg_Pitch(float Value, int PitchType, int Digits);
    void ShowHintMsg_Pan(int Value);
    void ShowHintMsg_Time(int Value);
	void ShowHintMsg_Gauge(const char* Msg , int Value, int Max);
    
    // param popup
    #ifdef __BORLANDC__
    void AdjustParamPopup(TMenuItem *Item, int ParamNum, int FirstItemIndex);
    #endif
    void AdjustParamPopup(HMENU Item, int ParamNum, int FirstItemIndex, int FirstID);
    
    // thread synchronisation / safety
    void Lock();
    void Unlock();

	virtual void Idle();
};



extern char AppHint[256];
extern char PluginPath[MaxPath];   // path to this DLL plugin (for loading of resources)

const int AbsPPN = 192 / 4;  // 192 PPQ

// see ShowHintMsg_Pitch
const int PitchType_Octaves   = -1;
const int PitchType_Semitones = 0;
const int PitchType_Cents     = 1;
const int PitchType_Hz        = 2;



void GetPluginPath();

int TranslateMidi(int, int, int);







// implementation of inline member functions

void TCPPFruityPlug::LockMix_Shared()
{
	TIOBuffer OBuffer;
	OBuffer.Flags = IO_Lock;
	PlugHost->GetOutBuffer(HostTag, 0, &OBuffer);
}

void TCPPFruityPlug::UnlockMix_Shared()
{
	TIOBuffer OBuffer;
	OBuffer.Flags = IO_Unlock;
	PlugHost->GetOutBuffer(HostTag, 0, &OBuffer);
}
