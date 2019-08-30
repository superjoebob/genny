#include "YM2612.h"
#include "IndexBaron.h"
#include "GennyLoaders.h"
#include "DrumSet.h"


//#include <algorithm>

using namespace std;
//YM2612Channel
//-------------------------------------------------------------------
float* YM2612Channel::getParameter(YM2612Param param, int channel, int op)
{	
	//Operator parameters
	if(param >= YM_DT && param <= YM_SSG)
	{
		op = max( 0, min( op, 4 ) );
		std::map<YM2612Param, float>::iterator it = _operators[op].find(param);
		if( it != _operators[op].end() )
			return &(*it).second;

		//Add the parameter if it is not yet added
		_operators[op][param] = 0;
		return &_operators[op][param];
	}
	//Global channel parameters
	else if(param >= YM_AMS && param <= YM_ALG)
	{
		std::map<YM2612Param, float>::iterator it = _parameters.find(param);
		if( it != _parameters.end() )
			return &(*it).second;

		//Add the parameter if it is not yet added
		float def = 0;
		if(param == YM_FRQ)
			def = 0.5f;
		if(param == YM_L_EN || param == YM_R_EN)
			def = 1.0f;
		if(param == SN_DT)
			def = 50.0f;

		_parameters[param] = def;
		return &_parameters[param];
	}

	return nullptr;
}

unsigned char YM2612Channel::getParameterChar(YM2612Param param, int channel, int op)
{
	float parm = *getParameter(param, channel, op);
	return (unsigned char)(parm + 0.5f); 
}

void YM2612Channel::catalogue(IndexBaron* baron)
{
	for(int op = 0; op < 4; op++)
	{
		for(int p = YM_DT; p <= YM_SSG; p++)
		{
			baron->addIndex(new IBYMParam((YM2612Param)p, op, 0, YM2612Param_getName((YM2612Param)p)));
		}
	}

	for(int p = YM_AMS; p <= YM_ALG; p++)
	{
		baron->addIndex(new IBYMParam((YM2612Param)p, -1, 0, YM2612Param_getName((YM2612Param)p)));
	}
}

void YM2612Channel::setFromBaron(IBIndex* param, float val)
{
	if(param->getType() == IB_YMParam)
	{
		IBYMParam* p = static_cast<IBYMParam*>(param);
		char op = p->getOperator();

		float* mVal = getParameter(p->getParameter(), 0, op);
		if(*mVal > val + 0.0001 || *mVal < val - 0.0001)
			_dirty = true;

		*mVal = val;
	}
}

float YM2612Channel::getFromBaron(IBIndex* param)
{
	if(param->getType() == IB_YMParam)
	{
		IBYMParam* p = static_cast<IBYMParam*>(param);
		char op = p->getOperator();
		return *getParameter(p->getParameter(), 0, op);
	}
	return 0.0f;
}


//YM2612Global
//-------------------------------------------------------------------
float* YM2612Global::getParameter(YM2612Param param, int channel, int op)
{
	//Global YM2612 parameters
	if(param >= YM_LFO_EN && param <= YM_SPECIAL)
	{
		std::map<YM2612Param, float>::iterator it = _parameters.find(param);
		if( it != _parameters.end() )
			return &(*it).second;

		//Add the parameter if it is not yet added
		_parameters[param] = 0;
		return &_parameters[param];
	}
}

unsigned char YM2612Global::getParameterChar(YM2612Param param, int channel, int op)
{
	float parm = *getParameter(param, channel, op);
	return (unsigned char)(parm + 0.5f); 
}


void YM2612Global::catalogue(IndexBaron* baron)
{
	for(int p = YM_LFO_EN; p <= YM_SPECIAL; p++)
	{
		baron->addIndex(new IBYMParam((YM2612Param)p, -1, -1));
	}
}

void YM2612Global::setFromBaron(IBIndex* param, float val)
{
	if(param->getType() == IB_YMParam)
	{
		IBYMParam* p = static_cast<IBYMParam*>(param);
		char op = p->getOperator();
		*getParameter(p->getParameter(), 0, op) = val;
	}
}

