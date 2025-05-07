#pragma once

#include "YM2612/YM2612.h"
#include "YM2612/YM2612Processor.h"
#include "VirtualInstrument.h"
#include "YM2612/SN76489Chip.h"
#include "VGMLogger.h"
#include "DrumSet.h"
#include "GennyExtParam.h"

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
	GPP_Ins17,
	GPP_Ins18,
	GPP_Ins19,
	GPP_Ins20,
	GPP_Ins21,
	GPP_Ins22,
	GPP_Ins23,
	GPP_Ins24,
	GPP_Ins25,
	GPP_Ins26,
	GPP_Ins27,
	GPP_Ins28,
	GPP_Ins29,
	GPP_Ins30,
	GPP_Ins31,
	GPP_Ins32,
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

	GIP_Ch3OpOctave = 55,
	GIP_Ch3OpTranspose = 56,

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

		case GIP_Ch3OpOctave:		return 6;
		case GIP_Ch3OpTranspose:	return 22;

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
		SNSPECIAL
	};
};


struct GennyInstrument
{
	std::map<GEParam, GennyExtParam*> _extendedParamMap;
	void mapExtParam(GEParam eParam, std::string sName, std::function <float(GennyInstrument*)> fGet, std::function<void(GennyInstrument*, float)> fSet, float fMin, float fMax, int op = 0, ParamDisplayType eType = ParamDisplayType::Integer)
	{
		_extendedParamMap.insert(std::pair<GEParam, GennyExtParam*>((GEParam)(((int)eParam) + op), new GennyExtParam((GEParam)(((int)eParam) + op), sName, this, fGet, fSet, fMin, fMax, eType)));
	}

	GennyExtParam* getExt(GEParam p, int op = 0) 
	{ 
		auto it = _extendedParamMap.find((GEParam)((int)p + op));
		if (it != _extendedParamMap.end())
			return (*it).second;

		return nullptr;
	}
	float getExtParam(GEParam p, int op = 0) { return _extendedParamMap[(GEParam)(((int)p) + op)]->get(); }
	void setExtParam(GEParam p, float v, int op = 0) { _extendedParamMap[(GEParam)(((int)p) + op)]->set(v); }

	int patchIndex;
	GennyInstrument(int iPatchIndex)
		:patchIndex(iPatchIndex)
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

		PingPongPanIndex = 0;
		PingPongString = "Off";

		HighRange = 0;
		LowRange = 127;
		Delay = 0;
		SelectedDrum = 0;
	    Type = GIType::FM;
		OperatorVelocity[0] = false;
		OperatorVelocity[1] = false;
		OperatorVelocity[2] = false;
		OperatorVelocity[3] = true;

		for (int i = 0; i < 4; i++)
		{
			OperatorEnable[i] = true;
			OperatorMidiChannel[i] = 0;
			OperatorTranspose[i] = 11;
			OperatorOctave[i] = 3;
			OperatorDetune[i] = 50;
		}


		Detune = 50;


		snMelodicEnable = false;
		soloMode = false;
		legatoMode = true;
		glide = 0;
		Ch3Special = false;
		LFOEnable = false;
		DACSampleRate = 1;

		EnableL = true;
		EnableR = true;
		Enable = true;

		//memset(DACSamplePath, 0, sizeof(int) * 32);
		//memset(DrumSampleData, 0, sizeof(char*) * 20);

		for (int i = 0; i < 4; i++)
		{
			int opNum = i;
			if (opNum == 1)
				opNum = 2;
			else if (opNum == 2)
				opNum = 1;

			mapExtParam(GEParam::OpEnable, "Enable Operator (OP" + std::to_string(opNum + 1) + ")", [i](auto ins) { return ins->OperatorEnable[i] ? 1.0f : 0.0f; }, [i](auto ins, float val) { ins->OperatorEnable[i] = (val > 0.5f ? true : false); }, 0.0f, 1.0f, i, ParamDisplayType::Bool);
			mapExtParam(GEParam::OpVelocity, "Operator TL Velocity Link (OP" + std::to_string(opNum + 1) + ")", [i](auto ins) { return ins->OperatorVelocity[i] ? 1.0f : 0.0f; }, [i](auto ins, float val) { ins->OperatorVelocity[i] = (val > 0.5f ? true : false); }, 0.0f, 1.0f, i, ParamDisplayType::Bool);
		}

