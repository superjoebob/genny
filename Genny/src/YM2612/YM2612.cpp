#include "YM2612.h"
#include "IndexBaron.h"
#include "GennyLoaders.h"
#include "DrumSet.h"
#include "GennyVST.h"

#include <windows.h>

//#include <algorithm>

using namespace std;

HANDLE serialHandle;
bool doWrite = false;
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
	else if(param >= YM_AMS && param <= YM_SPECIAL)
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

	for(int p = YM_AMS; p <= YM_SPECIAL; p++)
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
//float* YM2612Global::getParameter(YM2612Param param, int channel, int op)
//{
//	//Global YM2612 parameters
//	if(param >= YM_LFO_EN && param <= YM_SPECIAL)
//	{
//		std::map<YM2612Param, float>::iterator it = _parameters.find(param);
//		if( it != _parameters.end() )
//			return &(*it).second;
//
//		//Add the parameter if it is not yet added
//		_parameters[param] = 0;
//		return &_parameters[param];
//	}
//}
//
//unsigned char YM2612Global::getParameterChar(YM2612Param param, int channel, int op)
//{
//	float parm = *getParameter(param, channel, op);
//	return (unsigned char)(parm + 0.5f); 
//}
//
//
//void YM2612Global::catalogue(IndexBaron* baron)
//{
//	for(int p = YM_LFO_EN; p <= YM_SPECIAL; p++)
//	{
//		baron->addIndex(new IBYMParam((YM2612Param)p, -1, -1));
//	}
//}
//
//void YM2612Global::setFromBaron(IBIndex* param, float val)
//{
//	if(param->getType() == IB_YMParam)
//	{
//		IBYMParam* p = static_cast<IBYMParam*>(param);
//		char op = p->getOperator();
//		*getParameter(p->getParameter(), 0, op) = val;
//	}
//}
//
//float YM2612Global::getFromBaron(IBIndex* param)
//{
//	if(param->getType() == IB_YMParam)
//	{
//		IBYMParam* p = static_cast<IBYMParam*>(param);
//		char op = p->getOperator();
//		return *getParameter(p->getParameter(), 0, op);
//	}
//	return 0.0f;
//}



//YM2612
//-------------------------------------------------------------------
YM2612::YM2612(GennyVST* pVST)
	: _chipWrite(false)
	, _freqMult(0.0f)
	, _logger(nullptr)
	, _mutedDAC(false)
	, _drumSet(nullptr)
	, _sampleVolume(1.0f)
	, _updateNoteFreq(false)
	, _noteFreq(0.0f)
	, _mutex()
	, _vst(pVST)
	, _hardwareMode(false)
	//, _writingChunk(0)
	, _writingHigh(false)
	,_lastSample(0)
	,_writeSamples(false)
	,_emulationMute(false)
	,_invalidateDAC(false)
	,_sleep(false)
	,_clock(YM2612Clock::YM2612_NTSC)
	,_clockDivider(0.0f)
	,_mdmMode(true)
{
	clearCache();

	//_commandBuffer = new GennyData();

	//if (_hardwareMode)
	//{
	//	serialHandle = CreateFile("\\\\.\\COM3", GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	//	// Do some basic settings
	//	DCB serialParams = { 0 };
	//	serialParams.DCBlength = sizeof(serialParams);

	//	GetCommState(serialHandle, &serialParams);
	//	serialParams.BaudRate = CBR_256000;
	//	serialParams.ByteSize = 8;
	//	serialParams.StopBits = ONESTOPBIT;
	//	serialParams.Parity = NOPARITY;
	//	SetCommState(serialHandle, &serialParams);

	//	// Set timeouts
	//	COMMTIMEOUTS timeout = { 0 };
	//	timeout.ReadIntervalTimeout = 1;
	//	timeout.ReadTotalTimeoutConstant = 1;
	//	timeout.ReadTotalTimeoutMultiplier = 1;
	//	timeout.WriteTotalTimeoutConstant = 1;
	//	timeout.WriteTotalTimeoutMultiplier = 1;

	//	SetCommTimeouts(serialHandle, &timeout);

	//	char lpBuffer[] = "x";
	//	DWORD dNoOFBytestoWrite;         // No of bytes to write into the port
	//	DWORD dNoOfBytesWritten = 0;     // No of bytes written to the port
	//	dNoOFBytestoWrite = sizeof(lpBuffer);

	//	bool Status = WriteFile(serialHandle,        // Handle to the Serial port
	//		lpBuffer,     // Data to be written to the port
	//		dNoOFBytestoWrite,  //No of bytes to write
	//		&dNoOfBytesWritten, //Bytes written
	//		NULL);
	//}

}


