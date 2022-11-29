#pragma once

#include <map>

struct WaveData;
class DrumSet
{
public:
	DrumSet();
	void mapDrum(int note, WaveData* drum) { _drumMap[note] = drum; }
	WaveData* getDrum(int note, int samplerate = 11025, bool processed = false);

	bool setCurrentDrum(int drum, int samplerate = 11025, bool processed = false);
	bool setCurrentDrum(WaveData* drum);
	WaveData* getCurrentDrum(int samplerate = 11025, bool processed = false);

	bool hasDrums() { return _drumMap.size() > 0; }
	bool isPitchInstrument() { return _drumMap.size() == 1; }
	int pitchInstrumentIndex() { return _drumMap.begin()->first; }

	void setSampleRate(int sr);
	int _sampleRate;
private:
	std::map<int, WaveData*> _drumMap;
	int _currentDrum;
};