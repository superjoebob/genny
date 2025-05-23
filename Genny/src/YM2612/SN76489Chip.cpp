#include "sn76489.h"
#include "SN76489Chip.h"		
//#include "Genny2612.h"
#include "VGMLogger.h"
#include "GennyLoaders.h"
#include "GennyVST.h"
#include "sn76489_plusgx.h"

//-------------------------------------------------------------------------------------------------------
//SN76489Chip - Wrapper for a SN76489 implementation
//-------------------------------------------------------------------------------------------------------
SN76489Chip::SN76489Chip(GennyVST* pVST)
	:_impl(nullptr)
	,_clock(SN76489Clock::SN76489_NTSC)
	,_soundRate(0)
	,_logger(nullptr)
	,_lastWrite(0)
	,_mutex()
	,_hardwareMode(false)
	,_emulationMute(false)
	,_vst(pVST)
	,_hq(true)
{
	_channelVolumes[0] = 15;
	_channelVolumes[1] = 15;
	_channelVolumes[2] = 15;
	_channelVolumes[3] = 15;
}

//-------------------------------------------------------------------------
	
SN76489Chip::~SN76489Chip()
{
	_prevSample = 0;
	Terminate();
}

//-------------------------------------------------------------------------

void SN76489Chip::Initialize( SN76489Clock clock, int soundRate )
{
	_clock = clock;
	_soundRate = soundRate;

	//Initialize the resampler to resample at 44100hz
	_impl = new I76489Impl();	
	_impl->SN76489_Init( clock, soundRate );
	_impl->SN76489_Reset();


	_implHQ = new sn76489Plus_Impl();
	_implHQ->psg_init(PSG_TYPE::PSG_DISCRETE, _soundRate, 53693175);
	_implHQ->psg_reset();
	//_implHQ->psg_config()


	for( float noteSteps = 0; noteSteps < 12.0f; noteSteps+= 1.0f ) 
	{
		//for( int miniSteps = 0; miniSteps < 16; miniSteps++ )
		//{
			long freqDiv = clock / 18;
			float nKey = 60 + noteSteps; //60 = MIDI C-5

			float mFreq = 1.09f * pow( 2, (nKey/12) ); //1.09 = Lowest MIDI Note Frequency
			float freqNum = (mFreq * 1048576) / freqDiv;

			_freqs[(int)noteSteps] = freqNum;
		//}
	}
}

void SN76489Chip::setSampleRate(double rate)
{
	_soundRate = rate;
	if (_impl != nullptr)
		_impl->SN76489_SetSampleRate(_clock, rate);

	if(_implHQ != nullptr)
	{
		_implHQ->psg_init(PSG_TYPE::PSG_DISCRETE, _soundRate, 53693175);
		_implHQ->psg_reset();
	}
}

//-------------------------------------------------------------------------

void SN76489Chip::Terminate()
{
	if (_impl != nullptr)
	{
		delete _impl;
		_impl = nullptr;
	}

	if (_implHQ != nullptr)
	{
		delete _implHQ;
		_implHQ = nullptr;
	}
}

//-------------------------------------------------------------------------

//This function actually updates the state of the SN76489 emulation, and
//returns the requested number of samples(length). 

//void SN76489Chip::Update( float **buf, int length )
//{
//	for(int j = 0; j < length; j++ )
//	{
//		short sample = 0;
//		int samples = 0;
//		samples = _impl->SN76489_Update(&sample, _impl->SN76489_Clocks(1));
//		if(samples > 0)
//			_prevSample = sample;
//
//		float val = (_prevSample / (float)32767) * 100.0f;
//		buf[0][j] = val;
//		buf[1][j] = val;
//	}
//
//}

//-------------------------------------------------------------------------

//Simply writes a value into a register. 
void SN76489Chip::Write( unsigned char data, int channel, int mmData, int noteState)
{
	if(_channels[channel].currentDelay > 0 && channel >= 0)
	{
		_mutex.lock();
		if(_commands.size() == 0)
			_commands.push_back(SN76489CommandCluster(_channels[channel].currentDelay));
		else if(_commands.back().delaySamples != _commands.back().originalDelay || (_commands.back().delaySamples != _channels[channel].currentDelay))
			_commands.push_back(SN76489CommandCluster(_channels[channel].currentDelay));

		_commands.back().commands.push_back(SN76489Command(data, channel, mmData));
		_mutex.unlock();
	}
	else
	{
		if(data != _lastWrite)
		{
			if (!_emulationMute)
			{
				if (_hq)
					_implHQ->psg_write(0, data);
				else
					_impl->SN76489_Write(data);
			}

			if (_hardwareMode)
				_2612->SendMIDI(2, mmData >= 0 ? ((unsigned char)mmData) : data, noteState);

			if(_logger != nullptr)
				_logger->logWriteSN76489(data);
		}
		_lastWrite = data;
	}
}

