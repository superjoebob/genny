#pragma once
#include "YM2612Enum.h"
#include "ym2612_new.h"
#include "WaveData.h"
#include <vector>
#include <map>

//YMParams exists to offer an interface for classes
//which require searching for YM2612Params within
//data members.
class IndexBaron;
class IBIndex;
class VGMLogger;
class DrumSet;
class YMParams
{	
public:
	virtual float* getParameter(YM2612Param param, int channel, int op) = 0;
};

class YM2612Channel : YMParams
{
public:
	YM2612Channel() : _dirty(true), _currentDelay(0) {};
	float* getParameter(YM2612Param param, int channel, int op);
	unsigned char getParameterChar(YM2612Param param, int channel, int op);

	static unsigned int getNumParameters() { return (14 * 4) + 10; }

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
	int _currentDelay;
};

class YM2612Global : YMParams
{
public:
	float* getParameter(YM2612Param param, int channel, int op);
	unsigned char getParameterChar(YM2612Param param, int channel, int op);
	static unsigned int getNumParameters() { return 3; }

	void catalogue(IndexBaron* baron);
	void setFromBaron(IBIndex* param, float val);
	float getFromBaron(IBIndex* param);

private:
	//LFO_EN, LFO, Special Mode
	std::map<YM2612Param, float> _parameters;
};

struct YM2612Command
{
	int reg;
	unsigned char data;
	int channel;
	YM2612Command(int vRegister, unsigned char vData, int vChannel)
	{
		reg = vRegister;
		data = vData; 
		channel = vChannel;
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

class GennyPatch;
class YM2612 : YMParams
{
public:
	YM2612(void);
	~YM2612(void);

	void initialize(YM2612Clock clock = YM2612_NTSC, int soundRate = 44100);
	void noteOn(int note, int velocity, int channel, double* frequencyTable = nullptr, GennyPatch* patch = nullptr);
	void writeFrequencies(int note, int velocity, int channel, float vibrato = 0.0f, double* frequencyTable = nullptr, GennyPatch* patch = nullptr);
	void noteOff(int channel, int note);
	void killNote(int channel);

	void setParameter(YM2612Param param, int channel, int op, float value);
	void setParameterChar(YM2612Param param, int channel, int op, unsigned char value);
	void setFromBaron(IBIndex* param, int channel, float val);
	void setFromBaronGlobal(IBIndex* param, int channel, float val);
	float* getParameter(YM2612Param param, int channel, int op);
	unsigned char getParameterChar(YM2612Param param, int channel, int op);

	void writeParams(int channel);

	YM2612Impl* getImplementation() { return &_chip; }
	bool getChipWrite() const { return _chipWrite; }
	void setChipWrite(bool write) { _chipWrite = write; }

	void setLogger(VGMLogger* logger) { _logger = logger; }
	void updateDAC();
	void midiTick();
	void runSample();

	void setDACEnable(bool enable);
	void setDrumSet(DrumSet* drumset) { _drumSet = drumset; }
	void dirtyChannels();

	YM2612Channel* getChannel(int c) { return &_channels[c]; }

	//static unsigned int getNumParameters() { return YM2612Global::getNumParameters() + (YM2612Channel::getNumParameters() * 6); }

private:
	void writeParameter(YM2612Param param, int channel, int op);
	void writeData(int reg, unsigned char data, int channel, int realChannel);

	//Helpers
	unsigned char packParameter(YM2612Param param, int channel, int op);
	int getRegister( YM2612Param param, int channel, int op );
	
	//6 FM channels
	YM2612Channel _channels[6];
	YM2612Global _global;
	std::vector<YM2612CommandCluster> _commands;
	volatile bool _commandLock;
	bool _chipWrite;
	float _freqMult;
	YM2612Impl _chip;
	float _freqs[388];
	VGMLogger* _logger;

	bool _dacEnable;
	DrumSet* _drumSet;
	bool _mutedDAC;
	float _sampleVolume;

	int channelVelocity[6];
	int channelLSB[6];
	int channelMSB[6];

	bool _updateNoteFreq;
	double _noteFreq;
};

