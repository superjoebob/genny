#pragma once
#include "YM2612Enum.h"

#include "ym2612_plusgx.h"
#include "ym2612_new.h"
#include "ym3438.h"

#include "WaveData.h"
#include <vector>
#include <map>
#include <mutex>

#include "readerwriterqueue.h"

//YMParams exists to offer an interface for classes
//which require searching for YM2612Params within
//data members.
class IndexBaron;
class IBIndex;
class VGMLogger;
class DrumSet;
class GennyPatch;
class YMParams
{	
public:
	virtual float* getParameter(YM2612Param param, int channel, int op) = 0;
};

class YM2612Channel : YMParams
{
public:
	YM2612Channel() : _dirty(true), patch(nullptr) {};
	float* getParameter(YM2612Param param, int channel, int op);
	unsigned char getParameterChar(YM2612Param param, int channel, int op);

	static unsigned int getNumParameters() { return (14 * 4) + 10 + 3; }

	void catalogue(IndexBaron* baron);
	void setFromBaron(IBIndex* param, float val);
	float getFromBaron(IBIndex* param);

	bool getDirty() { return _dirty; }
	void setDirty(bool dirty) { _dirty = dirty; }

	//DT, MUL, TL, RS, AR, DR1, DR2, AM, DL1, RR, F1, F2, SSG
	std::map<YM2612Param, float> _operators[4];
	//L_EN, R_EN, AMS, FMS, FB, Freq Bend, SN_SR, SN_PERIODIC, SN_DT, Algorithm
	std::map<YM2612Param, float> _parameters;

	bool _dirty;

	void setPatch(GennyPatch* pPatch)
	{
		if (patch != pPatch)
		{
			patch = pPatch;
			_dirty = true;
		}
	}

	GennyPatch* patch;
};

struct DirtyThing
{
	DirtyThing() : dirty(true), parent(nullptr)
	{

	}

	void getDirty()
	{
  		dirty = true;
		if (parent != nullptr)
			parent->getDirty();
	}

	bool dirty;
	DirtyThing* parent;
};

struct YM2612HWParam : DirtyThing
{
	YM2612HWParam() : val(0), param(YM2612Param::YM_NONE), env(false) {}
	unsigned char val;
	YM2612Param param;
	bool env;
};

struct MappedParams : DirtyThing
{
	MappedParams() : param(YM2612Param::YM_NONE)
	{

	}

	YM2612Param param;
	std::vector<YM2612HWParam*> params;
	std::map<YM2612Param, YM2612HWParam*> map;
	void add(YM2612HWParam* p, YM2612Param parm, bool env = false)
	{
		map[parm] = p;
		params.push_back(p);
		p->param = parm;
		p->parent = this;
		p->env = env;
	}

	void trySet(YM2612Param parm, unsigned char val, bool noteTrigger = false)
	{
		auto it = map.find(parm);
		if (it != map.end())
		{
			if ((*it).second->val != val/* || (noteTrigger && (*it).second->env)*/)
			{
				(*it).second->val = val;
				(*it).second->getDirty();
			}
		}
	}	

	virtual void becomeDirty()
	{
		for (int i = 0; i < params.size(); i++)
		{
			params[i]->dirty = true;
		}
		dirty = true;
	}	
	
	virtual void becomeClean()
	{
		for (int i = 0; i < params.size(); i++)
		{
			params[i]->dirty = false;
		}
		dirty = false;
	}
};

struct YM2612HWChannel : MappedParams
{
	struct Operator : MappedParams
	{
		Operator()
		{
			add(&DT, YM2612Param::YM_DT);
			add(&MUL, YM2612Param::YM_MUL);
			add(&TL, YM2612Param::YM_TL, true);
			add(&KS, YM2612Param::YM_KS, true);
			add(&AR, YM2612Param::YM_AR, true);
			add(&DR, YM2612Param::YM_DR, true);
			add(&SR, YM2612Param::YM_SR, true);
			add(&AM, YM2612Param::YM_AM);
			add(&SL, YM2612Param::YM_SL, true);
			add(&RR, YM2612Param::YM_RR, true);
			add(&SSG, YM2612Param::YM_SSG);
		}