float YM2612Global::getFromBaron(IBIndex* param)
{
	if(param->getType() == IB_YMParam)
	{
		IBYMParam* p = static_cast<IBYMParam*>(param);
		char op = p->getOperator();
		return *getParameter(p->getParameter(), 0, op);
	}
	return 0.0f;
}



//YM2612
//-------------------------------------------------------------------
YM2612::YM2612(void)
	: _chipWrite(false)
	, _freqMult(0.0f)
	, _logger(nullptr)
	, _mutedDAC(false)
	, _drumSet(nullptr)
	, _sampleVolume(1.0f)
	, _updateNoteFreq(false)
	, _noteFreq(0.0f)
	, _commandLock(false)
{
	for(int i = 0; i < 6; i++)
	{
		channelVelocity[i] = 0;
		channelLSB[i] = 0;
		channelMSB[i] = 0;
	}

}


YM2612::~YM2612(void)
{
}

void YM2612::initialize(YM2612Clock clock, int soundRate)
{
	//_chip.YM2612Init( clock, soundRate );
	//_chip.YM2612Restore( (unsigned char*)&_chip.ym2612 );
	_chip.YM2612Init();
	_chip.YM2612LoadContext((unsigned char*)&_chip.ym2612);
	_chip.YM2612ResetChip();
	_chip.YM2612Config(0);

	//Frequency formula is (144 * noteFrequency * 2 ^ 20 / masterClock) / 2 ^ block - 1
	//_freqMult is (2 ^ 20 / 12) / (masterClock / 12) to make the returned number
	//smaller for proper use in the equation.
	_freqMult = (float)87381 / ((float)clock / 12);
}


void YM2612::noteOn(int note, int velocity, int channel, double* frequencyTable, GennyPatch* patch)
{
	noteOff(channel, note);
	if(channel == 5 && _drumSet != nullptr)
	{
		int drumNote = note / 100;
//#if !BUILD_VST
//		drumNote /= 100;
//#endif
		if(_drumSet->isPitchInstrument() && _drumSet->getDrum(_drumSet->pitchInstrumentIndex()) != NULL)
		{
			_updateNoteFreq = true;
			_drumSet->setCurrentDrum(_drumSet->pitchInstrumentIndex());
		}
		else
			_drumSet->setCurrentDrum(drumNote);

		if(_logger != nullptr && _logger->isLogging() && _drumSet->getCurrentDrum() != nullptr)
		{
			_logger->seekDACSample(_drumSet->getCurrentDrum()->streamStartPos);
		}
	}

	writeParams(channel);
	writeFrequencies(note, velocity, channel, 0.0f, frequencyTable, patch);
	writeData(getRegister(YM_NOTEON, channel % 3, 0), 0xF0 | (channel > 2 ? (channel + 1) : channel), channel % 3, channel);
}

void YM2612::killNote(int channel)
{
}

