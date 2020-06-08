#pragma once
/* YM2612Processor
-------------------------------------
This class is used to render samples from a YM2612 chip. 
It is separated from the YM2612 class to keep resampler 
logic separate, and to make YM2612 more lightweight. */
#include "resampler\Fir_Resampler.h"


class YM2612;
class SN76489Chip;
class YM2612Processor
{
public:
	YM2612Processor(void);
	~YM2612Processor(void);

	void initialize(YM2612* chip, SN76489Chip* chip2);
	void terminate();
	void update(float** buffer, int numSamples);
	void setMasterVolume(float vol) { _masterVolume = vol; }

private:
	YM2612* _chip;
	SN76489Chip* _snChip;

	//GXPlus implementation produces audio output at 53267hz, so we need 
	//to resample the audio output to 44100hz!
	Resampler _resampler;
	int _leftoverSamples;
	int _finishFrame;
	int _samples[4096];
	float _masterVolume;
	short prevSample;
	short** snSamples;
};

