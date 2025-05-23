#pragma once
#include <iostream>
#include <sstream>
#include <fstream>
#include <windows.h>

class YM2612;
class SN76489Chip;
class GennyVST;
struct WaveData;
struct GennyData;
class VGMLogger
{
public:
	VGMLogger();
	void initialize(YM2612* ymChip, SN76489Chip* snChip);
	void startLogging(GennyVST* vst, std::string file);
	void finishLogging();

	bool isLogging() { return _logging; }

	void logSample();
	void logWriteSN76489(unsigned char data);
	void logWriteYM2612(int port, unsigned char reg, unsigned char data);
	void seekDACSample(int seek);
	void logDACSample();
	void resetSampleCounter();
	void writeLoopPoint();

	void prepareDrum(WaveData* drum, float velocity);

private:
	GennyVST* _vst;
	YM2612* _ymChip;
	SN76489Chip* _snChip;
	std::fstream _dataStream;
	std::ostringstream _dacStream;
	std::ostringstream _commandStream;

	long _sampleCounter;
	long _DACsampleCounter;
	long _totalSamples;
	bool _logging;
	int _loopPosition;
	int _loopStartSample;
	int _drumStreamPosition;
};