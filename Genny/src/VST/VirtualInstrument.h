#pragma once
#include <vector>
#include <map>
#include "base\VSTBase.h"

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

	virtual int getPluginState (void** data, bool isPreset = false) { return 0; }
	virtual int setPluginState (void* data, int size, bool isPreset = false) { return 0; }

	virtual float getParameter(int index, VSTPatch* patch = nullptr) = 0;
	virtual int getParameterRange(int index) = 0;
	virtual void getParameterName (int index, char* text);
	virtual void getParameterValue (int index, char* text);
	virtual void setParameter(int index, float value, VSTPatch* patch = nullptr) = 0;
	virtual void setPatchIndex(int index);
	virtual void setCurrentPatch(VSTPatch* patch) = 0;
	void setTempCurrentPatchIndex(int index) { _currentPatch = _patches[index];}
	void setTempCurrentPatch(VSTPatch* patch) { _currentPatch = patch;}

	virtual VSTPatch* getCurrentPatch() { return _currentPatch; }
	virtual void getSamples(float** buffer, int numSamples) = 0;
	void setEditor(VSTEditor* editor);

	int getNumPatches() const { return _patches.size(); }
	virtual VSTPatch* getPatch(int idx) { return _patches[idx]; }
	int getPatchIndex(VSTPatch* patch);
	std::vector<VSTPatch*> getPatches() { return _patches; }
	std::vector<VSTPatch*> getPatches(std::string category);
	virtual void setProgramName(char* name);

	virtual void onMidiMessage(int channel, int message, int value) { };
	virtual void midiTick() { };

	virtual void noteOn(int note, float velocity, unsigned char channel, float panning, void* noteData = nullptr) = 0;
	virtual void updateNote(void* noteData){};
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
	float getSamplesPerTick() { return _base->getSamplesPerTick(); }
	VSTBase* getBase() { return _base;}
	
	std::vector<VSTPatch*> _patches;
	std::map<int, std::vector<int>> _midiLearn;
protected:
	VSTPatch* _currentPatch;
	VSTBase* _base;
};

