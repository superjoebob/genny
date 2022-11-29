#pragma once
#include <vector>
#include <map>
#include <string>
#include "base\VSTBase.h"

enum class AutomationTypeFlags
{
	UpdateValue = 1,		// update the value
	GetValue = 2,			// retrieves the value
	ShowHint = 4,			// updates the hint (if any)
	UpdateControl = 16,		// updates the wheel/knob
	FromMIDI = 32,			// value from 0 to 65536 has to be translated (& always returned, even if REC_GetValue isn't set)
	NoLink = 1024,			// don't check if wheels are linked (internal to plugins, useful for linked controls)
	InternalCtrl = 2048,	// sent by an internal controller - internal controllers should pay attention to those, to avoid nasty feedbacks
	PlugReserved = 4096,    // free to use by plugins
	UpdateUIThreaded = 8192,
	GetValueUI = 524288,
};

const int kPresetControlIndex = 980546;
const int kInstrumentControlIndex = 980547;
const int kMidiChannelStart = 980550;
const int kMidiChannelEnd = 980566;
const int kGlobalChannelStart = 980570;
const int kGlobalChannelEnd = 980580;
const int kChannelEnableStart = 980590;
const int kChannelEnableEnd = 980600;
const int kSelectedInstrument = 980610;
const int kInstrumentMappingStart = 980620;
const int kInstrumentMappingEnd = 980636;
const int kOctaveStart = 980637;
const int kOctaveEnd = 980700;
const int kTransposeStart = 980701;
const int kTransposeEnd = 980800;
const int kPanningStart = 980801;
const int kPanningEnd = 980900;

const int kRangeLowStart = 980901;
const int kRangeLowEnd = 981200;

const int kRangeHighStart = 981201;
const int kRangeHighEnd = 981400;

const int kDelayStart = 981401;
const int kDelayEnd = 981500;


const int kOperatorVelocityStart = 981600;
const int kOperatorVelocityEnd = 981604;


class VSTBase;
class IndexBaron;
class IBIndex;
struct VSTPatch
{
public:
	VSTPatch() {}
	VSTPatch(const std::string& name) : Name(name) {}

	std::string Name;
	std::string Prefix;
	virtual void catalogue(IndexBaron* baron) = 0;
	virtual void setFromBaron(IBIndex* param, float val) = 0;
	virtual float getFromBaron(IBIndex* param) = 0;
};

class VirtualInstrument
{
public:
	VirtualInstrument(void);
	~VirtualInstrument(void);
	virtual void initialize() {};
	VSTBase* initializeBase(void* data);
	virtual void sampleRateChanged(double newRate) { }

	virtual int getPluginState (void** data, bool isPreset = false) { return 0; }
	virtual int setPluginState (void* data, int size, bool isPreset = false) { return 0; }

	virtual float getParameter(int index, VSTPatch* patch = nullptr) = 0;
	virtual int getParameterRange(int index) = 0;
	virtual void getParameterName (int index, char* text);
	virtual void getParameterValue (int index, char* text);
	virtual void getChannelName(int index, char* text);
	virtual void setParameter(int index, float value, VSTPatch* patch = nullptr) = 0;
	virtual void setPatchIndex(int index);
	virtual void setCurrentPatch(VSTPatch* patch) = 0;
	void setTempCurrentPatchIndex(int index) { _currentPatch = _patches[index];}
	void setTempCurrentPatch(VSTPatch* patch) { _currentPatch = patch;}

	virtual VSTPatch* getCurrentPatch() { return _currentPatch; }
	virtual void getSamples(float** buffer, int numSamples) = 0;

#if BUILD_VST
	void setEditor(VSTEditor* editor);
#endif

	int getNumPatches() const { return (int)_patches.size(); }
	virtual VSTPatch* getPatch(int idx) { return _patches[idx]; }
	int getPatchIndex(VSTPatch* patch);
	std::vector<VSTPatch*> getPatches() { return _patches; }
	std::vector<VSTPatch*> getPatches(std::string category);
	virtual void setProgramName(char* name);

	//virtual int RunNewAutomation(int Index, int Value, int RECFlags) { return 0; }
	virtual int onAutomation(int parameterHash, int value, AutomationTypeFlags type = (AutomationTypeFlags)((int)AutomationTypeFlags::UpdateValue | (int)AutomationTypeFlags::UpdateControl)) { return 0; }

	virtual void MidiLearn(int paramTag, int legacyTag);
	virtual int MidiForget(int paramTag, int legacyTag);
	virtual void onMidiMessage(int channel, int message, int value) { };
	virtual void midiTick() { };
	virtual void midiOut(unsigned char pStatus, unsigned char pData1, unsigned char pData2, unsigned char pPort);
	virtual void midiFlush();

	virtual void noteOn(int note, float velocity, unsigned char channel, float panning, void* noteData = nullptr) = 0;
	virtual void updateNote(void* noteData, int samples){};
	virtual void noteOff(int note, unsigned char channel, void* noteData = nullptr) = 0;
	virtual void clearNotes() = 0;

	virtual void pitchChanged(int channel, int pitch) {}

	virtual std::string getEffectName() { return "GenericVST"; }
	virtual std::string getProductName() { return "GenericVST"; }
	virtual std::string getAuthorName() { return "Some Dude"; }

	virtual int getTotalPatchCount() = 0;
	virtual void setMasterVolume(float volume) {}

	virtual void saveData(IStream* stream, bool save) {}
	float getTempo() { return _base->getTempo(); }
	float getSamplesPerTick() { return (float)_base->getSamplesPerTick(); }
	VSTBase* getBase() { return _base;}
	
	std::vector<VSTPatch*> _patches;
	std::map<int, std::vector<int>> _midiLearn;
	bool _playingStatusChanged;
	float _globalPitchOffset[16];
protected:
	VSTPatch* _currentPatch;
	VSTBase* _base;
};