YM2612::~YM2612(void)
{
	//if(_hardwareMode)
	//	CloseHandle(serialHandle);
}


void YM2612::initialize(YM2612Clock clock, int soundRate)
{
	_clock = clock;
	_chip.YM2612Init(clock, soundRate);
	_chip.YM2612Restore((unsigned char*)&_chip.ym2612);
	_chip.YM2612LoadContext((unsigned char*)&_chip.ym2612);
	_chip.YM2612ResetChip();

	_chipAccurate.YM2612Init();
	_chipAccurate.YM2612LoadContext((unsigned char*)&_chipAccurate.ym2612);
	_chipAccurate.YM2612ResetChip();
	_chipAccurate.YM2612Config(0);



	//Frequency formula is (144 * noteFrequency * 2 ^ 20 / masterClock) / 2 ^ block - 1
	//_freqMult is (2 ^ 20 / 12) / (masterClock / 12) to make the returned number
	//smaller for proper use in the equation.
	_freqMult = (float)87381 / ((float)clock / 12);
	_clockDivider = pow(2, 20) / (float)clock;
	_clockDividerMegaMidi = pow(2, 20) / (float)YM2612Clock::YM2612_MEGAMIDI;
}

void YM2612::mute(int channel)
{
	//int reg = getRegister(YM2612Param::YM_RR, channel % 3, 0);
	//writeData(reg, 255, channel, channel);
	//reg = getRegister(YM2612Param::YM_RR, channel % 3, 1);
	//writeData(reg, 255, channel, channel);
	//reg = getRegister(YM2612Param::YM_RR, channel % 3, 2);
	//writeData(reg, 255, channel, channel);
	//reg = getRegister(YM2612Param::YM_RR, channel % 3, 3);
	//writeData(reg, 255, channel, channel);

	//reg = getRegister(YM2612Param::YM_TL, channel % 3, 0);
	//writeData(reg, 255, channel, channel);
	//reg = getRegister(YM2612Param::YM_TL, channel % 3, 1);
	//writeData(reg, 255, channel, channel);
	//reg = getRegister(YM2612Param::YM_TL, channel % 3, 2);
	//writeData(reg, 255, channel, channel);
	//reg = getRegister(YM2612Param::YM_TL, channel % 3, 3);
	//writeData(reg, 255, channel, channel);	
	//
	//reg = getRegister(YM2612Param::YM_AR, channel % 3, 0);
	//writeData(reg, 0, channel, channel);
	//reg = getRegister(YM2612Param::YM_AR, channel % 3, 1);
	//writeData(reg, 0, channel, channel);
	//reg = getRegister(YM2612Param::YM_AR, channel % 3, 2);
	//writeData(reg, 0, channel, channel);
	//reg = getRegister(YM2612Param::YM_AR, channel % 3, 3);
	//writeData(reg, 0, channel, channel);

	//noteOff(channel, 0);
}
//
//void YM2612::beginCommandChunk()
//{
//	if (_writingChunk == 0)
//		_commandBuffer->writeByte('?');
//	_writingChunk++;
//}
//
//void YM2612::endCommandChunk()
//{
//	_writingChunk--;
//
//	if (_writingChunk == 0)
//		_commandBuffer->writeByte('!');
//}