		YM2612HWParam DT;
		YM2612HWParam MUL;
		YM2612HWParam TL;
		YM2612HWParam KS;
		YM2612HWParam AR;
		YM2612HWParam DR;
		YM2612HWParam SR;
		YM2612HWParam AM;
		YM2612HWParam SL;
		YM2612HWParam RR;
		YM2612HWParam SSG;
	};


	YM2612HWChannel()
		:delay(0)
		,patch(nullptr)
	{
		add(&FB, YM2612Param::YM_FB);
		add(&Alg, YM2612Param::YM_ALG);
		add(&AMS, YM2612Param::YM_AMS);
		add(&FMS, YM2612Param::YM_FMS);

		for (int i = 0; i < 4; i++)
		{
			op[i].parent = this;
		}
	}

	virtual void becomeDirty()
	{
		MappedParams::becomeDirty();
		for (int i = 0; i < 4; i++)
		{
			op[i].becomeDirty();
		}
	}	
	
	virtual void becomeClean()
	{
		MappedParams::becomeClean();
		for (int i = 0; i < 4; i++)
		{
			op[i].becomeClean();
		}
	}

	YM2612HWParam FB;
	YM2612HWParam Alg;
	YM2612HWParam AMS;
	YM2612HWParam FMS;
	Operator op[4];

	int delay;
	const GennyPatch* patch;
};

//
//class YM2612Global : YMParams
//{
//public:
//	float* getParameter(YM2612Param param, int channel, int op);
//	unsigned char getParameterChar(YM2612Param param, int channel, int op);
//	static unsigned int getNumParameters() { return 3; }
//
//	void catalogue(IndexBaron* baron);
//	void setFromBaron(IBIndex* param, float val);
//	float getFromBaron(IBIndex* param);
//
//private:
//	//LFO_EN, LFO, Special Mode
//	std::map<YM2612Param, float> _parameters;
//};

struct YM2612Command
{
	int reg;
	unsigned char data;
	int channel;
	int logData;
	YM2612Command(int vRegister, unsigned char vData, int vChannel, int vLogData)
	{
		reg = vRegister;
		data = vData; 
		channel = vChannel;
		logData = vLogData;
	}
};

struct YM2612CommandCluster
{
	std::vector<YM2612Command> commands;
	int delaySamples;
	int originalDelay;

	YM2612CommandCluster(int vDelaySamples)
	{
		delaySamples = vDelaySamples;
		originalDelay = vDelaySamples;
	}
};

struct YM2612DelayedCommands
{
	std::mutex mutex;
	std::vector<YM2612Command> commands;
	void addCommand(YM2612Command command)
	{
		mutex.lock();
		hasCommands = true;
		commands.push_back(command);
		mutex.unlock();
	}

	bool hasCommands;
	YM2612DelayedCommands() : hasCommands(false) {}
};


class GennyPatch;
class GennyVST;
class YM2612Processor;
struct GennyData;
class YM2612 /* : YMParams*/
{
public:
	YM2612(GennyVST* pVST);
	~YM2612(void);

	void initialize(YM2612Clock clock = YM2612_NTSC, int soundRate = 44100);
	void mute(int channel);

	bool slotUpCurrentDrum(int note);
	void fireCurrentDrum(int note, int velocity, int channel, double* frequencyTable, GennyPatch* patch);


	void noteOn(int note, int velocity, int channel, double* frequencyTable = nullptr, GennyPatch* patch = nullptr, bool retrigger = false);
	void noteOnCh3Special(int note, int velocity, float vibrato, double* frequencyTable = nullptr, GennyPatch* patch = nullptr, bool retrigger = false, int operatorChannel = 0, bool freqsOnly = false, bool triggerUnsetOperators = false);
	void updateCh3SpecialMode(bool special);

