#pragma once
#include "VirtualInstrument.h"

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


class Genny2612;
class GennyInterface;
struct GennyPatch;
class GennyVST : public VirtualInstrument
{
public:
	GennyVST(void);
	~GennyVST(void);
	virtual void initialize();

	virtual int getPluginState (void** data, bool isPreset = false);
	virtual int setPluginState (void* data, int size, bool isPreset = false);

	virtual float getParameter(int index, VSTPatch* patch = nullptr);
	virtual int getParameterRange(int index);
	virtual void getParameterName(int index, char* text);
	virtual void getParameterValue(int index, char* text);
	virtual void setParameter(int index, float value, VSTPatch* patch = nullptr);
	virtual void setProgramName(char* name);

	void updateChannel(int channel, bool on);

	virtual void setCurrentPatch(VSTPatch* patch);
	virtual void getSamples(float** buffer, int numSamples);

	virtual void noteOn(int note, float velocity, unsigned char channel, float panning, void* noteData = nullptr);
	virtual void updateNote(void* noteData);
	virtual void noteOff(int note, unsigned char channel, void* noteData = nullptr);
	virtual void clearNotes();

	virtual std::string getEffectName() { return "Genny Dev"; }
	virtual std::string getProductName() { return "Genny Dev"; }
	virtual std::string getAuthorName() { return "Landon Podbielski"; }

	virtual int getTotalPatchCount();
	int getNumInstruments();
	virtual void setMasterVolume(float volume);

	void rejiggerInstruments(bool selected);

	virtual void saveData(IStream* stream, bool save);
	void startLogging(std::string file);
	void stopLogging();
	void loggingComplete();

	virtual void onMidiMessage(int channel, int message, int value);
	virtual void midiTick();

	void setFrequencyTable(double* table);
	double* getFrequencyTable();
	double* getDefaultFrequencyTable();
	Genny2612* getCore() { return _core; }

private:
	Genny2612* _core;
	GennyInterface* _editor;
	bool _patchesInitialized;
	bool _first;
	bool _saving;
	bool _switchingPreset;
	std::map<int, std::vector<int>> _midiLearn;
};

