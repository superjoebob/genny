#pragma once

#include <map>
#include <vector>

struct WaveData;
class DrumSet
{
public:
	DrumSet();
	~DrumSet()
	{
		for (int i = 0; i < _oldDrums.size(); i++)
		{
			delete _oldDrums[i];
		}
		_oldDrums.clear();

		std::map<int, WaveData*>::iterator it;
		for (it = _drumMap.begin(); it != _drumMap.end(); it++)
		{
			delete it->second;
		}
		_drumMap.clear();
	}
	void mapDrum(int note, WaveData* drum) 
	{
		WaveData* oldDrum = getDrum(note);
		if (oldDrum != nullptr)
			_oldDrums.push_back(oldDrum);

		auto it = std::find(_oldDrums.begin(), _oldDrums.end(), drum);
		if (it != _oldDrums.end())
			_oldDrums.erase(it);

		_drumMap[note] = drum; 
	}
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
	std::vector<WaveData*> _oldDrums;
	int _currentDrum;
};