		for (int i = 0; i < 10; i++)
		{
			if(i < 6)
				mapExtParam(GEParam::ChEnable, "Enable FM Channel " + std::to_string(i + 1), [i](auto ins) { return ins->Channels[i] ? 1.0f : 0.0f; }, [i](auto ins, float val) { ins->Channels[i] = (val > 0.5f ? true : false); }, 0.0f, 1.0f, i, ParamDisplayType::Bool);
			else if(i < 9)
				mapExtParam(GEParam::ChEnable, "Enable PSG Channel " + std::to_string((i - 6) + 1),[i](auto ins) { return ins->Channels[i] ? 1.0f : 0.0f; }, [i](auto ins, float val) { ins->Channels[i] = (val > 0.5f ? true : false); }, 0.0f, 1.0f, i, ParamDisplayType::Bool);
			else
				mapExtParam(GEParam::ChEnable, "Enable PSG Fuzz Channel", [i](auto ins) { return ins->Channels[i] ? 1.0f : 0.0f; }, [i](auto ins, float val) { ins->Channels[i] = (val > 0.5f ? true : false); }, 0.0f, 1.0f, i, ParamDisplayType::Bool);
		}

		mapExtParam(GEParam::InsMidiChannel, "Midi Channel", [](auto ins) { return ins->MidiChannel; }, [](auto ins, float val) { ins->MidiChannel = (int)val; }, 0.0f, 15.0f, 0, ParamDisplayType::MidiChannel);
		mapExtParam(GEParam::InsTranspose, "Transpose Semitone", [](auto ins) { return ins->Transpose; }, [](auto ins, float val) { ins->Transpose = (int)val; }, 0.0f, 22.0f, 0, (ParamDisplayType)((int)ParamDisplayType::Semitone | (int)ParamDisplayType::Centered));
		mapExtParam(GEParam::InsOctave, "Transpose Octave", [](auto ins) { return ins->Octave; }, [](auto ins, float val) { ins->Octave = (int)val; }, 0.0f, 6.0f, 0, (ParamDisplayType)((int)ParamDisplayType::Octave | (int)ParamDisplayType::Centered));
		mapExtParam(GEParam::InsPan, "Panning", [](auto ins) { return ins->Panning; }, [](auto ins, float val) { ins->Panning = (int)val; }, 0.0f, 255.0f, 0, ParamDisplayType::Panning);
		mapExtParam(GEParam::InsRangeLow, "Range Low", [](auto ins) { return ins->LowRange; }, [](auto ins, float val) { ins->LowRange = (int)val; }, 0.0f, 127.0f, 0, ParamDisplayType::MidiRange);
		mapExtParam(GEParam::InsRangeHigh, "Range High", [](auto ins) { return ins->HighRange; }, [](auto ins, float val) { ins->HighRange = (int)val; }, 0.0f, 127.0f, 0, ParamDisplayType::MidiRange);
		mapExtParam(GEParam::InsSoloMode, "Solo Mode", [](auto ins) { return ins->soloMode ? 1.0f : 0.0f; }, [](auto ins, float val) { ins->soloMode = (val > 0.5f ? true : false); }, 0.0f, 1.0f, 0, ParamDisplayType::Bool);
		mapExtParam(GEParam::InsSoloLegato, "Solo Mode Legato", [](auto ins) { return ins->legatoMode ? 1.0f : 0.0f; }, [](auto ins, float val) { ins->legatoMode = (val > 0.5f ? true : false); }, 0.0f, 1.0f, 0, ParamDisplayType::Bool);
		mapExtParam(GEParam::InsGlide, "Glide", [](auto ins) { return ins->glide; }, [](auto ins, float val) { ins->glide = (int)val; }, 0.0f, 32.0f);
		mapExtParam(GEParam::InsDelay, "Delay", [](auto ins) { return ins->Delay; }, [](auto ins, float val) { ins->Delay = (int)val; }, 0.0f, 32.0f);
		mapExtParam(GEParam::InsDetune, "Detune", [](auto ins) { return ins->Detune; }, [](auto ins, float val) { ins->Detune = (int)val; }, 0.0f, 100.0f, 0, (ParamDisplayType)((int)ParamDisplayType::Cents | (int)ParamDisplayType::Centered));
		mapExtParam(GEParam::SnMelodicEnable, "Enable PSG CH3-Fuzz Link", [](auto ins) { return ins->snMelodicEnable ? 1.0f : 0.0f; }, [](auto ins, float val) { ins->snMelodicEnable = (val > 0.5f ? true : false); }, 0.0f, 1.0f, 0, ParamDisplayType::Bool);