//void YM2612::writeFrequencies(int note, int velocity, int channel, float vibrato)
//{
	//coreWriteFrequencies(note * 100, velocity, channel, vibrato);

	/*int reg = getRegister(YM_TL, channel % 3, 3);

	float t = *getParameter(YM_TL, channel, 3);
	int vel = ((127.0f - t) / 127.0f) * velocity;

	float tlVol = *getParameter(YM_TL, channel, 3);
	unsigned char volume = tlVol * (127 - velocity);

	if(channel == 5)
	{
		_sampleVolume = ((float)velocity / 127.0f) * (1.0f - ((tlVol - 64) / 127.0f));
		_sampleVolume *= _sampleVolume;
		_sampleVolume *= _sampleVolume;
	}

	if(127 - vel != channelVelocity[channel])
	{
		writeData(reg, 127 - vel, channel);
		channelVelocity[channel] = 127 - vel;
	}



	int baseNote = note;

	//Calculate the frequency for midi note (1-127)
	float freqMult = (float)87381 / ((float)7670592 / 12);
	float freq0;
	float freq1;
	float bend = ((float)*getParameter(YM_FRQ, channel, 0) / (float)64);

	//261.6255653006 is the frequency for C6
	if(bend > 0)
		freq0 = (261.6255653006 + (15.5570656763 * bend)) * pow(1.05946309435, baseNote % 12);
	if(bend <= 0)
		freq0 = (261.6255653006 + (14.6839146725 * bend)) * pow(1.05946309435, baseNote % 12);

	if(bend > 0)
		freq1 = (261.6255653006 + (15.5570656763 * bend)) * pow(1.05946309435, (baseNote % 12) + 1);
	if(bend <= 0)
		freq1 = (261.6255653006 + (14.6839146725 * bend)) * pow(1.05946309435, (baseNote % 12) + 1);


	float freqDif = freq1 - freq0;
	float freq = freq0 + (freqDif * ((((note - (baseNote * 100.0f)) / 100.0f) + vibrato)));





	//Calculate the frequency for midi note (1-127)
	float freqMult = (float)87381 / ((float)7670592 / 12);
	float freq;
	float bend = ((float)*getParameter(YM_FRQ, channel, 0) / (float)64);

	//261.6255653006 is the frequency for C6
	if(bend > 0)
		freq = (261.6255653006 + (15.5570656763 * bend)) * pow(1.05946309435, note % 12);
	if(bend <= 0)
		freq = (261.6255653006 + (14.6839146725 * bend)) * pow(1.05946309435, note % 12);

	float fnum = (144 * freq * 0.136702729) / 8;
	int fnumi = (int)floor(fnum + 0.5);

	int lsb = (int)fnumi & 255;
	int msb = (((int)(note / 12))<<3) | (int)((int)(fnumi>>8));	




	if(channelLSB[channel] != lsb || channelMSB[channel] != msb)
	{
		writeData(getRegister(YM_FNUM2, channel % 3, 0), msb, channel);
		writeData(getRegister(YM_FNUM1, channel % 3, 0), lsb, channel);
		channelLSB[channel] = lsb;
		channelMSB[channel] = msb;
	}*/
//}

//#else

/*void YM2612::noteOn(int note, int velocity, int channel)
{
	if(channel == 5 && _drumSet != nullptr)
	{
		_drumSet->setCurrentDrum(note / 100);
		if(_logger != nullptr && _logger->isLogging() && _drumSet->getCurrentDrum() != nullptr)
		{
			_logger->seekDACSample(_drumSet->getCurrentDrum()->streamStartPos);
		}
	}

	noteOff(channel, note);
	writeParams(channel);
	writeFrequencies(note, velocity, channel);
	writeData(getRegister(YM_NOTEON, channel % 3, 0), 0xF0 | (channel > 2 ? (channel + 1) : channel), channel % 3);
}
#endif*/