void YM2612::noteOn(int note, int velocity, int channel, double* frequencyTable, GennyPatch* patch)
{
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
		{
			if (_dacEnable != false && _drumSet->setCurrentDrum(drumNote) == false)
				return;
		}

		if(_logger != nullptr && _logger->isLogging() && _drumSet->getCurrentDrum() != nullptr)
			_logger->seekDACSample(_drumSet->getCurrentDrum()->streamStartPos);

		if (_dacEnable != false && _drumSet != nullptr)
		{
			writeFrequencies(note, velocity, channel, 0.0f, frequencyTable, patch);
			return;
		}
	}


	noteOff(channel, note, false);

	writeFrequencies(note, velocity, channel, 0.0f, frequencyTable, patch);


	doWrite = true;
	writeData(getRegister(YM_NOTEON, channel % 3, 0), 0xF0 | (channel > 2 ? (channel + 1) : channel), channel % 3, channel);


	//if (_mdmMode)
	//{
	//	SendMIDIGenMDM(144, channel, note / 100, min(velocity, 127));
	//}

	writeParams(channel);



	doWrite = false;
}

void YM2612::killNote(int channel)
{
}

bool wrote = false;
void YM2612::writeFrequencies(int note, int velocity, int channel, float vibrato, double* frequencyTable, GennyPatch* patch)
{
	if (channel == 5 && patch != nullptr && _drumSet != nullptr)
	{
		float v = velocity / 127.0f; //Un-root the velocity for drum samples
		v = v * v;
		v = v * v;
		v *= 127;
		velocity = (int)v;

		float tlSampleVol = (*patch->InstrumentDef.Data._operators[3].find(YM_DRUMTL)).second;//*getParameter(YM_DRUMTL, channel, 3);
		_sampleVolume = (((float)velocity / 127.0f) * ((tlSampleVol / 100.0f) + ((max(tlSampleVol - 100, 0) / 27.0f) * 3.0f)));

		if (_dacEnable != false && _drumSet != nullptr && _updateNoteFreq == false)
		{
			return;
		}
	}


	for (int i = 0; i < 4; i++)
	{
		if ((patch == nullptr && i == 3) || patch->InstrumentDef.OperatorVelocity[i])
		{
			int reg = getRegister(YM_TL, channel % 3, i);

			float t = *getParameter(YM_TL, channel, i);
			int vel = ((127.0f - t) / 127.0f) * velocity;

			if (vel > 127) 
				vel = 127;

			float tlVol = *getParameter(YM_TL, channel, i);
			unsigned char volume = tlVol * (127 - velocity);

			//TODO fix optimization, this allowed less data to be written when VGM logging
			//if(127 - vel != channelVelocity[channel])
			{
				writeData(reg, max(127 - vel, 0), channel, channel);
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
		return;
	}

	//Lowest note frequency equals 8.17579891564371hz, which corresponds to midi note -1
	//Tune up to B0, which is 30.868hz

	//1200 cents in an octave, so the difference between (B -1) and (B0) is 22.69220109 which is the Hz in 1200 cents.

	double fnum = (144 * freq * _clockDivider) / pow(2.0, ((int)(baseNote / 12.0)) - 2.0);
	int fnumi = (int)floor(fnum + 0.5);
	int lsb = (int)fnumi & 255;
	int msb = (((int)(baseNote / 12))<<3) | (int)((int)(fnumi>>8));	

	int lsbMM = -1;
	int msbMM = -1;

	if (_hardwareMode)
	{
		//Special frequency writes for MEGA MIDI
		double fnumMM = (144 * freq * _clockDividerMegaMidi) / pow(2.0, ((int)(baseNote / 12.0)) - 2.0);
		int fnumiMM = (int)floor(fnumMM + 0.5);
		lsbMM = (int)fnumiMM & 255;
		msbMM = (((int)(baseNote / 12)) << 3) | (int)((int)(fnumiMM >> 8));
	}

	//TODO fix optimization, this allowed less data to be written when VGM logging
	if(channelLSB[channel] != lsb || channelMSB[channel] != msb)
	{
		writeData(getRegister(YM_FNUM2, channel % 3, 0), msb, channel, channel, false, msbMM);
		writeData(getRegister(YM_FNUM1, channel % 3, 0), lsb, channel, channel, false, lsbMM);
		channelLSB[channel] = lsb;
		channelMSB[channel] = msb;
	}


	//if (wrote)
	//{
	//	writeParameter(YM_SL, 0, 0);
	//	//writeParameter(YM_SL, 0, 1);
	//	//writeParameter(YM_SL, 0, 2);
	//	//writeParameter(YM_SL, 0, 3);
	//}

	//endCommandChunk();
}

void YM2612::noteOff(int channel, int note, bool fromMidiMessage)
{
	//beginCommandChunk();
	if (channel == 5 && _drumSet != nullptr && (_drumSet->getCurrentDrum() == _drumSet->getDrum(note / 100) || _drumSet->isPitchInstrument()))
		_drumSet->setCurrentDrum(-1);

	if (channel == 5 && _dacEnable != false && _drumSet != nullptr)
	{
		return;
	}


	//doWrite = true;
	writeData(getRegister(YM_NOTEON, channel % 3, 0),channel > 2 ? (channel + 1) : channel, channel % 3, channel);



	//if (fromMidiMessage && _mdmMode)
	//	SendMIDIGenMDM(128, channel, note / 100, 64);

	//doWrite = false;

	//endCommandChunk();
	//_chip.YM2612Update(nullptr, 0);
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



void YM2612::setFromBaron(IBIndex* param, int channel, float val, bool pForceDACWrite)
{
	if (!pForceDACWrite && channel == 5 && _dacEnable != false && _drumSet != nullptr)
		return;

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
//
//void YM2612::writeChannel(YM2612Channel& pFMChannel, int pChannelIndex)
//{
//	for (int i = 0; i < 4; i++)
//	{
//		auto it = pFMChannel._operators[i].begin();
//		while (it != pFMChannel._operators[i].end())
//		{
//			_values[(*it).first] = (*it).second;
//			writeParameter((*it).first, pChannelIndex, i);
//
//
//			it++;
//		}
//	}
//}

//void YM2612::setFromBaronGlobal(IBIndex* param, int channel, float val)
//{
//	if(param->getType() == IB_YMParam)
//	{
//		IBYMParam* ymParam = static_cast<IBYMParam*>(param);
//
//		if(ymParam->getInstrument() < 0)
//		{
//			_global.setFromBaron(param, val);
//			writeParameter(ymParam->getParameter(), channel, 0);
//		}
//	}
//}


float* YM2612::getParameter(YM2612Param param, int channel, int op)
{
	//Channel parameters
	if(param >= YM_DT && param <= YM_SPECIAL)
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
	//if (wrote == false)
	{
		if (_channels[channel].getDirty())
		{
			for (int op = 3; op >=0; op--)
			{
				for (int param = YM_DT; param <= YM_SSG; param++)
				{
					if (param == YM_MUL || param == YM_AR || param == YM_DR || param == YM_RR)
						continue;

					writeParameter((YM2612Param)param, channel, op);
				}
			}
			_channels[channel].setDirty(false);


		}

		writeParameter(YM_AMS, channel, 0);
		writeParameter(YM_FB, channel, 0);
		writeParameter(YM_LFO_EN, channel, 0);
		wrote = true;
	}
	//else
	//{
	//	writeParameter(YM_TL, channel, 0);
	//	writeParameter(YM_TL, channel, 1);
	//	writeParameter(YM_TL, channel, 2);
	//	writeParameter(YM_TL, channel, 3);

	//	writeParameter(YM_SL, channel, 0);
	//	writeParameter(YM_SL, channel, 1);
	//	writeParameter(YM_SL, channel, 2);
	//	writeParameter(YM_SL, channel, 3);

	//	writeParameter(YM_SR, channel, 0);
	//	writeParameter(YM_SR, channel, 1);
	//	writeParameter(YM_SR, channel, 2);
	//	writeParameter(YM_SR, channel, 3);
	//}

	//writeParameter(YM_SR, channel, 0);
	//writeParameter(YM_SR, channel, 1);
	//writeParameter(YM_SR, channel, 2);
	//writeParameter(YM_SR, channel, 3);

	//writeParameter(YM_SR, channel, 0);
	//writeParameter(YM_SR, channel, 1);
	//writeParameter(YM_SR, channel, 2);
	//writeParameter(YM_SR, channel, 3);


	//writeParameter(YM_SPECIAL, channel, 0);
}

void YM2612::writeParameter(YM2612Param param, int channel, int op)
{

	int reg = getRegister(param, channel % 3, op);
	if (reg != 0)
	{
		//if (_mdmMode /*&& !wrote*/)
		//{
		//	int cc = GetGenMDMCCForParam(param, op);
		//	if (cc >= 0)
		//	{
		//		unsigned char actualVal = getParameterChar(param, channel, op);
		//		if (param == YM2612Param::YM_TL)
		//			actualVal = 127 - actualVal;
		//		//else if (param == YM2612Param::YM_RR)
		//			
		//		//if(param != YM2612Param::YM_SR)
		//		actualVal = (int)((actualVal / (float)YM2612Param_getRange(param)) * 127);

		//		//if (param == YM2612Param::YM_SR)
		//		//	actualVal = 127 - actualVal;

		//		SendMIDIGenMDM(176, channel, cc, actualVal);
		//	}
		//}

		unsigned char value = packParameter(param, channel, op);
		writeData(reg, value, channel, channel);
	}
}


int YM2612::GetGenMDMCCForParam(YM2612Param vParam, int vOp)
{
	//if (vOp == 1)
	//	vOp = 2;
	//else if (vOp == 2)
	//	vOp = 1;

	switch (vParam)
	{
	case YM2612Param::YM_TL:
		return 16 + vOp;
	case YM2612Param::YM_MUL:
		return 20 + vOp;
	case YM2612Param::YM_DT:
		return 24 + vOp;
	case YM2612Param::YM_KS:
		return 39 + vOp;
	case YM2612Param::YM_AR:
		return 43 + vOp;

	case YM2612Param::YM_SL://CORRECT
		return 55 + vOp;

	case YM2612Param::YM_DR://CORRECT
		return 47 + vOp;

	case YM2612Param::YM_SR://DR2
		return 51 + vOp;


	case YM2612Param::YM_RR:
		return 59 + vOp;
	case YM2612Param::YM_AM:
		return 70 + vOp;

	case YM2612Param::YM_ALG:
		return 14;
	case YM2612Param::YM_FB:
		return 15;
	case YM2612Param::YM_AMS:
		return 76;
	case YM2612Param::YM_FMS:
		return 75;
	}

	return -1;
}

char chanInc = 0;
char regInc = 0;
char dataInc = 0;
void YM2612::SendMIDI(unsigned char chan, unsigned char reg, unsigned char data)
{
	if (_vst->megaMidiPort == 0)
		return;

	unsigned char status = (8 << 4) | (chan << 2) | ((reg & 1) << 1) | ((data & 1));
	_vst->midiOut(status, reg >> 1, data >> 1, _vst->megaMidiPort - 1);
}

void YM2612::SendMIDIGenMDM(int status, unsigned char chan, unsigned char cc, unsigned char data)
{
	if (_vst->genMDMPort == 0)
		return;

	unsigned char fullStatusVal = status | chan;
	_vst->midiOut(fullStatusVal, cc, data, _vst->genMDMPort - 1);
}

void YM2612::SendChipReset()
{
	if (_vst->megaMidiPort == 0)
		return;

	_vst->midiOut((9 << 4), 123, 124, _vst->megaMidiPort - 1);
}

void YM2612::writeData(int reg, unsigned char data, int channel, int realChannel, bool inLock, int specialLogData)
{
	if (inLock == false)
		_mutex.lock();

	if(_channels[realChannel]._currentDelay > 0 && realChannel >= 0)
	{
		if(_commands.size() == 0)
			_commands.push_back(YM2612CommandCluster(_channels[realChannel]._currentDelay));
		else if(_commands.back().delaySamples != _commands.back().originalDelay || (_commands.back().delaySamples != _channels[realChannel]._currentDelay))
			_commands.push_back(YM2612CommandCluster(_channels[realChannel]._currentDelay));

		_commands.back().commands.push_back(YM2612Command(reg, data, channel, specialLogData));

	}
	else
	{
		int val = reg + (channel * 400);

		auto it = _writeMap.find(val);
		if (((reg >= 160 && reg <= 169) || reg == 0x2A) || it == _writeMap.end() || (*it).second != data)
		{
			_writeMap.insert_or_assign(val, data);

			//The first 3 channels are parts 0 and 1, the last 3
			//are parts 2 and 3.
			if (channel <= 2)
			{
				if (!_emulationMute)
				{
					if (_vst->accurateEmulationMode)
					{
						_chipAccurate.YM2612Write(0, reg);
						_chipAccurate.YM2612Write(1, data);
					}
					else
					{
						_chip.YM2612Write(0, reg);
						_chip.YM2612Write(1, data);
					}
				}

				if (_hardwareMode && reg != 0x2A)
					SendMIDI(0, reg, specialLogData >= 0 ? ((unsigned char)specialLogData) : data);

				//We log YM_DAC elsewhere
				if (_logger != nullptr && _logger->isLogging() && reg != 42)
					_logger->logWriteYM2612(0, reg, data);
			}
			else
			{
				if (!_emulationMute)
				{
					if (_vst->accurateEmulationMode)
					{
						_chipAccurate.YM2612Write(2, reg);
						_chipAccurate.YM2612Write(3, data);
					}
					else
					{
						_chip.YM2612Write(2, reg);
						_chip.YM2612Write(3, data);
					}
				}

				if(_hardwareMode)
					SendMIDI(1, reg, specialLogData >= 0 ? ((unsigned char)specialLogData) : data);

				//We log YM_DAC elsewhere
				if (_logger != nullptr && _logger->isLogging() && reg != 42)
					_logger->logWriteYM2612(1, reg, data);
			}
		}
	}

	if (inLock == false)
		_mutex.unlock();
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
	case YM_L_EN:
	case YM_R_EN:
		{
			//AMS_FMS
			//------------------------------
			//  0   1   2   3   4   5   6   7
			// |L| |R|  |AMS|       |  FMS  |  

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
	case YM_L_EN:
	case YM_R_EN:
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

int wait = 0;
void YM2612::updateDAC()
{





	_hardwareMode = _vst->megaMidiPort > 0;
	_emulationMute = _vst->megaMidiPort > 0 && _vst->megaMidiVSTMute;

	if (_sleep)
		return;



	if(_logger != nullptr && _logger->isLogging())
		_logger->logSample();

	if (_dacEnable != false && _drumSet != nullptr)
	{
		WaveData* currentDrum = _drumSet->getCurrentDrum();
		if (currentDrum != nullptr)
		{
			float chipFreq = 44100;
			//float chipFreq = 53267.0f;
			if (_drumSet->isPitchInstrument())
			{
				double startFreq = pow(2.0, (48.0 / 12.0)) * 8.17579891564371;
				double endFreq = pow(2.0, (72.0 / 12.0)) * 8.17579891564371;
				double freqPos = 1.0 + (((endFreq - startFreq) - (_noteFreq - startFreq)) / (endFreq - startFreq));

				currentDrum->audioPosition += ((float)currentDrum->sampleRate) / (chipFreq) * (_noteFreq / startFreq);
			}
			else
				currentDrum->audioPosition += (float)currentDrum->sampleRate / (chipFreq);


			//if (currentDrum->sampleRate < 15000)
			//	currentDrum->waitTime += (float)currentDrum->sampleRate / (chipFreq);
			//else
				currentDrum->waitTime += 0.25f/*_hardwareMode ? 0.25f : 0.5f*/; //waiting 4 steps allows all samples to be interpreted as 11025hz, which is easier for the chip to deal with.


			if (currentDrum->waitTime >= 1.0f) 
			{
				if (currentDrum->release == false && currentDrum->loop && currentDrum->audioPosition >= currentDrum->endSample)
				{
					currentDrum->audioPosition = currentDrum->startSample;

					if (_logger != nullptr && _logger->isLogging())
						_logger->seekDACSample(currentDrum->streamStartPos);
				}

				currentDrum->waitTime -= 1.0f;
				if (currentDrum->audioPosition >= currentDrum->endSample)
				{
					writeData(getRegister(YM_DAC, -1, -1), 128, 0, 5);
					_drumSet->setCurrentDrum(-1);
					currentDrum->release = false;
				}
				else
				{
					//int currentSample = 0;
					//int numSamples = 0;
					//while (currentDrum->waitTime >= 1.0f && currentDrum->audioPosition < currentDrum->endSample)
					//{
					//	currentDrum->waitTime -= 1.0f;
					//	currentSample = currentDrum->audioData[currentDrum->audioPosition];
					//	currentDrum->audioPosition += 1;
					//	numSamples += 1;
					//}
					//currentSample /= numSamples;

					if (_logger != nullptr && _logger->isLogging())
						_logger->logDACSample();


					int first = currentDrum->audioData[(int)currentDrum->audioPosition];
					int second = first;
					if ((int)currentDrum->audioPosition < currentDrum->endSample - 1)
					{
						second = currentDrum->audioData[(int)currentDrum->audioPosition + 1];
					}

					float lerp = currentDrum->audioPosition - (int)currentDrum->audioPosition;
					int currentSample = (int)((first * (1.0f - lerp)) + (second * lerp));
					unsigned char sample = min(255, max(0, (int)((((float)currentSample - 128.0f) * _sampleVolume) + 0.5f) + 128));
					//unsigned char sample = min(255, max(0, currentSample * _sampleVolume));


					if(!_emulationMute)
						writeData(getRegister(YM_DAC, -1, -1), sample, 0, 5);
					
					if(_hardwareMode)
					{
						if (_writeSamples)
							SendMIDI(3, sample, _lastSample);
						_writeSamples = !_writeSamples;
						_lastSample = sample;
					}


					_mutedDAC = false;
				}

			}
		}
		else if (_mutedDAC == false)
		{
			_mutedDAC = true;
			writeData(getRegister(YM_DAC, -1, -1), 128, 0, 5);
		}
	}	

}

void YM2612::midiTick()
{
	if (_sleep)
		return;

	_mutex.lock();
	auto it = _commands.begin();
	while(it != _commands.end())
	{
		(*it).delaySamples -= 1;
		if ((*it).delaySamples <= 0)
		{
			int innerSz = (*it).commands.size();
			for (int j = 0; j < innerSz; j++)
			{
				writeData((*it).commands[j].reg, (*it).commands[j].data, (*it).commands[j].channel, -1, true, (*it).commands[j].logData);
			}

			it = _commands.erase(it);
		}
		else
			++it;
	}
	_mutex.unlock();

	/*if (_commandBuffer->dataPos > 0)
	{
		DWORD dNoOfBytesWritten = 0;     // No of bytes written to the port


		bool Status = WriteFile(serialHandle,        // Handle to the Serial port
			_commandBuffer->data,     // Data to be written to the port
			_commandBuffer->dataPos,  //No of bytes to write
			&dNoOfBytesWritten, //Bytes written
			NULL);

		_commandBuffer->dataPos = 0;
	}*/
}

void YM2612::runSample()
{
}

void YM2612::setDACEnable(bool enable)
{
	if(_dacEnable != enable || _invalidateDAC)
	{
		_dacEnable = enable;
		writeData(getRegister(YM_DACEN, -1, -1), _dacEnable ? 128 : 0, 0, 5);
		_invalidateDAC = false;
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

void YM2612::clearCache()
{
	dirtyChannels();
	_writeMap.clear();

	for (int i = 0; i < 6; i++)
	{
		channelVelocity[i] = -1;
		channelLSB[i] = -1;
		channelMSB[i] = -1;
	}

	_invalidateDAC = true;
}


void YM2612::Update(int *buffer, int length)
{
	if (_sleep)
		return;

	if (_vst->accurateEmulationMode)
		_chipAccurate.YM2612Update(buffer, length);
	else
		_chip.YM2612Update(buffer, length);
}