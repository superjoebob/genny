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

	std::map<int, std::vector<int>> _midiLearn;
};

struct VelocityMap
{

};


//Every patch but the first patch will contain an instrument definition.
//The first patch is the VST workspace, all selected patches will be 
//added into the first patch via the VST interface.
class GennyVST;
struct GennyPatch : VSTPatch
{
	//YM2612Global Globals;
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
		return NumInstruments + 11 + GennyInstrument::getNumParameters();
	}

	void catalogue(IndexBaron* baron);
	void setFromBaron(IBIndex* param, float val);
	float getFromBaron(IBIndex* param);
};

//struct GennySerializable
//{
//	virtual void serialize(GennyData& pData) = 0;
//	virtual void deserialize(GennyData& pData) = 0;
//};
//
//struct GennyOperator : GennySerializable //YMOperatorParameter
//{
//	enum Param
//	{
//		YMO_DT = 0,
//		YMO_MUL = 1,
//		YMO_TL = 2,
//		YMO_DRUMTL = 3,
//		YMO_KS = 4,
//		YMO_AR = 5,
//		YMO_DR = 6,
//		YMO_SR = 7,
//		YMO_AM = 8,
//		YMO_SL = 9,
//		YMO_RR = 10,
//		YMO_F1 = 11,
//		YMO_F2 = 12,
//		YMO_SSG = 13,
//		YMO_END = 14
//	};
//
//	//values is a list of values for operator parameters, Param values
//	//work as indexes into this list. For example: values[YMO_DT] = 5;
//	unsigned char values[YMO_END];
//	GennyOperator()
//	{
//		memset(&values, 0, YMO_END);
//	}
//
//	virtual void serialize(GennyData& pData)
//	{
//		for (int i = 0; i < YMO_END; i++)
//			pData.writeByte(values[i]);
//	}
//
//	virtual void deserialize(GennyData& pData)
//	{
//		for (int i = 0; i < YMO_END; i++)
//			values[i] = pData.readByte();
//	}
//};
//
//struct GennyFMChannel : GennySerializable
//{
//	enum Param
//	{
//		YMC_AMS = 0,
//		YMC_FMS = 1,
//		YMC_L_EN = 2,
//		YMC_R_EN = 3,
//		YMC_FB = 4,
//		YMC_ALG = 5,
//		YMC_LFO_EN = 6,
//		YMC_LFO = 7,
//		YMC_CH3SPECIAL = 8,
//		YMC_END = 9
//	};
//
//	GennyOperator operators[4];
//	unsigned char values[YMC_END];
//	GennyFMChannel()
//	{
//		memset(&values, 0, YMC_END);
//	}
//
//	virtual void serialize(GennyData& pData)
//	{
//		for (int i = 0; i < 4; i++)
//			operators[i].serialize(pData);
//
//		for (int i = 0; i < YMC_END; i++)
//			pData.writeByte(values[i]);
//	}
//
//	virtual void deserialize(GennyData& pData)
//	{
//		for (int i = 0; i < 4; i++)
//			operators[i].deserialize(pData);
//
//		for (int i = 0; i < YMC_END; i++)
//			values[i] = pData.readByte();
//	}
//};
//
//
//struct GennySNChannel : GennySerializable
//{
//	enum Param
//	{
//		SN_DUTYCYCLE = 0,
//		SN_PERIODIC = 1,
//		SN_DETUNE = 2,
//		SN_LEVEL = 3,
//		SN_END = 4
//	};
//
//	unsigned char values[SN_END];
//	GennySNChannel()
//	{
//		memset(&values, 0, SN_END);
//	}
//
//	virtual void serialize(GennyData& pData)
//	{
//		for (int i = 0; i < SN_END; i++)
//			pData.writeByte(values[i]);
//	}
//
//	virtual void deserialize(GennyData& pData)
//	{
//		for (int i = 0; i < SN_END; i++)
//			values[i] = pData.readByte();
//	}
//};
//
//
//struct GennyDMChannel : GennySerializable
//{
//	GennyDMChannel()
//	{
//	}	
//	
//	//void mapDrum(int note, WaveData* drum) { _drumMap[note] = drum; }
//	//WaveData* getDrum(int note);
//
//	//void setCurrentDrum(int drum);
//	//WaveData* getCurrentDrum();
//
//	virtual void serialize(GennyData& pData)
//	{
//	}
//
//	virtual void deserialize(GennyData& pData)
//	{
//	}
//private:
//	std::map<int, WaveData*> _drumMap;
//	int _currentDrum;
//};
//
//
//struct GennyPatchNew : VSTPatch, GennySerializable
//{
//	GIType::GIType type;
//	GennyFMChannel fm;
//	GennySNChannel square;
//	GennyDMChannel drums;
//
//	char midiChannel;
//	char octave;
//	char transpose;
//	unsigned char panning;
//
//	char delay;
//	unsigned char lowRange;
//	unsigned char highRange;
//	char selectedDrum;
//
//	bool operatorVelocity[4];
//
//	GennyPatchNew()
//	{
//		type = GIType::FM;
//
//		midiChannel = 0;
//		octave = 3;
//		transpose = 11;
//		panning = 127;
//
//		highRange = 0;
//		lowRange = 127;
//		delay = 0;
//		selectedDrum = 0;
//
//		operatorVelocity[0] = false;
//		operatorVelocity[1] = false;
//		operatorVelocity[2] = false;
//		operatorVelocity[3] = true;
//	}
//
//	virtual void serialize(GennyData& pData)
//	{
//		pData.writeByte((char)type);
//		fm.serialize(pData);
//
//		pData.writeByte(midiChannel);
//		pData.writeByte(octave);
//		pData.writeByte(transpose);
//		pData.writeByte(panning);
//
//		pData.writeByte(highRange);
//		pData.writeByte(lowRange);
//		pData.writeByte(delay);
//		pData.writeByte(selectedDrum);
//
//		pData.writeByte(operatorVelocity[0]);
//		pData.writeByte(operatorVelocity[1]);
//		pData.writeByte(operatorVelocity[2]);
//		pData.writeByte(operatorVelocity[3]);
//	}
//
//	virtual void deserialize(GennyData& pData)
//	{
//		type = (GIType::GIType)pData.readByte();
//		fm.deserialize(pData);
//
//		midiChannel = pData.readByte();
//		octave = pData.readByte();
//		transpose = pData.readByte();
//		panning = pData.readByte();
//
//		highRange = pData.readByte();
//		lowRange = pData.readByte();
//		delay = pData.readByte();
//		selectedDrum = pData.readByte();
//
//		operatorVelocity[0] = pData.readByte();
//		operatorVelocity[1] = pData.readByte();
//		operatorVelocity[2] = pData.readByte();
//		operatorVelocity[3] = pData.readByte();
//	}
//
//
//	//STUBBS, trying to kill the Baron
//	void catalogue(IndexBaron* baron) {}
//	void setFromBaron(IBIndex* param, float val) { }
//	float getFromBaron(IBIndex* param) { return 0.0f; }
//};

  
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
	
	bool getTrueStereo() { return _chip.getImplementation()->fm_enablePerNotePanning; }
	void setTrueStereo(bool pValue) { _chip.getImplementation()->fm_enablePerNotePanning = pValue; _chip.getImplementation2()->fm_enablePerNotePanning = pValue;}

	IndexBaron* getIndexBaron() { return _indexBaron; }
	void setFromBaron(IBIndex* param, int channel, float val, bool pForceDACWrite = false);
	//void setFromBaronGlobal(IBIndex* param, int channel, float val);
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

	void clearCache();
	void lfoChanged();

private:
	GennyPatch _patches[12];
	IndexBaron* _indexBaron;
	YM2612 _chip;
	SN76489Chip _snChip;
	YM2612Processor _processor;
	GennyVST* _owner;

	// A number that increments every time a note is pressed
	int _numNotes;

	// A number that increments every time a note is released
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

