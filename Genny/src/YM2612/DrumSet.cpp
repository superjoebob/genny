#include "DrumSet.h"
#include "WaveData.h"
DrumSet::DrumSet()
	:_currentDrum(-1)
{

}

WaveData* DrumSet::getDrum(int note)
{
	std::map<int, WaveData*>::iterator it = _drumMap.find(note);
	if(it != _drumMap.end())
		return (*it).second;
	return nullptr;
}

bool DrumSet::setCurrentDrum(int drum)
{
	if (_drumMap.find(drum) == _drumMap.end())
		return false;

	if(_currentDrum != drum && _currentDrum != -1 && _drumMap.find(_currentDrum) != _drumMap.end())
	{
		_drumMap[_currentDrum]->waitTime = 0.0f;
		_drumMap[_currentDrum]->audioPosition = 0;
	}
	_currentDrum = drum;

	_drumMap[drum]->waitTime = 0.0f;

	if(_drumMap[drum]->loop)
		_drumMap[drum]->audioPosition = 0;
	else
		_drumMap[drum]->audioPosition = _drumMap[drum]->startSample;

	_drumMap[drum]->flash = true;
	return true;
}

WaveData* DrumSet::getCurrentDrum()
{
	return getDrum(_currentDrum);
}