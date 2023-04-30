#include "Genny2612.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "GennyLoaders.h"
#include "sn76489.h"
#include "sn76489_plusgx.h"
#include "YM2612Enum.h"

#ifndef BUILD_VST
#include "fp_def.h"
#endif


//struct VibratoInfo
//{
//	VibratoInfo() :note(-1), channel(-1), velocity(0), noteData(nullptr), vibratoPhase(0.0f), vibratoInc(0.0f){}
//	int note;
//	int velocity;
//	int channel;
//	void* noteData;
//	float vibratoPhase;
//	float vibratoInc;
//	void Calculate();
//};

PingPongPan GennyInstrument::getAndIncrementPingPongPanning()
{
	if (PingPongPanning.size() > 0) 
	{
		PingPongPan p = PingPongPanning[PingPongPanIndex % PingPongPanning.size()];
		PingPongPanIndex++;
		return p;
	}
	return PingPongPan::None;
}

void GennyInstrument::parsePanString(std::string setting)
{
	PingPongPanning.clear();
	if (setting != "")
	{
		std::string currentNumber = "";
		std::string cleanString = "";
		int iCurrentNumber;
		for (int i = 0; i < setting.length(); i++)
		{
			char c = setting[i];
			if (c > 47 && c < 58) //number
			{
				currentNumber += c;
				cleanString += c;
			}
			else
			{
				if (currentNumber.length() > 0)
				{
					iCurrentNumber = atoi(currentNumber.c_str());
					currentNumber = "";
				}
				else
					iCurrentNumber = 1;
			}

			if (c == 'C')
			{
				cleanString += c;
				for (int i = 0; i < iCurrentNumber; i++)
				{
					PingPongPanning.push_back(PingPongPan::Center);
				}
			}
			else if (c == 'L')
			{
				cleanString += c;
				for (int i = 0; i < iCurrentNumber; i++)
				{
					PingPongPanning.push_back(PingPongPan::Left);
				}
			}
			else if (c == 'R')
			{
				cleanString += c;
				for (int i = 0; i < iCurrentNumber; i++)
				{
					PingPongPanning.push_back(PingPongPan::Right);
				}
			}
		}

		if (cleanString.length() > 8)
			cleanString = cleanString.substr(0, 7) + "..";

		if (cleanString.length() == 0)
			PingPongString = GennyExtParam::kDefaultPingPongSettings[0];
		else
			PingPongString = cleanString;
	}
	else
		PingPongString = GennyExtParam::kDefaultPingPongSettings[0];
}

void VibratoInfo::Calculate(float tempo, int samples, float sampleRate)
{

//#if BUILD_VST
//	int realNote = note - 96;
//	float speedMul = realNote + 1.0f;
//#else
	int realNote = note - 9600;
	float speedMul = (realNote / 100.0f) + 1.0f;
//#endif

	float secondFraction = (float)samples / (sampleRate * (240.0f / tempo));
	vibratoInc = secondFraction;
	vibratoPhase += vibratoInc * speedMul;
	while(vibratoPhase > 1.0f)
		vibratoPhase -= 1.0f;

	//vibratoInc = 
}

void GennyInstrument::catalogue(IndexBaron* baron)
{
	baron->addIndex(new IBInsParam(0, GIP_DAC, 0, 1, "DAC"));
	baron->addIndex(new IBInsParam(0, GIP_Channel0, 0, 1, "Global CH0"));
	baron->addIndex(new IBInsParam(0, GIP_Channel1, 0, 1, "Global CH1"));
	baron->addIndex(new IBInsParam(0, GIP_Channel2, 0, 1, "Global CH2"));
	baron->addIndex(new IBInsParam(0, GIP_Channel3, 0, 1, "Global CH3"));
	baron->addIndex(new IBInsParam(0, GIP_Channel4, 0, 1, "Global CH4"));
	baron->addIndex(new IBInsParam(0, GIP_Channel5, 0, 1, "Global CH5"));
	baron->addIndex(new IBInsParam(0, GIP_Channel6, 0, 1, "Global CH6"));
	baron->addIndex(new IBInsParam(0, GIP_Channel7, 0, 1, "Global CH7"));
	baron->addIndex(new IBInsParam(0, GIP_Channel8, 0, 1, "Global CH8"));
	baron->addIndex(new IBInsParam(0, GIP_Channel9, 0, 1, "Global CH9"));
	baron->addIndex(new IBInsParam(0, GIP_MidiChannel, 0, 15, "Midi CH"));
	baron->addIndex(new IBInsParam(0, GIP_FM, 0, 1, "FM Enable"));

	/*for(int i = 0; i < 32; i++)
	{
		baron->addIndex(new IBInsParam(0, (GennyInstrumentParam)(GIP_DACSamplePathStart + i), 0, 999999));
	}

	*/

	baron->addIndex(new IBInsParam(0, GIP_Octave, 0, GennyInstrumentParam_getRange(GIP_Octave), "Octave"));
	baron->addIndex(new IBInsParam(0, GIP_Transpose, 0, GennyInstrumentParam_getRange(GIP_Transpose), "Transpose"));
	baron->addIndex(new IBInsParam(0, GIP_Panning, 0, GennyInstrumentParam_getRange(GIP_Panning), "Panning"));


	baron->addIndex(new IBInsParam(0, GIP_RangeLow, 0, GennyInstrumentParam_getRange(GIP_RangeLow), "Range Low"));
	baron->addIndex(new IBInsParam(0, GIP_RangeHigh, 0, GennyInstrumentParam_getRange(GIP_RangeHigh), "Range High"));
	baron->addIndex(new IBInsParam(0, GIP_Delay, 0, GennyInstrumentParam_getRange(GIP_Delay), "Delay"));

	baron->addIndex(new IBInsParam(0, GIP_SelectedDrum, 0, GennyInstrumentParam_getRange(GIP_SelectedDrum), "Selected Drum"));

	for(int i = 0; i < 6; i++)
	{
		baron->addIndex(new IBInsParam(0, GIP_Extra, 0, 1, "Extra"));
	}

	Data.catalogue(baron);
}

