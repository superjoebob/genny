#pragma once

#include <map>

struct WaveData;
class DrumSet
{
public:
	DrumSet();
	void mapDrum(int note, WaveData* drum) { _drumMap[note] = drum; }
	WaveData* getDrum(int note);

	bool setCurrentDrum(int drum);
	WaveData* getCurrentDrum();

	bool hasDrums() { return _drumMap.size() > 0; }
	bool isPitchInstrument() { return _drumMap.size() == 1; }
	int pitchInstrumentIndex() { return _drumMap.begin()->first; }

private:
	std::map<int, WaveData*> _drumMap;
	int _currentDrum;
};