#ifdef _USE_AFX
	#include "stdafx.h"
	#include <afxwin.h>
#endif
#ifdef __BORLANDC__
    #include <vcl/SysInit.hpp>
#endif

#include <stdio.h>
#include "math.h"
#include "fp_cplug.h"
#include "fp_extra.h"
#include <string.h>

#ifdef __APPLE__
#include <stdlib.h>
#endif

char AppHint[256];
char PluginPath[MaxPath];     // path to this DLL plugin (for loading of resources)

#ifndef __BORLANDC__
HINSTANCE HInstance;
#endif



void GetPluginPath()
{
#ifndef __APPLE__
    char p[MaxPath];
    GetModuleFileNameA(HInstance, p, MaxPath);
    strcpy(PluginPath, p);
#endif
}



// 4 byte alignment
inline int AlignBlock_4(int Value)
{
	int n = 3;
	return (Value+3) & (~n);
}

// 16 byte alignment
inline int AlignBlock_16(int Value)
{
	int n = 15;
	return (Value+15) & (~n);
}

// 32 byte alignment
inline int AlignBlock_32(int Value)
{
	int n = 31;
	return (Value+31) & (~n);
}



// some constants for TAudioRenderer
const int Ramp_Res_Unit    = 1.0/1000.0;			// will take this number of samples to go from 0 to 1 at 44100Hz
const int Ramp_Res_Norm    = Ramp_Res_Unit;			// source is -1..1 normalized
const int Ramp_Res_Min     = Ramp_Res_Unit*10.0;	// source is -1..1 normalized, but quicker ramping, with minimum safety
const int Ramp_Res_FromInt = Ramp_Res_Unit*32768.0;	// source is integer



#define M_PI       3.14159265358979323846



// --- TAudioRenderer ---

TAudioRenderer::TAudioRenderer(TAudioRenderer *source)
{
	if (source)
		setSmpRate(source->getSmpRate());
	else
		setSmpRate(MixSmpRate_Default);
}

double TAudioRenderer::SamplesToMS(double Time)
{
	return Time*1000.0/fSmpRate;
}

double TAudioRenderer::MSToSamples(double Time)
{
	return Time*fSmpRate/1000.0;
}

int TAudioRenderer::getSmpRate()
{
	return fSmpRate;
}

void TAudioRenderer::setSmpRate(int newValue)
{
	fSmpRate = newValue;
	SmpRateScale = 44100.0/fSmpRate;

	// filters
	OmegaMul = M_PI*2.0/fSmpRate;
	FilterMaxFreq = fSmpRate*0.4999;

	// ramping
	Ramp_Res_Norm    = Ramp_Res_Unit*SmpRateScale;
	Ramp_Res_Min     = Ramp_Res_Norm*10;
	Ramp_Res_FromInt = Ramp_Res_Norm*32768;
}



// --- TCPPFruityPlug ---

// create the object
TCPPFruityPlug::TCPPFruityPlug(intptr_t SetHostTag, TFruityPlugHost *SetPlugHost, HINSTANCE instance)
    : TFruityPlug()
{
    HostTag = SetHostTag;
    PlugHost = SetPlugHost;

	HInstance = instance;	// call before GetPluginPath()!

    GetPluginPath();

#ifndef __APPLE__
    InitializeCriticalSection(&ThreadSafeCS);
#endif
    AudioRenderer.setSmpRate(MixSmpRate_Default);

	ProcessTempBuffer = NULL;
	MaxProcessLength = 0;

    #ifdef __BORLANDC__
    PopupParam = -1;
    #endif

#ifndef __APPLE__
	char *path = (char *)(PlugHost->Dispatcher(HostTag,FHD_GetProgPath,0,0));
	strcpy_s(ProgPath, MAX_PATH, path);
#endif
}


void _stdcall TCPPFruityPlug::DestroyObject()
{
#ifndef __APPLE__
    DeleteCriticalSection(&ThreadSafeCS);
#endif
	if (ProcessTempBuffer)
		free(ProcessTempBuffer);

	// inherited function (deletes this object!)
	TFruityPlug::DestroyObject();
}

