/*

FL Studio generator/effect plugins SDK
plugin & host classes

(99-15) gol


!!! Warnings:

-when multithreadable, a generator (not effect) adding to the output buffer, or a generator/effect adding to the send buffers, must lock the access in-between LockMix_Shared / UnlockMix_Shared




history:

(10/05/00)
- rewrote almost everything, a lot of changes & splits

(15/07/00)
- changed TVoiceParams & the way the levels are handled (Voice_Render)
- added FPF_TimeWarp

(26/07/00)
- added plugin flags combo's
- removed AlphaTable from TFruityPlugHost

(27/07/00)
- DefPoly now used

(26/09/00)
- TFruityPlugHost.MIDIOut declaration has changed
- implemented FPF_MIDITick

(27/09/00)
- added TFruityPlugHost.MIDIOut_Delayed & FPF_MIDIOut, reorganized TFruityPlugHost
- defined UseCriticalSection in FP_DelphiPlug
- updated FPE_Tempo
- added FPE_MaxPoly & FPD_KillAVoice

(11/10/00)
- added FPE_MIDI_Pan & FPE_MIDI_Vol

(01/11/00)
- updated TFruityPlugHost.Voice_Kill(Sender:Integer;KillHandle:LongBool);
  (KillHandle forces Fruity to ask back the plugin to destroy its voice, in case the plugin is still using that voice handle, or has destroyed it already, use False)

(27/11/00)
- added FPN_VoiceLevel & FPN_VoiceLevelHint to allow the plugin to define the function of the 2 extra per-voice params (filter cutoff & resonance by default)
  added FPD_UseVoiceLevels

(29/11/00)
- added TempBuffers (TFruityPlugHost)
- added AddWave_32FM_32FS_Ramp and AddWave_32FS_32FS_Ramp (TFruityPlugHost)

(13/12/00)
- updated FPE_MIDI_Vol
- added FHD_ActivateMIDI

(15/12/00)
- added FPF_MIDIIn, TFruityPlug.MIDIIn

(16/12/00)
- removed FPF_MIDIIn & FPF_MIDITick, added FHD_WantMIDIInput & FHD_WantMIDITick

(19/12/00)
- added FPD_WrapPlugin and FHD_LocatePlugin

(20/12/00)
- added FHD_KillAutomation
- added FPF_DemoVersion

(21/12/00)
- added FHD_SetNumPresets, FPN_Preset, FPD_SetPreset

(02/01/01)
- added noise wavetable
- updated PM_HQ

(15/02/01)
- added FHD_VSTiIdle
- now supports FPF_UseChanSample
- added FPD_SetCustomWaveTable

(19/03/01)
- updated FPD_WrapPlugin

(18/04/O1)
- added sample loading functions

(2/06/01)
- added FPN_OutCtrl, NumOutCtrls
- replaced FPF_EventController by FPF_WantNewTick (now implemented)

(12/07/01)
- added FHD_LocateDataFile
- fixed the implementation of TFruityPlugHost.Voice_Release in FruityLoops

(19/07/01)
- added FHD_GetParamMenuEntry
- (!) changed CurrentSDKVersion to 1

(24/07/01)
- added some info to WaveTables & TempBuffers

(29/07/01)
- added MsgIn, PlugMsg_Delayed, etc. (for the Fruity Vibrator)

(02/08/01)
- added FPF_Hybrid
- renamed FPF_UseChanSample to FPF_GetChanCustomShape & renamed FPD_SetCustomShape to FPD_ChanSampleChanged
- added FPF_GetChanSample (so that the plugin can get the sample as a custom 32FM shape, or a 16IS sample
- added FHLS_GetName
- changed & added processing mode flags (see PM_IsRendering)
- added GetSampleInfo

(13/02/02)
- removed MixingTick & MIDITick class variables

(10/07/02)
- added FPF_WantFitTime

(29/06/02)
- increased number of temp buffers to 4

(06/05/04)
- added FHLS_NoResampling

(18/07/04)
- added FPF_NewVoiceParams
  HOW TO UPDATE TO THIS:
  Step 1: Add FPF_NewVoiceParams to your plugin flags
  Step 2: Adapt your code so that you read the pitch & panning as floats instead of integers. Pitch is still in cents, and panning is in -1..1
  Note: ComputeLRVol was changed to ComputeLRVol_Old, and ComputeLRVol has been adapted to accept a -1..1 float panning parameter

(08/11/04)
- changed FPD_SetNumSends

(31/12/04)
- added FPD_SetSamplesPerTick

(21/03/05)
- added FPV_GetLength and FPV_GetColor for a plugin to retrieve voice color & length

(16/05/05)
- added (GM) and (G) comments telling from which thread(s) functions can be called
- added TFruityPlugHost.LockPlugin

(08/03/06)
- added FPF_IsDelphi (not in C++ version)

(02/11/06)
- added FPV_GetVelocity, FPV_GetRelVelocity, FPV_GetRelTime and FPV_SetLinkVelocity

(23/12/08)
- added FHD_GetNumInOut, GetInBuffer, GetOutBuffer, FPD_RoutingChanged
- GetInsBuffer now cannot use Ofs=0 (could bring problems)
So, GetInsBuffer, GetInBuffer & GetOutBuffer use Indexes that start at 1, 0 meaning the buffer during rendering

(27/01/09)
- added FHD_FloatAutomation, FPD_GetParamInfo, FPD_ProjLoaded, FPD_WrapperLoadState, FPD_ShowSettings, FPF_WantSettingsBtn

(02/11/11)
- MsgIn, PlugMsg_Delayed, PlugMsg_Kill are now deprecated. DO NOT USE THEM!

*/


//---------------------------------------------------------------------------
#ifndef FP_PLUGCLASS_H
#define FP_PLUGCLASS_H

#ifdef __APPLE__
#include <stdint.h>
#else
#include <objidl.h>
#endif
#include "fp_def.h"

#ifdef __APPLE__
#define _stdcall __stdcall
#define BOOL int
#define HINSTANCE intptr_t
#define HMENU intptr_t
#define DWORD int
#define HWND intptr_t
#define HANDLE intptr_t
#define NULL 0
#define MAX_PATH 256
#define RTL_CRITICAL_SECTION intptr_t
typedef unsigned long ULONG;
typedef long HRESULT;
typedef unsigned long long ULARGE_INTEGER;
typedef long long LARGE_INTEGER;
#endif