bool GennyInstrument::loadingOldPanning = false;
void GennyInstrument::setFromBaron(IBIndex* param, float val)
{
	if(param->getType() == IB_InsParam)
	{
		IBInsParam* p = static_cast<IBInsParam*>(param);
		switch(p->getParameter())
		{
		case GIP_DAC:
			//DAC = val > 0.5;
			break;
		case GIP_Channel0:
			Channels[0] = val > 0.5;
			break;
		case GIP_Channel1:
			Channels[1] = val > 0.5;
			break;
		case GIP_Channel2:
			Channels[2] = val > 0.5;
			break;
		case GIP_Channel3:
			Channels[3] = val > 0.5;
			break;
		case GIP_Channel4:
			Channels[4] = val > 0.5;
			break;
		case GIP_Channel5:
			Channels[5] = val > 0.5;
			break;
		case GIP_Channel6:
			Channels[6] = val > 0.5;
			break;
		case GIP_Channel7:
			Channels[7] = val > 0.5;
			break;
		case GIP_Channel8:
			Channels[8] = val > 0.5;
			break;
		case GIP_Channel9:
			Channels[9] = val > 0.5;
			break;
		case GIP_MidiChannel:
			MidiChannel = val;
			break;
		case GIP_FM:
			//FM = val > 0.5f;
			break;

		case GIP_Octave:
			Octave = (int)(val + 0.5f);
			break;
		case GIP_Transpose:
			Transpose = (int)(val + 0.5f);
			break;
		case GIP_Panning:

			if(loadingOldPanning)
				Panning = (int)((val / 2.0f) * 255);
			else
				Panning = (int)(val + 0.5f);

			break;

		case GIP_RangeLow:
			LowRange = (int)(val + 0.5f);
			break;		
		case GIP_RangeHigh:
			HighRange = (int)(val + 0.5f);
			break;		
		case GIP_Delay:
			Delay = (int)(val + 0.5f);
			break;
		case GIP_SelectedDrum:
			SelectedDrum = (int)(val + 0.5f);
			break;	

		case GIP_Extra:
			break;

		default:
	/*		if(p->getParameter() >= GIP_DACSamplePathStart && p->getParameter() <= GIP_DACSamplePathEnd)
			{
				DACSamplePath[p->getParameter() - GIP_DACSamplePathStart] = val;
				break;
			}*/
			break;

		}
	}
	else if(param->getType() == IB_YMParam)
	{
		Data.setFromBaron(param, val);
	}
}

float  GennyInstrument::getFromBaron(IBIndex* param)
{
	if(param->getType() == IB_InsParam)
	{
		IBInsParam* p = static_cast<IBInsParam*>(param);
		switch(p->getParameter())
		{
		case GIP_DAC: return 0.0f; //deprecated
		case GIP_Channel0: return Channels[0] ? 1.0f : 0.0f;
		case GIP_Channel1: return Channels[1] ? 1.0f : 0.0f;
		case GIP_Channel2: return Channels[2] ? 1.0f : 0.0f;
		case GIP_Channel3: return Channels[3] ? 1.0f : 0.0f;
		case GIP_Channel4: return Channels[4] ? 1.0f : 0.0f;
		case GIP_Channel5: return Channels[5] ? 1.0f : 0.0f;
		case GIP_Channel6: return Channels[6] ? 1.0f : 0.0f;
		case GIP_Channel7: return Channels[7] ? 1.0f : 0.0f;
		case GIP_Channel8: return Channels[8] ? 1.0f : 0.0f;
		case GIP_Channel9: return Channels[9] ? 1.0f : 0.0f;
		case GIP_MidiChannel: return MidiChannel;
		case GIP_FM: return 0.0f; //deprecated
		case GIP_Octave: return (float)Octave;
		case GIP_Transpose: return (float)Transpose;
		case GIP_Panning: return (float)Panning;
		case GIP_RangeLow: return (float)LowRange;
		case GIP_RangeHigh: return (float)HighRange;
		case GIP_SelectedDrum: return (float)SelectedDrum;
		case GIP_Delay: return (float)Delay;
		case GIP_Extra: return 0.0f;
		default:
			//if(p->getParameter() >= GIP_DACSamplePathStart && p->getParameter() <= GIP_DACSamplePathEnd)
			//{
			//	return DACSamplePath[p->getParameter() - GIP_DACSamplePathStart];
			//	break;
			//}
			break;
		}
	}
	else if(param->getType() == IB_YMParam)
	{
		return Data.getFromBaron(param);
	}
	return 0.0f;
}

void GennyPatch::catalogue(IndexBaron* baron)
{
	//Globals.catalogue(baron);
	for(int i = 0; i < kMaxInstruments; i++)
		baron->addIndex(new IBPatchParam((GennyPatchParam)(GPP_Ins01 + i), 0, 1));
	for(int i = 0; i < 10; i++)
		baron->addIndex(new IBPatchParam((GennyPatchParam)(GPP_Channel0 + i), 0, 1));

	baron->addIndex(new IBPatchParam(GPP_SelectedInstrument, 0, kMaxInstruments));
	InstrumentDef.catalogue(baron);
}

