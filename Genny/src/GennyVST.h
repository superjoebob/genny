#pragma once
#include "VirtualInstrument.h"
#include <mutex>

//long versionIndicator = nullptr //Before February 17, 2019 - Version 0.5 (no version indicator)
const long kVersionIndicator1 = (long)593829658389673; //February 17, 2019 - Version 1.0 //OBSOLETE! NO LONGER SUPPORTED!
const long kVersionIndicator2 = 1127443247; //February 20, 2019 - Version 1.01
const long kVersionIndicator3 = 1127443248; //June 21, 2019 - Version 1.02
const long kVersionIndicator4 = 1127443249; //Sept 16, 2019 - Version 1.03 (Added VST automation)
const long kVersionIndicator5 = 1127443250; //Nov 7, 2019 - Version 1.04 (Saved per operator velocity settings)
const long kVersionIndicator6 = 1127443251; //Nov 7, 2019 - Version 1.05 (Saved True Stereo setting)
const long kVersionIndicator7 = 1127443252; //Nov 12, 2019 - Version 1.06 (Combined 'Global' Parameters with normal ones)
const long kVersionIndicator8 = 1127443253; //Nov 19, 2019 - Version 1.07 (Added MEGA MIDI flags)
const long kVersionIndicator9 = 1127443254; //Nov 24, 2019 - Version 1.08 (Added EMULATION MODES)
const long kVersionIndicator10 = 1127443255; //Nov 24, 2019 - Version 1.09 (Corrected number of drum samples from 55 to 56)
const long kVersionIndicator11 = 1127443256; //March 19, 2021 (VST plugin now normalizes all saved parameter values from 0 to 1, to fit the VST standard)
const long kVersionIndicator12 = 1127443257; //Feb 1, 2022 (snMelodic value saving)
const long kVersionIndicator13 = 1127443258;
const long kVersionIndicator14 = 1127443259; //Feb 8, 2022 (soloMode, glide and legatoMode saving)
const long kVersionIndicator15 = 1127443260; //Feb 14, 2022 (extended parameter saving)
const long kVersionIndicator16 = 1127443261; //Feb 23, 2022 (automation inverse parameter saving)
const long kVersionIndicator17 = 1127443262; //March 8, 2022 (lfo is now saved from a variable in GennyVST)
const long kVersionIndicator18 = 1127443263; //March 13, 2022 - added note control information to save data
const long kVersionIndicator19 = 1127443264; //March 14, 2022 - changed the way instrument enabled params are saved
const long kVersionIndicator20 = 1227443200; //May 21, 2022 - increased instrument limit to 32, removed DAC path parameter
const long kVersionIndicator21 = 1227443201; //July 14, 2024 - added legacy mode for lining up automation when loading 1.16 projects
const long kLatestVersion = kVersionIndicator21;

#ifndef GENNY_VERSION_STRING
#define GENNY_VERSION_STRING "1.5"
#endif

struct automationMessage
{
public:
	int index;
	float value;
};

class Genny2612;
class GennyInterface;
struct GennyPatch;
class WaveData;
class GennyExtParam;
enum class ParamDisplayType;
class GennyVST : public VirtualInstrument
{
public:
	GennyVST(void);
	~GennyVST(void);
	virtual void initialize();
	virtual void sampleRateChanged(double newRate);

	virtual int getPluginState (void** data, bool isPreset = false);
	virtual int setPluginState (void* data, int size, bool isPreset = false);

	virtual float getParameter(int index, VSTPatch* patch = nullptr);
	virtual int getParameterRange(int index);
	virtual void getParameterName(int index, char* text);
	virtual void getParameterValue(int index, char* text);
	virtual void getChannelName(int index, char* text);
	virtual ParamDisplayType getParameterType(int index);
	virtual void setParameter(int index, float value, VSTPatch* patch = nullptr);
	virtual void setProgramName(char* name);
	virtual void setInstrumentPatchSelection(int index);

	void updateChannel(int channel, bool on);

	virtual void setCurrentPatch(VSTPatch* patch);
	virtual void getSamples(float** buffer, int numSamples);

	virtual void noteOn(int note, float velocity, unsigned char channel, float panning, void* noteData = nullptr);
	virtual void updateNote(void* noteData, int samples);
	virtual void noteOff(int note, unsigned char channel, void* noteData = nullptr);
	virtual void clearNotes();
	virtual void clearCache();

	virtual std::string getEffectName() { return "Genny"; }
	virtual std::string getProductName() { return "Genny"; }
	virtual std::string getAuthorName() { return "Landon Podbielski"; }

	virtual int getTotalPatchCount();
	int getNumInstruments();
	virtual void setMasterVolume(float volume);

	void rejiggerInstruments(bool selected);

	virtual void saveData(IStream* stream, bool save);
	void startLogging(std::string file);
	void stopLogging();
	void loggingComplete();

	//virtual int RunNewAutomation(int Index, int Value, int RECFlags);
	virtual void MidiCleanLearnedParam(int paramTag, int legacyTag);
	virtual void MidiLearn(int paramTag, int legacyTag);
	virtual int MidiForget(int paramTag, int legacyTag);
	virtual void onMidiMessage(int channel, int message, int value);	
	
	void showHint(int parameterTag);
	virtual int onAutomation(int parameterHash, int value, AutomationTypeFlags type = (AutomationTypeFlags)((int)AutomationTypeFlags::UpdateValue | (int)AutomationTypeFlags::UpdateControl));


	virtual void midiTick();

	void setFrequencyTable(double* table);
	double* getFrequencyTable();
	double* getDefaultFrequencyTable();
	Genny2612* getCore() { return _core; }

	unsigned char megaMidiPort;
	unsigned char genMDMPort;
	unsigned char bendRange;
	bool megaMidiVSTMute;
	bool accurateEmulationMode;
	bool _setParameterNormalizedValue;
	bool _loading16InstrumentMode;

	bool _automationInverse;
	char _hintString[128];

#if !BUILD_VST
	void assignNoteControl(int idx, int param);
	void unassignNoteControl(int idx, int param);
	bool noteControlIsAssigned(int idx, int param);
	void updateNoteControl(GennyPatch* patch, void* noteData);
#endif

	virtual void destroy();

	std::pair<int, int> _lastUIUpdate;
	std::map<int, std::vector<int>> _midiLearn;
	//std::vector<std::pair<int, int>> _midiUIUpdates;
	std::map<int, int> _midiUIUpdates;
	std::map<int, int> _midiUIUpdateHistory;
	bool _hasUIUpdates;
	bool _clearMidiUIUpdateHistory;

	std::mutex _mutex;
	int _midiLearnParameter;

	GennyExtParam* getExtParam(int pTag);
	WaveData* triggerWave;

	unsigned char lfo;

#if BUILD_VST
	std::mutex _automationMessageMutex;
	std::stack<automationMessage> _automationMessages;
#endif
	bool _versionTooOld;

private:

	std::map<int, std::vector<int>> _noteControlParams;
	Genny2612* _core;
	GennyInterface* _editor;
	bool _patchesInitialized;
	bool _first;
	bool _saving;
	bool _switchingPreset;
};

