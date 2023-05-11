#include "DrumSet.h"
#include "WaveData.h"
DrumSet::DrumSet()
	:_currentDrum(-1)
	,_sampleRate(0)
{

}

WaveData* DrumSet::getDrum(int note, int samplerate, bool processed)
{
	std::map<int, WaveData*>::iterator it = _drumMap.find(note);
	if (it != _drumMap.end())
	{
		return (*it).second;
	}
	return nullptr;
}
bool DrumSet::setCurrentDrum(WaveData* drum)
{
	for (auto it = _drumMap.begin(); it != _drumMap.end(); it++)
	{
		if ((*it).second == drum)
			return setCurrentDrum((*it).first);
	}
	return false;
}
bool DrumSet::setCurrentDrum(int drum, int samplerate, bool processed)
{
	if (_drumMap.find(drum) == _drumMap.end() && drum != -1)
		return false;

	if (_currentDrum != drum && _currentDrum != -1 && _drumMap.find(_currentDrum) != _drumMap.end())
	{
		_drumMap[_currentDrum]->waitTime = 0.0f;
		_drumMap[_currentDrum]->audioPosition = 0;

		/*if (processed)
		{
			WaveData* proc = _drumMap[_currentDrum]->getProcessed(samplerate);
			proc->waitTime = 0.0f;
			proc->audioPosition = 0;
		}*/
	}
	_currentDrum = drum;

	if (_drumMap.find(drum) != _drumMap.end())
	{
		WaveData* dr = _drumMap[drum];

		//if (processed)
		//	dr = dr->getProcessed(samplerate);

		dr->waitTime = 1.0f;

		if (dr->loop)
			dr->audioPosition = 0;
		else
			dr->audioPosition = _drumMap[drum]->_startSample;

		dr->flash = true;
		dr->fadeSamples = -1;
	}
	return true;
}

void DrumSet::setSampleRate(int sr)
{
	if (sr == 0)
		return;

	for (auto it = _drumMap.begin(); it != _drumMap.end(); it++)
	{
		(*it).second->reprocess(sr);
	}
	_sampleRate = sr;
}

WaveData* DrumSet::getCurrentDrum(int samplerate, bool processed)
{
	return getDrum(_currentDrum, samplerate, processed);
}