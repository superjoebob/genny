#include "YM2612.h"
#include "IndexBaron.h"
#include "GennyLoaders.h"
#include "DrumSet.h"
#include "GennyVST.h"
//#include "YM2612Processor.h"

#include <windows.h>

//#include <algorithm>

using namespace std;

HANDLE serialHandle;
//YM2612Channel
//-------------------------------------------------------------------
float* YM2612Channel::getParameter(YM2612Param param, int channel, int op)
{	
	//Operator parameters
	if(param >= YM_DT && param <= YM_SSG)
	{
		op = max( 0, min( op, 3 ) );
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
			baron->addIndex(new IBYMParam((YM2612Param)p, op, 0, 0, YM2612Param_getRange((YM2612Param)p), YM2612Param_getName((YM2612Param)p)));
		}
	}

	for(int p = YM_AMS; p <= YM_SPECIAL; p++)
	{
		IBYMParam* par = new IBYMParam((YM2612Param)p, -1, 0, 0, YM2612Param_getRange((YM2612Param)p), YM2612Param_getName((YM2612Param)p));
		if (p == YM2612Param::YM_LFO || p == YM2612Param::YM_LFO_EN)
			par->global = true;

		baron->addIndex(par);
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
std::thread::id this_id;
bool threadInit = false;
int kMaxPreparedClusters = 256;
int kCurrentPreparedCluster = 0;
int kThreadPreparedCluster = 0;
YM2612::YM2612(GennyVST* pVST)
	: _chipWrite(false)
	, _freqMult(0.0f)
	, _logger(nullptr)
	, _mutedDAC(0)
	, _drumSet(nullptr)
	, _sampleVolume(1.0f)
	, _updateNoteFreq(false)
	, _noteFreq(0.0f)
	, _vst(pVST)
	, _hardwareMode(false)
	//, _writingChunk(0)
	, _writingHigh(false)
	, _lastSample(0)
	, _writeSamples(false)
	, _emulationMute(false)
	, _sleep(false)
	, _clock(YM2612Clock::YM2612_NTSC)
	, _clockDivider(0.0f)
	, _mdmMode(true)
	, _dirtyCluster(false)
	, _currentCluster(-1)
	, _sampleRate(44100)
	, _lastDACSample(128)
	, _inSpecialMode3(-1)
	, _dacEnable(-1)
	, _resetDACSamplerate(true)
{
	clearCache();

	//_preparedClusters = new YM2612CommandCluster*[kMaxPreparedClusters];
	//for (int i = 0; i < kMaxPreparedClusters; i++)
	//{
	//	_preparedClusters[i] = nullptr;
	//}


	ch3SpecialOpNoteOnStates[0] = false;
	ch3SpecialOpNoteOnStates[1] = false;
	ch3SpecialOpNoteOnStates[2] = false;
	ch3SpecialOpNoteOnStates[3] = false;

	for (int i = 0; i < 6; i++)
	{
		channelPanStatesL[i] = 1;
		channelPanStatesR[i] = 1;
		channelPanStatesWrittenL[i] = -1;
		channelPanStatesWrittenR[i] = -1;
	}


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
	//if (_preparedClusters != nullptr)
	//{
	//	delete[] _preparedClusters;
	//	_preparedClusters = nullptr;
	//}
	//if(_hardwareMode)
	//	CloseHandle(serialHandle);
}


void YM2612::initialize(YM2612Clock clock, int soundRate)
{
	_clock = clock;
	_sampleRate = soundRate;

	_chip.YM2612Init(clock, soundRate);
	_chip.YM2612Restore((unsigned char*)&_chip.ym2612);
	_chip.YM2612LoadContext((unsigned char*)&_chip.ym2612);
	_chip.YM2612ResetChip();

	_chipAccurate.YM2612Init();
	_chipAccurate.YM2612LoadContext((unsigned char*)&_chipAccurate.ym2612);
	_chipAccurate.YM2612ResetChip();
	_chipAccurate.YM2612Config(0);


	//OPN2_Reset(&_chipNuked);
	//OPN2_SetChipType(ym3438_mode_ym2612);


	//Frequency formula is (144 * noteFrequency * 2 ^ 20 / masterClock) / 2 ^ block - 1
	//_freqMult is (2 ^ 20 / 12) / (masterClock / 12) to make the returned number
	//smaller for proper use in the equation.
	_freqMult = (float)87381 / ((float)clock / 12);
	_clockDivider = pow(2, 20) / (float)clock;
	_clockDividerMegaMidi = pow(2, 20) / (float)YM2612Clock::YM2612_MEGAMIDI;

	setSampleRate(_sampleRate);
}
void YM2612::setSampleRate(double rate)
{
	_sampleRate = rate;
	_chip.SetSampleRate((int)rate);
	LFOChanged();
}

void YM2612::LFOChanged()
{
	writeParameter(YM2612Param::YM_LFO, 0, 0);
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

bool YM2612::slotUpCurrentDrum(int note)
{
	if (_drumSet != nullptr)
	{
		int drumNote = note / 100;
		if (_drumSet->isPitchInstrument() && _drumSet->getDrum(_drumSet->pitchInstrumentIndex(), _drumSet->_sampleRate, _logger != nullptr || true) != NULL)
		{
			_updateNoteFreq = true;
			_drumSet->setCurrentDrum(_drumSet->pitchInstrumentIndex(), _drumSet->_sampleRate, _logger != nullptr || true);
		}
		else
		{
			if (_dacEnable != false && _drumSet->setCurrentDrum(drumNote, _drumSet->_sampleRate, _logger != nullptr || true) == false)
				return false;
		}
	}

	return true;
}

void YM2612::fireCurrentDrum(int note, int velocity, int channel, double* frequencyTable, GennyPatch* patch)
{
	if (_drumSet != nullptr)
	{
		float v = velocity / 127.0f; //Un-root the velocity for drum samples
		v = v * v;
		v = v * v;
		v *= 127;
		velocity = (int)v;

		float tlSampleVol = (*patch->InstrumentDef.Data._operators[3].find(YM_DRUMTL)).second;//*getParameter(YM_DRUMTL, channel, 3);
		_sampleVolume = (((float)velocity / 127.0f) * ((tlSampleVol / 100.0f) + ((max(tlSampleVol - 100, 0.0f) / 27.0f) * 3.0f)));

		if (_dacEnable)
		{
			int v = patch->getExtParam(GEParam::DACSamplerate);
			if (v > -1 && v < 4)
			{
				if (_hardwareMode && (_drumSet->_sampleRate != kDACSamplerates[v] || _resetDACSamplerate))
				{
					SendMIDI(2, (unsigned char)v, 10);
					_resetDACSamplerate = false;
				}

				_drumSet->setSampleRate(kDACSamplerates[v]);
			}

			if (_logger != nullptr)
			{
				WaveData* curDrum = _drumSet->getCurrentDrum(_drumSet->_sampleRate, _logger != nullptr || true);
				_logger->prepareDrum(curDrum, _sampleVolume);
				_logger->seekDACSample(curDrum->getStreamStart(_sampleVolume));
			}

			writeFrequencies(note, velocity, channel, 0.0f, frequencyTable, patch);
			return;
		}
	}
}

void YM2612::noteOn(int note, int velocity, int channel, double* frequencyTable, GennyPatch* patch, bool retrigger)
{
	if (retrigger == false)
		noteOff(channel, note, false);

	writeFrequencies(note, velocity, channel, 0.0f, frequencyTable, patch);
	if (retrigger == false)
	{
		unsigned char opEnable = patch->InstrumentDef.OperatorEnable[0] | (patch->InstrumentDef.OperatorEnable[1] << 1) | (patch->InstrumentDef.OperatorEnable[2] << 2) | (patch->InstrumentDef.OperatorEnable[3] << 3);
		writeData(getRegister(YM_NOTEON, channel % 3, 0), (opEnable << 4) | (channel > 2 ? (channel + 1) : channel), channel % 3, channel);
	}

	//if (_mdmMode)
	//{
	//	SendMIDIGenMDM(144, channel, note / 100, min(velocity, 127));
	//}
}

void YM2612::noteOnCh3Special(int note, int velocity, float vibrato, double* frequencyTable, GennyPatch* patch, bool retrigger, int operatorChannel, bool freqsOnly, bool triggerUnsetOperators)
{
	if (!freqsOnly)
	{
		if (retrigger == false)
			noteOffCh3Special(note, patch, operatorChannel, triggerUnsetOperators);

		if (operatorChannel == 0)
		{
			writeParams(2);
			updateCh3SpecialMode(true);
		}
	}

	for (int i = 0; i < 4; i++)
	{
		if (patch->InstrumentDef.OperatorEnable[i] && ((patch->InstrumentDef.OperatorMidiChannel[i] - 1) == operatorChannel || (triggerUnsetOperators && patch->InstrumentDef.OperatorMidiChannel[i] == 0)))
		{
			if(!freqsOnly)
				ch3SpecialOpNoteOnStates[i] = true;

			writeFrequencies(note + ((patch->InstrumentDef.OperatorOctave[i] - 3) * 1200) + ((patch->InstrumentDef.OperatorTranspose[i] - 11) * 100) + ((patch->InstrumentDef.OperatorDetune[i] - 50)), velocity, 2, vibrato, frequencyTable, patch, i, triggerUnsetOperators ? 2 : 1);
		}
	}

	if (!freqsOnly)
	{
		unsigned char opEnable = ch3SpecialOpNoteOnStates[0] | (ch3SpecialOpNoteOnStates[1] << 1) | (ch3SpecialOpNoteOnStates[2] << 2) | (ch3SpecialOpNoteOnStates[3] << 3);
		writeData(getRegister(YM_NOTEON, 2, 0), (opEnable << 4) | 2, 2, 2);
	}
}

void YM2612::noteOffCh3Special(int note, GennyPatch* patch, int operatorChannel, bool triggerUnsetOperators)
{
	for (int i = 0; i < 4; i++)
	{
		if (patch->InstrumentDef.OperatorEnable[i] && (patch->InstrumentDef.OperatorMidiChannel[i] - 1) == operatorChannel || (triggerUnsetOperators && patch->InstrumentDef.OperatorMidiChannel[i] == 0))
			ch3SpecialOpNoteOnStates[i] = false;
	}

	unsigned char opEnable = ch3SpecialOpNoteOnStates[0] | (ch3SpecialOpNoteOnStates[1] << 1) | (ch3SpecialOpNoteOnStates[2] << 2) | (ch3SpecialOpNoteOnStates[3] << 3);
	writeData(getRegister(YM_NOTEON, 2, 0), (opEnable << 4) | 2, 2, 2);
}

void YM2612::updateCh3SpecialMode(bool special)
{
	if (_inSpecialMode3 != special)
	{
		_inSpecialMode3 = special;
		writeData(getRegister(YM_SPECIAL, 2, 0), special ? (1 << 6) : 0, 2, 2);
	}
}

void YM2612::killNote(int channel)
{
}

void YM2612::panningChanged(int channel)
{
	if (channelPanStatesL[channel] != channelPanStatesWrittenL[channel])
	{
		channelPanStatesWrittenL[channel] = channelPanStatesL[channel];
		writeParameter(YM2612Param::YM_L_EN, channel, 0);
	}

	if (channelPanStatesR[channel] != channelPanStatesWrittenR[channel])
	{
		channelPanStatesWrittenR[channel] = channelPanStatesR[channel];
		writeParameter(YM2612Param::YM_R_EN, channel, 0);
	}
}

bool wrote = false;
void YM2612::writeFrequencies(int note, int velocity, int channel, float vibrato, double* frequencyTable, GennyPatch* patch, int specialOffset, int ch3Special)
{
	writeParams(channel);

	if(channel == 2)
		updateCh3SpecialMode(ch3Special);

	if (!ch3Special)
	{
		if (channel == 5 && patch != nullptr && _drumSet != nullptr && _dacEnable)
		{
			if (_drumSet != nullptr && _updateNoteFreq == false)
				return;
		}
	}

	if (specialOffset < 0)
	{
		for (int i = 0; i < 4; i++)
		{
			bool applies = false; ;
			if ((patch == nullptr && i == 3) || (patch->InstrumentDef.OperatorVelocity[i] && patch->InstrumentDef.OperatorEnable[i]))
			{
				int vel = min((int)((_channels[channel].op[i].TL.val / 127.0f) * velocity), 127);

				int reg = getRegister(YM_TL, channel % 3, i);
				writeData(reg, 127 - max(vel, 0), channel, channel);
			}
		}
	}
	else
	{
		if (patch->InstrumentDef.OperatorVelocity[specialOffset] && patch->InstrumentDef.OperatorEnable[specialOffset])
		{
			int vel = min((int)((_channels[channel].op[specialOffset].TL.val / 127.0f) * velocity), 127);

			int reg = getRegister(YM_TL, channel % 3, specialOffset);
			writeData(reg, 127 - max(vel, 0), channel, channel);
		}
	}

	//Frequency of C0
	//float cFreq = 16.35f;
	int baseNote = (note / 100);
	float amountToBend = (((note - (baseNote * 100.0f)) / 100.0f) + vibrato);
	float semitones = 0;

	//Lowest note frequency equals 8.17579891564371hz, which corresponds to midi note -1
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
	if(channelLSB[specialOffset + 1][channel] != lsb || channelMSB[specialOffset + 1][channel] != msb)
	{
		writeData(getRegister(YM_FNUM2, channel % 3, 0, specialOffset), msb, channel, channel, false, msbMM);
		writeData(getRegister(YM_FNUM1, channel % 3, 0, specialOffset), lsb, channel, channel, false, lsbMM);
		channelLSB[specialOffset + 1][channel] = lsb;
		channelMSB[specialOffset + 1][channel] = msb;
	}
}

void YM2612::noteOff(int channel, int note, bool fromMidiMessage)
{
	if (_drumSet != nullptr)
	{
		WaveData* curDrum = _drumSet->getCurrentDrum(_drumSet->_sampleRate, _logger != nullptr || true);
		if (channel == 5 && (curDrum == _drumSet->getDrum(note / 100, _drumSet->_sampleRate, _logger != nullptr || true) || _drumSet->isPitchInstrument()) && curDrum != nullptr)
			curDrum->fadeSamples = 100;

		if (channel == 5 && _dacEnable != false)
			return;
	}

	writeData(getRegister(YM_NOTEON, channel % 3, 0),channel > 2 ? (channel + 1) : channel, channel % 3, channel);

	//if (fromMidiMessage && _mdmMode)
	//	SendMIDIGenMDM(128, channel, note / 100, 64);
	//_chip.YM2612Update(nullptr, 0);
}

//void YM2612::setParameter(YM2612Param param, int channel, int op, float value)
//{
//	*getParameter(param, channel, op) = value;
//	if(_chipWrite) 
//		writeParameter(param, channel, op);
//}
//
//void YM2612::setParameterChar(YM2612Param param, int channel, int op, unsigned char value)
//{
//
//}

void YM2612::setFromBaron(IBIndex* param, int channel, float val)
{
	if (channel > 5 || (channel == 5 && _dacEnable != false && _drumSet != nullptr))
		return;

	if(param->getType() == IB_YMParam)
	{
		IBYMParam* ymParam = static_cast<IBYMParam*>(param);

		if (ymParam->getOperator() < 0)
			_channels[channel].trySet(ymParam->getParameter(), (unsigned char)(val + 0.5f));
		else
			_channels[channel].op[ymParam->getOperator()].trySet(ymParam->getParameter(), (unsigned char)(val + 0.5f));
	}
}

void YM2612::applyWholeChannel(const GennyPatch& patch, int channel)
{
	const YM2612Channel& params = patch.InstrumentDef.Data;
	for (int i = 0; i < 4; i++)
	{
		for (auto it = params._operators[i].begin(); it != params._operators[i].end(); it++)
		{
			_channels[channel].op[i].trySet((*it).first, (unsigned char)((*it).second + 0.5f), true);
		}
	}		
	
	for (auto it = params._parameters.begin(); it != params._parameters.end(); it++)
	{
		_channels[channel].trySet((*it).first, (unsigned char)((*it).second + 0.5f), true);
	}

	_channels[channel].patch = &patch;
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


//float* YM2612::getParameter(YM2612Param param, int channel, int op)
//{
//	//Channel parameters
//	if(param >= YM_DT && param <= YM_SPECIAL)
//	{
//		channel = max( 0, min( channel, 6 ) );
//		return _channels[channel].getParameter(param, channel, op);
//	}
//	return nullptr;
//}

//unsigned char YM2612::getParameterChar(YM2612Param param, int channel, int op)
//{
//	float parm = *getParameter(param, channel, op);
//	return (unsigned char)(parm + 0.5f); 
//}

void YM2612::writeParams(int channel)
{
	if (_channels[channel].dirty)
	{
		std::vector<YM2612HWParam*> parms = _channels[channel].params;
		int sz = parms.size();
		for (int i = 0; i < sz; i++)
		{
			if (parms[i]->dirty)
			{
				writeParameter(parms[i]->param, channel, 0);
				parms[i]->dirty = false;
			}
		}

		for (int op = 0; op < 4; op++)
		{
			if (_channels[channel].op[op].dirty)
			{
				parms = _channels[channel].op[op].params;
				sz = parms.size();
				for(int i = 0; i < sz; i++)
				{
					if (parms[i]->dirty)
					{
						writeParameter(parms[i]->param, channel, op);
						parms[i]->dirty = false;
					}
				}
				_channels[channel].op[op].dirty = false;
			}			
			
		}

		_channels[channel].dirty = false;
	}
}

void YM2612::writeParameter(YM2612Param param, int channel, int op)
{
	if (param == YM2612Param::YM_F1 || param == YM2612Param::YM_F2)
		return;

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

	unsigned char c = 0;
	if (reg > 127)
	{
		reg = reg & 0x7F;
		if (data > 127)
		{
			c = 3;
			data = data & 0x7F;
		}
		else
			c = 1;
	}
	else if (data > 127)
	{
		c = 2;
		data = data & 0x7F;
	}

	unsigned char status = (8 << 4) | (chan + (4 * c)) ;
	_vst->midiOut(status, reg, data, _vst->megaMidiPort - 1);
}

void YM2612::SendSamples(unsigned char s1, unsigned char s2)
{
	if (_vst->megaMidiPort == 0)
		return;

	unsigned char c = 0; 
	if (s1 > 127)
	{
		s1 = s1 & 0x7F;
	    if (s2 > 127)
		{
			c = 3;
			s2 = s2 & 0x7F;
		}
		else
			c = 1;
	}
	else if (s2 > 127)
	{
		c = 2;
		s2 = s2 & 0x7F;
	}

	unsigned char status = (9 << 4) | c;
	_vst->midiOut(status, s1, s2, _vst->megaMidiPort - 1);
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


void YM2612::fullStop(int channel)
{
	for (int i = 0; i < 6; i++)
	{
		if (channel < 0 || channel == i)
		{
			_channels[i].op[0].RR.val = 15;
			_channels[i].op[1].RR.val = 15;
			_channels[i].op[2].RR.val = 15;
			_channels[i].op[3].RR.val = 15;
			writeParameter(YM2612Param::YM_RR, i, 0);
			writeParameter(YM2612Param::YM_RR, i, 1);
			writeParameter(YM2612Param::YM_RR, i, 2);
			writeParameter(YM2612Param::YM_RR, i, 3);

			_channels[i].op[0].TL.val = 0;
			_channels[i].op[1].TL.val = 0;
			_channels[i].op[2].TL.val = 0;
			_channels[i].op[3].TL.val = 0;
			writeParameter(YM2612Param::YM_TL, i, 0);
			writeParameter(YM2612Param::YM_TL, i, 1);
			writeParameter(YM2612Param::YM_TL, i, 2);
			writeParameter(YM2612Param::YM_TL, i, 3);
			noteOff(i, 0);
		}
	}

	dirtyChannels(channel);
	clearCache(channel);
	_resetDACSamplerate = true;
}

void YM2612::writeData(int reg, unsigned char data, int channel, int realChannel, bool inLock, int specialLogData)
{
	if (threadInit == false)
	{
		threadInit = true;
		this_id = std::this_thread::get_id();
	}

	if (this_id != std::this_thread::get_id())
	{
		int qq = 1;
		this_id = std::this_thread::get_id();
	}



	if (inLock == false)
		writeDataMutex.lock();

	if(_channels[realChannel].delay > 0 && realChannel >= 0)
	{
		DelayedChipCommands* cmds = _processor->lockChipCommandsOffset(_channels[realChannel].delay);
		cmds->addCommand(true, reg, data, channel, specialLogData);
		_processor->unlockChipCommands(cmds);
	}
	else
	{
		int val = reg + (channel * 400);

		auto it = _writeMap.find(val);
		//160 to 174 are FNUM registers for note frequency. Caching is handled for these elsewhere.
		//0x2A is the DAC sample register
		if (((reg >= 160 && reg <= 174) || reg == 0x2A) || it == _writeMap.end() || (*it).second != data)
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
				if (_logger != nullptr && reg != 42)
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
				if (_logger != nullptr && reg != 42)
					_logger->logWriteYM2612(1, reg, data);
			}
		}
	}

	if (inLock == false)
		writeDataMutex.unlock();
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
			unsigned char dt = _channels[channel].op[op].DT.val;
			unsigned char mul = _channels[channel].op[op].MUL.val;
			return (dt << 4) | mul;
		}
		break;

	case YM_TL:
		{
 			return 127 - _channels[channel].op[op].TL.val;
		}
		break;

	case YM_KS:
	case YM_AR:
		{
			//RS_AR
			//------------------------------
			//  0   1   2   3   4   5   6   7
			//  |RS |  |X|  |      AR       |
			unsigned char rs = _channels[channel].op[op].KS.val;
			unsigned char ar = 31 - _channels[channel].op[op].AR.val;
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
			unsigned char am = _channels[channel].op[op].AM.val;
			unsigned char dr1 = 31 - _channels[channel].op[op].DR.val;
			return (am << 7) | dr1;
		}
		break;

	case YM_SR:
		{
			return _channels[channel].op[op].SR.val;
		}
		break;

	case YM_SL:
	case YM_RR:
		{
			//DL1_RR
			//------------------------------
			//  0   1   2   3   4   5   6   7
			//  |    DL1    |   |     RR    |
			unsigned char dl1 = 15 - _channels[channel].op[op].SL.val;
			unsigned char rr = 15 - _channels[channel].op[op].RR.val;
			return (dl1 << 4) | rr;
		}
		break;

	case YM_FB:
	case YM_ALG:
		{
			//FB_ALGORITHM
			//------------------------------
			//  0   1   2   3   4   5   6   7
			//          |   FB  |   |  ALG  |
			unsigned char fb = _channels[channel].FB.val;
			unsigned char alg = _channels[channel].Alg.val;
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

			unsigned char l = channelPanStatesWrittenL[channel] == 1;
			unsigned char r = channelPanStatesWrittenR[channel] == 1;

			unsigned char ams = _channels[channel].AMS.val;
			unsigned char fms = _channels[channel].FMS.val;

			if (_channels[channel].patch != nullptr && _channels[channel].patch->InstrumentDef.LFOEnable == false)
				ams = fms = 0;

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

			//As of 1.1 LFO is always enabled and AMS/FMS determine its sensitivity
			unsigned char en = 1;// getParameterChar(YM_LFO_EN, channel, op); 
			unsigned char lfo = _vst->lfo;
			return ((en << 3) | lfo);
		}
		break;

	case YM_SSG:
		{
	
			unsigned char c = _channels[channel].op[op].SSG.val;
			return GennyLoaders::ssgGennyToReg(c);
		}
		break;
	}

	return 0;
}