#pragma pack(push)
#pragma pack(4)


#ifdef __APPLE__
#define STDMETHODCALLTYPE __stdcall
class IStream
{
public:
virtual void QueryInterface() = 0;
virtual ULONG STDMETHODCALLTYPE AddRef( void) = 0;
virtual ULONG STDMETHODCALLTYPE Release( void) = 0;
virtual HRESULT STDMETHODCALLTYPE Read(void *pv, ULONG cb, ULONG *pcbRead) = 0;
virtual HRESULT STDMETHODCALLTYPE Write(const void *pv, ULONG cb, ULONG *pcbWritten) = 0;
virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition) = 0;
virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize) = 0;
virtual HRESULT STDMETHODCALLTYPE CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten) = 0;
virtual HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags) = 0;
virtual HRESULT STDMETHODCALLTYPE Revert( void) = 0;
virtual HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) = 0;
virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) = 0;
virtual HRESULT STDMETHODCALLTYPE Stat(void *pstatstg, DWORD grfStatFlag) = 0;
virtual HRESULT STDMETHODCALLTYPE Clone(IStream **ppstm) = 0;
};
#endif


// plugin info, common to all instances of the same plugin
typedef struct {
    int SDKVersion;    // =CurrentSDKVersion
    char *LongName;    // full plugin name (should be the same as DLL name)
    char *ShortName;   // & short version (for labels)
    int Flags;         // see FPF_Generator
    int NumParams;     // (maximum) number of parameters, can be overridden using FHD_SetNumParams
    int DefPoly;       // preferred (default) max polyphony (Fruity manages polyphony) (0=infinite)
    int NumOutCtrls;   // number of internal output controllers
	int NumOutVoices;  // number of internal output voices

    int Reserved[30];  // set to zero
} TFruityPlugInfo, *PFruityPlugInfo;


// voice handle (can be an index or a memory pointer (must be unique, that is *not* just the semitone #))
typedef intptr_t TVoiceHandle;
typedef TVoiceHandle TOutVoiceHandle;
typedef intptr_t TPluginTag;

// sample handle
typedef intptr_t TSampleHandle;

// sample region
typedef struct {
    int SampleStart;
	int SampleEnd;
    char Name[256];
    char Info[256];
    float Time;         // beat position, mainly for loop dumping (-1 if not supported)
    int KeyNum;         // linked MIDI note number (-1 if not supported)
    int Reserved[4];
} TSampleRegion, *PSampleRegion;

#pragma pack(pop)



#pragma pack(push)
#pragma pack(1)

// sample info, FILL CORRECTLY
typedef struct {
   int Size;              // size of this structure, MUST BE SET BY THE PLUGIN
   void *Data;            // pointer to the samples
   int Length;            // length in samples
   int SolidLength;       // length without ending silence
   int LoopStart;
   int LoopEnd;           // loop points (LoopStart=-1 if no loop points)
   double SmpRateConv;    // host samplerate*SmpRateConv = samplerate
   int NumRegions;        // number of regions in the sample (see GetSampleRegion)
   float NumBeats;        // length in beats
   float Tempo;  
   int NumChans;          // 1=mono, 2=stereo, MUST BE SET BY THE PLUGIN, to -1 if all formats are accepted
   int Format;            // 0=16I, 1=32F, MUST BE SET BY THE PLUGIN, to -1 if all formats are accepted
   int Reserved[13];      // future use

} TSampleInfo, *PSampleInfo;

#pragma pack(pop)



#pragma pack(push)
#pragma pack(4)

// see FPV_GetInfo
typedef struct {
	int Length;
	int Color;
	float Velocity;
	int Flags;
	int Reserved[8];
} TVoiceInfo, *PVoiceInfo;

#pragma pack(pop)



#pragma pack(push)
#pragma pack(1)

// see FHD_GetMixingTime
typedef struct {
	double t, t2;
} TFPTime, *PFPTime;

// see FHD_GetInName
typedef struct {
	char Name[256];		// user-defined name (can be empty)
	char VisName[256];	// visible name (can be guessed)
	int Color;
	int Index;			// real index of the item (can be used to translate plugin's own in/out into real mixer track #)
} TNameColor, *PNameColor;

// see GetInBuffer/GetOutBuffer
typedef struct {
	void *Buffer;
	//BOOL Filled;	// only valid for GetInBuffer, indicates if buffer is not empty
	DWORD Flags;  // see IO_Filled
} TIOBuffer, *PIOBuffer;

#pragma pack(pop)



// level params, used both for final voice levels (voice levels+parent channel levels) & original voice levels
// note: all params can go outside their defined range

// OLD, OBSOLETE VERSION, DO NOT USE!!!
typedef struct { 
    int Pan;        // panning (-64..64)
    float Vol;      // volume/velocity (0..1)
    int Pitch;      // pitch (in cents) (semitone=Pitch/100)
    float FCut;     // filter cutoff (0..1)
	float FRes;     // filter Q (0..1)
} TLevelParams_Old, *PLevelParams_Old;
typedef struct {
    TLevelParams_Old InitLevels;
	TLevelParams_Old FinalLevels;
} TVoiceParams_Old, *PVoiceParams_Old;


// NEW VERSION (all floats), USE THESE
typedef struct {
    float Pan;    // panning (-1..1)
    float Vol;    // volume/velocity (0..1)
    float Pitch;  // pitch (in cents) (semitone=Pitch/100)
    float FCut;   // filter cutoff (0..1)
	float FRes;   // filter Q (0..1)
} TLevelParams, *PLevelParams;

typedef struct {
    TLevelParams InitLevels;
    TLevelParams FinalLevels;
} TVoiceParams, *PVoiceParams;



// to add notes to the piano roll (current pattern)
#pragma pack(push)
#pragma pack(1)

typedef struct {
    int Position;  // in PPQ
    int Length;    // in PPQ
               
    // levels
    int Pan;       // default=0
    int Vol;       // default=100/128
    short Note;    // default=60
	short Color;   // 0..15 (=MIDI channel)
    int Pitch;     // default=0
    float FCut;    // default=0
    float FRes;    // default=0
} TNoteParams;