// returns true if freshly reallocated
bool TCPPFruityPlug::AllocTempBuffer(int Length, int NumBuffers)
{
	if (Length>MaxProcessLength)
	{
		MaxProcessLength = AlignBlock_4(Length);
		ProcessTempBuffer = (PWAV32FM)realloc(ProcessTempBuffer, MaxProcessLength*8*NumBuffers);
		return true;
	}
    else
		return false;
}

void TCPPFruityPlug::SetInstanceHandle(HINSTANCE instance)
{
	HInstance = instance;
}


void TCPPFruityPlug::ProcessAllParams()
{
    for (int n = 0; n < Info->NumParams; n++)
        ProcessParam(n, ProcessParam(n, 0, REC_GetValue), REC_UpdateValue | REC_UpdateControl);
}


// to safely skip a rendering (for plugins that don't render)
void TCPPFruityPlug::SkipRendering(void *SourceBuffer, void *DestBuffer, int Length)
{
    if (SourceBuffer != DestBuffer)
        memmove(DestBuffer, SourceBuffer, Length << 3);
}


// get the speed step to go through the LFO table at a given pitch (cents) (tuned around C5)
int TCPPFruityPlug::GetStep_Cents(int Pitch)
{
    return PitchMul * exp(NoteMul*Pitch);
}

float TCPPFruityPlug::GetStep_Cents(float Pitch)
{
    return PitchMul * exp(NoteMul*Pitch);
}

int TCPPFruityPlug::GetStep_Freq(int Freq)
{
    int x = this->AudioRenderer.getSmpRate();
	return Freq/x;
}


// thread synchronization for safety
void TCPPFruityPlug::Lock()
{
#ifndef __APPLE__
    EnterCriticalSection(&ThreadSafeCS);
#endif
}

void TCPPFruityPlug::Unlock()
{
#ifndef __APPLE__
    LeaveCriticalSection(&ThreadSafeCS);
#endif
}


// hints
void TCPPFruityPlug::ShowHintMsg(char *Msg)
{
    PlugHost->OnHint(HostTag, Msg);
}

void TCPPFruityPlug::ShowHintMsg_Direct(char *Msg)
{
    PlugHost->Dispatcher(HostTag, FHD_OnHint_Direct, 0, (int)Msg);
}


void TCPPFruityPlug::ShowHintMsg_Percent(int Value, int Max)
{
    char tempc[256];
    sprintf(tempc, "%d%", Value*100 / Max);
    PlugHost->OnHint(HostTag, tempc);
}

//const PitchTypeT:Array[-1..2] of String=(' octave',' semitone',' cent',' Hz');

void TCPPFruityPlug::ShowHintMsg_Pitch(int Value, int PitchType)
{
    char *PitchTypeT[4];
    PitchTypeT[0] = "octave";
    PitchTypeT[1] = "semitone";
    PitchTypeT[2] = "cent";
    PitchTypeT[3] = "Hz";

    char Msg[256];
    char M2[256];
    sprintf(M2, "%d %s", Value, PitchTypeT[PitchType+1]);   // PitchType = -1..2

    if (Value == 0 || Value == 1)
        sprintf(Msg, "+%s", M2);
    else if (Value > 1)
        sprintf(Msg, "+%ss", M2);
    else if (abs(Value) > 1)
        sprintf(Msg, "%ss", M2);
    else
        strcpy(Msg, M2);

    PlugHost->OnHint(HostTag, Msg);
}


void TCPPFruityPlug::ShowHintMsg_Pitch(float Value, int PitchType, int Digits)
{
    char *PitchTypeT[4];
    PitchTypeT[0] = "octave";
    PitchTypeT[1] = "semitone";
    PitchTypeT[2] = "cent";
    PitchTypeT[3] = "Hz";

    char Msg[256];
    char M2[256];

    sprintf(M2, "%%%d.f %%s", Digits);  // create the format string
    sprintf(M2, M2, Value, PitchTypeT[PitchType+1]);

    if (fabs(Value) > 1 && PitchType < 2)
        sprintf(Msg, "+%ss");
    else if (Value >= 0)
        sprintf(Msg, "+%s");

    PlugHost->OnHint(HostTag, Msg);
}