//Returns the register corresponding to a specific parameter. Some registers
//may be multi purpose registers, in which case it's important that you pack
//your data in a relevant way to ensure you're modifying the right parameter.
int YM2612::getRegister( YM2612Param param, int channel, int op, int specialOffset)
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

		if (specialOffset >= 0)
		{
			if(specialOffset > 0)
				return (168 + (specialOffset - 1));

			return (160 + channel);
		}

		return 160 + channel;

		break;
	case YM_FNUM2:
		if (specialOffset >= 0)
		{
			if (specialOffset > 0)
				return (172 + (specialOffset - 1));

			return (164 + channel);
		}

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


float inc = 0;
int other = 0;
void YM2612::updateDAC()
{
	_hardwareMode = _vst->megaMidiPort > 0;
	_emulationMute = _vst->megaMidiPort > 0 && _vst->megaMidiVSTMute;

	if (_sleep)
		return;



	if(_logger != nullptr)
		_logger->logSample();


	/*if (_vst->triggerWave != nullptr)
	{
		if (_vst->triggerWave->fadeSamples == 0)
			_vst->triggerWave = nullptr;
		else
		{
			channelPanStatesL[5] = true;
			channelPanStatesR[5] = true;
			panningChanged(5);
			setDACEnable(true);
		}
	}*/

	//other++;
	//if (other % 8 == 0)
	//{
	//	inc += 0.1f;
	//	float s1 = sin(inc)/* > 0.0f ? 1.0f : -1.0f*/;
	//	inc += 0.1f;
	//	float s2 = sin(inc)/* > 0.0f ? 1.0f : -1.0f*/;
	//	SendSamples(((s1 + 1.0f) / 2.0f) * 255, ((s2 + 1.0f) / 2.0f) * 255);
	//	//SendSamples(((s1 + 1.0f) / 2.0f) * 127, ((s2 + 1.0f) / 2.0f) * 127);
	//}
	//return;


	float rateMult = 44100.0f / _sampleRate;
	float drumSampleRate = 11025.0f;
	WaveData* currentDrum = nullptr;
	if (_drumSet != nullptr)
	{
		drumSampleRate = _drumSet->_sampleRate;
		currentDrum = _drumSet->getCurrentDrum(drumSampleRate, _logger != nullptr || true);
	}

	if (currentDrum == nullptr)
		currentDrum = _vst->triggerWave;		

	if (currentDrum != nullptr && currentDrum->fadeSamples != 0)
	{
		float pitchOffset = pow(2.0, ((currentDrum->_pitch / 50) / 12.0));
		//float rate = currentDrum->sampleRate * pitchOffset;
		float rate = drumSampleRate;

		//if (_dacEnable == false)
		//	setDACEnable(true);

		float chipFreq = (float)_sampleRate;
		currentDrum->waitTime += (drumSampleRate / _sampleRate)/*_hardwareMode ? 0.25f : 0.5f*/; //waiting 4 steps allows all samples to be interpreted as 11025hz, which is easier for the chip to deal with.

		if (currentDrum->waitTime >= 1.0f) 
		{
			if (currentDrum->release == false && currentDrum->loop && currentDrum->audioPosition >= currentDrum->_endSample)
			{
				currentDrum->audioPosition = currentDrum->_startSample;

				if (_logger != nullptr)
					_logger->seekDACSample(currentDrum->getStreamStart(_sampleVolume));
			}

			currentDrum->waitTime -= 1.0f;
			if (currentDrum->audioPosition >= currentDrum->_endSample)
			{
				writeData(getRegister(YM_DAC, -1, -1), 128, 0, 5);
				if (_vst->triggerWave != nullptr)
					_vst->triggerWave = nullptr;
				else if (_drumSet != nullptr)
					_drumSet->setCurrentDrum(-1, _drumSet->_sampleRate, _logger != nullptr || true);

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
				/*int first = currentDrum->audioData[(int)currentDrum->audioPosition];
				int second = first;
				if ((int)currentDrum->audioPosition < currentDrum->endSample - 1)
				{
					second = currentDrum->audioData[(int)currentDrum->audioPosition + 1];
				}

				float lerp = currentDrum->audioPosition - (int)currentDrum->audioPosition;
				int currentSample = (int)((first * (1.0f - lerp)) + (second * lerp));*/
					
					
				float vol = _sampleVolume;
				if (currentDrum->fadeSamples >= 0)
				{
					vol = (currentDrum->fadeSamples / 100.0f) * vol;
					currentDrum->fadeSamples -= 1;
				}
				vol *= (currentDrum->_amp / 100.0f) * 2.0f;

				//unsigned char sample = min(255, max(0, (int)((((float)currentSample - 128.0f) * vol) + 0.5f) + 128));


				unsigned char sample = min(255, max(0, (int)((((float)currentDrum->audioData[(int)currentDrum->audioPosition] - 128.0f) * vol) + 0.5f) + 128));


				_lastDACSample = sample;
				//unsigned char sample = min(255, max(0, currentSample * _sampleVolume));


				if(!_emulationMute)
					writeData(getRegister(YM_DAC, -1, -1), sample, 0, 5);
					
				if(_hardwareMode)
				{
					if (_writeSamples)
						SendSamples(_lastSample, sample);
					_writeSamples = !_writeSamples;
					_lastSample = sample;
				}


				_mutedDAC = 0;
			}

		}

		//float noteInc = pow(2.0, ((currentDrum->pitch / 50) / 12.0));
		//float chipFreq = 53267.0f;
		if (_drumSet != nullptr && _drumSet->isPitchInstrument())
		{
			double startFreq = pow(2.0, (48.0 / 12.0)) * 8.17579891564371;
			double endFreq = pow(2.0, (72.0 / 12.0)) * 8.17579891564371;
			double freqPos = 1.0 + (((endFreq - startFreq) - (_noteFreq - startFreq)) / (endFreq - startFreq));

			currentDrum->audioPosition += ((((float)currentDrum->sampleRate) / chipFreq)/* *noteInc*/) * (_noteFreq / startFreq);
		}
		else
		{
			int audPos = currentDrum->audioPosition;
			currentDrum->audioPosition += (float)(rate / _sampleRate)/* *noteInc*/;
			if (audPos != (int)currentDrum->audioPosition)
			{
				if (_logger != nullptr)
					_logger->logDACSample();
			}

			//currentDrum->audioPosition += 1;
		}



	}
	else if (_mutedDAC == 0)
	{
		_mutedDAC = 1;
		writeData(getRegister(YM_DAC, -1, -1), 128, 0, 5);
		if (_hardwareMode)
		{
			SendSamples(128, 128);
			_writeSamples = false;
			_lastSample = 128;
		}

		_vst->triggerWave = nullptr;

		/*float mul = _mutedDAC / 10.0f;
		unsigned char sample = (unsigned char)(_lastDACSample + (mul * (128 - _lastDACSample)));
		writeData(getRegister(YM_DAC, -1, -1), sample, 0, 5);

		if (_hardwareMode)
		{
			if (_mutedDAC == 10 && !_writeSamples)
			{
				SendMIDI(3, 128, _lastSample);
				_writeSamples = false;
				_lastSample = 128;
			}
			else
			{
				if (_writeSamples)
					SendMIDI(3, sample, _lastSample);
				_writeSamples = !_writeSamples;
				_lastSample = sample;
			}
		}*/
	}

}