void YM2612::writeFrequencies(int note, int velocity, int channel, float vibrato, double* frequencyTable, GennyPatch* patch)
{
//#if BUILD_VST
//	note *= 100;
//#endif

	if (channel == 5)
	{
		float tlSampleVol = *getParameter(YM_DRUMTL, channel, 3);
		_sampleVolume = (((float)velocity / 127.0f) * ((tlSampleVol / 100.0f) + ((max(tlSampleVol - 100, 0) / 27.0f) * 3.0f)));
	}


	for (int i = 0; i < 4; i++)
	{
		if ((patch == nullptr && i == 3) || patch->InstrumentDef.OperatorVelocity[i])
		{
			int reg = getRegister(YM_TL, channel % 3, i);

			float t = *getParameter(YM_TL, channel, i);
			int vel = ((127.0f - t) / 127.0f) * velocity;

			float tlVol = *getParameter(YM_TL, channel, i);
			unsigned char volume = tlVol * (127 - velocity);

			//TODO fix optimization, this allowed less data to be written when VGM logging
			//if(127 - vel != channelVelocity[channel])
			{
				writeData(reg, 127 - vel, channel, channel);
				channelVelocity[channel] = 127 - vel;
			}
		}
	}

	//Frequency of C0
	//float cFreq = 16.35f;

	int baseNote = (note / 100);
	float amountToBend = (((note - (baseNote * 100.0f)) / 100.0f) + vibrato);
	float semitones = 0;

	double baseFrequency = 8.17579891564371;
	if(frequencyTable != NULL)
	{		
		baseFrequency = frequencyTable[0];
		if(baseNote > 0 && baseNote < 127)
		{
			if(amountToBend > 0)
				amountToBend *= frequencyTable[baseNote + 2] - frequencyTable[baseNote + 1];
			else
				amountToBend *= frequencyTable[baseNote + 1] - frequencyTable[baseNote];
		}
		semitones = frequencyTable[baseNote + 1] + amountToBend;
	}
	else
	{
		//default bend by 100 semitones
		amountToBend *= 100;
		semitones = (baseNote * 100) + amountToBend;
	}


	double musicalFrequency = pow(2.0, ((semitones / 100.0) / 12.0)) * baseFrequency;
	double freq = musicalFrequency;

	if(_updateNoteFreq)
	{
		_noteFreq = musicalFrequency;
		_updateNoteFreq = false;
	}



	double fnum = (144 * freq * 0.136702729) / pow(2.0, ((int)(baseNote / 12.0)) - 2.0);
	int fnumi = (int)floor(fnum + 0.5);

	int lsb = (int)fnumi & 255;
	int msb = (((int)(baseNote / 12))<<3) | (int)((int)(fnumi>>8));	

	
	//Lowest note frequency equals 8.17579891564371hz, which corresponds to midi note -1
	//Tune up to B0, which is 30.868hz

	//1200 cents in an octave, so the difference between (B -1) and (B0) is 22.69220109 which is the Hz in 1200 cents.








	//setParameter( YM_FNUM2, 0, (float)msb / CHAR_MAX, channel ); 
	//setParameter( YM_FNUM1, 0, (float)lsb / CHAR_MAX, channel );

	//TODO fix optimization, this allowed less data to be written when VGM logging
	//if(channelLSB[channel] != lsb || channelMSB[channel] != msb)
	{
		writeData(getRegister(YM_FNUM2, channel % 3, 0), msb, channel, channel);
		writeData(getRegister(YM_FNUM1, channel % 3, 0), lsb, channel, channel);
		channelLSB[channel] = lsb;
		channelMSB[channel] = msb;
	}
}

void YM2612::noteOff(int channel, int note)
{
	if(channel == 5 && _drumSet != nullptr && (_drumSet->getCurrentDrum() == _drumSet->getDrum(note / 100) || _drumSet->isPitchInstrument()))
		_drumSet->setCurrentDrum(-1);

	writeData(getRegister(YM_NOTEON, channel % 3, 0),channel > 2 ? (channel + 1) : channel, channel % 3, channel);

	_chip.YM2612Update(nullptr, 0);
}

void YM2612::setParameter(YM2612Param param, int channel, int op, float value)
{
	*getParameter(param, channel, op) = value;
	if(_chipWrite) 
		writeParameter(param, channel, op);
}

void YM2612::setParameterChar(YM2612Param param, int channel, int op, unsigned char value)
{

}

void YM2612::setFromBaron(IBIndex* param, int channel, float val)
{
	if(param->getType() == IB_YMParam)
	{
		IBYMParam* ymParam = static_cast<IBYMParam*>(param);

		if(ymParam->getInstrument() >= 0)
		{
			_channels[channel].setFromBaron(param, val);
			writeParameter(ymParam->getParameter(), channel, ymParam->getOperator());
		}
	}
}

void YM2612::setFromBaronGlobal(IBIndex* param, int channel, float val)
{
	if(param->getType() == IB_YMParam)
	{
		IBYMParam* ymParam = static_cast<IBYMParam*>(param);

		if(ymParam->getInstrument() < 0)
		{
			_global.setFromBaron(param, val);
			writeParameter(ymParam->getParameter(), channel, 0);
		}
	}
}


float* YM2612::getParameter(YM2612Param param, int channel, int op)
{
	//Global YM2612 parameters
	if(param >= YM_LFO_EN && param <= YM_SPECIAL)
	{
		return _global.getParameter(param, channel, op);
	}
	//Channel parameters
	else if(param >= YM_DT && param <= YM_ALG)
	{
		channel = max( 0, min( channel, 6 ) );
		return _channels[channel].getParameter(param, channel, op);
	}
	return nullptr;
}

unsigned char YM2612::getParameterChar(YM2612Param param, int channel, int op)
{
	float parm = *getParameter(param, channel, op);
	return (unsigned char)(parm + 0.5f); 
}