void GennyPatch::setFromBaron(IBIndex* param, float val)
{
	if(param->getType() == IB_PatchParam)
	{
		IBPatchParam* p = static_cast<IBPatchParam*>(param);
		
		if(p->getParameter() >= GPP_Ins01 && p->getParameter() <= GPP_Ins32)
		{
			int index = (int)p->getParameter() - (int)GPP_Ins01;
			Instruments[index] = (int)val;
			return;
		}
			
		if(p->getParameter() >= GPP_Channel0 && p->getParameter() <= GPP_Channel9)
		{
			int index = (int)p->getParameter() - (int)GPP_Channel0;
			Channels[index] = (int)val;
			return;
		}

		if(p->getParameter() == GPP_SelectedInstrument)
			SelectedInstrument = (int)val;
	}
	else if(param->getType() == IB_YMParam)
	{
		IBYMParam* p = static_cast<IBYMParam*>(param);
		//if(p->getInstrument() < 0)
		//	return Globals.setFromBaron(param, val);
		//else
			return InstrumentDef.setFromBaron(param, val);
	}
	else if(param->getType() == IB_InsParam)
	{
		IBInsParam* p = static_cast<IBInsParam*>(param);
		return InstrumentDef.setFromBaron(param, val);
	}
}

float GennyPatch::getFromBaron(IBIndex* param)
{
	if (param->getType() == IB_YMParam)
	{
		IBYMParam* p = static_cast<IBYMParam*>(param);
		//if(p->getInstrument() < 0)
		//	return Globals.getFromBaron(param);
		//else
		return InstrumentDef.getFromBaron(param);
	}
	else if(param->getType() == IB_PatchParam)
	{
		IBPatchParam* p = static_cast<IBPatchParam*>(param);

		if(p->getParameter() >= GPP_Ins01 && p->getParameter() <= GPP_Ins32)
		{
			int index = (int)p->getParameter() - (int)GPP_Ins01;
			return (float)Instruments[index];
		}

		if(p->getParameter() >= GPP_Channel0 && p->getParameter() <= GPP_Channel9)
		{
			int index = (int)p->getParameter() - (int)GPP_Channel0;
			return (float)Channels[index];	
		}

		if(p->getParameter() == GPP_SelectedInstrument)
			return (float)SelectedInstrument;
	}
	else if(param->getType() == IB_InsParam)
	{
		IBInsParam* p = static_cast<IBInsParam*>(param);
		return InstrumentDef.getFromBaron(param);
	}
}

Genny2612::Genny2612(GennyVST* owner):
	_indexBaron(NULL),
	_owner(owner),
	_numNotes(0),
	_releases(0),
	_logging(false),
	_chip(owner),
	_snChip(owner),
	_initializedChannels(false)
{
	memset(_channelPatches, 0, sizeof(int) * kNumNoteChannels);
	memset(channelDirty, 0, sizeof(char) * kNumNoteChannels);

	_defaultFrequencyTable = new double[129];

	_defaultFrequencyTable[0] = 8.17579891564371;
	for(int i = 1; i < 129; i++)
		_defaultFrequencyTable[i] = 100 * (i - 1);

	_frequencyTable = _defaultFrequencyTable;

	//_snChip._commandBuffer = _chip._commandBuffer;
	_snChip._2612 = &_chip;

	_indexBaron = new IndexBaron();
	GennyPatch defaultPatch(0);
	defaultPatch.catalogue(_indexBaron);


	//_patches[0].catalogue(_indexBaron);
}


Genny2612::~Genny2612(void)
{
	if(_frequencyTable != NULL)
		delete[] _frequencyTable;

	delete[] _defaultFrequencyTable;
	delete _indexBaron;
}

void Genny2612::initialize()
{
	double sampleRate = _owner->getBase()->_sampleRate;
	_chip.initialize(YM2612_NTSC, (int)sampleRate);
	_snChip.Initialize(SN76489Clock::SN76489_NTSC, (int)sampleRate);

	_processor.initialize(&_chip, &_snChip, sampleRate);
	_logger.initialize(&_chip, &_snChip);


}

void Genny2612::setSampleRate(double rate)
{
	_processor.setSampleRate(rate); 
	_chip.setSampleRate(rate);
	_snChip.setSampleRate(rate); 
}

