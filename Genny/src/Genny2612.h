#pragma once

#include "YM2612/YM2612.h"
#include "YM2612/YM2612Processor.h"
#include "VirtualInstrument.h"
#include "YM2612/SN76489Chip.h"
#include "VGMLogger.h"
#include "DrumSet.h"

class IndexBaron;
class IBInsParam;
class IBIndex;
enum GennyPatchParam
{
	GPP_Ins01,
	GPP_Ins02,
	GPP_Ins03,
	GPP_Ins04,
	GPP_Ins05,
	GPP_Ins06,
	GPP_Ins07,
	GPP_Ins08,
	GPP_Ins09,
	GPP_Ins10,
	GPP_Ins11,
	GPP_Ins12,
	GPP_Ins13,
	GPP_Ins14,
	GPP_Ins15,
	GPP_Ins16,
	GPP_Channel0,
	GPP_Channel1,
	GPP_Channel2,
	GPP_Channel3,
	GPP_Channel4,
	GPP_Channel5,
	GPP_Channel6,
	GPP_Channel7,
	GPP_Channel8,
	GPP_Channel9,
	GPP_SelectedInstrument
};

enum GennyInstrumentParam
{
	GIP_Enabled = 0,
	GIP_DAC = 1,
	GIP_Channel0 = 2,
	GIP_Channel1 = 3,
	GIP_Channel2 = 4,
	GIP_Channel3 = 5,
	GIP_Channel4 = 6,
	GIP_Channel5 = 7,
	GIP_Channel6 = 8,
	GIP_Channel7 = 9,
	GIP_Channel8 = 10,
	GIP_Channel9 = 11,
	GIP_MidiChannel = 12,
	GIP_FM = 13,
	GIP_DACSamplePathStart = 14,
	GIP_DACSamplePathEnd = 46,
	GIP_Octave = 47,
	GIP_Transpose = 48,
	GIP_Panning = 49,
	GIP_Extra = 50,

	GIP_RangeLow = 51,
	GIP_RangeHigh = 52,
	GIP_Delay = 53,
	GIP_SelectedDrum = 54,

	GIP_None
};

static unsigned char GennyInstrumentParam_getRange(GennyInstrumentParam param)
{
	switch(param)
	{
		case GIP_Octave:			return 6;
		case GIP_Transpose:			return 22;
		case GIP_Panning:			return 255;

		case GIP_RangeLow:			return 127;
		case GIP_RangeHigh:			return 127;
		case GIP_Delay:				return 32;
		case GIP_SelectedDrum:		return 20;

	}
	return 0;
}

static float getStep(GennyInstrumentParam param)
{
	return 1.0f / GennyInstrumentParam_getRange(param);
}

static float GIP_ToFloat(GennyInstrumentParam param, unsigned char val)
{
	return min(((val + 1) / (float)GennyInstrumentParam_getRange(param)), 1.0f) - (getStep(param) * 0.75f);
}

namespace GIType
{
	enum GIType
	{
		NONE = 0,
		FM,
		SN,
		SNDRUM,
		DAC,
	};
};

struct GennyInstrument
{
	GennyInstrument()
	{
		memset(Channels, 1, sizeof(bool) * 10);
		Channels[6] = false;
		Channels[7] = false;
		Channels[8] = false;
		Channels[9] = false;
		MidiChannel = 0;
		Octave = 3;
		Transpose = 11;
		Panning = 127;

		HighRange = 0;
		LowRange = 127;
		Delay = 0;
		SelectedDrum = 0;
	    Type = GIType::FM;
		OperatorVelocity[0] = false;
		OperatorVelocity[1] = false;
		OperatorVelocity[2] = false;
		OperatorVelocity[3] = true;

		memset(DACSamplePath, 0, sizeof(int) * 32);
		memset(DrumSampleData, 0, sizeof(char*) * 20);

	}
	
	//bool DAC;
	//bool FM;

	//DrumSet aren't parameters, they're not saved or loaded so they're not included in getNumParameters
	DrumSet Drumset;

	YM2612Channel Data;
	std::string Name;
	bool Channels[10];
	unsigned int MidiChannel;

	char** DrumSampleData[20];

	float DACSamplePath[32];
	void setSamplePath(const char* str)
	{
		std::string strin = str;
		if(strin.length() > 125)
			strin = strin.substr(0, 125);
		memcpy(DACSamplePath, strin.c_str(), strin.length() + 1);
	}
	char* getSamplePath()
	{
		if(DACSamplePath[0] != 0)
			return (char*)DACSamplePath;
		return nullptr;
	}
	int Octave;
	int Transpose;
	int Panning;

	int Delay;
	int LowRange;
	int HighRange;
	int SelectedDrum;

	int Extra[6];

