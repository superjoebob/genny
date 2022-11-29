#ifndef _SN76489_CHIP_
#define _SN76489_CHIP_
#include <vector>
#include <mutex>

//=======================================================================================================
// SN76489Chip
// Author: Landon Podbielski
// Description: What a mess
//-------------------------------------------------------------------------------------------------------

enum SN76489Clock
{
	SN76489_MEGAMIDI = 4000000,
	//SN76489_MEGAMIDI = 7600482,
	SN76489_NTSC = 3579545,
	SN76489_PAL = 7600482
};

enum EnvelopeState
{
	ES_Attack,
	ES_AttackDecay,
	ES_SustainDecay,
	ES_Release,
	ES_Finished
};

struct SimpleEnvelope
{
	SimpleEnvelope() 
	{

	}
	float lev = 0;
	float atk = 0;
	float dr1 = 0;
	float sus = 0;
	float dr2 = 0;
	float rr = 0;
	float pos = 0;
	float currentLevel = 0;
	float noteVelocity = 0;

	int sr = 0;
	int periodic = 0;
	bool melodicMode = false;

	EnvelopeState state = EnvelopeState::ES_Finished;
	int currentDelay = 0;
};

struct SN76489Command
{
	unsigned char data;
	int channel;
	int mmData;
	SN76489Command(unsigned char vData, int vChannel, int vMMData = -1)
	{
		data = vData; 
		channel = vChannel;
		mmData = vMMData;
	}
};

struct SN76489CommandCluster
{
	std::vector<SN76489Command> commands;
	int delaySamples;
	int originalDelay;

	SN76489CommandCluster(int vDelaySamples)
	{
		delaySamples = vDelaySamples;
		originalDelay = vDelaySamples;
	}
};

//-------------------------------------------------------------------------------------------------------
//SN76489Chip - Wrapper for a SN76489 implementation
//-------------------------------------------------------------------------------------------------------
class I76489Impl;
struct GennyPatch;
class VGMLogger;
struct GennyData;
class YM2612;
class sn76489Plus_Impl;
class YM2612Processor;
class GennyVST;
class SN76489Chip
{
public:
	SN76489Chip(GennyVST* pVST);
	~SN76489Chip();
	YM2612* _2612;

	void Initialize( SN76489Clock clock = SN76489_NTSC, int soundRate = 44100 );
	void setSampleRate(double rate);
	void Terminate();
	void Update( float **buf, int length );
	void Write( unsigned char data, int channel, int mmData = -1, int noteState = 0);
	void WriteTone( int channel, int value, SimpleEnvelope env, int mmValue = -1 );
	void WriteVolume( int channel, float value );

	void setVolume(int channel, float volume, bool noteOn = false);
	void noteOn(int note, int velocity, int channel, SimpleEnvelope env, double* frequencyTable = nullptr, bool retrigger = false);
	void writeFrequencies(int note, int velocity, int channel, SimpleEnvelope env, bool noteOnEvent = false, float vibrato = 0.0f, double* frequencyTable = nullptr);
	void noteOff(int channel);
	void updateEnvelopes(int sampleRate);

	float GetFreq(int freq) const { return _freqs[freq]; }

	SimpleEnvelope getEnvelope(int env) { return _channels[env]; }

	I76489Impl* getCore() { return _impl; }
	I76489Impl* _impl;
	sn76489Plus_Impl* _implHQ;

	void setLogger(VGMLogger* logger) { _logger = logger; }
	void tick();
	SimpleEnvelope* getChannel(int c) { return &_channels[c]; }

	bool _hardwareMode;
	bool _emulationMute;
	void clearCache();
	void fullStop();
	void setProcessor(YM2612Processor* processor) { _processor = processor; }

	SN76489Clock _clock;
	bool _hq;
private:
	int _soundRate;
	float _freqs[12];
	std::mutex _mutex;
	std::vector<SN76489CommandCluster> _commands;
	GennyVST* _vst;
	YM2612Processor* _processor;

	SimpleEnvelope _channels[4];
	unsigned char _channelVolumes[4];
	VGMLogger* _logger;
	unsigned char _lastWrite;
	short _prevSample;

};

#endif //_SN76489_CHIP_