static int numLogged = 0;
void Genny2612::noteOn(const int note, float velocity, unsigned char channel, float panning, void* data, bool soloTrigger)
{
	//If the note is higher than 95 it is an effect note (vibrato) and should not be played.
 	bool isEffect = false;
	int logNote = note; 

	if(note >= 9600 || note < 300)
		isEffect = true;
	logNote /= 100;

	if(isEffect)
	{
		if (note < 300)
		{
			if (_logging)
			{
				if (logNote == 2 && _logger.isLogging())
				{
					_logger.writeLoopPoint();
					return;
				}
				else if (logNote == 0 && _logger.isLogging() == false)
				{
					_chip.setDACEnable(false);
					_chip.dirtyChannels();
					_logger.startLogging(_owner, _loggingFile);
					_snChip.setLogger(&_logger);
					_chip.setLogger(&_logger);
					return;
				}
				else if (logNote == 1 && _logger.isLogging() == true)
				{
					stopLogging();
					return;
				}
			}
		}
		else
		{
			VibratoInfo vibrato;
			vibrato.note = note;
			vibrato.noteData = data;
			vibrato.channel = channel;

			//release functions as velocity with vibrato note info
			vibrato.velocity = velocity * 127;

			_vibratoNotes.push_back(vibrato);
		}
		return;
	}
	
	_numNotes += 1;

	std::vector<int> channels;
	GennyPatch* patch0 = static_cast<GennyPatch*>(_owner->getPatch(0));
	IndexBaron* baron = _indexBaron;
	for(int i = 0; i < kMaxInstruments; i++)
	{
		channels.clear();

		int ins = patch0->Instruments[i];
		if(ins == -1)
			continue;

		GennyPatch* thisPatch = static_cast<GennyPatch*>(_owner->getPatch(i));
		if (thisPatch->InstrumentDef.Enable == false)
			continue;


		GennyPatch* patch = static_cast<GennyPatch*>(_owner->getPatch(ins));
		int midiChannel = thisPatch->InstrumentDef.MidiChannel;
		int octave = thisPatch->InstrumentDef.Octave;
		int transpose = thisPatch->InstrumentDef.Transpose;

		int rangeLow = thisPatch->InstrumentDef.HighRange;
		int rangeHigh = thisPatch->InstrumentDef.LowRange;

		bool validChannel = midiChannel == channel;
		int operatorMidiIndex = -1;

		//If this isn't the main instrument channel, but it's a Ch3Special, check for operator channels
		if(validChannel == false && patch->InstrumentDef.Ch3Special) 
		{
			for (int i = 0; i < 4; i++)
			{
				if (patch->InstrumentDef.OperatorMidiChannel[i] - 1 == channel)
				{
					operatorMidiIndex = i;
					validChannel = true;
					break;
				}
			}
		}

		if(validChannel)
		{
			//Figure out which channels this instrument uses
			int firstChannel = -1;
			if (patch->InstrumentDef.Ch3Special)
			{
				if (operatorMidiIndex >= 0)
				{
					channels.push_back(10);
					channels.push_back(11);
					channels.push_back(12);
					channels.push_back(13);
				}
				else if(patch0->Channels[2])
					channels.push_back(2);
			}
			else
			{
				if (patch->InstrumentDef.Type == GIType::FM)
				{
					for (int j = 0; j < 6; j++)
					{
						if (static_cast<GennyPatch*>(_owner->getPatch(i))->InstrumentDef.Channels[j] && patch0->Channels[j])
							channels.push_back(j);
					}
				}
				else if (patch->InstrumentDef.Type == GIType::DAC)
				{
					if (static_cast<GennyPatch*>(_owner->getPatch(i))->InstrumentDef.Channels[5] && patch0->Channels[5])
						channels.push_back(5);
				}
				else if (patch->InstrumentDef.Type != GIType::NONE)
				{
					for (int j = 6; j < 10; j++)
					{
						if (static_cast<GennyPatch*>(_owner->getPatch(i))->InstrumentDef.Channels[j] && patch0->Channels[j])
							channels.push_back(j);
					}
				}
			}

			if(channels.size() == 0)
				continue;

			NoteInfo* best = &_channels[channels[0]];
			int bestIndex = -1;
			int bestScore = -999999;
			for(int j = 0; j < channels.size(); j++)
			{
				NoteInfo* info = &_channels[channels[j]];

				//Pick the first available channel initially
				if (bestIndex == -1)
				{
					best = &_channels[channels[j]];
					bestIndex = channels[j];
				}

				int score = 0;

				//Notes that aren't currently being used are the BEST
				if(info->note == -1)
					score += 50000;

				//Older notes are better
				score -= info->num;

				//Older releases are better
				score -= info->release;

				//Notes that have lived for a long time are better
				score += (min(info->age, 200000) / 200000.0f) * 500;

				//Add points for higher notes
				//score += info->note;

				if (patch->InstrumentDef.Ch3Special)
				{
					if (patch->InstrumentDef.MidiChannel == channel && channels[j] == 2)
						score += 100000;
				}
				else
				{
					if (thisPatch->InstrumentDef.snMelodicEnable && channels[j] == 8)
						continue; //We do not want to trigger PSG channel 2 when using melodic SN (we want to trigger the noise channel)

					//If in solo mode we want a channel that already has a note with the same patch, if possible
					if (thisPatch->InstrumentDef.soloMode && info->instrumentPatch == thisPatch && info->note != -1)
					{
						score += 55000;

						if (info->instrumentPatch->InstrumentDef.Delay == thisPatch->InstrumentDef.Delay)
							score += 5000;
					}
				}		

				if (score > bestScore)
				{
					best = info;
					bestIndex = channels[j];
					bestScore = score;
				}
			}

			if(bestIndex != -1)
			{
				bool fmChannel = patch->InstrumentDef.Type == GIType::FM || patch->InstrumentDef.Type == GIType::DAC;

				int rlNote = note;
				int newNote = note;
				if(rlNote / 100 < rangeLow || rlNote / 100 > rangeHigh)
					continue;

				if (thisPatch->InstrumentDef.snMelodicEnable && bestIndex == 8)
					continue;

				NoteInfo& ch = _channels[bestIndex];
				ch.note = note;
				if (thisPatch->InstrumentDef.glide != 0)
				{
					ch.glideSpeed = thisPatch->InstrumentDef.glide;

					if (soloTrigger)
						rlNote = newNote = ch.noteGlideCurrent;
					else if (thisPatch->lastPortaNote >= 0)
						rlNote = newNote = thisPatch->lastPortaNote;

					thisPatch->lastPortaNote = ch.note;
				}
				else
					ch.glideSpeed = 0;

				newNote += (octave - (GennyInstrumentParam_getRange(GIP_Octave) / 2)) * 1200;
				newNote += (transpose - (GennyInstrumentParam_getRange(GIP_Transpose) / 2)) * 100;			
				newNote += (thisPatch->InstrumentDef.Detune - 50);


				//Pull down snMelodic notes
				if (thisPatch->InstrumentDef.snMelodicEnable)
					newNote += (1200 * 2) - 100;


#ifdef BUILD_VST
				newNote += _owner->_globalPitchOffset[channel] * (_owner->bendRange * 100);
#endif

				rlNote = newNote / 100;

				int logNote = rlNote;


				ch.noteGlideFrom = newNote;
				ch.glideSamplesRun = 0;
				ch.num = _numNotes;
				ch.channel = channel;
				ch.noteData = data;
				ch.instrumentPatch = thisPatch;
				ch.patch = patch;
				ch.velocity = velocity;




				PingPongPan ppong = thisPatch->InstrumentDef.getAndIncrementPingPongPanning();
				if (ppong != PingPongPan::None)
				{
					float panOffset = (thisPatch->InstrumentDef.Panning / 100.0f) - 0.5f;
					if (panOffset == 0)
						panOffset = 1;

					if(ppong == PingPongPan::Left)
						panning = -panOffset;
					else if (ppong == PingPongPan::Right)
						panning = panOffset;
					else
						panning = 0;
				}


				ch.panning = panning;
				ch.operatorChannel = channel;
				ch.age = 0;

#if !BUILD_VST
				_owner->updateNoteControl(ch.patch, data);
#endif

				bool retrigger = false;
				if (thisPatch->InstrumentDef.soloMode)
				{
					retrigger = (ch.noteStack.size() != 0) && thisPatch->InstrumentDef.legatoMode;
					auto it = ch.noteStack.begin();
					while (it != ch.noteStack.end())
					{
						if ((*it).note == ch.note)
							break;

						it++;
					}

					if (it == ch.noteStack.end())
						ch.noteStack.push_back(SoloNoteInfo(ch.note, data));
				}

				if (bestIndex >= 10)
				{
					ch.triggerUnsetOperators = false;
					_chip.noteOnCh3Special(newNote, velocity * 127, 0.0f, _frequencyTable, patch, retrigger, channel);
					continue; //Operator Frequency channels need no further initialization
				}

				//Channel index guaranteed to be within FM/SN limits past this point
				ch.triggerUnsetOperators = true;		
				float tempo = _owner->getTempo(); 
				int samplesPerTick = _owner->getSamplesPerTick(); 
				float samplesPerSecond = (float)_owner->getBase()->_sampleRate;
				float delayInBeats = thisPatch->InstrumentDef.Delay / 64.0f;
				float samplesPerBeat = (60.0f / tempo) * samplesPerSecond;
				float ticksPerBeat = samplesPerBeat / samplesPerTick;
				ch.delay = (int)(samplesPerBeat * delayInBeats);

				if(_channelPatches[bestIndex] != patch || channelDirty[bestIndex])
				{
					channelDirty[bestIndex] = false;
					_channelPatches[bestIndex] = patch;

					if(bestIndex < 6)
						_chip.applyWholeChannel(*patch, bestIndex);
				}

				if(fmChannel)
				{
					_chip._channels[bestIndex].delay = ch.delay;
					if( patch->InstrumentDef.Type == GIType::DAC)
					{
		/*				char* cpath = patch->InstrumentDef.getSamplePath();
						if(cpath != nullptr)
						{
							std::string path = cpath;
							if(path.length() > 0 && patch->InstrumentDef.Drumset.hasDrums() == false)
							{
								std::fstream file(path.c_str(), std::ios::binary | std::ios::in | std::ios::ate);
								std::ifstream::pos_type size = file.tellg();
								char* memblock = new char [size];
								file.seekg (0, std::ios::beg);
								file.read (memblock, size);
								file.close();

								GennyData data;
								data.data = memblock;
								data.dataPos = 0;
								data.handle = 0;
								data.size = size;

								int pos = 0;
								GennyLoaders::loadDPACK(patch, data, &pos);
								delete[] memblock;
							}
						}*/

						_chip.setDACEnable(true);
						_chip.setDrumSet(&patch->InstrumentDef.Drumset);
						if (_chip.slotUpCurrentDrum(newNote))
						{
							//Update panning
							panningChanged(thisPatch, bestIndex);
							_chip.fireCurrentDrum(newNote, velocity * 127, bestIndex, _frequencyTable, patch);
						}
					}
					else 
					{
						_chip.mute(bestIndex);

						if(bestIndex == 5)
							_chip.setDACEnable(false);

						//Update panning
						panningChanged(thisPatch, bestIndex);

						if (patch->InstrumentDef.Ch3Special)
							_chip.noteOnCh3Special(newNote, velocity * 127, 0.0f, _frequencyTable, patch, retrigger, 0, false, true);
						else
							_chip.noteOn(newNote, velocity * 127, bestIndex, _frequencyTable, patch, retrigger);
					}

					_owner->updateChannel(bestIndex, true);
				}
				else
				{
					int snDT = patch->getFromBaron(baron->getIndex(baron->getYMParamIndex(SN_DT))) - 50;
					newNote += snDT;
					ch.detune = snDT;

					//Update panning
					panningChanged(thisPatch, bestIndex);

					SimpleEnvelope env;
					env.lev = (*patch->InstrumentDef.Data.getParameter(YM_TL, 0, 0) / (float)YM2612Param_getRange(YM_TL));
					env.atk = 1.0f - (*patch->InstrumentDef.Data.getParameter(YM_AR, 0, 0) / (float)YM2612Param_getRange(YM_AR));
					env.dr1 = 1.0f - (*patch->InstrumentDef.Data.getParameter(YM_DR, 0, 0) / (float)YM2612Param_getRange(YM_DR));
					env.sus = 1.0f - (*patch->InstrumentDef.Data.getParameter(YM_SL, 0, 0) / (float)YM2612Param_getRange(YM_SL));
					env.dr2 = *patch->InstrumentDef.Data.getParameter(YM_SR, 0, 0) / (float)YM2612Param_getRange(YM_SR);
					env.rr = 1.0f - (*patch->InstrumentDef.Data.getParameter(YM_RR, 0, 0) / (float)YM2612Param_getRange(YM_RR));


					if (thisPatch->InstrumentDef.snMelodicEnable) //Shift Register 3 takes its tone from Tone 2, '4' signals this special behavior
					{
						env.melodicMode = true;
						env.sr = 3;
					}
					else
						env.sr = patch->InstrumentDef.Data.getParameterChar(SN_SR, 0, -1);

					env.periodic = patch->InstrumentDef.Data.getParameterChar(SN_PERIODIC, 0, -1);

					env.state = ES_Attack;
					env.currentDelay = (int)(samplesPerBeat * delayInBeats);

					_snChip.noteOn(newNote, velocity * 127, bestIndex - 6, env, _frequencyTable, retrigger);
					_owner->updateChannel(bestIndex, true);
				}
			}
		}
	}
}