#pragma pack(pop)
    
typedef struct {
    int Target;              // 0=step seq (not supported yet), 1=piano roll
    int Flags;               // see NPF_EmptyFirst
    int PatNum;              // -1 for current
    int ChanNum;             // -1 for plugin's channel, or selected channel if plugin is an effect
    int Count;               // the # of notes in the structure
    TNoteParams NoteParams[1];  // array of notes (variable size)
} TNotesParams, *PNotesParams;


// param menu entry
typedef struct {
    char *Name;    // name of the menu entry (or menu separator if '-')
    int Flags;  // checked or disabled, see FHP_Disabled
} TParamMenuEntry, *PParamMenuEntry;




// plugin class
class TFruityPlug {
public:
    // *** params ***

    TPluginTag HostTag;        // free for the host to use (parent object reference, ...), passed as 'Sender' to the host
    PFruityPlugInfo Info;
    HWND EditorHandle;       // handle to the editor window panel (created by the plugin)

    BOOL MonoRender;         // last rendered voice rendered mono data (not used yet)

    int Reserved[32];        // for future use, set to zero


    // *** functions ***
    // (G) = called from GUI thread, (M) = called from mixer thread, (GM) = both, (S) = called from MIDI synchronization thread
    // (GM) calls are normally thread-safe

    // messages (to the plugin)
    virtual void _stdcall DestroyObject();  // (G)
    virtual intptr_t _stdcall Dispatcher(intptr_t ID, intptr_t Index, intptr_t Value) = 0;  // (GM)
    virtual void _stdcall Idle_Public() = 0;  // (G) (used to be Idle())
    virtual void _stdcall SaveRestoreState(IStream *Stream, BOOL Save) = 0;  // (G)

    // names (see FPN_Param) (Name must be at least 256 chars long)
    virtual void _stdcall GetName(int Section, int Index, int Value, char *Name) = 0;  // (GM)

    // events
    virtual int _stdcall ProcessEvent(int EventID, int EventValue, int Flags) = 0;  // (GM)
    virtual int _stdcall ProcessParam(int Index, int Value, int RECFlags) = 0;  // (GM)

    // effect processing (source & dest can be the same)
    virtual void _stdcall Eff_Render(PWAV32FS SourceBuffer, PWAV32FS DestBuffer, int Length) = 0;  // (M)
    // generator processing (can render less than length)
    virtual void _stdcall Gen_Render(PWAV32FS DestBuffer, int &Length) = 0;  // (M)

    // voice handling
    virtual TVoiceHandle _stdcall TriggerVoice(PVoiceParams VoiceParams, intptr_t SetTag) = 0;  // (GM)
    virtual void _stdcall Voice_Release(TVoiceHandle Handle) = 0;  // (GM)
    virtual void _stdcall Voice_Kill(TVoiceHandle Handle) = 0;  // (GM)
    virtual int _stdcall Voice_ProcessEvent(TVoiceHandle Handle, int EventID, int EventValue, int Flags) = 0;  // (GM)
    virtual int _stdcall Voice_Render(TVoiceHandle Handle, PWAV32FS DestBuffer, int &Length) = 0;  // (GM)


    // (see FPF_WantNewTick) called before a new tick is mixed (not played)
    // internal controller plugins should call OnControllerChanged from here
    virtual void _stdcall NewTick() = 0;  // (M)

    // (see FHD_WantMIDITick) called when a tick is being played (not mixed) (not used yet)
    virtual void _stdcall MIDITick() = 0;  // (S)

    // MIDI input message (see FHD_WantMIDIInput & TMIDIOutMsg) (set Msg to MIDIMsg_Null if it has to be killed)
    virtual void _stdcall MIDIIn(int &Msg) = 0;  // (GM)

    // buffered messages to itself (see PlugMsg_Delayed)
    virtual void _stdcall MsgIn(intptr_t Msg) = 0;  // (S)

    // voice handling
    virtual int _stdcall OutputVoice_ProcessEvent(TOutVoiceHandle Handle, int EventID, int EventValue, int Flags) = 0;  // (GM)
    virtual void _stdcall OutputVoice_Kill(TVoiceHandle Handle) = 0;  // (GM)

	TFruityPlug();
    virtual ~TFruityPlug();
};




// plugin host class
class TFruityPlugHost {
public:
    // *** params ***

    int HostVersion;     // current FruityLoops version stored as 01002003 (integer) for 1.2.3
    int Flags;           // reserved

    // windows
    HANDLE AppHandle;    // application handle, for slaving windows

    // handy wavetables (32Bit float (-1..1), 16384 samples each)
    // 6 are currently defined (sine, triangle, square, saw, analog saw, noise)
    // those pointers are fixed
	// (obsolete, avoid)
    PWaveT WaveTables[10];

    // handy free buffers, guaranteed to be at least the size of the buffer to be rendered (float stereo)
    // those pointers are variable, please read & use while rendering only
    // those buffers are contiguous, so you can see TempBuffer[0] as a huge buffer
    PWAV32FS TempBuffers[4];

    // reserved for future use
    int Reserved[30];    // set to zero


    // *** functions ***

    // messages (to the host) (Sender=plugin tag)
	virtual intptr_t _stdcall Dispatcher(TPluginTag Sender, intptr_t ID, intptr_t Index, intptr_t Value) = 0;
    // for the host to store changes
    virtual void _stdcall OnParamChanged(TPluginTag Sender, int Index, int Value) = 0;
    // for the host to display hints
    virtual void _stdcall OnHint(TPluginTag Sender, char *Text) = 0;

    // compute left & right levels using pan & volume info (OLD, OBSOLETE VERSION, USE ComputeLRVol INSTEAD)
    virtual void _stdcall ComputeLRVol_Old(float &LVol, float &RVol, int Pan, float Volume) = 0;

    // voice handling (Sender=voice tag)
    virtual void _stdcall Voice_Release(intptr_t Sender) = 0;
    virtual void _stdcall Voice_Kill(intptr_t Sender, BOOL KillHandle) = 0;
    virtual int _stdcall Voice_ProcessEvent(intptr_t Sender, intptr_t EventID, intptr_t EventValue, intptr_t Flags) = 0;