void YM2612::writeParams(int channel)
{
	if(_channels[channel].getDirty())
	{
		for(int op = 0; op < 4; op++)
		{
			for(int param = YM_DT; param <= YM_SSG; param++)
			{
				if(param == YM_MUL || param == YM_AR || param == YM_DR || param == YM_RR )
					continue;

				writeParameter((YM2612Param)param, channel, op);
			}
		}
		_channels[channel].setDirty(false);
	}

	writeParameter(YM_AMS, channel, 0);
	writeParameter(YM_FB, channel, 0);
	writeParameter(YM_LFO_EN, channel, 0);
	//writeParameter(YM_SPECIAL, channel, 0);
}

void YM2612::writeParameter(YM2612Param param, int channel, int op)
{
	int reg = getRegister(param, channel % 3, op);
	unsigned char value = packParameter(param, channel, op);
	writeData(reg, value, channel, channel);
}

void YM2612::writeData(int reg, unsigned char data, int channel, int realChannel)
{
	if(_channels[realChannel]._currentDelay > 0 && realChannel >= 0)
	{
		while (_commandLock) {}
		_commandLock = true;

		if(_commands.size() == 0)
			_commands.push_back(YM2612CommandCluster(_channels[realChannel]._currentDelay));
		else if(_commands.back().delaySamples != _commands.back().originalDelay || (_commands.back().delaySamples != _channels[realChannel]._currentDelay))
			_commands.push_back(YM2612CommandCluster(_channels[realChannel]._currentDelay));

		_commands.back().commands.push_back(YM2612Command(reg, data, channel));

		_commandLock = false;
	}
	else
	{
		//The first 3 channels are parts 0 and 1, the last 3
		//are parts 2 and 3.
		if(channel <= 2)
		{
			_chip.YM2612Write(0, reg);
			_chip.YM2612Write(1, data);

			//We log YM_DAC elsewhere
			if(_logger != nullptr && _logger->isLogging() && reg != 42)
				_logger->logWriteYM2612(0, reg, data);
		}
		else
		{
			_chip.YM2612Write(2, reg);
			_chip.YM2612Write(3, data);

			//We log YM_DAC elsewhere
			if(_logger != nullptr && _logger->isLogging() && reg != 42)
				_logger->logWriteYM2612(1, reg, data);
		}
	}
}

unsigned char YM2612::packParameter(YM2612Param param, int channel, int op)
{
	switch(param)
	{
	case YM_DT:
	case YM_MUL:
		{
			//DT_MUL
			//------------------------------
			//  0   1   2   3   4   5   6   7
			//      |  DT   |   |    MUL    |
			unsigned char dt = getParameterChar(YM_DT, channel, op);
			unsigned char mul = getParameterChar(YM_MUL, channel, op);
			return (dt << 4) | mul;
		}
		break;

	case YM_TL:
		{
			return getParameterChar(YM_TL, channel, op);
		}
		break;

	case YM_KS:
	case YM_AR:
		{
			//RS_AR
			//------------------------------
			//  0   1   2   3   4   5   6   7
			//  |RS |  |X|  |      AR       |
			unsigned char rs = getParameterChar(YM_KS, channel, op);
			unsigned char ar = getParameterChar(YM_AR, channel, op);
			return (rs << 6) | ar;
		}
		break;

	case YM_AM:
	case YM_DR:
		{
			//AM_DR1
			//------------------------------
			//  0   1   2   3   4   5   6   7
			//|AM|          |      DR1      |
			unsigned char am = getParameterChar(YM_AM, channel, op);
			unsigned char dr1 = getParameterChar(YM_DR, channel, op);
			return (am << 7) | dr1;
		}
		break;

	case YM_SR:
		{
			return getParameterChar(YM_SR, channel, op);
		}
		break;

	case YM_SL:
	case YM_RR:
		{
			//DL1_RR
			//------------------------------
			//  0   1   2   3   4   5   6   7
			//  |    DL1    |   |     RR    |
			unsigned char dl1 = getParameterChar(YM_SL, channel, op);
			unsigned char rr = getParameterChar(YM_RR, channel, op);
			return (dl1 << 4) | rr;
		}
		break;

	case YM_F1:
		{
			return getParameterChar(YM_F1, channel, op);
		}
		break;

	case YM_F2:
		{
			return getParameterChar(YM_F2, channel, op);
		}
		break;

	case YM_FB:
	case YM_ALG:
		{
			//FB_ALGORITHM
			//------------------------------
			//  0   1   2   3   4   5   6   7
			//          |   FB  |   |  ALG  |
			unsigned char fb = getParameterChar(YM_FB, channel, op);
			unsigned char alg = getParameterChar(YM_ALG, channel, op);
			return (fb << 3) | alg;
		}
		break;

	case YM_AMS:
	case YM_FMS:
		{
			//AMS_FMS
			//------------------------------
			//  0   1   2   3   4   5   6   7
			// |L| |R|  |AMS|       |  FMS  |  

			//Set left and right channel enables to 1 for now
			unsigned char l = getParameterChar(YM_L_EN, channel, op);
			unsigned char r = getParameterChar(YM_R_EN, channel, op);;
			unsigned char ams = getParameterChar(YM_AMS, channel, op);
			unsigned char fms = getParameterChar(YM_FMS, channel, op);
			return (((((l << 1) | r) << 2) | ams) << 4) | fms;
		}
		break;

	case YM_LFO: 
	case YM_LFO_EN:
		{
			//LFO LFO_EN
			//--------------------------------------
			//  0   1   2   3      4            5   6   7
			//				    |LFO_EN|   |  LFO  |
			unsigned char en = getParameterChar(YM_LFO_EN, channel, op);
			unsigned char lfo = getParameterChar(YM_LFO, channel, op);
			return ((en << 3) | lfo);
		}
		break;

	case YM_SPECIAL:
		{
			return getParameterChar(YM_SPECIAL, channel, op);
		}
		break;

	case YM_SSG:
		{
			unsigned char c = getParameterChar(YM_SSG, channel, op);
			return GennyLoaders::ssgGennyToReg(c);
		}
		break;
	}
}