//-------------------------------------------------------------------------

//Writes a tone value to a channel
void SN76489Chip::WriteTone( int channel, int value, SimpleEnvelope env, int mmValue)
{
	if (channel > 3)
	{
		int qq = 1;
	}

	unsigned char chan = channel % 4;
	unsigned char ld = 1;
	unsigned char type = 0;
	unsigned char data = 14;
	unsigned char write = ld; 
	unsigned char dat01 = value & 15;
	unsigned char dat02 = (value & 1008) >> 4;

	int dat01MM = -1;
	int dat02MM = -1;
	if (mmValue >= 0)
	{
		dat01MM = mmValue & 15;
		dat02MM = (mmValue & 1008) >> 4;
	}


	if(channel == 3)
	{
		//Noise is different
		write = (((((((ld << 2) | chan) << 1) | type) << 2) | env.periodic) << 2) | env.sr;
		Write(write, channel);	
	}
	else
	{
		write = (((((ld << 2) | chan) << 1) | type) << 4);
		Write(write | dat01, channel, dat01MM >= 0 ? (write | dat01MM) : -1);	

		ld = 0;
		write = (ld << 7);
		Write(write | dat02, channel, dat02MM >= 0 ? (write | dat02MM) : -1);
	}
}

//-------------------------------------------------------------------------

//Writes a volume value to a channel
void SN76489Chip::WriteVolume( int channel, float value )
{
	unsigned char ld = 1;
	unsigned char chan = channel % 4;
	unsigned char data = (unsigned char)((1.0f - value) * 15);
	unsigned char type = 1;
	unsigned char write = ld;

	write = (((((ld << 2) | chan) << 1) | type) << 4) | data;
	Write(write, channel);
}

void SN76489Chip::setVolume(int channel, float volume, bool noteOn)
{
	volume = min(volume, 1.0f);
	volume = 1.0f - volume;
	//LATCH
	//--------------------------------------
	//  7   6    5     4      3   2   1   0
	//	1	|CHAN|	 |TYPE|	  |   DATA    |
	//TYPE: 0 = Tone/Noise; 1 = Volume;

	//DATA
	//----------------------------------
	//  7   6   5   4   3   2   1   0
	//	0	    |       DATA        |
	if((unsigned char)(volume * 15) != (unsigned char)_channelVolumes[channel])
	{
		unsigned char latch = 1; 
		unsigned char chan = channel;
		unsigned char type = 1;
		unsigned char data = volume * 15;
		unsigned char write = (((((latch << 2) | chan) << 1) | type) << 4) | data; 

		if (noteOn)
		{
			if(channel == 3)
				Write(write, channel, -1, volume == 1.0f ? 4 : 3);
			else
				Write(write, channel, -1, volume == 1.0f ? 2 : 1);
		}
		else
			Write(write, channel, -1, 0);

		_channelVolumes[channel] = volume * 15;
	}
}

void SN76489Chip::noteOn(int note, int velocity, int channel, SimpleEnvelope env, double* frequencyTable, bool retrigger)
{
	if(!retrigger)
		_channels[channel] = env;

	writeFrequencies(note, velocity, channel, _channels[channel], true, 0.0f, frequencyTable);
	setVolume( channel, 0.0 );
}

void SN76489Chip::writeFrequencies(int note, int velocity, int channel, SimpleEnvelope env, bool noteOnEvent, float vibrato, double* frequencyTable)
{
	int baseNote = (note / 100);
	//freq0 = (523.25 + (15.5570656763 * bend)) * pow(1.05946309435, baseNote - 60);
	//freq1 = (523.25 + (15.5570656763 * bend)) * pow(1.05946309435, (baseNote + 1) - 60);
	//float freqDif = freq1 - freq0;
	//
	//float freq = freq0 + (freqDif * (((note - (baseNote * 100.0f)) / 100.0f) + vibrato));


	float amountToBend = (((note - (baseNote * 100.0f)) / 100.0f) + vibrato);
	float semitones = 0;

	double baseFrequency = 8.17579891564371;
	if(frequencyTable != NULL)
	{		
		baseFrequency = frequencyTable[0];
		if(baseNote > 0 && baseNote < 127)
		{
			if(amountToBend > 0)
				amountToBend *= (float)(frequencyTable[baseNote + 2] - frequencyTable[baseNote + 1]);
			else
				amountToBend *= (float)(frequencyTable[baseNote + 1] - frequencyTable[baseNote]);
		}
		semitones = frequencyTable[baseNote + 1] + amountToBend;
	}
	else
	{
		//default bend by 100 semitones
		amountToBend *= 100;
		semitones = (baseNote * 100) + amountToBend;
	}
	  

	double musicalFrequency = (pow(2.0, (((semitones + 1200) / 100.0) / 12.0)) * baseFrequency);
	double freq = musicalFrequency;


	unsigned short reg = (_clock / (freq * 32.0));
	int regMM = -1;
	if (_hardwareMode)
		regMM = (float)SN76489Clock::SN76489_MEGAMIDI / (2.0f * freq * 16.0f);


	if (env.melodicMode)
	{
		WriteTone(2, reg, env, regMM);
		//setVolume(2, 0); //Mute channel 3
	}
	
	if(channel != 3 || noteOnEvent)
		WriteTone(channel, reg, env, regMM);

	_channels[channel].noteVelocity = (velocity / 127.0f);
}

