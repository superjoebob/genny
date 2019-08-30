#include "Genny2612.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "GennyLoaders.h"
#include "sn76489.h"
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

bool Genny2612::channelDirty[10] = { true, true, true, true, true, true, true, true, true, true };

void VibratoInfo::Calculate(float tempo, int samples)
{

#if BUILD_VST
	int realNote = note - 96;
	float speedMul = realNote + 1.0f;
#else
	int realNote = note - 9600;
	float speedMul = (realNote / 100.0f) + 1.0f;
#endif

	float secondFraction = (float)samples / (44100.0f * (240.0f / tempo));
	vibratoInc = secondFraction;
	vibratoPhase += vibratoInc * speedMul;
	while(vibratoPhase > 1.0f)
		vibratoPhase -= 1.0f;

	//vibratoInc = 
}

void GennyInstrument::catalogue(IndexBaron* baron)
{
	baron->addIndex(new IBInsParam(0, GIP_DAC, "DAC"));
	baron->addIndex(new IBInsParam(0, GIP_Channel0, "Global CH0"));
	baron->addIndex(new IBInsParam(0, GIP_Channel1, "Global CH1"));
	baron->addIndex(new IBInsParam(0, GIP_Channel2, "Global CH2"));
	baron->addIndex(new IBInsParam(0, GIP_Channel3, "Global CH3"));
	baron->addIndex(new IBInsParam(0, GIP_Channel4, "Global CH4"));
	baron->addIndex(new IBInsParam(0, GIP_Channel5, "Global CH5"));
	baron->addIndex(new IBInsParam(0, GIP_Channel6, "Global CH6"));
	baron->addIndex(new IBInsParam(0, GIP_Channel7, "Global CH7"));
	baron->addIndex(new IBInsParam(0, GIP_Channel8, "Global CH8"));
	baron->addIndex(new IBInsParam(0, GIP_Channel9, "Global CH9"));
	baron->addIndex(new IBInsParam(0, GIP_MidiChannel, "Midi CH"));
	baron->addIndex(new IBInsParam(0, GIP_FM, "FM Enable"));

	for(int i = 0; i < 32; i++)
	{
		baron->addIndex(new IBInsParam(0, (GennyInstrumentParam)(GIP_DACSamplePathStart + i)));
	}
	baron->addIndex(new IBInsParam(0, GIP_Octave, "Octave"));
	baron->addIndex(new IBInsParam(0, GIP_Transpose, "Transpose"));
	baron->addIndex(new IBInsParam(0, GIP_Panning, "Panning"));


	baron->addIndex(new IBInsParam(0, GIP_RangeLow, "Range Low"));
	baron->addIndex(new IBInsParam(0, GIP_RangeHigh, "Range High"));
	baron->addIndex(new IBInsParam(0, GIP_Delay, "Delay"));

	baron->addIndex(new IBInsParam(0, GIP_SelectedDrum, "Selected Drum"));

	for(int i = 0; i < 6; i++)
	{
		baron->addIndex(new IBInsParam(0, GIP_Extra, "Extra"));
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
			if(p->getParameter() >= GIP_DACSamplePathStart && p->getParameter() <= GIP_DACSamplePathEnd)
			{
				DACSamplePath[p->getParameter() - GIP_DACSamplePathStart] = val;
				break;
			}
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
			if(p->getParameter() >= GIP_DACSamplePathStart && p->getParameter() <= GIP_DACSamplePathEnd)
			{
				return DACSamplePath[p->getParameter() - GIP_DACSamplePathStart];
				break;
			}
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
	Globals.catalogue(baron);
	for(int i = 0; i < 16; i++)
		baron->addIndex(new IBPatchParam((GennyPatchParam)(GPP_Ins01 + i)));
	for(int i = 0; i < 10; i++)
		baron->addIndex(new IBPatchParam((GennyPatchParam)(GPP_Channel0 + i)));
	baron->addIndex(new IBPatchParam(GPP_SelectedInstrument));
	InstrumentDef.catalogue(baron);
}

void GennyPatch::setFromBaron(IBIndex* param, float val)
{
	if(param->getType() == IB_PatchParam)
	{
		IBPatchParam* p = static_cast<IBPatchParam*>(param);
		
		if(p->getParameter() >= GPP_Ins01 && p->getParameter() <= GPP_Ins16)
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
		if(p->getInstrument() < 0)
			return Globals.setFromBaron(param, val);
		else
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
	if(param->getType() == IB_PatchParam)
	{
		IBPatchParam* p = static_cast<IBPatchParam*>(param);

		if(p->getParameter() >= GPP_Ins01 && p->getParameter() <= GPP_Ins16)
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
	else if(param->getType() == IB_YMParam)
	{
		IBYMParam* p = static_cast<IBYMParam*>(param);
		if(p->getInstrument() < 0)
			return Globals.getFromBaron(param);
		else
			return InstrumentDef.getFromBaron(param);
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
	_logging(false)
{
	memset(_channelPatches, 0, sizeof(int) * 10);

	_defaultFrequencyTable = new double[129];

	_defaultFrequencyTable[0] = 8.17579891564371;
	for(int i = 1; i < 129; i++)
		_defaultFrequencyTable[i] = 100 * (i - 1);

	_frequencyTable = _defaultFrequencyTable;
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
	_indexBaron = new IndexBaron();
	_patches[0].catalogue(_indexBaron);

	_chip.initialize();
	_processor.initialize();
	_processor.setChip(&_chip);

	_snChip.Initialize(SN76489_NTSC, 44100);
	_processor.setSNChip(&_snChip);
}

static int numLogged = 0;
void Genny2612::noteOn(int note, float velocity, unsigned char channel, float panning, void* data)
{
	//If the note is higher than 95 it is an effect note (vibrato) and should not be played.
 	bool isEffect = false;
	int logNote = note;

#if BUILD_VST
	note *= 100;
	velocity /= 127.0f;
#endif

//#ifdef BUILD_VST
//	if(note >= 96)
//		isEffect = true;
//#else
	if(note >= 9600)
		isEffect = true;
	logNote /= 100;
//#endif			
	if(isEffect)
	{
		if(_logging)
		{
			if(logNote == 125 && _logger.isLogging())
			{
				_logger.writeLoopPoint();	
				return;
			}
			else if(logNote == 126 && _logger.isLogging() == false)
			{
				_chip.setDACEnable(false);
				_chip.dirtyChannels();
				_logger.startLogging(_owner, _loggingFile);
				_snChip.setLogger(&_logger);
				_chip.setLogger(&_logger);		
				return;
			}
			else if(logNote == 127 && _logger.isLogging() == true)
			{
				stopLogging();
				return;
			}
		}





		VibratoInfo vibrato;
		vibrato.note = note;
		vibrato.noteData = data;
		vibrato.channel = channel;

		//release functions as velocity with vibrato note info
		vibrato.velocity = velocity * 127;

		_vibratoNotes.push_back(vibrato);
		return;
	}

	
	
	_numNotes += 1;

	std::vector<int> channels;
	GennyPatch* patch0 = static_cast<GennyPatch*>(_owner->getPatch(0));
	IndexBaron* baron = _indexBaron;
	for(int i = 0; i < 16; i++)
	{
		channels.clear();

		int ins = patch0->Instruments[i];
		if(ins == -1 || static_cast<GennyPatch*>(_owner->getPatch(2))->Instruments[i] >= 0)
			continue;

		GennyPatch* patch = static_cast<GennyPatch*>(_owner->getPatch(ins));
		GennyPatch* thisPatch = static_cast<GennyPatch*>(_owner->getPatch(i));
		int midiChannel = thisPatch->InstrumentDef.MidiChannel;
		int octave = thisPatch->InstrumentDef.Octave;
		int transpose = thisPatch->InstrumentDef.Transpose;

		int rangeLow = thisPatch->InstrumentDef.HighRange;
		int rangeHigh = thisPatch->InstrumentDef.LowRange;

		if(midiChannel == channel)
		{
			//Figure out which channels this instrument uses
			int firstChannel = -1;
			for(int j = 0; j < 10; j++)
			{
				bool enabled = static_cast<GennyPatch*>(_owner->getPatch(i))->InstrumentDef.Channels[j];
				if(enabled)
				{
					channels.push_back(j);
				}
			}

			if(channels.size() == 0)
				continue;

			NoteInfo* best = &_channels[channels[0]];
			int bestIndex = -1;
			for(int j = 0; j < channels.size(); j++)
			{
				if(patch0->Channels[channels[j]] == true)
				{
					best = &_channels[channels[j]];
					bestIndex = channels[j];
					break;
				}
			} 
			for(int j = 0; j < channels.size(); j++)
			{
				NoteInfo* info = &_channels[channels[j]];
				bool isReleasing = false;
#ifndef BUILD_VST
				PlugVoice* voice = (PlugVoice*)info->noteData;
				if(voice != nullptr)
					isReleasing = voice->State == -1;
#endif

				if((info->note == -1 || isReleasing) && best->note != -1 && patch0->Channels[channels[j]])
				{
					best = info;
					bestIndex = channels[j];
					continue;
				}
				if(info->note != -1 && best->note == -1)
					continue;

				if(info->num < best->num)
				{
					if(info->release < best->release && patch0->Channels[channels[j]])
					{
						best = info;
						bestIndex = channels[j];
					}
				}
			}
			if(bestIndex != -1)
			{
				if(patch0->Channels[bestIndex] == false)
					continue;

				int rlNote = note;
				int newNote = note;

				if(rlNote / 100 < rangeLow || rlNote / 100 > rangeHigh)
					continue;

//#ifndef BUILD_VST
				newNote += (octave - (GennyInstrumentParam_getRange(GIP_Octave) / 2)) * 1200;
				newNote += (transpose - (GennyInstrumentParam_getRange(GIP_Transpose) / 2)) * 100;
				rlNote = newNote / 100;

				int logNote = rlNote;

//#else
//				newNote += (octave - (GennyInstrumentParam_getRange(GIP_Octave) / 2)) * 12;
//				newNote += (transpose - (GennyInstrumentParam_getRange(GIP_Transpose) / 2));
//
//				int logNote = newNote / 100;
//#endif





				//if(_channels[bestIndex].noteData != nullptr)
				//{	
				//	setFromBaron(baron->getIndex(baron->getYMParamIndex(YM_TL, 0)), bestIndex, 0x7F);
				//	setFromBaron(baron->getIndex(baron->getYMParamIndex(YM_TL, 1)), bestIndex, 0x7F);
				//	setFromBaron(baron->getIndex(baron->getYMParamIndex(YM_TL, 2)), bestIndex, 0x7F);
				//	setFromBaron(baron->getIndex(baron->getYMParamIndex(YM_TL, 3)), bestIndex, 0x7F);	
				//	noteOff(_channels[bestIndex].note, bestIndex, _channels[bestIndex].noteData);
				//}

//#ifdef BUILD_VST
				_channels[bestIndex].note = note;
//#else
//				_channels[bestIndex].note = (note / 100);
//#endif
				_channels[bestIndex].num = _numNotes;
				_channels[bestIndex].channel = channel;
				_channels[bestIndex].noteData = data;
				_channels[bestIndex].instrumentPatch = static_cast<GennyPatch*>(_owner->getPatch(i));
				_channels[bestIndex].patch = patch;
				_channels[bestIndex].velocity = velocity * 127;
				_channels[bestIndex].floatVelocity = velocity;

				float pan = (int)(thisPatch->InstrumentDef.Panning - 127) / 127.0f;
				float completePan = max(min(panning + pan, 1.0f), -1.0f);

				float lPan = 0.0f;
				float rPan = 0.0f;

#ifndef BUILD_VST
				_owner->getBase()->PlugHost->ComputeLRVol(lPan, rPan, completePan, velocity);
#endif
				
				int specialIndex = getIndexBaron()->getYMParamIndex(YM_SPECIAL);
				float enableSpecialPanning = patch0->getFromBaron(baron->getIndex(specialIndex)) > 0.5f;

				float tempo = _owner->getTempo(); 
				int samplesPerTick = _owner->getSamplesPerTick(); 
				float samplesPerSecond = 44100.0f;
				float delayInBeats = thisPatch->InstrumentDef.Delay / 64.0f;
				float samplesPerBeat = (60.0f / tempo) * samplesPerSecond;
				float ticksPerBeat = samplesPerBeat / samplesPerTick;

				if(bestIndex <= 5)
				{
					_chip.getChannel(bestIndex)->_currentDelay = (int)(ticksPerBeat * delayInBeats);
					IBIndex* left = baron->getIndex(baron->getYMParamIndex(YM_L_EN));
					IBIndex* right = baron->getIndex(baron->getYMParamIndex(YM_R_EN));
					
					_chip.getImplementation()->fm_enablePerNotePanning = enableSpecialPanning;
					if(enableSpecialPanning)
					{
						patch->setFromBaron(left, 1);
						setFromBaron(left, bestIndex, 1);
						patch->setFromBaron(right, 1);
						setFromBaron(right, bestIndex, 1); 

						_chip.getImplementation()->fm_perNoteVolumeL[bestIndex] = lPan;
						_chip.getImplementation()->fm_perNoteVolumeR[bestIndex] = rPan;
					}
					else
					{
						patch->setFromBaron(left, completePan <= 0.5f);
						setFromBaron(left, bestIndex, completePan <= 0.5f);
						patch->setFromBaron(right, completePan >= -0.5f);
						setFromBaron(right, bestIndex, completePan >= -0.5f); 
					}
				}
				else
				{
					int snDT = patch->getFromBaron(baron->getIndex(baron->getYMParamIndex(SN_DT))) - 50;
					newNote += snDT;
					_channels[bestIndex].detune = snDT;
				 
					_snChip.getCore()->sn_enablePerNotePanning = enableSpecialPanning;				
					if(enableSpecialPanning)
					{
					_snChip.getCore()->sn_perNoteVolumeL[bestIndex - 6] = lPan;
					_snChip.getCore()->sn_perNoteVolumeR[bestIndex - 6] = rPan; 
					}
				}


				//if(_channelPatches[bestIndex] != patch || channelDirty[bestIndex])
				{
					channelDirty[bestIndex] = false;
					_channelPatches[bestIndex] = patch;
					if(bestIndex < 6)
					{ 
						int count = GennyPatch::getNumParameters();
						for(int q = 0; q < count; q++)
						{
							setFromBaron(baron->getIndex(q), bestIndex, patch->getFromBaron(baron->getIndex(q)));
						}
					}
				}

				if(patch->InstrumentDef.Type == GIType::FM || patch->InstrumentDef.Type == GIType::DAC)
				{
					if( patch->InstrumentDef.Type == GIType::DAC)
					{
						char* cpath = patch->InstrumentDef.getSamplePath();
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
								GennyPatch* newPatch = GennyLoaders::loadDPACK("Hamburger", data, &pos);
								patch->InstrumentDef.Drumset = newPatch->InstrumentDef.Drumset;
								delete newPatch;
							}
						}

						_chip.setDACEnable(true);
						_chip.setDrumSet(&patch->InstrumentDef.Drumset);
					}
					else if(bestIndex == 5)
					{
						//Disable DAC if another instrument is trying to use channel 6
						_chip.setDACEnable(false);
					}


					
	/*				setFromBaron(baron->getIndex(baron->getYMParamIndex(YM_RR, 0)), bestIndex, 15);
					setFromBaron(baron->getIndex(baron->getYMParamIndex(YM_RR, 1)), bestIndex, 15);
					setFromBaron(baron->getIndex(baron->getYMParamIndex(YM_RR, 2)), bestIndex, 15);
					setFromBaron(baron->getIndex(baron->getYMParamIndex(YM_RR, 3)), bestIndex, 15);
					setFromBaron(baron->getIndex(baron->getYMParamIndex(YM_SL, 0)), bestIndex, 15);
					setFromBaron(baron->getIndex(baron->getYMParamIndex(YM_SL, 1)), bestIndex, 15);
					setFromBaron(baron->getIndex(baron->getYMParamIndex(YM_SL, 2)), bestIndex, 15);
					setFromBaron(baron->getIndex(baron->getYMParamIndex(YM_SL, 3)), bestIndex, 15);*/

					_chip.noteOn(newNote, velocity * 127, bestIndex, _frequencyTable, patch);
					_owner->updateChannel(bestIndex, true);
				}
				else
				{
					SimpleEnvelope env;
					env.lev = 1.0f - (*patch->InstrumentDef.Data.getParameter(YM_TL, 0, 0) / (float)YM2612Param_getRange(YM_TL));
					env.atk = *patch->InstrumentDef.Data.getParameter(YM_AR, 0, 0) / (float)YM2612Param_getRange(YM_AR);
					env.dr1 = *patch->InstrumentDef.Data.getParameter(YM_DR, 0, 0) / (float)YM2612Param_getRange(YM_DR);
					env.sus = *patch->InstrumentDef.Data.getParameter(YM_SL, 0, 0) / (float)YM2612Param_getRange(YM_SL);
					env.dr2 = *patch->InstrumentDef.Data.getParameter(YM_SR, 0, 0) / (float)YM2612Param_getRange(YM_SR);
					env.rr = *patch->InstrumentDef.Data.getParameter(YM_RR, 0, 0) / (float)YM2612Param_getRange(YM_RR);

					env.sr = patch->InstrumentDef.Data.getParameterChar(SN_SR, 0, -1);
					env.periodic = patch->InstrumentDef.Data.getParameterChar(SN_PERIODIC, 0, -1);

					env.state = ES_Attack;
					env.currentDelay = (int)(samplesPerBeat * delayInBeats);

					_snChip.noteOn(newNote, velocity * 127, bestIndex - 6, env, _frequencyTable);
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

void Genny2612::updateNote(void* noteData)
{
	//Note updates only happen in the FL version
#ifndef BUILD_VST
	for(int i = 0; i < 10; i++)
	{
		if(_channels[i].noteData == noteData)
		{
			int offset = (_channels[i].instrumentPatch->InstrumentDef.Octave - (GennyInstrumentParam_getRange(GIP_Octave) / 2)) * 1200;
			offset += (_channels[i].instrumentPatch->InstrumentDef.Transpose - (GennyInstrumentParam_getRange(GIP_Transpose) / 2)) * 100;
			offset += _channels[i].detune;


			PlugVoice* voice = (PlugVoice*)noteData;

			int velocity = (int)(sqrt(sqrt(voice->Params->InitLevels.Vol)) * 127.0f);

			if(velocity > 127)
				velocity = 127;
			if(velocity < 0)
				velocity = 0;

			float vibrato = 0.0f;
			for(int iVibrato = 0; iVibrato < _vibratoNotes.size(); iVibrato++)
			{
				if(_vibratoNotes[iVibrato].channel == _channels[i].channel)
					vibrato = sin(_vibratoNotes[iVibrato].vibratoPhase * (3.14159265f * 2)) * ((min(_vibratoNotes[iVibrato].velocity, 127) / 127.0f) * 1.0f);
			}



			if(i < 6)
				_chip.writeFrequencies((voice->Params->FinalLevels.Pitch + 6000) + offset, velocity, i, vibrato, _frequencyTable, _channels[i].patch);
			else
				_snChip.writeFrequencies((voice->Params->FinalLevels.Pitch + 6000) + offset, velocity, i - 6, _snChip.getEnvelope(i - 6), false, vibrato, _frequencyTable);
		}
	}
#endif
}

void Genny2612::noteOff(int note, int channel, void* noteData)
{
#ifdef BUILD_VST
	note = (note * 100);
#endif

	_releases += 1;
	bool didRelease = false;
	for(int i = 0; i < 10; i++)
	{
		//int rlNote = note;
		//int offset = 0;
		//if(_channels[i].instrumentPatch != nullptr)
		//	offset = (_channels[i].instrumentPatch->InstrumentDef.Octave - (GennyInstrumentParam_getRange(GIP_Octave) / 2)) * 12;

		//rlNote += offset;



		if((_channels[i].noteData == nullptr && _channels[i].note == note && _channels[i].channel == channel) || (_channels[i].noteData != nullptr && _channels[i].noteData == noteData))
		{

#if BUILD_VST
			int offset = (_channels[i].instrumentPatch->InstrumentDef.Octave - (GennyInstrumentParam_getRange(GIP_Octave) / 2));
			offset += (_channels[i].instrumentPatch->InstrumentDef.Transpose - (GennyInstrumentParam_getRange(GIP_Transpose) / 2));
#else
			int offset = (_channels[i].instrumentPatch->InstrumentDef.Octave - (GennyInstrumentParam_getRange(GIP_Octave) / 2)) * 1200;
			offset += (_channels[i].instrumentPatch->InstrumentDef.Transpose - (GennyInstrumentParam_getRange(GIP_Transpose) / 2)) * 100;
#endif

			if(i < 6)
				_chip.noteOff(i, note + offset);
			else
				_snChip.noteOff(i - 6);

			_channels[i].note = -1;
			_channels[i].release = _releases;
			_owner->updateChannel(i, false);
			_channels[i].noteData = NULL;
			didRelease = true;
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

void Genny2612::update(float** buffer, int numSamples)
{
	_processor.update(buffer, numSamples);

	int size = _vibratoNotes.size();
	float tempo = _owner->getTempo();
	for(int i = 0; i < size; i++)
		_vibratoNotes[i].Calculate(tempo, numSamples);

#if BUILD_VST
	for(int i = 0; i < 10; i++)
	{
		if(_channels[i].note != -1 && _channels[i].instrumentPatch != nullptr)
		{
			int offset = (_channels[i].instrumentPatch->InstrumentDef.Octave - (GennyInstrumentParam_getRange(GIP_Octave) / 2)) * 1200;
			offset += (_channels[i].instrumentPatch->InstrumentDef.Transpose - (GennyInstrumentParam_getRange(GIP_Transpose) / 2)) * 100;
			offset += _channels[i].detune;


			int velocity = (int)(sqrt(sqrt(_channels[i].floatVelocity)) * 127.0f);

			if (velocity > 127)
				velocity = 127;
			if (velocity < 0)
				velocity = 0;

			float vibrato = 0.0f;
			for (int iVibrato = 0; iVibrato < _vibratoNotes.size(); iVibrato++)
			{
				if (_vibratoNotes[iVibrato].channel == _channels[i].channel)
					vibrato = sin(_vibratoNotes[iVibrato].vibratoPhase * (3.14159265f * 2)) * ((min(_vibratoNotes[iVibrato].velocity, 127) / 127.0f) * 1.0f);
			}



			if(i < 6)
				_chip.writeFrequencies(_channels[i].note + offset, velocity, i, vibrato, _frequencyTable, _channels[i].patch);
			else
				_snChip.writeFrequencies(_channels[i].note + offset, velocity, i - 6, _snChip.getEnvelope(i - 6), false, vibrato, _frequencyTable);
		}
	}
#endif
}

void Genny2612::setFromBaron(IBIndex* param, int channel, float val)
{
	_chip.setFromBaron(param, channel, val);

	//if(param->getType() == IB_YMParam)
	//{
	//	char cc = _indexBaron->getCCfromParam(param);
	//	if(cc != 0)
	//	{
	//		IBYMParam* parm = (IBYMParam*)param;

	//		//if(parm->getOperator() == 0)
	//		{
	//			int range = YM2612Param_getRange(parm->getParameter());

	//			char midiRange = _indexBaron->getCCRange(param);
	//			char midiVal = midiRange - ((val / (float)range) * midiRange);
	//			_owner->getBase()->sendMidiMessage(cc, midiVal, channel, 0 + 0);
	//		}
	//	}
	//}
}

void Genny2612::setFromBaronGlobal(IBIndex* param, int channel, float val)
{
	_chip.setFromBaronGlobal(param, channel, val);
}

void Genny2612::getParameterName(int index, char* text)
{
	IBIndex* idx = _indexBaron->getIndex(index);
	std::string name = idx->getName().c_str();
	strncpy(text, name.c_str(), name.length() + 1); 
}

void Genny2612::getParameterValue(int index, char* text)
{
	IBIndex* idx = _indexBaron->getIndex(index);
	std::string name = idx->getValue(static_cast<GennyPatch*>(_owner->getCurrentPatch())).c_str();
	strncpy(text, name.c_str(), name.length() + 1); 
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
}