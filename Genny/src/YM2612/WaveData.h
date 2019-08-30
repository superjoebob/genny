#pragma once
#include <iostream>
#include <fstream>

class GennyData;
struct WaveData
{
	WaveData(GennyData* data);
	WaveData(char* data, int size);
	~WaveData();
	void SetData(unsigned char* data, int size);
	WaveData* GetChunk(int start, int end);

	int sampleRate;
	int bitRate;

	int size;
	unsigned char* audioData;
	int audioPosition;
	float waitTime;

	int streamStartPos;
	bool valid;
	bool flash;
	std::string sampleName;

	int startSample;
	int endSample;
	int loopSampleStart;
	int loopSampleEnd;

	bool panLeft;
	bool panRight;
	bool loop;
	bool normalize;
	bool release;
};