    // thread synchronisation / safety
    virtual void _stdcall LockMix() = 0;  // will prevent any new voice creation & rendering
    virtual void _stdcall UnlockMix() = 0;


    // delayed MIDI out message (see TMIDIOutMsg) (will be sent once the MIDI tick has reached the current mixer tick
    virtual void _stdcall MIDIOut_Delayed(TPluginTag Sender, intptr_t Msg) = 0;
    // direct MIDI out message
    virtual void _stdcall MIDIOut(TPluginTag Sender, intptr_t Msg) = 0;

    // adds a mono float buffer to a stereo float buffer, with left/right levels & ramping if needed
    // how it works: define 2 float params for each voice: LastLVol & LastRVol. Make them match LVol & RVol before the *first* rendering of that voice (unless ramping will occur from 0 to LVol at the beginning).
    // then, don't touch them anymore, just pass them to the function.
    // the level will ramp from the last ones (LastLVol) to the new ones (LVol) & will adjust LastLVol accordingly
    // LVol & RVol are the result of the ComputeLRVol function
    // for a quick & safe fade out, you can set LVol & RVol to zero, & kill the voice when both LastLVol & LastRVol will reach zero
    virtual void _stdcall AddWave_32FM_32FS_Ramp(void *SourceBuffer, void *DestBuffer, int Length, float LVol, float RVol, float &LastLVol, float &LastRVol) = 0;
    // same, but takes a stereo source
    // note that left & right channels are not mixed (not a true panning), but might be later
    virtual void _stdcall AddWave_32FS_32FS_Ramp(void *SourceBuffer, void *DestBuffer, int Length, float LVol, float RVol, float &LastLVol, float &LastRVol) = 0;

    // sample loading functions (FruityLoops 3.1.1 & over)
    // load a sample (creates one if necessary)
    // FileName must have room for 256 chars, since it gets written with the file that has been 'located'
    // only 16Bit 44Khz Stereo is supported right now, but fill the format correctly!
    // see FHLS_ShowDialog
    virtual bool _stdcall LoadSample(TSampleHandle &Handle, char *FileName, PWaveFormatExtensible NeededFormat, int Flags) = 0;
    virtual void * _stdcall GetSampleData(TSampleHandle Handle, int &Length) = 0;
    virtual void _stdcall CloseSample(TSampleHandle Handle) = 0;

    // time info
    // get the current mixing time, in ticks (integer result)
	// obsolete, use FHD_GetMixingTime & FHD_GetPlaybackTime
    virtual int _stdcall GetSongMixingTime() = 0;
    // get the current mixing time, in ticks (more accurate, with decimals)
    virtual double _stdcall GetSongMixingTime_A() = 0;
    // get the current playing time, in ticks (with decimals)
    virtual double _stdcall GetSongPlayingTime() = 0;

    // internal controller
    virtual void _stdcall OnControllerChanged(TPluginTag Sender, intptr_t Index, intptr_t Value) = 0;

    // get a pointer to one of the send buffers (see FPD_SetNumSends)
    // those pointers are variable, please read & use while processing only
    // the size of those buffers is the same as the size of the rendering buffer requested to be rendered
    virtual void * _stdcall GetSendBuffer(intptr_t Num) = 0;

    // ask for a message to be dispatched to itself when the current mixing tick will be played (to synchronize stuff) (see MsgIn)
    // the message is guaranteed to be dispatched, however it could be sent immediately if it couldn't be buffered (it's only buffered when playing)
    virtual void _stdcall PlugMsg_Delayed(TPluginTag Sender, intptr_t Msg) = 0;
    // remove a buffered message, so that it will never be dispatched
    virtual void _stdcall PlugMsg_Kill(TPluginTag Sender, intptr_t MSg) = 0;

    // get more details about a sample
    virtual void _stdcall GetSampleInfo(TSampleHandle Handle, PSampleInfo Info) = 0;

    // distortion (same as TS404) on a piece of mono or stereo buffer
    // DistType in 0..1, DistThres in 1..10
    virtual void _stdcall DistWave_32FM(int DistType, int DistThres, void *SourceBuffer, int Length, float DryVol, float WetVol, float Mul) = 0;

    // same as GetSendBuffer, but Num is an offset to the mixer track assigned to the generator (Num=0 will then return the current rendering buffer)
    // to be used by generators ONLY, & only while processing
    virtual void * _stdcall GetMixBuffer(int Num) = 0;

	// get a pointer to the insert (add-only) buffer following the buffer a generator is currently processing in
	// Ofs is the offset to the current buffer, +1 means next insert track, -1 means previous one, 0 is forbidden
	// only valid during Gen_Render
	// protect using LockMix_Shared
    virtual void * _stdcall GetInsBuffer(TPluginTag Sender, int Ofs) = 0;

    // ask the host to prompt the user for a piece of text (s has room for 256 chars)
    // set x & y to -1 to have the popup screen-centered
    // if false is returned, ignore the results
    // set c to -1 if you don't want the user to select a color
	virtual bool _stdcall PromptEdit(int x, int y, char *SetCaption, char *s, int &c) = 0;

    // same as LockMix/UnlockMix, but stops the sound (to be used before lengthy operations)
    virtual void _stdcall SuspendOutput() = 0;
    virtual void _stdcall ResumeOutput() = 0;

    // get the region of a sample
    virtual void _stdcall GetSampleRegion(TSampleHandle Handle, int RegionNum, PSampleRegion Region) = 0;

    // compute left & right levels using pan & volume info (USE THIS AFTER YOU DEFINED FPF_NewVoiceParams
    virtual void _stdcall ComputeLRVol(float &LVol, float &RVol, float Pan, float Volume) = 0;

    // alternative to LockMix/UnlockMix that won't freeze audio
    // can only be called from the GUI thread
    // warning: not very performant, avoid using
	virtual void _stdcall LockPlugin(TPluginTag Sender) = 0;
	virtual void _stdcall UnlockPlugin(TPluginTag Sender) = 0;