		mapExtParam(GEParam::Op3Special, "Channel 3 Special Mode", [](auto ins) { return ins->Ch3Special ? 1.0f : 0.0f; }, [](auto ins, float val) { ins->Ch3Special = (val > 0.5f ? true : false); }, 0.0f, 1.0f, 0, ParamDisplayType::Bool);
		mapExtParam(GEParam::LFOEnable, "LFO Enable", [](auto ins) { return ins->LFOEnable ? 1.0f : 0.0f; }, [](auto ins, float val) { ins->LFOEnable = (val > 0.5f ? true : false); }, 0.0f, 1.0f, 0, ParamDisplayType::Bool);

		mapExtParam(GEParam::InsEnableL, "Enable L", [](auto ins) { return ins->EnableL ? 1.0f : 0.0f; }, [](auto ins, float val) { ins->EnableL = (val > 0.5f ? true : false); }, 0.0f, 1.0f, 0, ParamDisplayType::Bool);
		mapExtParam(GEParam::InsEnableR, "Enable R", [](auto ins) { return ins->EnableR ? 1.0f : 0.0f; }, [](auto ins, float val) { ins->EnableR = (val > 0.5f ? true : false); }, 0.0f, 1.0f, 0, ParamDisplayType::Bool);
		mapExtParam(GEParam::InsEnable, "Enable Ins " + std::to_string(patchIndex + 1), [](auto ins) { return ins->Enable ? 1.0f : 0.0f; }, [](auto ins, float val) { ins->Enable = (val > 0.5f ? true : false); }, 0.0f, 1.0f, 0, ParamDisplayType::Bool);

		for (int i = 0; i < 4; i++)
		{
			int opNum = i;
			if (opNum == 1)
				opNum = 2;
			else if (opNum == 2)
				opNum = 1;

			mapExtParam(GEParam::Op3SpecialMidi, "CH3 Special Midi Channel (OP" + std::to_string(opNum + 1) + ")", [i](auto ins) { return ins->OperatorMidiChannel[i]; }, [i](auto ins, float val) { ins->OperatorMidiChannel[i] = (char)val; }, 0.0f, kMaxInstruments, i, ParamDisplayType::MidiChannel);
			mapExtParam(GEParam::Op3SpecialTranspose, "CH3 Special Semitone (OP" + std::to_string(opNum + 1) + ")", [i](auto ins) { return ins->OperatorTranspose[i]; }, [i](auto ins, float val) { ins->OperatorTranspose[i] = (char)val; }, 0.0f, 22.0f, i, (ParamDisplayType)((int)ParamDisplayType::Semitone | (int)ParamDisplayType::Centered));
			mapExtParam(GEParam::Op3SpecialOctave, "CH3 Special Octave (OP" + std::to_string(opNum + 1) + ")", [i](auto ins) { return ins->OperatorOctave[i]; }, [i](auto ins, float val) { ins->OperatorOctave[i] = (char)val; }, 0.0f, 6.0f, i, (ParamDisplayType)((int)ParamDisplayType::Octave | (int)ParamDisplayType::Centered));
			mapExtParam(GEParam::Op3SpecialDetune, "CH3 Special Detune (OP" + std::to_string(opNum + 1) + ")", [i](auto ins) { return ins->OperatorDetune[i]; }, [i](auto ins, float val) { ins->OperatorDetune[i] = (char)val; }, 0.0f, 100.0f, i, (ParamDisplayType)((int)ParamDisplayType::Cents | (int)ParamDisplayType::Centered));
		}

