#pragma once
/* YM2612Processor
-------------------------------------
This class is used to render samples from a YM2612 chip. 
It is separated from the YM2612 class to keep resampler 
logic separate, and to make YM2612 more lightweight. */
#include "resampler\Fir_Resampler.h"
#include <mutex>
#include <vector>

struct ChipCommand
{
	bool ym;
	int reg;
	unsigned char data;
	int channel;
	int logData;
	ChipCommand(bool vYM, int vRegister, unsigned char vData, int vChannel, int vLogData)
	{
		reg = vRegister;
		data = vData;
		channel = vChannel;
		logData = vLogData;
		ym = vYM;
	}
};

struct DelayedChipCommands
{
	std::mutex mutex;
	std::vector<ChipCommand> commands;
	void addCommand(bool vYM, int vRegister, unsigned char vData, int vChannel, int vLogData)
	{
		hasCommands = true;
		commands.push_back(ChipCommand(vYM, vRegister, vData, vChannel, vLogData));
	}

	bool hasCommands;
	DelayedChipCommands() : hasCommands(false) {}
};

class FO_LPF;
class YM2612;
class SN76489Chip;
class YM2612Processor
{
public:
	YM2612Processor(void);
	~YM2612Processor(void);

	void initialize(YM2612* chip, SN76489Chip* chip2, double sampleRate);
	void setSampleRate(double rate);
	//void terminate();
	void update(float** buffer, int numSamples);
	void setMasterVolume(float vol) { _masterVolume = vol; }
	DelayedChipCommands* lockChipCommands(unsigned int loopPosition)
	{
		loopPosition = loopPosition % _delayedCommandLoopSize;
		_delayedCommandLoop[loopPosition].mutex.lock();
		return &_delayedCommandLoop[loopPosition];
	}

	DelayedChipCommands* lockChipCommandsIfAvailable(unsigned int loopPosition)
	{
		if (_delayedCommandLoop[loopPosition % _delayedCommandLoopSize].hasCommands)
		{
			_delayedCommandLoop[loopPosition % _delayedCommandLoopSize].mutex.lock();
			return &_delayedCommandLoop[loopPosition % _delayedCommandLoopSize];
		}

		return nullptr;
	}

	DelayedChipCommands* lockChipCommandsOffset(unsigned int loopPositionOffset)
	{
		_delayedCommandLoop[(_delayedCommandLoopPosition + loopPositionOffset) % _delayedCommandLoopSize].mutex.lock();
		return &_delayedCommandLoop[(_delayedCommandLoopPosition + loopPositionOffset) % _delayedCommandLoopSize];
	}

	void unlockChipCommands(DelayedChipCommands* commands)
	{
		commands->mutex.unlock();
	}

private:
	YM2612* _chip;
	SN76489Chip* _snChip;

	DelayedChipCommands* _delayedCommandLoop;
	unsigned int _delayedCommandLoopSize;
	unsigned int _delayedCommandLoopPosition;

	//GXPlus implementation produces audio output at 53267hz, so we need 
	//to resample the audio output to 44100hz!
	Resampler _resampler;
	int _leftoverSamples;
	int _finishFrame;
	int _samples[4096];
	float _masterVolume;
	short prevSample;
	short** snSamples;
	double _sampleRate;
	FO_LPF* _lowpass;
};

