#include "YM2612Processor.h"
#include "YM2612.h"
#include "SN76489Chip.h"
#include "sn76489.h"

YM2612Processor::YM2612Processor(void)
	: _leftoverSamples(0)
	, _finishFrame(0)
	, _chip(nullptr)
	, _masterVolume(1.0f)
	, prevSample(0)
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
}

void YM2612Processor::initialize(YM2612* chip, SN76489Chip* chip2)
{
	_chip = chip;
	_snChip = chip2;

	_resampler.Fir_Resampler_initialize(4096);
	//_resampler.Fir_Resampler_time_ratio( 53693175 / (double)44100 / (144.0 * 7.0), 0.995 );

	//_resampler.Fir_Resampler_time_ratio(((8000000 / 144.0f) * 1000) / (double)44100 / (144.0 * 7.0), 0.995);
	_resampler.Fir_Resampler_time_ratio((_chip->_clock / 144.0f) / (double)44100, 0.995);
}

void YM2612Processor::terminate()
{
	_resampler.Fir_Resampler_shutdown();
}

#ifdef BUILD_VST
void YM2612Processor::update(float** buffer, int numSamples)
{
	if(_chip == nullptr)
		return;

	float* out1 = buffer[0];
	float* out2 = buffer[1];

	float amp = 3.0 * _masterVolume;
	float snamp = 4.5 * _masterVolume;
	for(int i = 0; i < numSamples; i++)
	{
		out1[i] = 0.0f;
		out2[i] = 0.0f;

		_chip->runSample();
		_snChip->tick();


		if (_snChip->_emulationMute == false)
		{
			_snChip->getCore()->SN76489_Update(snSamples, 1);

			out1[i] += (snSamples[0][0] / (float)32767) * snamp;
			out2[i] += (snSamples[1][0] / (float)32767) * snamp;
		}

		_snChip->updateEnvelopes();
		_chip->updateDAC();


		if (_chip->_emulationMute == false)
		{
			int avail = 0;
			do
			{
				_chip->Update(_resampler.Fir_Resampler_buffer(), 1);
				_resampler.Fir_Resampler_write( 2 );

				avail = _resampler.Fir_Resampler_avail();
			} while( avail == 0 );
			int num = _resampler.Fir_Resampler_read( _samples, avail ); 

			out1[i] += (float)(_samples[0] / (float)32767) * amp;
			out2[i] += (float)(_samples[1] / (float)32767) * amp;
		}
	}

}
#else
//#include "fp_def.h"
typedef float TWAV32FS[1][2];
typedef TWAV32FS *PWAV32FS;
void YM2612Processor::update(float** buffer, int numSamples)
{
	PWAV32FS buf = (PWAV32FS)buffer;

	if(_chip == nullptr)
		return;

	float amp = 3.0 * _masterVolume;
	float snamp = 4.5 * _masterVolume;
	for(int i = 0; i < numSamples; i++)
	{
		_chip->runSample();
		_snChip->tick();

		if (_snChip->_emulationMute == false)
		{
			_snChip->getCore()->SN76489_Update(snSamples, 1);

			//Mix in SN76489
			//float val = (prevSample / (float)32767) * 5.0f;
			(*buf)[i][0] += (snSamples[0][0] / (float)32767) * snamp;
			(*buf)[i][1] += (snSamples[1][0] / (float)32767) * snamp;
		}

		_snChip->updateEnvelopes();
		_chip->updateDAC();

		if (_chip->_emulationMute == false)
		{
			int avail = 0;
			do
			{
				_chip->Update(_resampler.Fir_Resampler_buffer(), 1);
				_resampler.Fir_Resampler_write(2);

				avail = _resampler.Fir_Resampler_avail();
			} while (avail == 0);
			int num = _resampler.Fir_Resampler_read(_samples, avail);


			//_chip->getImplementation()->YM2612Update(_samples, 1);


			(*buf)[i][0] += (float)(_samples[0] / (float)32767) * amp;
			(*buf)[i][1] += (float)(_samples[1] / (float)32767) * amp;
		}
	}
}
#endif

