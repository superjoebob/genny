#include "YM2612Processor.h"
#include "YM2612.h"
#include "SN76489Chip.h"
#include "sn76489.h"
#include "sn76489_plusgx.h"
#include "blip_buf.h"

YM2612Processor::YM2612Processor(void)
	: _leftoverSamples(0)
	, _finishFrame(0)
	, _chip(nullptr)
	, _masterVolume(1.0f)
	, prevSample(0)
	, _sampleRate(0)
	, _delayedCommandLoopSize(0)
	, _delayedCommandLoop(nullptr)
	, _delayedCommandLoopPosition(0)
{
	snSamples = new short*[2];
	for( int i = 0; i < 2; ++i )
	{
		snSamples[ i ]  = new short[ 1 ];
		snSamples[ i ][0] = 0; 
	}
}


YM2612Processor::~YM2612Processor(void)
{
	if (snSamples != nullptr)
	{
		for (int i = 0; i < 2; ++i)
		{
			delete[] snSamples[i];
		}
		delete[] snSamples;
		snSamples = nullptr;
	}	
	
	if (_delayedCommandLoop != nullptr)
	{
		delete[] _delayedCommandLoop;
		_delayedCommandLoop = nullptr;
	}

	_resampler.Fir_Resampler_shutdown();
}

void YM2612Processor::initialize(YM2612* chip, SN76489Chip* chip2, double sampleRate)
{
	_chip = chip;
	_snChip = chip2;

	chip->setProcessor(this);
	chip2->setProcessor(this);

	_resampler.Fir_Resampler_initialize(4096);
	//_resampler.Fir_Resampler_time_ratio( 53693175 / (double)44100 / (144.0 * 7.0), 0.995 );

	setSampleRate(sampleRate);
}

void YM2612Processor::setSampleRate(double rate)
{
	_resampler.Fir_Resampler_time_ratio((_chip->_clock / 144.0f) / rate, 0.995); 
	_sampleRate = rate;
	_snChip->setSampleRate(rate);

	_delayedCommandLoopSize = rate * 2;
	if (_delayedCommandLoop != nullptr)
		delete[] _delayedCommandLoop;

	_delayedCommandLoop = new DelayedChipCommands[_delayedCommandLoopSize];
}

//void YM2612Processor::terminate()
//{
//}