    // multithread processing synchronisation / safety
    virtual void _stdcall LockMix_Shared_Old() = 0;
    virtual void _stdcall UnlockMix_Shared_Old() = 0;

	// multi-in/output (for generators & effects) (only valid during Gen/Eff_Render)
	// !!! Index starts at 1, to be compatible with GetInsBuffer (Index 0 would be Eff_Render's own buffer)
	virtual void _stdcall GetInBuffer(TPluginTag Sender, intptr_t Index, PIOBuffer IBuffer) = 0;	// returns (read-only) input buffer Index (or Nil if not available).
	virtual void _stdcall GetOutBuffer(TPluginTag Sender, intptr_t Index, PIOBuffer OBuffer) = 0;	// returns (add-only) output buffer Index (or Nil if not available). Use LockMix_Shared when adding to this buffer.


    // output voices (VFX "voice effects")
    virtual TOutVoiceHandle _stdcall TriggerOutputVoice(TVoiceParams *VoiceParams, intptr_t SetIndex, intptr_t SetTag) = 0;  // (GM)
    virtual void _stdcall OutputVoice_Release(TOutVoiceHandle Handle) = 0;  // (GM)
    virtual void _stdcall OutputVoice_Kill(TOutVoiceHandle Handle) = 0;  // (GM)
    virtual int _stdcall OutputVoice_ProcessEvent(TOutVoiceHandle Handle, intptr_t EventID, intptr_t EventValue, intptr_t Flags) = 0;  // (GM)

};




const // history:
      // 0: original version
      // 1: new popup menu system
      int CurrentSDKVersion=1;


// plugin flags
const int FPF_Generator         =1;         // plugin is a generator (not effect)
const int FPF_RenderVoice       =1 << 1;   // generator will render voices separately (Voice_Render) (not used yet)
const int FPF_UseSampler        =1 << 2;   // 'hybrid' generator that will stream voices into the host sampler (Voice_Render)
const int FPF_GetChanCustomShape=1 << 3;   // generator will use the extra shape sample loaded in its parent channel (see FPD_ChanSampleChanged)
const int FPF_GetNoteInput      =1 << 4;   // plugin accepts note events (not used yet, but effects might also get note input later)
const int FPF_WantNewTick       =1 << 5;   // plugin will be notified before each mixed tick (& be able to control params (like a built-in MIDI controller) (see NewTick))
const int FPF_NoProcess         =1 << 6;   // plugin won't process buffers at all (FPF_WantNewTick, or special visual plugins (Fruity NoteBook))
const int FPF_NoWindow          =1 << 10;  // plugin will show in the channel settings window & not in its own floating window
const int FPF_Interfaceless     =1 << 11;  // plugin doesn't provide its own interface (not used yet)
const int FPF_TimeWarp          =1 << 13;  // supports timewarps, that is, can be told to change the playing position in a voice (direct from disk music tracks, ...) (not used yet)
const int FPF_MIDIOut           =1 << 14;  // plugin will send MIDI out messages (only those will be enabled when rendering to a MIDI file)
const int FPF_DemoVersion       =1 << 15;  // plugin is a demo version, & the host won't save its automation
const int FPF_CanSend           =1 << 16;  // plugin has access to the send tracks, so it can't be dropped into a send track or into the master
const int FPF_MsgOut            =1 << 17;  // plugin will send delayed messages to itself (will require the internal sync clock to be enabled)
const int FPF_HybridCanRelease  =1 << 18;  // plugin is a hybrid generator & can release its envelope by itself. If the host's volume envelope is disabled, then the sound will keep going when the voice is stopped, until the plugin has finished its own release
const int FPF_GetChanSample     =1 << 19;  // generator will use the sample loaded in its parent channel (see FPD_ChanSampleChanged)
const int FPF_WantFitTime       =1 << 20;  // fit to time selector will appear in channel settings window (see FPD_SetFitTime)
const int FPF_NewVoiceParams    =1 << 21;  // MUST BE USED - tell the host to use TVoiceParams instead of TVoiceParams_Old
const int FPF_Reserved1         =1 << 22;  // don't use (Delphi version specific)
const int FPF_CantSmartDisable  =1 << 23;  // plugin can't be smart disabled
const int FPF_WantSettingsBtn   =1 << 24;  // plugin wants a settings button on the titlebar (mainly for the wrapper)



// useful combo's
const int FPF_Type_Effect       =FPF_NewVoiceParams;                                      // for an effect (Eff_Render)
const int FPF_Type_FullGen      =FPF_Generator | FPF_GetNoteInput | FPF_NewVoiceParams;   // for a full standalone generator (Gen_Render)
const int FPF_Type_HybridGen    =FPF_Type_FullGen | FPF_UseSampler | FPF_NewVoiceParams;  // for an hybrid generator (Voice_Render)
const int FPF_Type_Visual       =FPF_NoProcess | FPF_NewVoiceParams;                      // for a visual plugin that doesn't use the wave data


// plugin dispatcher ID's
// called from GUI thread unless specified
const int FPD_ShowEditor        =0;     // shows the editor (ParentHandle in Value)
const int FPD_ProcessMode       =1;     // sets processing mode flags (flags in value) (can be ignored)
const int FPD_Flush             =2;     // breaks continuity (empty delay buffers, filter mem, etc.) (warning: can be called from the mixing thread) (GM)
const int FPD_SetBlockSize      =3;     // max processing length (samples) (in value)
const int FPD_SetSampleRate     =4;     // sample rate in Value
const int FPD_WindowMinMax      =5;     // allows the plugin to set the editor window resizable (min/max PRect in index, sizing snap PPoint in value)
const int FPD_KillAVoice        =6;     // (in case the mixer was eating way too much CPU) the plugin is asked to kill its weakest voice & return 1 if it did something (not used yet)
const int FPD_UseVoiceLevels    =7;     // return 0 if the plugin doesn't support the default per-voice level Index
										// return 1 if the plugin supports the default per-voice level Index (filter cutoff (0) or filter resonance (1))
										// return 2 if the plugin supports the per-voice level Index, but for another function (then check FPN_VoiceLevel)
                              //=8;     (private message)