void YM2612::midiTick()
{
	if (_sleep)
		return;


	//if (_currentCluster != nullptr)
	//{
	//	YM2612CommandCluster c = _currentCluster;
	//	_currentCluster 
	//}

	//while (true)
	//{
	//	YM2612CommandCluster clust = YM2612CommandCluster(-1);
	//	if (_pendingCommands.try_dequeue(clust))
	//		_commands.push_back(clust);
	//	else
	//		break;
	//}

	/*_mutex.lock();
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
	_mutex.unlock();*/

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
	if(_dacEnable != enable)
	{
		_dacEnable = enable;
		writeData(getRegister(YM_DACEN, -1, -1), _dacEnable ? 128 : 0, 0, 5);
	}
}

void YM2612::dirtyChannels(int channel)
{
	for (int i = 0; i < 6; i++)
	{
		if(channel < 0 || channel == i)
			_channels[i].becomeDirty();
	}
}

void YM2612::clearCache(int channel)
{
	_writeMap.clear();
	for (int c = 0; c < 5; c++)
	{
		for (int i = 0; i < 6; i++)
		{
			if (channel < 0 || channel == i)
			{
				channelLSB[c][i] = -1;
				channelMSB[c][i] = -1;
			}
		}
	}

	for (int c = 0; c < 6; c++)
	{
		if (channel < 0 || channel == c)
		{
			//rewrite pan states
			channelPanStatesWrittenL[c] = -1;
			channelPanStatesWrittenR[c] = -1;
		}
	}

	if(channel < 0 || channel == 2)
		_inSpecialMode3 = -1;

	if (channel < 0 || channel == 5)
		_dacEnable = -1;
}


void YM2612::Update(int *buffer, int length)
{
	if (_sleep)
		return;

	if (_vst->accurateEmulationMode)
		_chipAccurate.YM2612Update(buffer, length);
	else
	{
		_chip.YM2612Update(buffer, length);
		//OPN2_Clock(&_chipNuked, buffer);
	}

	_dirtyCluster = true;
}