#if !BUILD_VST
typedef float TWAV32FS[1][2];
typedef TWAV32FS* PWAV32FS;
#endif
void YM2612Processor::update(float** buffer, int numSamples)
{
	if(_chip == nullptr)
		return;

#if !BUILD_VST
	PWAV32FS buf = (PWAV32FS)buffer;
#else
	float* out1 = buffer[0];
	float* out2 = buffer[1];
#endif

	float amp = 3.0 * _masterVolume;
	float snamp = 4.5 * _masterVolume;
	for(int i = 0; i < numSamples; i++)
	{
#if BUILD_VST
		out1[i] = 0.0f;
		out2[i] = 0.0f;
#endif

		//Delayed Commands
		//--------------------------------------------------
		DelayedChipCommands* cmds = lockChipCommandsIfAvailable(_delayedCommandLoopPosition);
		if (cmds != nullptr)
		{
			_chip->writeDataMutex.lock();

			for (int i = 0; i < cmds->commands.size(); i++)
			{
				if (cmds->commands[i].ym)
					_chip->writeData(cmds->commands[i].reg, cmds->commands[i].data, cmds->commands[i].channel, -1, true, cmds->commands[i].logData);
			}

			cmds->commands.clear();
			_chip->writeDataMutex.unlock();

			unlockChipCommands(cmds);
		}
		_delayedCommandLoopPosition = (_delayedCommandLoopPosition + 1) % _delayedCommandLoopSize;
		//--------------------------------------------------

		_chip->runSample();
		_snChip->tick();

		if (_snChip->_emulationMute == false)
		{
			if (_snChip->_hq)
			{
				blip_t* b = _snChip->_implHQ->snd.blips[0];
				int numNeeded = blip_clocks_needed(b, 1);
				_snChip->_implHQ->psg_end_frame(numNeeded);
				blip_end_frame(b, numNeeded);
				int numAvail = blip_samples_avail(b);
				if (numAvail > 0)
				{
					short samp[2];
					blip_read_samples(b, samp, 1);
					snSamples[0][0] = samp[0];
					snSamples[1][0] = samp[1];
				}
			}
			else
				_snChip->getCore()->SN76489_Update(snSamples, 1);

#if BUILD_VST
			out1[i] += (snSamples[0][0] / (float)32767) * snamp;
			out2[i] += (snSamples[1][0] / (float)32767) * snamp;
#else
			(*buf)[i][0] += (snSamples[0][0] / (float)32767) * snamp;
			(*buf)[i][1] += (snSamples[1][0] / (float)32767) * snamp;
#endif
		}

		_snChip->updateEnvelopes((int)_sampleRate);
		_chip->updateDAC();

		if (_chip->_emulationMute == false)
		{
			int avail = 0;
			while (_resampler.Fir_Resampler_avail() == 0)
			{
				_chip->Update(_resampler.Fir_Resampler_buffer(), 1);
				_resampler.Fir_Resampler_write(2);
			}

			int num = _resampler.Fir_Resampler_read(_samples, 1);

#if BUILD_VST
			out1[i] += (float)(_samples[0] / (float)32767) * amp;
			out2[i] += (float)(_samples[1] / (float)32767) * amp;
#else
			(*buf)[i][0] += (float)(_samples[0] / (float)32767) * amp;
			(*buf)[i][1] += (float)(_samples[1] / (float)32767) * amp;
#endif
		}
	}

}
//
////#include "fp_def.h"
//typedef float TWAV32FS[1][2];
//typedef TWAV32FS *PWAV32FS;
//void YM2612Processor::update(float** buffer, int numSamples)
//{
//	PWAV32FS buf = (PWAV32FS)buffer;
//
//	if(_chip == nullptr)
//		return;
//
//	float amp = 3.0 * _masterVolume;
//	float snamp = 4.5 * _masterVolume;
//	for(int i = 0; i < numSamples; i++)
//	{
//		_chip->runSample();
//		_snChip->tick();
//
//		if (_snChip->_emulationMute == false)
//		{
//			if (_snChip->_hq)
//			{
//				blip_t* b = _snChip->_implHQ->snd.blips[0];
//				int numNeeded = blip_clocks_needed(b, 1);
//				_snChip->_implHQ->psg_end_frame(numNeeded);
//				blip_end_frame(b, numNeeded);
//				int numAvail = blip_samples_avail(b);
//				if (numAvail > 0)
//				{
//					short samp[2];
//					blip_read_samples(b, samp, 1);
//					snSamples[0][0] = samp[0];
//					snSamples[1][0] = samp[1];
//				}
//			}
//			else
//				_snChip->getCore()->SN76489_Update(snSamples, 1);
//
//			//Mix in SN76489
//			//float val = (prevSample / (float)32767) * 5.0f;
//			(*buf)[i][0] += (snSamples[0][0] / (float)32767) * snamp;
//			(*buf)[i][1] += (snSamples[1][0] / (float)32767) * snamp;
//		}
//
//		_snChip->updateEnvelopes((int)_sampleRate);
//		_chip->updateDAC();
//
//		if (_chip->_emulationMute == false) 
//		{
//			int avail = 0;
//			while (_resampler.Fir_Resampler_avail() == 0)
//			{
//				_chip->Update(_resampler.Fir_Resampler_buffer(), 1);
//				_resampler.Fir_Resampler_write(2);
//			}	
//		
//			int num = _resampler.Fir_Resampler_read(_samples, 1);
//			(*buf)[i][0] += (float)(_samples[0] / (float)32767) * amp;
//			(*buf)[i][1] += (float)(_samples[1] / (float)32767) * amp;
//		}
//	}
//}