const int FPD_SetPreset         =9;     // set internal preset Index (mainly for wrapper)
const int FPD_ChanSampleChanged =10;    // (see FPF_GetChanCustomShape) sample has been loaded into the parent channel, & given to the plugin
										// either as a wavetable (FPF_GetChanCustomshape) (pointer to shape in Value, same format as WaveTables)
										// or as a sample (FPF_GetChanSample) (TSampleHandle in Index)
const int FPD_SetEnabled        =11;    // the host has enabled/disabled the plugin (state in Value) (warning: can be called from the mixing thread) (GM)
const int FPD_SetPlaying        =12;    // the host is playing (song pos info is valid when playing) (state in Value) (warning: can be called from the mixing thread) (GM)
const int FPD_SongPosChanged    =13;    // song position has been relocated (by other means than by playing of course) (warning: can be called from the mixing thread) (GM)
const int FPD_SetTimeSig        =14;    // PTimeSigInfo in Value (G)
const int FPD_CollectFile       =15;    // let the plugin tell which files need to be collected or put in zip files. File # in Index, starts from 0 until no more filenames are returned (PChar in Result).
const int FPD_SetInternalParam  =16;    // (private message to known plugins, ignore) tells the plugin to update a specific, non-automated param
const int FPD_SetNumSends       =17;    // tells the plugin how many send tracks there are (fixed to 4, but could be set by the user at any time in a future update) (number in Value) (!!! will be 0 if the plugin is in the master or a send track, since it can't access sends)
const int FPD_LoadFile          =18;    // when a file has been dropped onto the parent channel's button (filename in Value)
const int FPD_SetFitTime        =19;    // set fit to time in beats (FLOAT time in value (need to typecast))
const int FPD_SetSamplesPerTick =20;    // # of samples per tick (changes when tempo, PPQ or sample rate changes) (FLOAT in Value (need to typecast)) (warning: can be called from the mixing thread) (GM)
const int FPD_SetIdleTime       =21;    // set the freq at which Idle is called (can vary), ms time in Value
const int FPD_SetFocus          =22;    // the host has focused/unfocused the editor (focused in Value) (plugin can use this to steal keyboard focus)
const int FPD_Transport         =23;    // special transport messages, from a controller. See GenericTransport.pas for Index. Must return 1 if handled.
const int FPD_MIDIIn            =24;    // live MIDI input preview, allows the plugin to steal messages (mostly for transport purposes). Must return 1 if handled. Packed message (only note on/off for now) in Value.
const int FPD_RoutingChanged    =25;    // mixer routing changed, must check FHD_GetInOuts if necessary
const int FPD_GetParamInfo      =26;    // retrieves info about a parameter. Param number in Index, see PI_Float for the result
const int FPD_ProjLoaded        =27;    // called after a project has been loaded, to leave a chance to kill automation (that could be loaded after the plugin is created) if necessary
const int FPD_WrapperLoadState  =28;    // (private message to the plugin wrapper) load a (VST1, DX) plugin state, pointer in Index, length in Value
const int FPD_ShowSettings      =29;    // called when the settings button on the titlebar is switched. On/off in Value (1=active). See FPF_WantSettingsBtn
const int FPD_SetIOLatency      =30;    // input/output latency (Index,Value) of the output, in samples (only for information)
const int FPD_PreferredNumIO    =32;    // (message from Patcher) retrieves the preferred number (0=default, -1=none) of audio inputs (Index=0), audio outputs (Index=1) or voice outputs (Index=2)



// GetName sections
const int FPN_Param             =0;     // retrieve name of param Index
const int FPN_ParamValue        =1;     // retrieve text label of param Index for value Value (used in event editor)
const int FPN_Semitone          =2;     // retrieve name of note Index (used in piano roll), for color (=MIDI channel) Value
const int FPN_Patch             =3;     // retrieve name of patch Index (not used yet)
const int FPN_VoiceLevel        =4;     // retrieve name of per-voice param Index (default is filter cutoff (0) & resonance (1)) (optional)
const int FPN_VoiceLevelHint    =5;     // longer description for per-voice param (works like FPN_VoiceLevels)
const int FPN_Preset            =6;     // for plugins that support internal presets (mainly for the wrapper plugin), retrieve the name for program Index
const int FPN_OutCtrl           =7;     // for plugins that output controllers, retrieve the name of output controller Index
const int FPN_VoiceColor        =8;     // retrieve name of per-voice color (MIDI channel) Index
const int FPN_OutVoice          =9;     // for plugins that output voices, retrieve the name of output voice Index


// processing mode flags
const int PM_Normal             =0;     // realtime processing (default)
const int PM_HQ_Realtime        =1;     // high quality, but still realtime processing
const int PM_HQ_NonRealtime     =2;     // non realtime processing (CPU does not matter, quality does) (normally set when rendering only)
const int PM_IsRendering        =16;    // is rendering if this flag is set
//const int PM_IPMask             =7 << 8;  // 3 bits value for interpolation quality (0=none (obsolete), 1=linear, 2=6 point hermite (default), 3=32 points sinc, 4=64 points sinc, 5=128 points sinc, 6=256 points sinc)
const int PM_IPMask             =0xFFFF << 8;  // 16 bits value for interpolation number of points



// ProcessParam flags
const int REC_UpdateValue       =1;     // update the value
const int REC_GetValue          =2;     // retrieves the value
const int REC_ShowHint          =4;     // updates the hint (if any)
const int REC_UpdateControl     =16;    // updates the wheel/knob
const int REC_FromMIDI          =32;    // value from 0 to 65536 has to be translated (& always returned, even if REC_GetValue isn't set)
const int REC_NoLink            =1024;  // don't check if wheels are linked (internal to plugins, useful for linked controls)
const int REC_InternalCtrl      =2048;  // sent by an internal controller - internal controllers should pay attention to those, to avoid nasty feedbacks
const int REC_PlugReserved      =4096;  // free to use by plugins