void Genny2612::startNote(int patchIndex, int pan)
{

}

void Genny2612::midiTick()
{
	_chip.midiTick();
}

void Genny2612::updateNote(void* noteData, int samples)
{
	for (int i = 0; i < kNumNoteChannels; i++)
	{
		NoteInfo& ch = _channels[i];
		if (ch.instrumentPatch == nullptr)
			continue;

#if BUILD_VST
		if (ch.note != -1)
#else
		if (ch.noteData == noteData && noteData != nullptr)
#endif
		{
			int offset = (ch.instrumentPatch->InstrumentDef.Octave - (GennyInstrumentParam_getRange(GIP_Octave) / 2)) * 1200;
			offset += (ch.instrumentPatch->InstrumentDef.Transpose - (GennyInstrumentParam_getRange(GIP_Transpose) / 2)) * 100;
			if (i > 5 && i < 10)
			{
				IndexBaron* baron = getIndexBaron();
				int snDT = ch.patch->getFromBaron(baron->getIndex(baron->getYMParamIndex(SN_DT))) - 50;
				offset += snDT;
			}
			offset += (ch.instrumentPatch->InstrumentDef.Detune - 50);

			//Pull down snMelodic notes
			if (ch.instrumentPatch->InstrumentDef.snMelodicEnable)
				offset += (1200 * 2) - 100;

			/*if (ch.patch->InstrumentDef.Ch3Special)
			{
				offset += ((ch.patch->InstrumentDef.OperatorOctave[3] - 3) * 1000);
				offset += ((ch.patch->InstrumentDef.OperatorTranspose[3] - 11) * 100);
				offset += (ch.patch->InstrumentDef.OperatorDetune[3] - 50);
			}*/

#if BUILD_VST
			offset += _owner->_globalPitchOffset[ch.channel] * (_owner->bendRange * 100);
			int velocity = (int)(sqrt(sqrt(ch.velocity)) * 127.0f);
#else
			PlugVoice* voice = (PlugVoice*)noteData;
			int velocity = (int)(sqrt(sqrt(voice->Params->InitLevels.Vol)) * 127.0f);
			_owner->updateNoteControl(ch.patch, noteData);
#endif

			float vibrato = 0.0f;
			for (int iVibrato = 0; iVibrato < _vibratoNotes.size(); iVibrato++)
			{
				if (_vibratoNotes[iVibrato].channel == ch.channel)
					vibrato = sin(_vibratoNotes[iVibrato].vibratoPhase * (3.14159265f * 2)) * ((min(_vibratoNotes[iVibrato].velocity, 127) / 127.0f) * 1.0f);
			}

			int glissOffset = 0;
			if (ch.glideSpeed >= 0)
			{
				float tempo = _owner->getTempo();
				int samplesPerTick = _owner->getSamplesPerTick();

				float samplesPerSecond = (float)_owner->getBase()->_sampleRate;
				float glideTimeInBeats = ch.glideSpeed / 64.0f;
				float samplesPerBeat = (60.0f / tempo) * samplesPerSecond;
				float realGlideTime = glideTimeInBeats * samplesPerBeat;
				ch.glideSamplesRun += samples;

				float progress = 1.0f - min(max(ch.glideSamplesRun / realGlideTime, 0.0f), 1.0f);
				glissOffset = (ch.noteGlideFrom - ch.note) * progress;
			}

#if BUILD_VST
			int offsetPitch = ch.note + glissOffset;
#else
			int offsetPitch = (voice->Params->FinalLevels.Pitch + 6000) + glissOffset;
#endif
			ch.noteGlideCurrent = offsetPitch;
			offsetPitch += offset;

			ch.age += samples;

			if (ch.patch->InstrumentDef.Ch3Special)
				_chip.noteOnCh3Special(offsetPitch, velocity, vibrato, _frequencyTable, ch.patch, false, ch.operatorChannel, true, ch.triggerUnsetOperators);
			else if (i < 6)
				_chip.writeFrequencies(offsetPitch, velocity, i, vibrato, _frequencyTable, ch.patch);
			else if (i < 10)
				_snChip.writeFrequencies(offsetPitch, velocity, i - 6, _snChip.getEnvelope(i - 6), false, vibrato, _frequencyTable);
		}
	}
}

