#pragma once
#include <functional>
#include <string>

const int kMaxInstruments = 16;


enum class PingPongPan
{
	Left,
	Right,
	Center,
	None
};

enum class GEParam
{
	None = -1,

	PerPatchParams_START = 0,
	OpVelocity = 0,
	OpVelocity1 = 0,
	OpVelocity2 = 1,
	OpVelocity3 = 2,
	OpVelocity4 = 3,

	OpEnable = 4,
	OpEnable1 = 4,
	OpEnable2 = 5,
	OpEnable3 = 6,
	OpEnable4 = 7,

	Op3Special = 8,

	Op3SpecialMidi = 9,
	Op3SpecialMidi1 = 9,
	Op3SpecialMidi2 = 10,
	Op3SpecialMidi3 = 11,
	Op3SpecialMidi4 = 12,
	
	Op3SpecialTranspose = 13,
	Op3SpecialTranspose1 = 13,
	Op3SpecialTranspose2 = 14,
	Op3SpecialTranspose3 = 15,
	Op3SpecialTranspose4 = 16,

	Op3SpecialOctave = 17,
	Op3SpecialOctave1 = 17,
	Op3SpecialOctave2 = 18,
	Op3SpecialOctave3 = 19,
	Op3SpecialOctave4 = 20,

	Op3SpecialDetune = 21,
	Op3SpecialDetune1 = 21,
	Op3SpecialDetune2 = 22,
	Op3SpecialDetune3 = 23,
	Op3SpecialDetune4 = 24,

	SnMelodicEnable = 25,
	LFOEnable = 26,

	DACSamplerate = 27,

	PerPatchParams_END = 30,

	InsParams_START = 31,
	ChEnable = 31,
	ChEnable0 = 31,
	ChEnable1 = 32,
	ChEnable2 = 33,
	ChEnable3 = 34,
	ChEnable4 = 35,
	ChEnable5 = 36,
	ChEnable6 = 37,
	ChEnable7 = 38,
	ChEnable8 = 39,
	ChEnable9 = 40,

	InsMidiChannel = 41,
	InsTranspose = 42,
	InsOctave = 43,
	InsPan = 44,
	InsRangeLow = 45,
	InsRangeHigh = 46,
	InsSoloMode = 47,
	InsSoloLegato = 48,
	InsGlide = 49,
	InsDelay = 50,
	InsDetune = 51,
	InsEnableL = 52,
	InsEnableR = 53,
	InsEnable = 54,
	InsParams_END = 60,
	TOTAL_EXT_PARAMS = 60,
};

const int kExtParamsStart = 22500;
const int kExtParamsEnd = 32727;
const int kNumPingPongSettings = 9;
enum class ParamDisplayType
{
	Integer = 0,
	Float = 1,
	Percent = 2,
	Bool = 4,
	Semitone = 8,
	Octave = 16,
	Cents = 32,
	Panning = 64,
	MidiRange = 128,
	Centered = 256,
	MidiChannel = 512,
	DT_FUN = 1024,
	SampleRate = 2048
};



const int kSamplePanningMessage = 40000020;

const int kNumDACSamplerates = 4;
const int kDACSamplerates[kNumDACSamplerates]
{
	8000, 11025, 16000, 22050
};


struct GennyInstrument;
namespace VSTGUI { class CControl; }
class GennyExtParam
{
public:
	static bool isExtParam(int tag);

	const GEParam param;
	GennyInstrument* ins;
	std::function <float(GennyInstrument*)> getFunc;
	std::function <void(GennyInstrument*, float)> setFunc;
	float rangeMin;
	float rangeMax;
	std::string name;
	ParamDisplayType type;

	static const char** kDefaultPingPongSettings;
	bool isInsParam() { return param > GEParam::InsParams_START && param < GEParam::InsParams_END; }

	VSTGUI::CControl* lastAttachedControl;

	GennyExtParam(GEParam eParam, std::string sName, GennyInstrument* pIns, std::function <float(GennyInstrument*)> fGet, std::function<void(GennyInstrument*, float)> fSet, float fMin, float fMax, ParamDisplayType eType = ParamDisplayType::Integer)
		:param(eParam)
		, getFunc(fGet)
		, setFunc(fSet)
		, rangeMin(fMin)
		, rangeMax(fMax)
		, ins(pIns)
		, lastAttachedControl(nullptr)
		, name(sName)
		, type(eType)
	{

	}

	GennyExtParam() : param(GEParam::None), lastAttachedControl(nullptr) {}

	float get() { return getFunc(ins); }
	void set(float val) { return setFunc(ins, val); }
	int getTag();

	static int parsePatchFromTag(int tag) { return (int)((tag - kExtParamsStart) / (int)GEParam::TOTAL_EXT_PARAMS); }
	static GEParam parseParamFromTag(int tag) { return (GEParam)((tag - kExtParamsStart) % (int)GEParam::TOTAL_EXT_PARAMS); }
};