		mapExtParam(GEParam::DACSamplerate, "Drums Samplerate", [](auto ins) { return ins->DACSampleRate; }, [](auto ins, float val) { ins->DACSampleRate = val; }, 0.0f, 3.0f, 0, ParamDisplayType::SampleRate);
	}


	~GennyInstrument()
	{
		for (auto it = _extendedParamMap.begin(); it != _extendedParamMap.end(); it++)
		{
			delete (*it).second;
		}
		_extendedParamMap.clear();
	}

	void initializeInstrumentSettings()
	{
		MidiChannel = 0;
		Octave = 3;
		Transpose = 11;
		Panning = 127;
		HighRange = 0;
		LowRange = 127;
		Delay = 0;
		SelectedDrum = 0;
	}
	
	//bool DAC;
	//bool FM;

	//DrumSet aren't parameters, they're not saved or loaded so they're not included in getNumParameters
	DrumSet Drumset;

	YM2612Channel Data;
	std::string Name;
	bool Channels[10];
	unsigned int MidiChannel;

	//char** DrumSampleData[20];

	//float DACSamplePath[32];
	//void setSamplePath(const char* str)
	//{
	//	std::string strin = str;
	//	if(strin.length() > 125)
	//		strin = strin.substr(0, 125);
	//	memcpy(DACSamplePath, strin.c_str(), strin.length() + 1);
	//}
	//char* getSamplePath()
	//{
	//	if(DACSamplePath[0] != 0)
	//		return (char*)DACSamplePath;
	//	return nullptr;
	//}
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
		int dacToFM = 13;
		int sampleStartParams = 16;
		int finalPlusExtra = 13;
		return dacToFM + sampleStartParams + finalPlusExtra + YM2612Channel::getNumParameters();
	}	
	
	static unsigned int getNumParametersPreV19()
	{
		return 16 + YM2612Channel::getNumParameters() + 32 + 10;
	}

	static unsigned int getNumParametersV20()
	{
		return 16 + YM2612Channel::getNumParameters() + /*32*/ +10;
	}

	void catalogue(IndexBaron* baron);
	void setFromBaron(IBIndex* param, float val);
	float getFromBaron(IBIndex* param);

	static bool loadingOldPanning;
	GIType::GIType Type;
	bool OperatorVelocity[4];
	bool OperatorEnable[4];
	char OperatorMidiChannel[4];
	char OperatorTranspose[4];
	char OperatorOctave[4];
	unsigned char OperatorDetune[4];
	bool snMelodicEnable;
	bool soloMode;
	bool legatoMode;
	unsigned char glide;
	bool Ch3Special;
	unsigned char Detune;
	bool LFOEnable;
	unsigned char DACSampleRate;

	bool EnableL;
	bool EnableR;
	bool Enable;

	std::string PingPongString;
	std::vector<PingPongPan> PingPongPanning;
	int PingPongPanIndex;
	PingPongPan getAndIncrementPingPongPanning();

	void parsePanString(std::string setting);


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
	static const unsigned int NumInstruments = kMaxInstruments;

	//Links to patch indexes for getPatch(0), links to instrument order remapping for getPatch(1), links to instrument enabled status for getPatch(2)
	int Instruments[NumInstruments];
	int SelectedInstrument;

	bool Channels[10];
	GennyExtParam* getExt(GEParam p, int op = 0) { return InstrumentDef.getExt(p, op); }
	float getExtParam(GEParam p, int op = 0) { return InstrumentDef.getExtParam(p, op); }
	void setExtParam(GEParam p, float v, int op = 0) { InstrumentDef.setExtParam(p, v, op); }

	//Actual instrument definitions for all but first patch
	GennyInstrument InstrumentDef;

	void setInstrumentMode(GIType::GIType mode, bool alterChannels)
	{
		if (mode != _instrumentMode)
		{
			if (alterChannels)
			{
				if (mode == GIType::SN)
				{
					InstrumentDef.Channels[0] = false;
					InstrumentDef.Channels[1] = false;
					InstrumentDef.Channels[2] = false;
					InstrumentDef.Channels[3] = false;
					InstrumentDef.Channels[4] = false;
					InstrumentDef.Channels[5] = false;
					InstrumentDef.Channels[6] = true;
					InstrumentDef.Channels[7] = true;
					InstrumentDef.Channels[8] = true;
					InstrumentDef.Channels[9] = false;
					InstrumentDef.snMelodicEnable = false;
				}
				else if (mode == GIType::SNDRUM)
				{
					InstrumentDef.Channels[0] = false;
					InstrumentDef.Channels[1] = false;
					InstrumentDef.Channels[2] = false;
					InstrumentDef.Channels[3] = false;
					InstrumentDef.Channels[4] = false;
					InstrumentDef.Channels[5] = false;
					InstrumentDef.Channels[6] = false;
					InstrumentDef.Channels[7] = false;
					InstrumentDef.Channels[8] = false;
					InstrumentDef.Channels[9] = true;
					InstrumentDef.snMelodicEnable = false;
				}
				else if (mode == GIType::SNSPECIAL)
				{
					InstrumentDef.Channels[0] = false;
					InstrumentDef.Channels[1] = false;
					InstrumentDef.Channels[2] = false;
					InstrumentDef.Channels[3] = false;
					InstrumentDef.Channels[4] = false;
					InstrumentDef.Channels[5] = false;
					InstrumentDef.Channels[6] = false;
					InstrumentDef.Channels[7] = false;
					InstrumentDef.Channels[8] = true;
					InstrumentDef.Channels[9] = true;
					InstrumentDef.snMelodicEnable = true;
				}
				else if (mode == GIType::SNSPECIAL)
				{
					InstrumentDef.Channels[0] = false;
					InstrumentDef.Channels[1] = false;
					InstrumentDef.Channels[2] = false;
					InstrumentDef.Channels[3] = false;
					InstrumentDef.Channels[4] = false;
					InstrumentDef.Channels[5] = false;
					InstrumentDef.Channels[6] = false;
					InstrumentDef.Channels[7] = false;
					InstrumentDef.Channels[8] = true;
					InstrumentDef.Channels[9] = true;
					InstrumentDef.snMelodicEnable = true;
				}
				else if (mode == GIType::DAC)
				{
					InstrumentDef.Channels[0] = false;
					InstrumentDef.Channels[1] = false;
					InstrumentDef.Channels[2] = false;
					InstrumentDef.Channels[3] = false;
					InstrumentDef.Channels[4] = false;
					InstrumentDef.Channels[5] = true;
					InstrumentDef.Channels[6] = false;
					InstrumentDef.Channels[7] = false;
					InstrumentDef.Channels[8] = false;
					InstrumentDef.Channels[9] = false;
					InstrumentDef.snMelodicEnable = false;
				}
				else if (mode == GIType::FM)
				{
					InstrumentDef.Channels[0] = true;
					InstrumentDef.Channels[1] = true;
					InstrumentDef.Channels[2] = true;
					InstrumentDef.Channels[3] = true;
					InstrumentDef.Channels[4] = true;
					InstrumentDef.Channels[5] = true;
					InstrumentDef.Channels[6] = false;
					InstrumentDef.Channels[7] = false;
					InstrumentDef.Channels[8] = false;
					InstrumentDef.Channels[9] = false;
					InstrumentDef.snMelodicEnable = false;
				}
			}
			
			_instrumentMode = mode;
		}
	}

	int lastPortaNote;

	GennyPatch(int patchIndex) : InstrumentDef(patchIndex) { memset(Instruments, -1, sizeof(int) * NumInstruments); memset(Channels, 1, sizeof(bool) * 10); SelectedInstrument = 0; _instrumentMode = GIType::NONE; lastPortaNote = -1; }
	GennyPatch(const std::string& name) : VSTPatch(name), InstrumentDef(-1) { memset(Instruments, -1, sizeof(int) * NumInstruments); memset(Channels, 1, sizeof(bool) * 10); SelectedInstrument = 0; _instrumentMode = GIType::NONE; lastPortaNote = -1; }
	GennyPatch(int patchIndex, const std::string& name) : VSTPatch(name), InstrumentDef(patchIndex) { memset(Instruments, -1, sizeof(int) * NumInstruments); memset(Channels, 1, sizeof(bool) * 10); SelectedInstrument = 0; _instrumentMode = GIType::NONE; lastPortaNote = -1; }

	static unsigned int getNumParameters()
	{
		return NumInstruments + 11 + GennyInstrument::getNumParameters();
	}

	static unsigned int getNumParametersPreV19()
	{
		return 16 + 11 + GennyInstrument::getNumParametersPreV19();
	}

	static unsigned int getNumParametersV20()
	{
		//32 = NumInstruments in V20
		return 32 + 11 + GennyInstrument::getNumParametersV20();
	}

	void catalogue(IndexBaron* baron);
	void setFromBaron(IBIndex* param, float val);
	float getFromBaron(IBIndex* param);

	virtual ~GennyPatch()
	{
	}