void Genny2612::noteOff(const int note, int channel, void* noteData)
{
 	_releases += 1;
	bool didRelease = false;
	for(int i = 0; i < kNumNoteChannels; i++)
	{
		NoteInfo& ch = _channels[i];

		if (ch.instrumentPatch == nullptr)
			continue;

		if (ch.instrumentPatch->InstrumentDef.soloMode)
		{
			auto it = ch.noteStack.begin();
			while (it != ch.noteStack.end())
			{
				if ((*it).note == note)
					break;

				it++;
			}

			if (it != ch.noteStack.end())
				ch.noteStack.erase(it);
		}

		if(((ch.noteData == nullptr && ch.note == note && ch.channel == channel) || (ch.noteData != nullptr && ch.noteData == noteData)))
		{
			int offset = (ch.instrumentPatch->InstrumentDef.Octave - (GennyInstrumentParam_getRange(GIP_Octave) / 2)) * 1200;
			offset += (ch.instrumentPatch->InstrumentDef.Transpose - (GennyInstrumentParam_getRange(GIP_Transpose) / 2)) * 100;

			bool off = true;
			if (ch.instrumentPatch->InstrumentDef.soloMode)
			{
				int nextNote = -1;
				if(ch.noteStack.size() > 0)
				{
					off = false;
					noteOn(ch.noteStack[ch.noteStack.size() - 1].note, ch.velocity, ch.channel, ch.panning, ch.noteStack[ch.noteStack.size() - 1].noteData, true);
				}
			}

			if(off)
			{
				if (ch.patch->InstrumentDef.Ch3Special)
					_chip.noteOffCh3Special(note + offset, ch.patch, ch.operatorChannel, ch.triggerUnsetOperators);
				else if (i < 6)
					_chip.noteOff(i, note + offset);
				else if(i < 10)
					_snChip.noteOff(i - 6);

				if (ch.instrumentPatch != nullptr)
					ch.instrumentPatch->lastPortaNote = -1;

				ch.noteStack.clear();
				ch.note = -1;
				ch.noteGlideFrom = -1;
				ch.noteGlideCurrent = -1;
				ch.release = _releases;
				_owner->updateChannel(i, false);

				ch.noteData = nullptr;
				didRelease = true;
			}

		}
	}

	if(didRelease == false)
	{
		//Remove vibrato notes on release.
		int size = _vibratoNotes.size();
		for(int i = 0; i < size; i++)
		{
			if((_vibratoNotes[i].noteData == nullptr && _vibratoNotes[i].note == note && _vibratoNotes[i].channel == channel) || (_vibratoNotes[i].noteData != nullptr && _vibratoNotes[i].noteData == noteData))
			{
				_vibratoNotes.erase(_vibratoNotes.begin() + i);
				size--;
				i--;
			}
		}
	}
}