//Returns the register corresponding to a specific parameter. Some registers
//may be multi purpose registers, in which case it's important that you pack
//your data in a relevant way to ensure you're modifying the right parameter.
int YM2612::getRegister( YM2612Param param, int channel, int op )
{
	switch( param )
	{
	case YM_DT:
	case YM_MUL:
		return (48 + channel) + (op * 4);
		break;
	case YM_TL:
		return (64 + channel) + (op * 4);
		break;
	case YM_KS:
	case YM_AR:
		return (80 + channel) + (op * 4);
		break;
	case YM_AM:
	case YM_DR:
		return (96 + channel) + (op * 4);
		break;
	case YM_SR:
		return (112 + channel) + (op * 4);
		break;
	case YM_SL:
	case YM_RR:
		return (128 + channel) + (op * 4);
		break;
	case YM_SSG:
		return (144 + channel) + (op * 4);
		break;
	case YM_FNUM1:
		return 160 + channel;
		break;
	case YM_FNUM2:
		return 164 + channel;
		break;
	case YM_ALG:
	case YM_FB:
		return 176 + channel;
		break;
	case YM_AMS:
	case YM_FMS:
		return 0xB4 + channel;
		break;

	case YM_NOTEON:
		return 0x28;
		break;
		case YM_SPECIAL:
		return 0x27;
		break;
	case YM_F1:
		return 0xA9 - op;
		break;
	case YM_F2:
		return 0xAD - op;
		break;
	case YM_LFO:
	case YM_LFO_EN:
		return 0x22;
		break;
	case YM_DACEN:
		return 0x2B;
		break;
	case YM_DAC:
		return 0x2A;
	}
	return 0;
}