void TCPPFruityPlug::ShowHintMsg_Pan(int Value)
{
    char Msg[128];
    int n;

    n = Value * 100 / 64;
    if (n == 0)
        strcpy(Msg, "Centered");
    else
    {
        sprintf(Msg, "%d%% ", abs(n));
        if (n < 0)
            strcpy(Msg, strcat(Msg, "left"));
        else
            strcpy(Msg, strcat(Msg, "right"));
    }

    PlugHost->OnHint(HostTag, Msg);
}

// show the time in 0:00 format at 192 PPQ
void TCPPFruityPlug::ShowHintMsg_Time(int Value)
{
    char Msg[128];

    sprintf(Msg, "%d:%.2d", Value / AbsPPN, Value % AbsPPN);
    PlugHost->OnHint(HostTag, Msg);
}

void TCPPFruityPlug::CheckItem(HMENU Menu, int Item, int ParamNum, int Index)
{
    int funcresult = PlugHost->Dispatcher(HostTag, FHD_GetParamMenuFlags, ParamNum, Index);

    unsigned int check = 0;
#ifndef __APPLE__
    if ((funcresult & FHP_Checked) != 0)  check = MF_CHECKED;
    else  check = MF_UNCHECKED;
    CheckMenuItem(Menu, Item, MF_BYPOSITION | check);

    if ((funcresult & FHP_Disabled) == 0)  check = MF_ENABLED;
    else  check = MF_DISABLED;
    EnableMenuItem(Menu, Item, MF_BYPOSITION | check);
#endif
}


// adjust a common param popup (VCL version)
#ifdef __BORLANDC__
void TCPPFruityPlug::AdjustParamPopup(TMenuItem *Item, int ParamNum, int FirstItemIndex)
{
	if (!Item)
        return;

    // delete the old entries
    int count = Item->Count;
    while (count > FirstItemIndex)  {
        Item->Delete(count-1);
        count--;
    }

    // add (append) new ones
    int n = 0;
    unsigned int flags;
    unsigned int id;
    PParamMenuEntry MenuEntry = 0;
    TMenuItem *NewItem;

    PopupParam = ParamNum;    

    do {
        // get menu entry
        MenuEntry = (PParamMenuEntry)(PlugHost->Dispatcher(HostTag, FHD_GetParamMenuEntry, ParamNum, n));
        if (MenuEntry) {
            // create, fill & add item
            NewItem = new TMenuItem(Item);
            NewItem->Caption = MenuEntry->Name;
            NewItem->Tag = n;
            NewItem->Checked = (MenuEntry->Flags & FHP_Checked);
            NewItem->Enabled = !(MenuEntry->Flags & FHP_Disabled);
            NewItem->OnClick = MenuClick;
            Item->Add(NewItem);
            n++;
        }
    }
    while (MenuEntry);
}
#endif

// adjust a common param popup (Windows API version)
void TCPPFruityPlug::AdjustParamPopup(HMENU Item, int ParamNum, int FirstItemIndex, int FirstID)

{
#ifndef __APPLE__
    if (!Item)
        return;

    // delete the old entries
    int count = GetMenuItemCount(Item);
    while (count > FirstItemIndex)  {
        DeleteMenu(Item, count-1, MF_BYPOSITION);
        count--;
    }

    // add (append) new ones
    int n = 0;
    unsigned int flags;
    unsigned int id;
    PParamMenuEntry MenuEntry = 0;

    do {
        // get menu entry
        MenuEntry = (PParamMenuEntry)(PlugHost->Dispatcher(HostTag, FHD_GetParamMenuEntry, ParamNum, n));
        if (MenuEntry) {
            // create, fill & add item
            flags = MF_STRING;
            if (MenuEntry->Flags & FHP_Disabled)
                flags = flags | MF_GRAYED;
            else
                flags = flags | MF_ENABLED;
            if (MenuEntry->Flags & FHP_Checked)
                flags = flags | MF_CHECKED;
            else
                flags = flags | MF_UNCHECKED;
            if (strcmp(MenuEntry->Name, "-") == 0)
                flags = flags | MF_SEPARATOR;

            id = FirstID + n;
            AppendMenuA(Item, flags, id, MenuEntry->Name);
            n++;
        }
    }
    while (MenuEntry);
#endif
}