// event ID's
const int FPE_Tempo             =0;     // FLOAT tempo in value (need to typecast), & average samples per tick in Flags (DWORD) (warning: can be called from the mixing thread) (GM)
const int FPE_MaxPoly           =1;     // max poly in value (infinite if <=0) (only interesting for standalone generators)
// since MIDI plugins, or other plugin wrappers won't support the voice system, they should be notified about channel pan, vol & pitch changes
const int FPE_MIDI_Pan          =2;     // MIDI channel panning (0..127) in EventValue + pan in -64..64 in Flags (warning: can be called from the mixing thread) (GM)
const int FPE_MIDI_Vol          =3;     // MIDI channel volume (0..127) in EventValue + volume as normalized float in Flags (need to typecast) (warning: can be called from the mixing thread) (GM)
const int FPE_MIDI_Pitch        =4;     // MIDI channel pitch in *cents* (to be translated according to current pitch bend range) in EventValue (warning: can be called from the mixing thread) (GM)


// voice handles
const int FVH_Null              =-1;

// TFruityPlug.Voice_ProcessEvent ID's
const int FPV_Retrigger         =0;     // monophonic mode can retrigger releasing voices (not used yet)

// TFruityPlugHost.Voice_ProcessEvent ID's
const int FPV_GetLength         =1;     // retrieve length in ticks (not reliable) in Result (-1 if undefined)
const int FPV_GetColor          =2;     // retrieve color (0..15) in Result, can be mapped to MIDI channel
const int FPV_GetVelocity       =3;     // retrieve note on velocity (0..1) in Result (typecast as a float) (this is computed from InitLevels.Vol)
const int FPV_GetRelVelocity    =4;     // retrieve release velocity (0..1) in Result (typecast as a float) (to be called from Voice_Release) (use this if some release velocity mapping is involved)
const int FPV_GetRelTime        =5;     // retrieve release time multiplicator (0..2) in Result (typecast as a float) (to be called from Voice_Release) (use this for direct release multiplicator)
const int FPV_SetLinkVelocity   =6;     // set if velocity is linked to volume or not (in EventValue)


// Voice_Render function results
const int FVR_Ok                =0;
const int FVR_NoMoreData        =1;     // for sample streaming, when there's no more sample data to fill any further buffer (the voice will then be killed by the host)




// host dispatcher ID's
const int FHD_ParamMenu         =0;     // the popup menu for each control (Index=param index, Value=popup item index (see FHP_EditEvents))
const int FHD_GetParamMenuFlags =1;     // (OBSOLETE, see FHD_GetParamMenuEntry) before the popup menu is shown, you must ask the host to tell if items are checked or disabled (Index=param index, Value=popup item index, Result=flags (see FHP_Disabled))
const int FHD_EditorResized     =2;     // to notify the host that the editor (EditorHandle) has been resized
const int FHD_NamesChanged      =3;     // to notify the host that names (GetName function) have changed, with the type of names in Value (see the FPN_ constants)
const int FHD_ActivateMIDI      =4;     // makes the host enable its MIDI output, useful when a MIDI out plugin is created (but not useful for plugin wrappers)
const int FHD_WantMIDIInput     =5;     // plugin wants to be notified about MIDI messages (for processing or filtering) (switch in Value)
const int FHD_WantMIDITick      =6;     // plugin wants to receive MIDITick events, allowing MIDI out plugins (not used yet)
                          //=7;     (private message)