private:	
	//Tells the first 16 patches which instrument mode they're in
	GIType::GIType _instrumentMode;
};
  
struct SoloNoteInfo
{
	SoloNoteInfo(int noteVal, void* pNoteData)
	{
		note = noteVal;
		noteData = pNoteData;
	}
	int note;
	void* noteData;
};

struct NoteInfo
{
	NoteInfo() :note(-1), age(0), operatorChannel(-1), triggerUnsetOperators(false), num(0), channel(-1), release(0), noteData(nullptr), instrumentPatch(nullptr), velocity(0), detune(0), patch(nullptr), delay(0), panning(0), noteGlideFrom(-1), noteGlideCurrent(-1), glideSpeed(0), glideSamplesRun(0){}
	int note;
	int noteGlideFrom;
	int noteGlideCurrent;
	int glideSpeed;
	int glideSamplesRun;

	std::vector<SoloNoteInfo> noteStack; //For solo mode

	bool triggerUnsetOperators;
	int operatorChannel;
	int num;
	
	//How old this note is (in samples)
	int age; 

	int release;
	int channel;
	float velocity;
	int detune;
	int delay;
	float panning;
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
	void Calculate(float tempo, int samples, float sampleRate);
};

const int kNumNoteChannels = 14;
class Genny2612
{
public:

