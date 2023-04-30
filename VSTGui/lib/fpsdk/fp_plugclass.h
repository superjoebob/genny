/*

FL Studio generator/effect plugins SDK
plugin & host classes

(99-21)

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
#define DWORD unsigned int
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
    float Vol;    // volume/velocity (0.0 = -inf dB .. 1.0 = 0 dB) - note: can go above 1.0!
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


// 
typedef struct  {
	void* DestP;
	int MaxWidth, MaxHeight;
	int Id; // 0=mixer, 1=playlist
	int ReturnedWidth, ReturnedHeight;
	char NamedPlugin[128];
} TRenderWindowBitmapInfo, *PRenderWindowBitmapInfo;

// see FHD_SetContextInfo and CI_ constants ... allows changes that require additional info
typedef struct {
	intptr_t Param;
	intptr_t IntValue;
	float FloatValue;
} TContextInfo, *PContextInfo;

typedef struct {
  int ParamIndex;
  char* StringValue;
  double ConvertedData;
} TConvertStringToValueData, *PConvertStringToValueData;


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
    // (M) calls are done inside the plugin lock (LockPlugin / UnlockPlugin)
    // + TriggerVoice and Voice_ functions are also called inside the plugin lock
    // + assume that any other call is not locked! (so call LockPlugin / UnlockPlugin where necessary, but no more than that)
	// + don't call back to the host while inside a LockPlugin / UnlockPlugin block

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
    virtual int _stdcall Voice_ProcessEvent(TVoiceHandle Handle, intptr_t EventID, intptr_t EventValue, intptr_t Flags) = 0;  // (GM)
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
    virtual int _stdcall OutputVoice_ProcessEvent(TOutVoiceHandle Handle, intptr_t EventID, intptr_t EventValue, intptr_t Flags) = 0;  // (GM)
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
	// for the host to display hints (call from GUI thread!)
    virtual void _stdcall OnHint(TPluginTag Sender, char *Text) = 0;

    // compute left & right levels using pan & volume info (OLD, OBSOLETE VERSION, USE ComputeLRVol INSTEAD)
    virtual void _stdcall ComputeLRVol_Old(float &LVol, float &RVol, int Pan, float Volume) = 0;

    // voice handling (Sender=voice tag)
    virtual void _stdcall Voice_Release(intptr_t Sender) = 0;
    virtual void _stdcall Voice_Kill(intptr_t Sender, BOOL KillHandle) = 0;
    virtual int _stdcall Voice_ProcessEvent(intptr_t Sender, intptr_t EventID, intptr_t EventValue, intptr_t Flags) = 0;

    // thread synchronisation / safety
	virtual void _stdcall LockMix_Old() = 0;  // will prevent any new voice creation & rendering
    virtual void _stdcall UnlockMix_Old() = 0;


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
    // if 0 is returned, ignore the results
    // set c to -1 if you don't want the user to select a color
    virtual BOOL _stdcall PromptEdit(int x, int y, char *SetCaption, char *s, int &c) = 0;

    // deprecated, use SuspendOutput and ResumeOutput instead
    virtual void _stdcall SuspendOutput_Old() = 0;
    virtual void _stdcall ResumeOutput_Old() = 0;

    // get the region of a sample
    virtual void _stdcall GetSampleRegion(TSampleHandle Handle, int RegionNum, PSampleRegion Region) = 0;

    // compute left & right levels using pan & volume info (USE THIS AFTER YOU DEFINED FPF_NewVoiceParams)
    virtual void _stdcall ComputeLRVol(float &LVol, float &RVol, float Pan, float Volume) = 0;

    // use this instead of PlugHost.LockMix
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

    // ask the host to prompt the user for a piece of text, color, icon ... See PEO_ constants for SetOptions. Text should be null or a pointer to an allocated buffer with at least 255 characters!
    virtual BOOL _stdcall PromptEdit_Ex(int x, int y, const char* SetCaption, char* Text, int& Color1, int& Color2, int& IconIndex, int FontHeight, int SetOptions) = 0;

    // SuspendOutput removes the plugin from all processing lists, so Eff/Gen_Render and voice functions will no longer be called.
    // To be used around lengthy operations (instead of straightforward locking)
    virtual void _stdcall SuspendOutput(TPluginTag Sender) = 0;
    virtual void _stdcall ResumeOutput(TPluginTag Sender) = 0;
};




const // history:
      // 0: original version
      // 1: new popup menu system
      int CurrentSDKVersion=1;


// plugin flags
const int FPF_Generator         =1;        // plugin is a generator (not effect)
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
const int FPF_DemoVersion       =1 << 15;  // plugin is a trial version, & the host won't save its automation
const int FPF_CanSend           =1 << 16;  // plugin has access to the send tracks, so it can't be dropped into a send track or into the master
const int FPF_MsgOut            =1 << 17;  // plugin will send delayed messages to itself (will require the internal sync clock to be enabled)
const int FPF_HybridCanRelease  =1 << 18;  // plugin is a hybrid generator & can release its envelope by itself. If the host's volume envelope is disabled, then the sound will keep going when the voice is stopped, until the plugin has finished its own release
const int FPF_GetChanSample     =1 << 19;  // generator will use the sample loaded in its parent channel (see FPD_ChanSampleChanged)
const int FPF_WantFitTime       =1 << 20;  // fit to time selector will appear in channel settings window (see FPD_SetFitTime)
const int FPF_NewVoiceParams    =1 << 21;  // MUST BE USED - tell the host to use TVoiceParams instead of TVoiceParams_Old
const int FPF_Reserved1         =1 << 22;  // don't use (Delphi version specific)
const int FPF_CantSmartDisable  =1 << 23;  // plugin can't be smart disabled
const int FPF_WantSettingsBtn   =1 << 24;  // plugin wants a settings button on the titlebar (mainly for the wrapper)
const int FPF_CanStealKBFocus   =1 << 25;  // plugin can steal keyboard focus away from FL
const int FPF_VFX               =1 << 26;  // is VFX plugin
const int FPF_MacNeedsNSView 	=1 << 27;  // On Mac: This plugin requires a NSView parent



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
const int FPD_SetTimeSig        =14;    // PTimeSigInfo in Value (GM)
const int FPD_CollectFile       =15;    // let the plugin tell which files need to be collected or put in zip files. File # in Index, starts from 0 until no more filenames are returned (PChar in Result).
const int FPD_SetInternalParam  =16;    // (private message to known plugins, ignore) tells the plugin to update a specific, non-automated param
const int FPD_SetNumSends       =17;    // tells the plugin how many send tracks there are (fixed to 4, but could be set by the user at any time in a future update) (number in Value) (!!! will be 0 if the plugin is in the master or a send track, since it can't access sends)
const int FPD_LoadFile          =18;    // when a file has been dropped onto the parent channel's button (LFT_ type in Index, filename in Value). Result should be 0 if not handled, 1 if handled and 2 if a dropped file should be rejected
                                        // LFT_DownloadDataPack option is used to download Flex packs: Result is -1 if failed, or Pack index on success
const int FPD_SetFitTime        =19;    // set fit to time in beats (FLOAT time in value (need to typecast))
const int FPD_SetSamplesPerTick =20;    // # of samples per tick (changes when tempo, PPQ or sample rate changes) (FLOAT in Value (need to typecast)) (warning: can be called from the mixing thread) (GM)
const int FPD_SetIdleTime       =21;    // set the freq at which Idle is called (can vary), ms time in Value
const int FPD_SetFocus          =22;    // the host has focused/unfocused the editor (focused in Value) (plugin can use this to steal keyboard focus ... also see FPD_StealKBFocus)
const int FPD_Transport         =23;    // special transport messages, from a controller. See GenericTransport.pas for Index. Must return 1 if handled.
const int FPD_MIDIIn            =24;    // live MIDI input preview, allows the plugin to steal messages (mostly for transport purposes). Must return 1 if handled. Packed message (only note on/off for now) in Value.
const int FPD_RoutingChanged    =25;    // mixer routing changed, must check FHD_GetInOuts if necessary. See RCV_ constants for the meaning of the Value parameter.
const int FPD_GetParamInfo      =26;    // retrieves info about a parameter. Param number in Index, see PI_Float for the result
const int FPD_ProjLoaded        =27;    // called after a project has been loaded, to leave a chance to kill automation (that could be loaded after the plugin is created) if necessary
const int FPD_WrapperLoadState  =28;    // (private message to the plugin wrapper) load a (VST1, DX) plugin state, pointer in Index, length in Value
const int FPD_ShowSettings      =29;    // called when the settings button on the titlebar is switched. On/off in Value (1=active). See FPF_WantSettingsBtn
const int FPD_SetIOLatency      =30;    // input/output latency (Index,Value) of the output, in samples (only for information)
const int FPD_PreferredNumIO    =32;    // (message from Patcher) retrieves the preferred number (0=default, -1=none) of audio inputs (Index=0), audio outputs (Index=1) or voice outputs (Index=2)
const int FPD_GetGUIColor       =33;    // retrieves the darkest background color of the GUI (Index=0 for background), for a nicer border around it
const int FPD_CloseAllWindows   =34;    // hide all windows opened by the plugin (except the plugin editor window)
const int FPD_RenderWindowBitmap=35;    // used by ZgeViz
const int FPD_StealKBFocus      =36;    // switch stealing keyboard focus off or on (Value = 0 or 1)
const int FPD_GetHelpContext    =37;    // for plugins that want to show specific help pages, like Patcher. Return the context as a UTF-8 encoded PAnsiChar as the result. Return 0 or an empty string for the default context.
const int FPD_RegChanged		=38;    // notify plugin about registration change
const int FPD_ArrangeWindows 	=39; 	// arrange subwindows into the workspace (Value = workspace PRect)
const int FPD_PluginLoaded		=40; 	// done opening the plugin - note that SaveRestoreState is called before this!
const int FPD_ContextInfoChanged=41; 	// Index holds the type of information (see CI_ constants), call FHD_GetContextInfo for the new value(s)
const int FPD_ProjectInfoChanged=42;    // Index holds the value that changed (see GPI_ contants)
const int FPD_GetDemoPlugins    =43;    // Returns ; delimited list (formatted as "productCode|name") of plugins in demo mode. If Value is 1, it should only list plugins that were saved as a demo.
const int FPD_UnLockDemoPlugins =44;    // Tells a plugin to recheck demo mode and unlock purchased plugins
const int FPD_ColorWasPicked = 46; // called after FHD_PickVoiceColor finishes. The new color value (an index, not RGB) is passed in Value.
const int FPD_IsInDebugMode = 47; // return 0 for no, 1 for yes
const int FPD_ColorsHaveChanged = 48; // some shared colors have changed. Index indicates the palette (see CP_ constants).
const int FPD_GetStateSizeEstimate = 49; //get plugin estimated state size
const int FPD_UseIncreasedMIDIResolution = 50; // return 1 if increased MIDI resolution is supported
const int FPD_ConvertStringToValue = 51;  //let plugin do string to value conversion, value is pointer to TConvertStringToValueData record , used for custom type in value
const int FPD_GetParamType = 52; //return control (Index) param type, see //FPD_GetParamType options below


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
const int REC_FromMIDI          =32;    // value from 0 to FromMIDI_Max has to be translated (& always returned, even if REC_GetValue isn't set)
const int REC_NoLink            =1024;  // don't check if wheels are linked (internal to plugins, useful for linked controls)
const int REC_InternalCtrl      =2048;  // sent by an internal controller - internal controllers should pay attention to those, to avoid nasty feedbacks
const int REC_PlugReserved      =4096;  // free to use by plugins



// event ID's
const int FPE_Tempo             =0;     // FLOAT tempo in value (need to typecast), & average samples per tick in Flags (DWORD) (warning: can be called from the mixing thread) (GM)
const int FPE_MaxPoly           =1;     // max poly in value (infinite if <=0) (only interesting for standalone generators)
// since MIDI plugins, or other plugin wrappers won't support the voice system, they should be notified about channel pan, vol & pitch changes
const int FPE_MIDI_Pan          =2;     // MIDI channel panning (0..127) in EventValue, FL panning in -64..+64 in Flags (warning: can be called from the mixing thread) (GM)
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
const int FPV_GetInfo 			=7; 	// retrieve info about the voice (some of which also available above) (PVoiceInfo in EventValue)

// TVoiceInfo.Flags
const int VoiceInfo_FromPattern	=1;	// voice is received from score, not played live

// Voice_Render function results
const int FVR_Ok                =0;
const int FVR_NoMoreData        =1;     // for sample streaming, when there's no more sample data to fill any further buffer (the voice will then be killed by the host)

// Tool indexes for FHD_OpenTool
const int OTI_ControlCreator =0;        // open Control Creator, no params
const int OTI_Diagnostics 	 =1; // open Diagnostics, no params

// FPD_LoadFile types
const int LFT_Generic          =0;
const int LFT_Patcherize       =1;
const int LFT_DroppedFile      =2;
const int LFT_DownloadDataPack =3;

// FHD_GetMixingTime / FHD_GetLocalTime flags
const int GT_Beats          = 0;          // beats
const int GT_AbsoluteMS     = 1;          // absolute milliseconds
const int GT_RunningMS      = 2;          // running milliseconds
const int GT_MSSinceStart   = 3;          // milliseconds since soundcard restart
const int GT_Ticks          = 4;          // ticks
const int GT_LocalTime      = 1 << 31;   // time relative to song start

const int GT_FlagsMask      = 0xFFFFFF00;
const int GT_TimeFormatMask = 0x000000FF;

// PromptEdit_Ex option values
const int PEO_Text     	= 1 << 0;
const int PEO_Icon     	= 1 << 1;
const int PEO_Color    	= 1 << 2;
const int PEO_Gradient 	= 1 << 3;
const int PEO_Presets  = 1 << 4;
const int PEO_Password = 1 << 5;

// external media dialogs flags
const int EMD_SearchImages 	= 1;
const int EMD_SearchVideos 	= 2;
const int EMD_DownloadFile 	= 4;

// FPD_GetParamType options
const int PT_Default = 0;
const int PT_Db = 1;
const int PT_Hz = 2;
const int PT_Centered = 3;
const int PT_Ms = 4;
const int PT_Percent = 5;
const int PT_Time = 6;
const int PT_Value = 7;
const int PT_Number = 8;
const int PT_Text = 9;

const int PT_Last = PT_Text;

// host dispatcher ID's
const int FHD_ParamMenu         =0;     // the popup menu for each control (Index=param index, Value=popup item index (see FHP_EditEvents))
const int FHD_GetParamMenuFlags =1;     // [OBSOLETE, see FHD_GetParamMenuEntry] before the popup menu is shown, you must ask the host to tell if items are checked or disabled (Index=param index, Value=popup item index, Result=flags (see FHP_Disabled))
const int FHD_EditorResized     =2;     // to notify the host that the editor (EditorHandle) has been resized
const int FHD_NamesChanged      =3;     // to notify the host that names (GetName function) have changed, with the type of names in Value (see the FPN_ constants)
const int FHD_ActivateMIDI      =4;     // makes the host enable its MIDI output, useful when a MIDI out plugin is created (but not useful for plugin wrappers)
const int FHD_WantMIDIInput     =5;     // plugin wants to be notified about MIDI messages (for processing or filtering) (switch in Value)
const int FHD_WantMIDITick      =6;     // plugin wants to receive MIDITick events, allowing MIDI out plugins (not used yet)
const int FHD_LocatePlugin      =7;     // ask the host to find a plugin, pass the simple filename in Value, full path is returned as Result (both PAnsiChar). Set Index to 1 if you want host to show a warning if plugin could not be found.
const int FHD_KillAutomation    =8;     // ask the host to kill the automation linked to the plugin, for params # between Index & Value (included) (can be used for a trial version of the plugin)
const int FHD_SetNumPresets     =9;     // tell the host how many (Value) internal presets the plugin supports (mainly for wrapper)
const int FHD_SetNewName        =10;    // sets a new short name for the parent (PChar in Value)
const int FHD_VSTiIdle          =11;    // used by the VSTi wrapper, because the dumb VSTGUI needs idling for his knobs
const int FHD_SelectChanSample  =12;    // ask the parent to open a selector for its channel sample (see FPF_UseChanSample)
const int FHD_WantIdle          =13;    // plugin wants to receive the idle message (enabled by default) (Value=0 for disabled, 1 for enabled when UI is visible, 2 for always enabled)
const int FHD_LocateDataFile    =14;    // ask the host to search for a file in its search paths, pass the simple filename in Value, full path is returned as Result (both PChar) (Result doesn't live long, please copy it asap)
const int FHD_ShowPlugSelector  =15;    // ask the host to show the plugin selector (Index: see SPSF flags)
const int FHD_TicksToTime       =16;    // translate tick time (Value) into Bar:Step:Tick (PSongTime in Index) (warning: it's *not* Bar:Beat:Tick)
const int FHD_AddNotesToPR      =17;    // add a note to the piano roll, PNotesParams in Value
const int FHD_GetParamMenuEntry =18;    // before the popup menu is shown, you must fill it with the entries set by the host (Index=param index, Value=popup item index (starting from 0), Result=PParamMenuEntry, or null pointer if no more entry)
const int FHD_MsgBox            =19;    // make fruity show a message box (PChar in Index [formatted as 'Title|Message'], flags in Value (MB_OkCancel, MB_IconWarning, etc.), result in IDOk, IDCancel format (as in TApplication.MessageBox)
const int FHD_NoteOn            =20;    // preview note on (semitone in Index low word, color in index high word (0=default), velocity in Value)
const int FHD_NoteOff           =21;    // preview note off (semitone in Index, color in index high word, velocity in Value (-1=default otherwise 0..127))
const int FHD_OnHint_Direct     =22;    // same as OnHint, but show it immediately (to show a progress while you're doing something) (PChar in Value)
const int FHD_SetNewColor       =23;    // sets a new color for the parent (color in Value) (see FHD_SetNewName);
const int FHD_GetInstance       =24;    // (Windows) returns the module instance of the host (could be an exe or a DLL, so not the process itself)
const int FHD_KillIntCtrl       =25;    // ask the host to kill anything linked to an internal controller, for # between Index & Value (included) (used when undeclaring internal controllers)
const int FHD_CheckProdCode     =26;    // reserved
const int FHD_SetNumParams      =27;    // override the # of parameters (for plugins that have a different set of parameters per instance) (number of parameters in Value)
const int FHD_PackDataFile      =28;    // ask the host to pack an absolute filename into a local filemane, pass the simple filename in Value, packed path is returned as Result (both PChar) (Result doesn't live long, please copy it asap)
const int FHD_GetPath           =29;    // ask the host for a path specified by Index (see GP_ constants) (returned as Result)
const int FHD_SetLatency        =30;    // set plugin latency, if any (samples in Value)
const int FHD_CallDownloader    =31;    // call the presets downloader (optional plugin name PAnsiChar in Value)
const int FHD_EditSample		=32;	// edits sample in Edison (PChar in Value, Index=1 means an existing Edison can be re-used)
const int FHD_SetThreadSafe     =33;    // plugin is thread-safe, doing its own thread-sync using LockMix_Shared (switch in Value)
const int FHD_SmartDisable      =34;    // plugin asks FL to exit or enter smart disabling (if currently active), mainly for generators when they get MIDI input (switch in Value)
const int FHD_SetUID            =35;    // sets a unique identifying string for this plugin. This will be used to save/restore custom data related to this plugin. Handy for wrapper plugins. (PChar in Value)
const int FHD_GetMixingTime     =36;    // get mixer time, Index is the time format required (see GT_... constants). Value is a pointer to a TFPTime, which is filled with an optional offset in samples
const int FHD_GetPlaybackTime   =37;    // get playback time, same as above
const int FHD_GetSelTime        =38;    // get selection time in t & t2, same as above. Returns 0 if no selection (t & t2 are then filled with full song length).
const int FHD_GetTimeMul        =39;    // get current tempo multiplicator, that's not part of the song but used for fast-forward
const int FHD_Captionize        =40;    // captionize the plugin (useful when dragging) (captionized in Value)
const int FHD_SendSysEx         =41;    // send a SysEx string (pointer to array in Value, the first integer being the length of the string, the rest being the string), through port Index, immediately (do not abuse)
const int FHD_LoadAudioClip     =42;    // send an audio file to the playlist as an audio clip, starting at the playlist selection. Options in Index (see LAC_ constants). FileName as PAnsiChar in Value.
const int FHD_LoadInChannel     =43;    // send a file to the selected channel(s) (mainly for Edison), FileName as PChar in Value
const int FHD_ShowInBrowser     =44;    // locates the file in the browser & jumps to it (Index is one of SIB_ constants, PAnsiChar filename in Value)
const int FHD_DebugLogMsg       =45;    // adds message to the debug log (PChar in Value)
const int FHD_GetMainFormHandle =46;    // gets the handle of the main form (HWND in Value, 0 if none)
const int FHD_GetProjDataPath   =47;    // [OBSOLETE - use FHD_GetPath instead] ask the host where the project data is, to store project data (returned as Result)
const int FHD_SetDirty          =48;    // mark project as dirty (not required for automatable parameters, only for tweaks the host can't be aware of)
const int FHD_AddToRecent       =49;    // add file to recent files (PChar in Value)
const int FHD_GetNumInOut       =50;    // ask the host how many inputs (Index=0) are routed to this effect (see GetInBuffer), or how many outputs (Index=1) this effect is routed to (see GetOutBuffer)
const int FHD_GetInName         =51;    // ask the host the name of the input Index (!!! first = 1), in Value as a PNameColor, Result=0 if failed (Index out of range)
const int FHD_GetOutName        =52;    // ask the host the name of the ouput Index (!!! first = 1), in Value as a PNameColor, Result=0 if failed (Index out of range)
const int FHD_ShowEditor        =53;    // make host bring plugin's editor (visibility in Value, -1 to toggle)
const int FHD_FloatAutomation   = 54;   // (for the plugin wrapper only) ask the host to turn 0..FromMIDI_Max automation into 0..1 float, for params # between Index & Value (included)
const int FHD_ShowSettings      =55;    // called when the settings button on the titlebar should be updated switched. On/off in Value (1=active). See FPF_WantSettingsBtn
const int FHD_NoteOnOff         =56;    // generators only! note on/off (semitone in Index low word, color in index high word, NOT recorded in bit 30, velocity in Value (<=0 = note off))
const int FHD_ShowPicker        =57;    // show picker (mode [0=plugins, 1=project] in Index, categories [gen=0/FX=1/both=-1/Patcher (includes VFX)=-2] in Value)
const int FHD_GetIdleOverflow   =58;    // ask the host for the # of extra frames Idle should process, generally 0 if no overflow/frameskip occured
const int FHD_ModalIdle         =59;    // used by FL plugins, when idling from a modal window, mainly for the smoothness hack
const int FHD_RenderProject     =60;    // prompt the rendering dialog in song mode
const int FHD_GetProjectInfo    =61;    // get project title, author, comments, URL, naked filename (Index), (returned as Result as a *PWideChar*)
const int FHD_ForceDetached     =62;    // used by Wrapper in OSX to force the plugin form to be detached
const int FHD_StartDrag         =63;    // sent by Patcher when starting dragging a preset
const int FHD_EndDrag           =64;    // sent by Patcher when finished dragging a preset
const int FHD_PreviewKey        =65;    // chance for host to handle keyboard messages, Index=flags in lower 16 bits (see KUD constants) and virtual key in second 16 bits, Value=KeyData from WM_KeyUp or WM_KeyDown message (0 if not available), returns 1 if handled and 0 if not
const int FHD_RenderWindowBitmap=66;    // used by ZgeViz
const int FHD_UpdateStealKBFocus=67;    // the plugin will steal kb input or not (Value is 1 or 0)
							  //=68;    // [OBSOLETE]
const int FHD_GetPluginMenuMode =69;    // returns the view mode of the favorite plugin menus in FL: 0=categories 1=tree 2=flat
const int FHD_OpenTool          =70;    // open application in System\Tools folder. Index=tool to start (see OTI_ControlCreator), Value=PAnsiChar with command line params
const int FHD_GetPathManager	=71;	// returns IPathManager instance (pointer)
const int FHD_RegisterSideInput =72;	// let the host know that you intend to use a sidechained input, so latency can be calculated. Index=input index (first=1), Value=see RSIO_ constants
const int FHD_RegisterSideOutput=73;	// let the host know that you intend to use a sidechained output, so latency can be calculated. Index=output index (depends on use of GetInsBuffer or GetOutBuffer), Value=see RSIO_ constants

const int FHD_ReportError		=74; 	// report error during plugin load (will show combined dialog for all missing plugins after project is loaded or MsgBox in case we are adding plugin to project)
const int FHD_ShowStandardParamMenu=75; // ask FL to pop up a parameter menu, so the plugin doesn't have to implement it itself. Index is the parameter index.
const int FHD_GetContextInfo	=76; 	// get information about various things. Index is the information type (see CI_ constants), Value and result depend on the type
const int FHD_SetContextInfo	=77; 	// change some piece of context information. Index is the information type (see CI_ constants), Value and result depend on the type
const int FHD_GetExternalMedia	=78;    // set Flags (bits) as index, for example : EMD_SearchImages or EMD_DownloadFile to search and download images
const int FHD_Transport         =79;    // allows the plugin to control FL through some of the messages in GenericTransport. Index=message, Value=release/switch/hold value. Currently only FPT_Play and FPT_Stop are supported. Returns -1 if can't be handled, 0 if not handled, 1 if handled by focused plugin, 2 if handled by focused form, 4 if handled by menu, 5 if delayed, 8 if handled globally.
const int FHD_DownloadMissing   =80;    // notify FL about missing data pack
const int FHD_DownloadFinished  =81;    // notify FL that a pack download is finished
const int FHD_DebugBuild        =82;    // tell FL to show a [DEBUG] warning in the plugin window caption. Value is 0 (release) or 1 (debug)
const int FHD_PickVoiceColor    =83;    // Show the piano roll's color picker. Index = screen co-ordinates with x in first 2 bytes and y in next 2 bytes, Value = current color number (not an RGB value). Will call FPD_ColorWasPicked when the user selects a color.
const int FHD_GetColorRGBValue  =84;    // Get the RGB value for a color in a palette. Index is the color palette (see CP_ constants for available palettes). Value is the index in the palette. If Value is -1, this returns the count of colors in the palette.
const int FHD_ShowException     =85;    // Show application exception. Index is Exception.Message string. Value is Stack-trace string.
const int FHD_GetTranslationMoFile =86; // Get the current translation object (for Plugins)
const int FHD_PresetSelected    =87;    // tell the host internal preset is changed

     



// param popup menu item indexes (same order as param menu in FL)
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


// FHD_KeyUpDown flags
const int KUD_Down         =1 << 0;
const int KUD_Shift        =1 << 1;
const int KUD_Control      =1 << 2;
const int KUD_Alt          =1 << 3;
const int KUD_Command      =1 << 4;

// FHD_ShowPlugSelector flags (for Index parameter)
const int SPSF_Effects    = 1 << 0;
const int SPSF_Generators = 1 << 1;
const int SPSF_MoreWindow = 1 << 2;

// FHD_ShowInBrowser flags (for Index parameter)
const int SIB_FileName         = 0;   // Value parameter is a filename to show
const int SIB_GenPluginPresets = 1;   // Value parameter is the name of a generator plugin for which to show the presets
const int SIB_EffPluginPresets = 2;   // Value parameter is the name of an effect plugin for which to show the presets
const int SIB_GenPluginFileName = 3;  // Value parameter is a filename to show, plugin is generator
const int SIB_EffPluginFileName = 4;  // Value parameter is a filename to show, plugin is effect

// FHD_GetPath values (for Index parameter)
const int GP_ProgPath            = 0;   // the location of the engine, which may NOT be where the executable is, but where the data path will be
const int GP_ProjDataPath        = 1;   // the location of the project data, to store project data
const int GP_PluginFavPath       = 2;   // the full path to generators (Value=1) or effect (Value=0) favorite presets path (plugin database)
const int GP_PluginPresetPath    = 3;   // the path containing the plugin's own presets
const int GP_GenPluginPresetPath = 4;   // the preset path for a specific generator plugin, plugin UID in Value
const int GP_EffPluginPresetPath = 5;   // the preset path for a specific effect plugin, plugin UID in Value
const int GP_PluginDatabase      = 6;	// the root of the plugin database folder
const int GP_SharedDlls          = 7;   // the location of the shared dlls

// FHD_GetProjectInfo, FPD_ProjectInfoChanged Index values
const int GPI_Title = 0;
const int GPI_Author = 1;
const int GPI_Comments = 2;
const int GPI_URL = 3;
const int GPI_FileName = 4;

// FHD_RenderProject Index flags
const int RPF_UsesFLWindows = 1;
const int RPF_IsVideoExport = 2;
const int RPF_IsVideoUpload = 4;

// path indexes for the host IPathManager (see FHD_GetPathManager)
const int PI_User = 0;
const int PI_Factory = 1;

const int PathIndexesConst[2] = { PI_User, PI_Factory };
const char PathIndexNamesConst[2][8] = { "User", "Factory"};

// data path identifiers for the host IPathManager (see FHD_GetPathManager)
const int FLData_Loops = 1000;
const int FLData_Backup = 1001;
const int FLData_AutoSave = 1002;
const int FLData_Logs = 1003;
const int FLData_Settings = 1004;
const int FLData_AboutURL = 1005;
const int FLData_Screenshots = 1006;
const int FLData_Recorded = 1007;
const int FLData_Rendered = 1008;
const int FLData_SlicedBeats = 1009;
const int FLData_ClipboardFiles = 1010;
const int FLData_PluginPresets = 1011;
const int FLData_Thumbnails = 1012;
const int FLData_Templates = 1013;
const int FLData_ChannelPresets = 1015;
const int FLData_MixerPresets = 1016;
const int FLData_Scores = 1017;
const int FLData_ProjectBones = 1018;
const int FLData_EffectPlugins = 1019; // use the value returned by GetSettingsSubPath as the first sub folder!
const int FLData_GeneratorPlugins = 1020; // use the value returned by GetSettingsSubPath as the first sub folder!
const int FLData_Internet = 1021;
const int FLData_PluginDatabase = 1022;
const int FLData_Tools = 1023;
const int FLData_BrowserConfig = 1024;
const int FLData_SystemData = 1025;
const int FLData_NameLists = 1026;
const int FLData_TypingToPiano = 1027;
const int FLData_ToolbarPresets = 1028;
const int FLData_TouchKbPresets = 1029;
const int FLData_HardwareSpecific = 1030;
const int FLData_Mapping = 1031;
const int FLData_GenericMapping = 1032;
const int FLData_Styles = 1033;
const int FLData_Help = 1034;
const int FLData_Speech = 1035;
const int FLData_Envelopes = 1036;
const int FLData_Packs = 1037;
const int FLData_NewPluginArchive = 1038;
const int FLData_Languages = 1039;
const int FLData_Impulses = 1040;
const int FLData_NoteColorPresets = 1041;

// values for the Value parameter of FHD_RegisterSideInput and FHD_RegisterSideOutput
const int RSIO_Unregister = 0; // unregister a previously registered I/O
const int RSIO_Register = 1 << 0; // register a new I/O as used

// flags for the Value parameter of FPD_RoutingChanged
const int RCV_NamesChanged = 1 << 0; // only one or more track names have changed
const int RCV_SideIOCleared = 1 << 1; // if the plugin uses any side I/O, the host has unregistered them all and the plugin should re-register them.

const int RCV_Default = RCV_NamesChanged;

//values for FHD_ReportError
const int RE_OpeningUnknown = 0;
const int RE_OpeningEffectIntoChannel = 1;
const int RE_UnknownFormat = 2;
const int RE_DxPlugNotFound = 3;
const int RE_UnknownPluginType = 4;
const int RE_PluginCrashOnSettings = 5;

// result values (errors) for FPD_WrapPlugin
const int PE_None = 0;
const int PE_UnknownError = -1;
const int PE_ExceptionWhenOpening = -2;
const int PE_NotStereo = 10;
//const int PE_VST_NoReplacing=11;
//const int PE_VST_BadVersion =12;
//const int PE_VST_VST2       =13;
//const int PE_DX_NotSupported=20;
const int PE_NotFound = 30;
const int PE_NotSupported = 40; // invalid PlugClass
const int PE_VFXinFL = 50;
const int PE_FileNotFound = 60;
// values from FHD_ReportError mapped to PE messages
const int PE_RE_OpeningUnknown = 70;
const int PE_RE_OpeningEffectIntoChannel = 71;
const int PE_RE_UnknownFormat = 72;
const int PE_RE_DxPlugNotFound = 73;
const int PE_RE_UnknownPluginType = 74;
const int PE_RE_PluginCrashOnSettings = 75;

// context info types
// requested info is returned as the result of the function and passed in Value to set it (unless specified otherwise)
const int CI_TrackName = 0; // (R/W) PAnsiChar encoded as UTF-8
const int CI_TrackIndex = 1; // (R)
const int CI_TrackColor = 2; // (R/W) color is RGBA
const int CI_TrackSelected = 3; // (R/W) the track is selected (0=false 1=true, 2=selected with other tracks)
const int CI_TrackFocused = 4; // (R) the track is focused for user input (0=false 1=true)
const int CI_TrackIsOutput = 5; // (R) the track sends directly to an audio device output (0=false, 1=true)
const int CI_TrackVolume = 6; // (R/W) (float+string) the value of the tracks' volume slider. Info is floating point (single / float) cast to an int32
const int CI_TrackPan = 7; // (R/W) (float+string) the value of the track's panning knob, as a single / float (-1..1) cast to int32
const int CI_TrackMuteSolo = 8; // (R/W) flags indicate mute and solo state for a track (see CIMS_ constants)
const int CI_TrackSendCount = 9; // (R) returns the send count for the plugin's track
const int CI_TrackSendLevel = 10; // (R/W) (float+string) get or set the level for a specific send of this track. On read, Value holds the send index. On write, Value holds a pointer to a TContextInfo record with the new value in FloatValue.
const int CI_TrackMaxVolume = 11; // (R) get the maximum value for mixer track volume
const int CI_TrackMaxSendLevel = 12; // (R) get the maximum value for mixer track send level
// flags to be included with info type (e.g. CI_TrackName or CI_AsString), for info that can be both a string and an int or float value
const int CI_AsValue = 1 << 0;
const int CI_AsString = 1 << 31;
// flags for CI_TrackMuteSolo
const int CIMS_Muted = 1 << 0;
const int CIMS_Solo = 1 << 1;

// color palettes for FHD_GetColorValue and other palette functions
const int CP_VoiceColors = 0;

// options for FHD_LoadAudioClip
const int LAC_Stretch_Default = 0;
const int LAC_Stretch_Auto = 1;
const int LAC_Stretch_Resample = 2;
const int LAC_Stretch_Stretching = 3;
const int LAC_Stretch_E3ProDefault = 4;
const int LAC_Stretch_E3Mono = 5;
const int LAC_Stretch_Slices = 6;
const int LAC_Stretch_SliceMap = 7;

const int LAC_StretchMask = 0x0000000F; // stretch options are in the first 4 bits

//---------------------------------------------------------------------------
#endif   // FP_PLUGCLASS_H