void Genny2612::clearNotes(GennyPatch* instrument, int channel)
{
	if (instrument == nullptr && channel < 0)
	{
		clearCache(); //Full clear
		_vibratoNotes.clear();
	}
	else //Targeted clear
	{
		//Reset pingpongs
		for (int i = 0; i < kMaxInstruments; i++)
		{
			if (instrument == nullptr || _owner->_patches[i] == instrument)
				((GennyPatch*)_owner->_patches[i])->InstrumentDef.PingPongPanIndex = 0;
		}
	}

	for (int i = 0; i < kNumNoteChannels; i++)
	{
		NoteInfo& ch = _channels[i];
		if ((instrument == nullptr || ch.instrumentPatch == instrument) && (channel < 0 || i == channel))
		{
			int size = _vibratoNotes.size();
			for (int i2 = 0; i2 < size; i2++)
			{
				if ((_vibratoNotes[i2].noteData == nullptr && _vibratoNotes[i2].note == ch.note && _vibratoNotes[i2].channel == channel) || (_vibratoNotes[i2].noteData != nullptr && _vibratoNotes[i2].noteData == ch.noteData))
				{
					_vibratoNotes.erase(_vibratoNotes.begin() + i2);
					size--;
					i2--;
				}
			}

			_channelPatches[i] = nullptr;
			channelDirty[i] = true;
			if (i <= 5)
			{
				_chip.clearCache(i);
				_chip.fullStop(i);
			}
			else if (i < 10)
				_snChip.fullStop(i - 6);

			if (ch.note >= 0 && ch.instrumentPatch != nullptr)
			{
#if BUILD_VST
				int offset = (ch.instrumentPatch->InstrumentDef.Octave - (GennyInstrumentParam_getRange(GIP_Octave) / 2));
				offset += (ch.instrumentPatch->InstrumentDef.Transpose - (GennyInstrumentParam_getRange(GIP_Transpose) / 2));
#else
				int offset = (ch.instrumentPatch->InstrumentDef.Octave - (GennyInstrumentParam_getRange(GIP_Octave) / 2)) * 1200;
				offset += (ch.instrumentPatch->InstrumentDef.Transpose - (GennyInstrumentParam_getRange(GIP_Transpose) / 2)) * 100;
#endif

				if (i < 6)
					_chip.noteOff(i, ch.note + offset);
				else if(i < 10)
					_snChip.noteOff(i - 6);

				ch.noteStack.clear();
				ch.note = -1;
				ch.noteGlideFrom = -1;
				ch.noteGlideCurrent = -1;
				ch.release = _releases;
				ch.noteData = NULL;
			}

			ch.instrumentPatch = nullptr;
			if(i < 10)
				_owner->updateChannel(i, false);
		}
	}

}