void _stdcall TCPPFruityPlug::NewTick()
{
}

intptr_t _stdcall TCPPFruityPlug::Dispatcher(intptr_t ID, intptr_t Index, intptr_t Value)
{
    return 0;
}

void _stdcall TCPPFruityPlug::Eff_Render(PWAV32FS SourceBuffer, PWAV32FS DestBuffer, int Length)
{
}

void _stdcall TCPPFruityPlug::Gen_Render(PWAV32FS DestBuffer, int &Length)
{
}

void _stdcall TCPPFruityPlug::GetName(int Section, int Index, int Value, char *Name)
{
}

void _stdcall TCPPFruityPlug::Idle_Public()
{
}

int _stdcall TCPPFruityPlug::ProcessParam(int Index, int Value, int RECFlags)
{
    return Value;
}

void _stdcall TCPPFruityPlug::SaveRestoreState(IStream *Stream, BOOL Save)
{
}

TVoiceHandle _stdcall TCPPFruityPlug::TriggerVoice(PVoiceParams VoiceParams, intptr_t SetTag)
{
    return 0;
}

void _stdcall TCPPFruityPlug::Voice_Kill(TVoiceHandle Handle)
{
}

int _stdcall TCPPFruityPlug::Voice_ProcessEvent(TVoiceHandle Handle, int EventID, int EventValue, int Flags)
{
    return 0;
}

void _stdcall TCPPFruityPlug::Voice_Release(TVoiceHandle Handle)
{
}

int _stdcall TCPPFruityPlug::Voice_Render(TVoiceHandle Handle, PWAV32FS DestBuffer, int &Length)
{
    return 0;
}

int _stdcall TCPPFruityPlug::ProcessEvent(int EventID, int EventValue, int Flags)
{
    return 0;
}

void _stdcall TCPPFruityPlug::MIDITick()
{
}

void _stdcall TCPPFruityPlug::MIDIIn(int &Msg)
{
}

#ifdef __BORLANDC__
void __fastcall TCPPFruityPlug::MenuClick(TObject *Sender)
{
    if (PopupParam >= 0 && Sender) {
        TMenuItem *item = (TMenuItem *)Sender;
        PlugHost->Dispatcher(HostTag, FHD_ParamMenu, PopupParam, item->Tag);
        PopupParam = -1;        
    }
}
#endif

void _stdcall TCPPFruityPlug::MsgIn(intptr_t Msg)
{
}

// translate a controller value (0..65536)
int TranslateMidi(int Value, int Min, int Max)
{
	int result = Min + (Value * 1.0f / (Max-Min+1) * (Max-Min+1));
	if (result > Max)
		result = Max;
	return result;
}

const int HintPBMax   = 20;  // 20 steps for the hint progress bar
const int HintPBFirst = 'a';
const int HintPBLast  = HintPBFirst+HintPBMax;

void TCPPFruityPlug::ShowHintMsg_Gauge(const char* Msg , int Value, int Max)
{
	// limit value to 0..Max
	if (Value < 0)		Value = 0;
	if (Value > Max)	Value = Max;

	if (Max < 1)
		Max = 1;

	char s[1024];

	int v = HintPBFirst;
	v = v + (Value * (HintPBMax+1) / Max);
	if (v > HintPBLast)
		v = HintPBLast;
#ifdef __APPLE__
	snprintf(s, 1024, "^.%d%s", v, Msg);
#else
	sprintf_s(s, 1024, "^.%d%s", v, Msg);
#endif
	ShowHintMsg_Direct(s);
}

void TCPPFruityPlug::Idle()
{
}