void YM2612::updateDAC()
{
	//if(_logger != nullptr && _logger->isLogging())
	//	_logger->logSample();

	if(_dacEnable == false || _drumSet == nullptr)
		return;

	WaveData* currentDrum = _drumSet->getCurrentDrum();
	if(currentDrum != nullptr)
	{
		if(_drumSet->isPitchInstrument())
		{	
			double startFreq = pow(2.0, (48.0 / 12.0)) * 8.17579891564371;
			double endFreq = pow(2.0, (72.0 / 12.0)) * 8.17579891564371;
			double freqPos = 1.0 + (((endFreq - startFreq) - (_noteFreq - startFreq)) / (endFreq - startFreq));

			currentDrum->waitTime += ((float)currentDrum->sampleRate) / (53267.0f) * (_noteFreq / startFreq);  
		}
		else
			currentDrum->waitTime += (float)currentDrum->sampleRate / (53267.0f);

		if(currentDrum->waitTime >= 1.0f)
		{
			if(currentDrum->release == false && currentDrum->loop && currentDrum->audioPosition >= currentDrum->endSample)
			{
				currentDrum->audioPosition = currentDrum->startSample;
			}

			if(currentDrum->audioPosition >= currentDrum->endSample)
			{
				currentDrum->waitTime = 0;
				writeData(getRegister(YM_DAC, -1, -1), 128, 0, 5);
				_drumSet->setCurrentDrum(-1);
				currentDrum->release = false;
			}
			else
			{
				int currentSample = 0;
				int numSamples = 0;
				while(currentDrum->waitTime >= 1.0f && currentDrum->audioPosition < currentDrum->endSample)
				{
					currentDrum->waitTime -= 1.0f;
					currentSample += currentDrum->audioData[currentDrum->audioPosition];
					currentDrum->audioPosition += 1;
					numSamples += 1;
				}
				currentSample /= numSamples;

				if(_logger != nullptr && _logger->isLogging())
					_logger->logDACSample();

				//writeData(getRegister(YM_DAC, -1, -1), min(255, max(0, ((((int)currentDrum->audioData[currentDrum->audioPosition] - 127) * _sampleVolume) + 127))), 0);
				//writeData(getRegister(YM_DAC, -1, -1), min(255, max(0, (int)((((float)currentDrum->audioData[currentDrum->audioPosition] - 128.0f) * _sampleVolume) + 0.5f) + 128)), 0, 5);



				//writeData(getRegister(YM_DAC, -1, -1), min(255, max(0, (int)((((float)currentDrum->audioData[currentDrum->audioPosition] - 128.0f)) + 0.5f) + 128)), 0, 5);
				
				//writeData(getRegister(YM_DAC, -1, -1), currentDrum->audioData[currentDrum->audioPosition] * _sampleVolume, 0, 5);
				//writeData(getRegister(YM_DAC, -1, -1), min(255, max(0, ((((int)currentDrum->audioData[currentDrum->audioPosition] - 127) * _sampleVolume) + 127))), 0);
				writeData(getRegister(YM_DAC, -1, -1), min(255, max(0, (int)((((float)currentSample - 128.0f) * _sampleVolume) + 0.5f) + 128)), 0, 5);

				//writeData(getRegister(YM_DAC, -1, -1), (int)(currentDrum->audioData[currentDrum->audioPosition] * _sampleVolume), 0, 5);
				_mutedDAC = false;
			}
			
		}
	}
	else if(_mutedDAC == false)
	{
		_mutedDAC = true;
		writeData(getRegister(YM_DAC, -1, -1), 128, 0, 5);
	}
}

void YM2612::midiTick()
{
	while (_commandLock) {}
	_commandLock = true;

	int sz = _commands.size();
	for (int i = 0; i < sz; i++)
	{
		_commands[i].delaySamples -= 1;
		if (_commands[i].delaySamples <= 0)
		{
			int innerSz = _commands[i].commands.size();
			for (int j = 0; j < innerSz; j++)
				writeData(_commands[i].commands[j].reg, _commands[i].commands[j].data, _commands[i].commands[j].channel, -1);

			_commands.erase(_commands.begin() + i);
			i--;
			sz--;
		}
	}
	_commandLock = false;
}

void YM2612::runSample()
{

	_commandLock = false;
}

void YM2612::setDACEnable(bool enable)
{
	if(_dacEnable != enable)
	{
		_dacEnable = enable;
		writeData(getRegister(YM_DACEN, -1, -1), _dacEnable ? 128 : 0, 0, 5);
	}
}

void YM2612::dirtyChannels()
{
	_channels[0].setDirty(true);
	_channels[1].setDirty(true);
	_channels[2].setDirty(true);
	_channels[3].setDirty(true);
	_channels[4].setDirty(true);
	_channels[5].setDirty(true);
}