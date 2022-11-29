#pragma once
#include <iostream>
#include <fstream>

const int kNumDACVelos = 16;

struct GennyData;
struct WaveData
{
	static unsigned char* Resample(unsigned char* source, int sourceLength, int sourceSampleRate, int targetSampleRate, int& newSize);
	WaveData(GennyData* data);
	WaveData(char* data, int size);
	~WaveData();
	void SetData(unsigned char* data, int size);
	WaveData* GetChunk(int start, int end);

	int sampleRate;
	int bitRate;

	int originalSampleRate;
	int originalDataSize;
	unsigned char* originalAudioData;

	int size;
	unsigned char* audioData;


	float audioPosition;
	float waitTime;
	int fadeSamples;

	int getStreamStartIndex(float velocity);
	int getStreamStart(float velocity);
	int streamStarts[kNumDACVelos];
	int streamStartPos;
	bool valid;
	bool flash;
	std::string sampleName;

	int _startSample;
	int _endSample;
	int _loopSampleStart;
	int _loopSampleEnd;		
	int _originalStartSample;
	int _originalEndSample;
	int _originalLoopSampleStart;
	int _originalLoopSampleEnd;
	void setStartSample(int val);
	void setEndSample(int val);
	void setLoopSampleStart(int val);
	void setLoopSampleEnd(int val);
	bool panLeft;
	bool panRight;
	bool loop;
	bool normalize;
	bool release;

	char _amp;
	short _pitch;
	void setAmp(char amp) { if (_amp != amp) { _amp = amp; _dirty = true; } }
	void setPitch(short pitch) { if (_pitch != pitch) { _pitch = pitch; _dirty = true; } }
	WaveData* getProcessed(int dacSamplerate);
	void reprocess(int dacSamplerate, unsigned char* presampledData = nullptr, int presampledDataSize = -1); 
private:
	bool _dirty;

	WaveData* _processed;
};