	Genny2612(GennyVST* owner);
	~Genny2612(void);
	void legacy(bool legacy);
	void initialize();
	void setSampleRate(double rate);

	void update(float** buffer, int numSamples);
	void noteOn(const int note, float velocity, unsigned char channel, float panning, void* data = nullptr, bool soloTrigger = false);
	void updateNote(void* noteData, int samples);
	void noteOff(const int note, int channel, void* noteData = nullptr);
	void clearNotes(GennyPatch* instrument = nullptr, int channel = -1);

	void writeParams(int channel) {_chip.writeParams(channel);}
	void startNote(int channel, int pan);
	void midiTick();
	bool getTrueStereo() { return _chip.getImplementation()->fm_enablePerNotePanning; }
	void setTrueStereo(bool pValue) { _chip.getImplementation()->fm_enablePerNotePanning = pValue; _chip.getImplementation2()->fm_enablePerNotePanning = pValue;}

	IndexBaron* getIndexBaron() { return _indexBaron; }
	void setFromBaron(IBIndex* param, int channel, float val);
	//void setFromBaronGlobal(IBIndex* param, int channel, float val);
	GennyPatch* getChannelPatch(int channel) { return _channelPatches[channel]; }
	void clearChannelPatch(int channel) { _channelPatches[channel] = nullptr; }
	GennyPatch* getInstrumentPatch(int channel) { return _channels[channel].instrumentPatch; }
	void setMasterVolume(float volume) {_processor.setMasterVolume(volume);}
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

	bool channelDirty[kNumNoteChannels];

	void clearCache();
	void paramChanged(GennyPatch* patch, YM2612Param param, int channel, int op = 0);
	void panningChanged(GennyPatch* instrument, int channel);

	YM2612 _chip;

private:
	//GennyPatch _patches[12];
	IndexBaron* _indexBaron;
	SN76489Chip _snChip;
	YM2612Processor _processor;
	GennyVST* _owner;
	bool _initializedChannels = false;

	// A number that increments every time a note is pressed
	int _numNotes;

	// A number that increments every time a note is released
	int _releases;

	NoteInfo _channels[kNumNoteChannels];
	std::vector<VibratoInfo> _vibratoNotes;
	GennyPatch* _channelPatches[kNumNoteChannels];
	VGMLogger _logger;

	bool _logging;
	std::string _loggingFile;
	double* _frequencyTable;
	double* _defaultFrequencyTable;
};