void SN76489Chip::noteOff(int channel)
{
	if(_channels[channel].state != ES_Finished)
	{
		_channels[channel].state = ES_Release;
		_channels[channel].pos = 0.0f;
	}
}



void SN76489Chip::updateEnvelopes(int sampleRate)
{
	
	/*if (_triangleUp)
	{
		_triangleInc += _waveSpeed;
		if (_triangleInc > 1.0f)
		{
			_triangleInc = 1.0f;
			_triangleUp = false;
		}
	}
	else
	{
		_triangleInc -= _waveSpeed;
		if (_triangleInc < -1.0f)
		{
			_triangleInc = -1.0f;
			_triangleUp = true;
		}
	}
	float triangleMul = (_triangleInc + 1.0f) / 2.0f;*/
	//triangleMul = 1.0f;


	float rateMult = 44100.0f / sampleRate;
	for(int i = 0; i < 4; i++)
	{
		SimpleEnvelope& channel = _channels[i];

		if(channel.state == ES_Finished)
			continue;

		bool trigger = channel.pos == 0;
		channel.pos += 0.00025f * rateMult;
		float attack = (1.0f - channel.atk) * 2.0f;
		float dr1 = channel.dr1 * 2.0f;

		if(channel.dr2 > 0.99f)
			channel.dr2 = 0.99f;

		float dr2 = (1.0f - channel.dr2) * 20.0f;
		float rr = channel.rr * 10.0f;

		if(channel.state != ES_Release)
		{
			if(attack <= 0.0001f)
				channel.currentLevel = channel.lev;
			else
				channel.currentLevel = (channel.pos / attack) * channel.lev;

			if(channel.pos > attack)
			{
				float travel = channel.pos - attack;
				channel.currentLevel = channel.lev - ((travel / dr1) * (channel.sus * channel.lev)); 
				if(channel.pos > attack + dr1)
				{
					travel = channel.pos - attack - dr1;

					if(dr2 > 19.99f)
						dr2 = 999999.0f;

					if(travel > dr2)
						travel = dr2;
					channel.currentLevel = ((1.0f - channel.sus) * channel.lev) * (1.0f - (travel / dr2));
				}
			}

			if (channel.currentLevel > 99999.0f)
				channel.currentLevel = channel.lev;

			setVolume(i, (channel.currentLevel * channel.noteVelocity), trigger);
		}
		else
		{
			if(channel.pos > rr)
			{
				setVolume(i, 0, true);
				channel.state = ES_Finished;
			}
			else
				setVolume(i, ((channel.currentLevel * channel.noteVelocity) * (1.0f - (channel.pos / rr))));

		}


		
	}
}
void SN76489Chip::fullStop(int channel)
{
	clearCache(channel);
	for (int i = 0; i < 4; i++)
	{
		if (channel < 0 || channel == i)
		{
			SimpleEnvelope& channel = _channels[i];
			channel.state = ES_Finished;
			_channelVolumes[i] = 20;
			setVolume(i, 0);
		}
	}
}

void SN76489Chip::tick()
{
	_hq = _vst->accurateEmulationMode;

	_mutex.lock();
	int sz = _commands.size();
	for(int i = 0; i < sz; i++)
	{
		_commands[i].delaySamples -= 1;
		if(_commands[i].delaySamples <= 0)
		{
			int innerSz = _commands[i].commands.size();
			for(int j = 0; j < innerSz; j++)
				Write(_commands[i].commands[j].data, -1);

			_commands.erase(_commands.begin() + i);
			i--;
			sz--;
		}
	}
	_mutex.unlock();
}

void SN76489Chip::clearCache(int channel)
{
	_lastWrite = 0;
}