const int FHD_KillAutomation    =8;     // ask the host to kill the automation linked to the plugin, for params # between Index & Value (included) (can be used for a demo version of the plugin)
const int FHD_SetNumPresets     =9;     // tell the host how many (Value) internal presets the plugin supports (mainly for wrapper)
const int FHD_SetNewName        =10;    // sets a new short name for the parent (PChar in Value)
const int FHD_VSTiIdle          =11;    // used by the VSTi wrapper, because the dumb VSTGUI needs idling for his knobs
const int FHD_SelectChanSample  =12;    // ask the parent to open a selector for its channel sample (see FPF_UseChanSample)
const int FHD_WantIdle          =13;    // plugin wants to receive the idle message (enabled by default) (Value=0 for disabled, 1 for enabled when UI is visible, 2 for always enabled)
const int FHD_LocateDataFile    =14;    // ask the host to search for a file in its search paths, pass the simple filename in Value, full path is returned as Result (both PChar) (Result doesn't live long, please copy it asap)
const int FHD_TicksToTime       =16;    // translate tick time (Value) into Bar:Step:Tick (PSongTime in Index) (warning: it's *not* Bar:Beat:Tick)
const int FHD_AddNotesToPR      =17;    // add a note to the piano roll, PNotesParams in Value
const int FHD_GetParamMenuEntry =18;    // before the popup menu is shown, you must fill it with the entries set by the host (Index=param index, Value=popup item index (starting from 0), Result=PParamMenuEntry, or null pointer if no more entry)
const int FHD_MsgBox            =19;    // make fruity show a message box (PChar in Index [formatted as 'Title|Message'], flags in Value (MB_OkCancel, MB_IconWarning, etc.), result in IDOk, IDCancel format (as in TApplication.MessageBox)
const int FHD_NoteOn            =20;    // preview note on (semitone in Index low word, color in index high word (0=default), velocity in Value)
const int FHD_NoteOff           =21;    // preview note off (semitone in Index)
const int FHD_OnHint_Direct     =22;    // same as OnHint, but show it immediately (to show a progress while you're doing something) (PChar in Value)
const int FHD_SetNewColor       =23;    // sets a new color for the parent (color in Value) (see FHD_SetNewName);
const int FHD_GetInstance       =24;    // (Windows) returns the module instance of the host (could be an exe or a DLL, so not the process itself)
const int FHD_KillIntCtrl       =25;    // ask the host to kill anything linked to an internal controller, for # between Index & Value (included) (used when undeclaring internal controllers)
const int FHD_CheckProdCode     =26;    // reserved
const int FHD_SetNumParams      =27;    // override the # of parameters (for plugins that have a different set of parameters per instance) (number of parameters in Value)
const int FHD_PackDataFile      =28;    // ask the host to pack an absolute filename into a local filemane, pass the simple filename in Value, packed path is returned as Result (both PChar) (Result doesn't live long, please copy it asap)
const int FHD_GetProgPath       =29;    // ask the host where the engine is, which may NOT be where the executable is, but where the data path will be (returned as Result)
const int FHD_SetLatency        =30;    // set plugin latency, if any (samples in Value)
const int FHD_CallDownloader    =31;    // call the presets downloader (optional plugin name PAnsiChar in Value)
const int FHD_EditSample		=32;	// edits sample in Edison (PChar in Value, Index=1 means an existing Edison can be re-used)
const int FHD_SetThreadSafe     =33;    // plugin is thread-safe, doing its own thread-sync using LockMix_Shared (switch in Value)
const int FHD_SmartDisable      =34;    // plugin asks FL to exit or enter smart disabling (if currently active), mainly for generators when they get MIDI input (switch in Value)
const int FHD_SetUID            =35;    // sets a unique identifying string for this plugin. This will be used to save/restore custom data related to this plugin. Handy for wrapper plugins. (PChar in Value)
const int FHD_GetMixingTime     =36;    // get mixer time, Index is the time format required (0 for Beats, 1 for absolute ms, 2 for running ms, 3 for ms since soundcard restart), Value is a pointer to a TFPTime, which is filled with an optional offset in samples
const int FHD_GetPlaybackTime   =37;    // get playback time, same as above
const int FHD_GetSelTime        =38;    // get selection time in t & t2, same as above. Returns 0 if no selection (t & t2 are then filled with full song length).
const int FHD_GetTimeMul        =39;    // get current tempo multiplicator, that's not part of the song but used for fast-forward
const int FHD_Captionize        =40;    // captionize the plugin (useful when dragging) (captionized in Value)
const int FHD_SendSysEx         =41;    // send a SysEx string (pointer to array in Value, the first integer being the length of the string, the rest being the string), through port Index, immediately (do not abuse)
const int FHD_LoadAudioClip     =42;    // send an audio file to the playlist as an audio clip, starting at the playlist selection (mainly for Edison), FileName as PChar in Value
const int FHD_LoadInChannel     =43;    // send a file to the selected channel(s) (mainly for Edison), FileName as PChar in Value
const int FHD_ShowInBrowser     =44;    // locates the file in the browser & jumps to it (PChar in Value)
const int FHD_DebugLogMsg       =45;    // adds message to the debug log (PChar in Value)
const int FHD_GetMainFormHandle =46;    // gets the handle of the main form (HWND in Value, 0 if none)
const int FHD_GetProjDataPath   =47;    // ask the host where the project data is, to store project data (returned as Result)
const int FHD_SetDirty          =48;    // mark project as dirty (not required for automatable parameters, only for tweaks the host can't be aware of)
const int FHD_AddToRecent       =49;    // add file to recent files (PChar in Value)
const int FHD_GetNumInOut       =50;    // ask the host how many inputs (Index=0) are routed to this effect (see GetInBuffer), or how many outputs (Index=1) this effect is routed to (see GetOutBuffer)
const int FHD_GetInName         =51;    // ask the host the name of the input Index (!!! first = 1), in Value as a PNameColor, Result=0 if failed (Index out of range)
const int FHD_GetOutName        =52;    // ask the host the name of the ouput Index (!!! first = 1), in Value as a PNameColor, Result=0 if failed (Index out of range)
const int FHD_ShowEditor        =53;    // make host bring plugin's editor (visibility in Value, -1 to toggle)
const int FHD_FloatAutomation   =54;    // (for the plugin wrapper only) ask the host to turn 0..65536 automation into 0..1 float, for params # between Index & Value (included)
const int FHD_ShowSettings      =55;    // called when the settings button on the titlebar should be updated switched. On/off in Value (1=active). See FPF_WantSettingsBtn
const int FHD_NoteOnOff         =56;    // note on/off (semitone in Index low word, color in index high word, NOT recorded in bit 30, velocity in Value (<=0 = note off))
const int FHD_ShowPicker        =57;    // show picker (mode [0=plugins, 1=project] in Index, categories [gen=0/FX=1/both=-1/Patcher (includes VFX)=-2] in Value)
const int FHD_GetIdleOverflow   =58;    // ask the host for the # of extra frames Idle should process, generally 0 if no overflow/frameskip occured
const int FHD_ModalIdle         =59;    // used by FL plugins, when idling from a modal window, mainly for the smoothness hack
const int FHD_RenderProject     =60;    // prompt the rendering dialog in song mode
const int FHD_GetProjectInfo    =61;    // get project title, author, comments, URL (Index), (returned as Result as a *PWideChar*)


     



// param popup menu item indexes (same order as param menu in FruityLoops)
// note that it can be a Windows popup menu or anything else
// OBSOLETE (compatibility only): now the plugin doesn't know about those menu entries, that can be freely changed by the host
/*
const int FHP_Edit              =0;     // Edit events
const int FHP_EditNewWindow     =1;     // Edit events in new window
const int FHP_Init              =2;     // Init with this position
const int FHP_Link              =3;     // Link to MIDI controller
*/

// param popup menu item flags
const int FHP_Disabled          =1;
const int FHP_Checked           =2;

// sample loading flags
const int FHLS_ShowDialog       =1;     // tells the sample loader to show an open box, for the user to select a sample
const int FHLS_ForceReload      =2;     // force it to be reloaded, even if the filename is the same (in case you modified the sample)
const int FHLS_GetName          =4;     // don't load the sample, instead get its filename & make sure that the format is correct (useful after FPD_ChanSampleChanged)
const int FHLS_NoResampling     =8;     // don't resample to the host sample rate
      

// TNotesParams flags
const int NPF_EmptyFirst        =1;     // delete everything before adding the notes
const int NPF_UseSelection      =2;     // dump inside piano roll selection if any

// param flags (see FPD_GetParamInfo)
const int PI_CantInterpolate    =1;     // makes no sense to interpolate parameter values (when values are not levels)
const int PI_Float              =2;     // parameter is a normalized (0..1) single float. (Integer otherwise)
const int PI_Centered           =4;     // parameter appears centered in event editors



// GetInBuffer / GetOutBuffer flags
// input
const int IO_Lock               =0;     // GetOutBuffer, before adding to the buffer
const int IO_Unlock             =1;     // GetOutBuffer, after adding to the buffer
// output
const int IO_Filled             =1;     // GetInBuffer, tells if the buffer is filled






//---------------------------------------------------------------------------
#endif   // FP_PLUGCLASS_H