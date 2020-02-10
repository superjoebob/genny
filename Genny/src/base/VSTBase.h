#pragma once
#ifdef BUILD_VST
//#include "public.sdk/source/vst2.x/audioeffectx.h"
#include "plugin-bindings/aeffguieditor.h"
typedef AEffGUIEditor VSTEditor;

class VirtualInstrument;
class VSTBase : public AudioEffectX
{
public:
	VSTBase(VirtualInstrument* parent, void* data);
	~VSTBase(void);

	virtual void initialize();

	virtual void process (float** inputs, float** outputs, VstInt32 sampleFrames);
	virtual void processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames);
	virtual VstInt32 processEvents (VstEvents* events);
	virtual VstIntPtr dispatcher(VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt);

	virtual VstInt32 getChunk (void** data, bool isPreset = false);
	virtual VstInt32 setChunk (void* data, VstInt32 byteSize, bool isPreset = false);

	virtual float getParameter (VstInt32 index);
	virtual void getParameterName (VstInt32 index, char* text);
	virtual void getParameterDisplay (VstInt32 index, char* text);
	virtual void setParameter (VstInt32 index, float value);
	virtual void setProgram(VstInt32 program);
	virtual void setProgramName (char* name);
	virtual void getProgramName (char* name);
	virtual bool getProgramNameIndexed (VstInt32 category, VstInt32 index, char* text);

	virtual bool getEffectName (char* name);
	virtual bool getProductString (char* text);
	virtual bool getVendorString (char* text);

	virtual void MidiLearn(int paramTag);
	virtual void MidiForget(int paramTag);
	virtual void MidiOut(unsigned char pStatus, unsigned char pData1, unsigned char pData2, unsigned char pPort);
	virtual void MidiFlush();

	virtual VstInt32 getNumMidiInputChannels();
	virtual VstInt32 getNumMidiOutputChannels();
	virtual VstInt32 getMidiProgramName (VstInt32 channel, MidiProgramName* midiProgramName);
	virtual VstInt32 getCurrentMidiProgram (VstInt32 channel, MidiProgramName* currentProgram);
	virtual VstInt32 getMidiProgramCategory (VstInt32 channel, MidiProgramCategory* category);
	virtual bool hasMidiProgramsChanged (VstInt32 channel);
	virtual bool getMidiKeyName (VstInt32 channel, MidiKeyName* keyName);
	int getSamplesPerTick() { return 1; }

	virtual bool getOutputProperties(VstInt32 index, VstPinProperties* properties);
	virtual VstInt32 canDo (char* text);

	float getTempo() { return _tempo; }
	int _midiLearnParameter;
	int _midiForgetParameter;

protected:
	void FillProgram (VstInt32 channel, VstInt32 prg, MidiProgramName* mpn);
	VirtualInstrument* _parent;
	VstInt32 _channelPrograms[16];
	float _tempo;

	VstEvents* _currentEvents;
};
#else


#include "fp_cplug.h"
#include <vector>
typedef void* VSTEditor;

struct FruityPlugInfo
{
	int Tag;
	TFruityPlugHost* Host;
};

struct PlugVoice {
	int HostTag;
	PVoiceParams Params;
	//int Pos[nOsc];
	int State;
};

class VirtualInstrument;
class PluginGUIEditor;
struct VSTPatch;
class VSTBase : public TCPPFruityPlug
{
public:
	VSTBase(VirtualInstrument* parent, void* data);
	~VSTBase(void);

	virtual void initialize();

	virtual void _stdcall Gen_Render(PWAV32FS DestBuffer, int &Length);
	//virtual void process (float** inputs, float** outputs, int sampleFrames);
	//virtual void processReplacing (float** inputs, float** outputs, int sampleFrames);
	virtual void setProgram(int program);

	virtual void _stdcall SaveRestoreState(IStream *Stream, BOOL Save);
	virtual TVoiceHandle _stdcall TriggerVoice(PVoiceParams VoiceParams, intptr_t SetTag);
	virtual void _stdcall Voice_Release(TVoiceHandle Handle);
	virtual void _stdcall Voice_Kill(TVoiceHandle Handle);
	void KillAllVoices();
	virtual int _stdcall Voice_ProcessEvent(TVoiceHandle Handle, int EventID, int EventValue, int Flags);
	virtual int _stdcall Voice_Render(TVoiceHandle Handle, PWAV32FS DestBuffer, int &Length){return 0;}

	virtual intptr_t _stdcall Dispatcher(intptr_t ID, intptr_t Index, intptr_t Value);
	virtual void _stdcall GetName(int Section, int Index, int Value, char *Name);
	virtual int _stdcall ProcessEvent(int EventID, int EventValue, int Flags);
	virtual int _stdcall ProcessParam(int Index, int Value, int RECFlags);
	virtual void _stdcall Idle_Public();

	virtual void _stdcall Eff_Render(PWAV32FS SourceBuffer, PWAV32FS DestBuffer, int Length) {}
	virtual void _stdcall NewTick();
	virtual void _stdcall MIDITick() {}
	virtual void _stdcall MIDIIn(int &Msg);
	virtual void _stdcall MsgIn(int Msg);
	virtual void MidiOut(unsigned char pStatus, unsigned char pData1, unsigned char pData2, unsigned char pPort);
	virtual void MidiFlush();


	virtual void setEditor(void* editor);

	void sendMidiMessage(char cc, int value, int channel, int port);
	virtual float getParameter (int index, VSTPatch* patch = nullptr);
	virtual void setParameter (int index, float value, VSTPatch* patch = nullptr);

	float getTempo() { return _tempo; }
	int getSamplesPerTick() { return _samplesPerTick; }

	/*virtual VstInt32 processEvents (VstEvents* events);

	virtual float getParameter (VstInt32 index);
	virtual void setParameter (VstInt32 index, float value);
	virtual void setProgram(VstInt32 program);
	virtual void setProgramName (char* name);
	virtual void getProgramName (char* name);
	virtual bool getProgramNameIndexed (VstInt32 category, VstInt32 index, char* text);

	virtual bool getEffectName (char* name);
	virtual bool getProductString (char* text);
	virtual bool getVendorString (char* text);

	virtual VstInt32 getNumMidiInputChannels();
	virtual VstInt32 getNumMidiOutputChannels();
	virtual VstInt32 getMidiProgramName (VstInt32 channel, MidiProgramName* midiProgramName);
	virtual VstInt32 getCurrentMidiProgram (VstInt32 channel, MidiProgramName* currentProgram);
	virtual VstInt32 getMidiProgramCategory (VstInt32 channel, MidiProgramCategory* category);
	virtual bool hasMidiProgramsChanged (VstInt32 channel);
	virtual bool getMidiKeyName (VstInt32 channel, MidiKeyName* keyName);

	virtual bool getOutputProperties(VstInt32 index, VstPinProperties* properties);
	virtual VstInt32 canDo (char* text);*/

protected:
	VirtualInstrument* _parent;

	PluginGUIEditor* _editor;
	TFruityPlugHost* _host;

	std::vector<PlugVoice*> _voices;
	float _tempo;
	int _samplesPerTick;
	int _totalParameters;
};
#endif