	void panningChanged(int channel);
	void writeFrequencies(int note, int velocity, int channel, float vibrato = 0.0f, double* frequencyTable = nullptr, GennyPatch* patch = nullptr, int op = -1, int ch3Special = 0);
	void noteOff(int channel, int note, bool fromMidiMessage = true);
	void noteOffCh3Special(int note, GennyPatch* patch = nullptr, int operatorChannel = 0, bool triggerUnsetOperators = false);
	void killNote(int channel);
	void setSampleRate(double rate);

	int GetGenMDMCCForParam(YM2612Param vParam, int vOp);


	void SendMIDI(unsigned char chan, unsigned char reg, unsigned char data);
	void SendMIDIGenMDM(int status, unsigned char chan, unsigned char cc, unsigned char data);
	void SendSamples(unsigned char s1, unsigned char s2);
	void SendChipReset();

	//void setParameter(YM2612Param param, int channel, int op, float value);
	//void setParameterChar(YM2612Param param, int channel, int op, unsigned char value);
	void setFromBaron(IBIndex* param, int channel, float val);
	void applyWholeChannel(const GennyPatch& patch, int channel);

	void writeChannel(YM2612Channel& pFMChannel, int pChannelIndex);
	void LFOChanged();

	//void setFromBaronGlobal(IBIndex* param, int channel, float val);
	//float* getParameter(YM2612Param param, int channel, int op);
	//unsigned char getParameterChar(YM2612Param param, int channel, int op);

	void writeParams(int channel);

	YM2612Impl* getImplementation() { return &_chip; }
	ymnew::YM2612Impl* getImplementation2() { return &_chipAccurate; }
	bool getChipWrite() const { return _chipWrite; }
	void setChipWrite(bool write) { _chipWrite = write; }

	void setLogger(VGMLogger* logger) { _logger = logger; }
	void updateDAC();
	void midiTick();
	void runSample();

	void setDACEnable(bool enable);
	void setDrumSet(DrumSet* drumset) { _drumSet = drumset; }
	void dirtyChannels();

	GennyData* _commandBuffer;
	bool _hardwareMode;
	bool _emulationMute;
	bool _mdmMode;
	void clearCache();

	void Update(int *buffer, int length);

	bool _sleep;

	YM2612Clock _clock;
	float _clockDivider;
	float _clockDividerMegaMidi;

	void writeParameter(YM2612Param param, int channel, int op);
	void writeData(int reg, unsigned char data, int channel, int realChannel, bool inLock = false, int specialLogData = -1);

	//Helpers
	unsigned char packParameter(YM2612Param param, int channel, int op);
	int getRegister(YM2612Param param, int channel, int op, int specialOffset = -1);

	void fullStop();
	void setProcessor(YM2612Processor* processor) { _processor = processor; }

	GennyVST* _vst;
	std::mutex writeDataMutex;

	//6 FM channels
	YM2612HWChannel _channels[6];
	int channelPanStatesL[6];
	int channelPanStatesR[6];
	double _sampleRate;
	float _sampleVolume;

private:

	int channelPanStatesWrittenL[6];
	int channelPanStatesWrittenR[6];

	//YM2612Global _global;
	std::vector<YM2612CommandCluster> _commands;
	moodycamel::ReaderWriterQueue<YM2612CommandCluster> _pendingCommands;

	YM2612Processor* _processor;



	bool _resetDACSamplerate;
	int _inSpecialMode3;
	std::map<YM2612Param, float> _values;
	bool _chipWrite;
	float _freqMult;
	YM2612Impl _chip;
	ymnew::YM2612Impl _chipAccurate;
	ym3438_t _chipNuked;



	YM2612CommandCluster _currentCluster;
	YM2612CommandCluster** _preparedClusters;
	bool _dirtyCluster = false;



	float _freqs[388];
	VGMLogger* _logger;

	int _dacEnable;
	DrumSet* _drumSet;
	int _mutedDAC;
	unsigned char _lastDACSample;

	int channelLSB[5][6];
	int channelMSB[5][6];


	bool ch3SpecialOpNoteOnStates[4];

	bool _updateNoteFreq;
	double _noteFreq;

	std::map<int, unsigned char> _writeMap;

	//int _writingChunk;
	bool _writingHigh;
	unsigned char _lastSample;
	bool _writeSamples;
};