	static unsigned int getNumParameters()
	{
		return 16 + YM2612Channel::getNumParameters() + 32 + 10;
	}
	void catalogue(IndexBaron* baron);
	void setFromBaron(IBIndex* param, float val);
	float getFromBaron(IBIndex* param);

	static bool loadingOldPanning;
	GIType::GIType Type;
	bool OperatorVelocity[4];
};

//Every patch but the first patch will contain an instrument definition.
//The first patch is the VST workspace, all selected patches will be 
//added into the first patch via the VST interface.
class GennyVST;
struct GennyPatch : VSTPatch
{
	YM2612Global Globals;
	static const unsigned int NumInstruments = 16;

	//Links to patch indexes for getPatch(0), links to instrument order remapping for getPatch(1), links to instrument enabled status for getPatch(2)
	int Instruments[NumInstruments];
	int SelectedInstrument;

	bool Channels[10];

	//Actual instrument definitions for all but first patch
	GennyInstrument InstrumentDef;

	//Tells the first 16 patches which instrument mode they're in
	GIType::GIType InstrumentMode;

	GennyPatch() { memset(Instruments, -1, sizeof(int) * NumInstruments); memset(Channels, 1, sizeof(bool) * 10); SelectedInstrument = 0; InstrumentMode = GIType::NONE; }
	GennyPatch(const std::string& name) : VSTPatch(name) { memset(Instruments, -1, sizeof(int) * NumInstruments); memset(Channels, 1, sizeof(bool) * 10); SelectedInstrument = 0; InstrumentMode = GIType::NONE; }

	static unsigned int getNumParameters()
	{
		return YM2612Global::getNumParameters() + NumInstruments + 11 + GennyInstrument::getNumParameters();
	}

	void catalogue(IndexBaron* baron);
	void setFromBaron(IBIndex* param, float val);
	float getFromBaron(IBIndex* param);
};

  
struct NoteInfo
{
	NoteInfo() :note(-1), num(0), channel(-1), release(0), noteData(nullptr), instrumentPatch(nullptr), velocity(0), detune(0), patch(nullptr), floatVelocity(0.0f){}
	int note;
	int num;
	int release;
	int channel;
	int velocity;
	float floatVelocity;
	int detune;
	void* noteData;
	GennyPatch* instrumentPatch;
	GennyPatch* patch;
};

struct VibratoInfo
{
	VibratoInfo() :note(-1), channel(-1), velocity(0), noteData(nullptr), vibratoPhase(0.0f), vibratoInc(0.0f){}
	int note;
	int velocity;
	int channel;
	void* noteData;
	float vibratoPhase;
	float vibratoInc;
	void Calculate(float tempo, int samples);
};

class Genny2612
{
public:
	Genny2612(GennyVST* owner);
	~Genny2612(void);
	void initialize();
	void update(float** buffer, int numSamples);
	void noteOn(int note, float velocity, unsigned char channel, float panning, void* data = nullptr);
	void updateNote(void* noteData);
	void noteOff(int note, int channel, void* noteData = nullptr);
	void writeParams(int channel) {_chip.writeParams(channel);}
	void startNote(int channel, int pan);
	void midiTick();
	

	IndexBaron* getIndexBaron() { return _indexBaron; }
	void setFromBaron(IBIndex* param, int channel, float val);
	void setFromBaronGlobal(IBIndex* param, int channel, float val);
	GennyPatch* getChannelPatch(int channel) { return _channelPatches[channel]; }
	GennyPatch* getInstrumentPatch(int channel) { return _channels[channel].instrumentPatch; }
	void setMasterVolume(float volume) {_processor.setMasterVolume(volume);}
	virtual void getParameterName(int index, char* text);
	virtual void getParameterValue(int index, char* text);
	void setFrequencyTable(double* table) 
	{ 
		if(_frequencyTable != NULL && _frequencyTable != _defaultFrequencyTable)
			delete[] _frequencyTable;

		if(table == NULL)
			_frequencyTable = _defaultFrequencyTable;
		else
			_frequencyTable = table; 
	}

	double* getFrequencyTable()
	{
		return _frequencyTable;
	}

	double* getDefaultFrequencyTable()
	{
		return _defaultFrequencyTable;
	}

	void startLogging(std::string file);
	void stopLogging();

	static bool channelDirty[10];

private:
	GennyPatch _patches[12];
	IndexBaron* _indexBaron;
	YM2612 _chip;
	SN76489Chip _snChip;
	YM2612Processor _processor;
	GennyVST* _owner;

	int _numNotes;
	int _releases;

	NoteInfo _channels[10];
	std::vector<VibratoInfo> _vibratoNotes;
	GennyPatch* _channelPatches[10];
	VGMLogger _logger;

	bool _logging;
	std::string _loggingFile;
	double* _frequencyTable;
	double* _defaultFrequencyTable;
};