int waiter = 0;
void Genny2612::update(float** buffer, int numSamples)
{
	if (_snChip._hardwareMode != _chip._hardwareMode)
		clearCache();

	_chip._hardwareMode = _owner->megaMidiPort > 0;
	_chip._emulationMute = _owner->megaMidiPort > 0 && _owner->megaMidiVSTMute;
	_snChip._hardwareMode = _chip._hardwareMode;
	_snChip._emulationMute = _chip._emulationMute;
	if (_initializedChannels == false)
	{
		_chip.fullStop();
		_snChip.fullStop();
		_initializedChannels = true;
	}

	_processor.update(buffer, numSamples);

	int size = _vibratoNotes.size();
	float tempo = _owner->getTempo();
	for (int i = 0; i < size; i++)
		_vibratoNotes[i].Calculate(tempo, numSamples, _owner->getBase()->_sampleRate);

#if BUILD_VST
	updateNote(nullptr, numSamples);
#endif

	_owner->midiFlush();


}

void Genny2612::setFromBaron(IBIndex* param, int channel, float val)
{
	_chip.setFromBaron(param, channel, val);
}

void Genny2612::startLogging(std::string file)
{
	_logging = true;
	_loggingFile = file;
}

void Genny2612::stopLogging()
{
	_logging = false;
	if(_logger.isLogging())
		_logger.finishLogging();

	_owner->loggingComplete();

	_chip.setLogger(nullptr);
	_snChip.setLogger(nullptr);
}


void Genny2612::clearCache()
{
	for (int i = 0; i < kNumNoteChannels; i++)
	{
		_channelPatches[i] = nullptr; 
		channelDirty[i] = true;
	}

	_chip.clearCache();
	_snChip.clearCache();

	//Reset pingpongs
	for (int i = 0; i < kMaxInstruments; i++)
	{
		((GennyPatch*)_owner->_patches[i])->InstrumentDef.PingPongPanIndex = 0;
	}
}

void Genny2612::paramChanged(GennyPatch* patch, YM2612Param param, int channel, int op)
{
	if (channel < 0 || channel >= kNumNoteChannels)
		return;

	if (_channels[channel].patch == patch)
	{
		_chip.writeParameter(param, channel, op);
	}
}

void Genny2612::panningChanged(GennyPatch* instrument, int channel)
{
	if (_channels[channel].instrumentPatch == instrument)
	{
		float lPan = 0.0f;
		float rPan = 0.0f;
		float speciallPan = 0.0f;
		float specialrPan = 0.0f;

		float pan = (int)(instrument->InstrumentDef.Panning - 127) / 127.0f;
		float completePan = max(min(_channels[channel].panning + pan, 1.0f), -1.0f);

#ifdef BUILD_VST
		speciallPan = 1.0f - completePan; 
		if (speciallPan > 1.0f)
			speciallPan = 1.0f;

		specialrPan = 1.0f + completePan;
		if (specialrPan > 1.0f)
			specialrPan = 1.0f;
#else
		_owner->getBase()->PlugHost->ComputeLRVol(lPan, rPan, completePan, _channels[channel].velocity);

		_owner->getBase()->PlugHost->ComputeLRVol(speciallPan, specialrPan, completePan, 1.4f);
#endif

		IndexBaron* baron = getIndexBaron();
		if (baron->enableTrueStereo)
		{
			if (instrument->InstrumentDef.EnableL == false)
				speciallPan = 0;
			if (instrument->InstrumentDef.EnableR == false)
				specialrPan = 0;
		}

		if (_channels[channel].patch->InstrumentDef.Type == GIType::DAC)
		{
			WaveData* cur = _channels[channel].patch->InstrumentDef.Drumset.getCurrentDrum(_chip._sampleRate, _logger.isLogging());
			if (cur != nullptr)
			{
				if (cur->panLeft == false)
					speciallPan = 0;
				if (cur->panRight == false)
					specialrPan = 0;
			}
		}

		if (channel < 6)
		{
			_chip.getImplementation()->fm_enablePerNotePanning = baron->enableTrueStereo;
			_chip.getImplementation2()->fm_enablePerNotePanning = baron->enableTrueStereo;
			IBIndex* left = baron->getIndex(baron->getYMParamIndex(YM_L_EN));
			IBIndex* right = baron->getIndex(baron->getYMParamIndex(YM_R_EN));

			_chip.channelPanStatesL[channel] = (completePan <= 0.5f && instrument->InstrumentDef.EnableL && speciallPan != 0) ? 1 : 0;
			_chip.channelPanStatesR[channel] = (completePan >= -0.5f && instrument->InstrumentDef.EnableR && specialrPan != 0) ? 1 : 0;
   			_chip.panningChanged(channel);

			if (baron->enableTrueStereo)
			{
				_chip.getImplementation()->fm_perNoteVolumeL[channel] = (float)speciallPan;
				_chip.getImplementation()->fm_perNoteVolumeR[channel] = (float)specialrPan;
				_chip.getImplementation2()->fm_perNoteVolumeL[channel] = (float)speciallPan;
				_chip.getImplementation2()->fm_perNoteVolumeR[channel] = (float)specialrPan;
			}
		}
		else if (channel < 10)
		{
			_snChip._impl->sn_enablePerNotePanning = baron->enableTrueStereo;
			_snChip._implHQ->sn_enablePerNotePanning = baron->enableTrueStereo;
			if (baron->enableTrueStereo)
			{
				_snChip._impl->sn_perNoteVolumeL[channel - 6] = speciallPan;
				_snChip._impl->sn_perNoteVolumeR[channel - 6] = specialrPan;
				_snChip._implHQ->sn_perNoteVolumeL[channel - 6] = speciallPan;
				_snChip._implHQ->sn_perNoteVolumeR[channel - 6] = specialrPan;
			}
		}
	}
}


