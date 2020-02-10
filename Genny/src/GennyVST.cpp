#include "GennyVST.h"
#include "Genny2612.h"
#include "base/VSTBase.h"
#include "IndexBaron.h"
#include "GennyInterface.h"
#include "../resource.h"

#include "GennyLoaders.h"

#ifdef BUILD_VST
AudioEffect* createEffectInstance (audioMasterCallback audioMaster)
{
	GennyVST* p = new GennyVST();
	return (AudioEffect*)p->initializeBase(audioMaster);
}
#else
extern "C" TFruityPlug* _stdcall CreatePlugInstance(TFruityPlugHost *Host, int Tag)
{
	GennyVST* p = new GennyVST();

	FruityPlugInfo info;
	info.Host = Host;
	info.Tag = Tag;
	return (TFruityPlug*)p->initializeBase(&info);
};
#endif


GennyVST::GennyVST(void):
	_editor(NULL),
	_patchesInitialized(false),
	_first(true),
	_saving(false),
	_switchingPreset(false),
	megaMidiPort(0),
	megaMidiVSTMute(false),
	accurateEmulationMode(false)
{
	_core = new Genny2612(this);
	_core->initialize();
}


GennyVST::~GennyVST(void)
{
	for(int i = 0; i < _patches.size(); i++)
	{
		delete _patches[i];
	}
}

void GennyVST::initialize()
{
	GennyInterface* g = new GennyInterface(_base, this);	
	_editor = g;
	_base->setEditor(g);
}

//long versionIndicator = nullptr //Before February 17, 2019 - Version 0.5 (no version indicator)
long kVersionIndicator1 = 593829658389673; //February 17, 2019 - Version 1.0 //OBSOLETE! NO LONGER SUPPORTED!
long kVersionIndicator2 = 1127443247; //February 20, 2019 - Version 1.01
long kVersionIndicator3 = 1127443248; //June 21, 2019 - Version 1.02
long kVersionIndicator4 = 1127443249; //Sept 16, 2019 - Version 1.03 (Added VST automation)
long kVersionIndicator5 = 1127443250; //Nov 7, 2019 - Version 1.04 (Saved per operator velocity settings)
long kVersionIndicator6 = 1127443251; //Nov 7, 2019 - Version 1.05 (Saved True Stereo setting)
long kVersionIndicator7 = 1127443252; //Nov 12, 2019 - Version 1.06 (Combined 'Global' Parameters with normal ones)
long kVersionIndicator8 = 1127443253; //Nov 19, 2019 - Version 1.07 (Added MEGA MIDI flags)
long kVersionIndicator9 = 1127443254; //Nov 24, 2019 - Version 1.08 (Added EMULATION MODES)
long kVersionIndicator10 = 1127443255; //Nov 24, 2019 - Version 1.09 (Corrected number of drum samples from 55 to 56)
bool isVersionNumber(long kVersion)
{
	if (kVersion == kVersionIndicator1 ||
		kVersion == kVersionIndicator2 ||
		kVersion == kVersionIndicator3 ||
		kVersion == kVersionIndicator4 || 
		kVersion == kVersionIndicator5 ||
		kVersion == kVersionIndicator6 ||
		kVersion == kVersionIndicator7 ||
		kVersion == kVersionIndicator8 ||
		kVersion == kVersionIndicator9 ||
		kVersion == kVersionIndicator10)
		return true;

	return false;
}

bool checkVersionLessThan(long kVersion, long kIsLessThan)
{
	if(isVersionNumber(kVersion) == false)
		return true;

	if(kVersion < kIsLessThan)
		return true;


	return false;
}

bool checkVersionGreaterThanOrEqualTo(long kVersion, long kIsGreaterThanOrEqualTo)
{	
	if(isVersionNumber(kVersion) == false)
		return false;

	if(kVersion >= kIsGreaterThanOrEqualTo)
		return true;

	return false;
}

int GennyVST::getPluginState (void** data, bool isPreset)
{
	std::stringstream stream;

	_first = false;
	int numPatches = getTotalPatchCount();
	int numParams = GennyPatch::getNumParameters();
	int written = 0;
	_saving = true;
	

	stream.write((char*)(&written), sizeof(int));
	written += sizeof(int);
	
	stream.write((char*)(&kVersionIndicator10), sizeof(long));
	written += sizeof(long);

	int hasFrequencyTable = getFrequencyTable() != getDefaultFrequencyTable() ? 1 : 0;
	stream.write((char*)(&hasFrequencyTable), sizeof(int));
	written += sizeof(int);

	if(hasFrequencyTable > 0)
	{
		double* table = getFrequencyTable();
		for(int i = 0; i < 129; i++)
		{
			stream.write((char*)(&table[i]), sizeof(double));
			written += sizeof(double);
		}
	}
		
	stream.write((char*)(&numPatches), sizeof(int));
	written += sizeof(int);
	for(int i = 0; i < numPatches; i++)
	{
		setPatchIndex(i);

		int nameLength = _currentPatch->Name.length();
		stream.write((char*)(&nameLength), sizeof(int));
		written += sizeof(int);

		stream.write(const_cast<char*>(_currentPatch->Name.c_str()), nameLength + 1);
		written += nameLength + 1;

		//Write drum samples
		GennyPatch* gennypatch = (GennyPatch*)_currentPatch;
		
		char instype = (char)gennypatch->InstrumentDef.Type;
		stream.write(&instype, 1);
		written += 1;

		if (gennypatch->InstrumentDef.Type == GIType::DAC)
		{
			GennyData dat;
			GennyLoaders::saveGDAC(gennypatch, &dat);
			stream.write(dat.data, dat.dataPos);

			written += dat.dataPos;
			delete[] dat.data;
		}

		for(int j = 0; j < numParams; j++)
		{
			float param = getParameter(j);	
			stream.write((char*)(&param), sizeof(float));
			written += sizeof(float);
		}

		stream.write((char*)&gennypatch->InstrumentDef.OperatorVelocity[0], 1);
		written += 1;
		stream.write((char*)&gennypatch->InstrumentDef.OperatorVelocity[1], 1);
		written += 1;
		stream.write((char*)&gennypatch->InstrumentDef.OperatorVelocity[2], 1);
		written += 1;
		stream.write((char*)&gennypatch->InstrumentDef.OperatorVelocity[3], 1);
		written += 1;
	}

	int numMidiLearn = _midiLearn.size();
	stream.write((char*)(&numMidiLearn), sizeof(int));
	written += sizeof(int);

	for (std::map<int, std::vector<int>>::iterator it = _midiLearn.begin(); it != _midiLearn.end(); ++it)
	{
		stream.write((char*)(&it->first), sizeof(int));
		written += sizeof(int);

		int len = it->second.size();
		stream.write((char*)(&len), sizeof(int));
		written += sizeof(int);
		for (std::vector<int>::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++)
		{
			int val = *it2;
			stream.write((char*)(&val), sizeof(int));
			written += sizeof(int);
		}
	}

	stream.write((char*)&_core->getIndexBaron()->enableTrueStereo, 1);
	written += 1;

	stream.write((char*)&megaMidiPort, 1);
	written += 1;

	stream.write((char*)&megaMidiVSTMute, 1);
	written += 1;

	stream.write((char*)&accurateEmulationMode, 1);
	written += 1;

	stream.seekp(0);
	stream.write((char*)(&written), sizeof(int));

	_saving = false;

	GennyPatch* patch = static_cast<GennyPatch*>(getPatch(0));
	setPatchIndex(patch->Instruments[patch->SelectedInstrument]);

	*data = new char[written];

	stream.seekg(0);
	stream.read ((char*)(*data), written);
	return written;
}

int GennyVST::setPluginState (void* data, int size, bool isPreset)
{	
	std::stringstream stream(std::string((char*)data));

	int numPatches = getTotalPatchCount();
	int numParams = GennyPatch::getNumParameters();
	unsigned long written = 0;
	_saving = true;

	int readPos = 0;

	int readSize = (((unsigned char*)data)[readPos + 3] << 24) +(((unsigned char*)data)[readPos + 2] << 16) + (((unsigned char*)data)[readPos + 1] << 8) + ((unsigned char*)data)[readPos];
	readPos += sizeof(int);

	
	long versionNumber = 0;
	memcpy(&versionNumber, &((char*)data)[readPos], sizeof(versionNumber));
	if(isVersionNumber(versionNumber) == false)
		numPatches = 122;
	else
	{		
		readPos += sizeof(versionNumber);
	}

	int hasFrequencyTable = ((int)((unsigned char*)data)[readPos + 3] << (int)24) +((int)((unsigned char*)data)[readPos + 2] << (int)16) + ((int)((unsigned char*)data)[readPos + 1] << (int)8) + (int)((unsigned char*)data)[readPos];
	readPos += sizeof(int);
	if(hasFrequencyTable > 0)
	{
		double* table = new double[129];
		for(int i = 0; i < 129; i++)
		{
			double val = 0.0f;
			memcpy(&val, &((char*)data)[readPos], sizeof(val));
			readPos += sizeof(val);

			table[i] = val;
		}
		setFrequencyTable(table);
	}


	//1.0 writes number of patches into the save data
	if(checkVersionGreaterThanOrEqualTo(versionNumber, kVersionIndicator1))
	{
		numPatches = (((unsigned char*)data)[readPos + 3] << 24) +(((unsigned char*)data)[readPos + 2] << 16) + (((unsigned char*)data)[readPos + 1] << 8) + ((unsigned char*)data)[readPos];
		readPos += sizeof(int);
	}

	for(int i = 0; i < numPatches; i++)
	{
		if(i > getTotalPatchCount())
		{
			_patches.push_back(new GennyPatch());
		}

		setPatchIndex(i);

		int nameLength = (((unsigned char*)data)[readPos + 3] << 24) +(((unsigned char*)data)[readPos + 2] << 16) + (((unsigned char*)data)[readPos + 1] << 8) + ((unsigned char*)data)[readPos];
		readPos += sizeof(int);
 
		char* name = new char[nameLength + 1];
		memcpy(name, &((char*)data)[readPos], nameLength + 1);
		readPos += nameLength + 1;
		_currentPatch->Name = name;


		//Read drum samples
		GennyPatch* gennypatch = (GennyPatch*)_currentPatch;
		DrumSet* set = &gennypatch->InstrumentDef.Drumset;


		int numDrums = 56;
		if (checkVersionLessThan(versionNumber, kVersionIndicator10))
			numDrums = 55;


		if (checkVersionLessThan(versionNumber, kVersionIndicator10))
		{
			for (int i = 36; i < numDrums; i++)
			{
				char has = ((char*)data)[readPos];
				readPos++;
				if (has == 1)
				{
					int sampleRate = 0;
					memcpy(&sampleRate, &((char*)data)[readPos], sizeof(sampleRate));
					readPos += sizeof(sampleRate);

					int sampleSize = 0;
					memcpy(&sampleSize, &((char*)data)[readPos], sizeof(sampleSize));
					readPos += sizeof(sampleSize);

					char* sampleData = new char[sampleSize];
					memcpy(sampleData, &((char*)data)[readPos], sampleSize);
					readPos += sampleSize;

					//Resample
					if (sampleRate > 11025)
					{
						float sampInc = sampleRate / 11025.0f;
						if (sampInc < 1)
							sampInc = 1;

						int newSize = sampleSize / (sampInc);
						char* sampleData2 = new char[newSize];
						int idx = 0;
						for (int i = 0; i < sampleSize; i)
						{
							if (idx >= newSize)
								break;

							sampleData2[idx] = sampleData[i];
							i += (int)sampInc;
							idx++;
						}

						delete[] sampleData;
						sampleData = sampleData2;
						sampleSize = newSize;

						sampleRate = 11025;
					}




					WaveData* wave = new WaveData(sampleData, sampleSize);
					wave->sampleRate = sampleRate;

					WaveData* drum = set->getDrum(i);
					if (drum != nullptr)
						delete drum;

					set->mapDrum(i, wave);
				}
			}
		}






		
		if(checkVersionLessThan(versionNumber, kVersionIndicator2))
			GennyInstrument::loadingOldPanning = true;

		if (checkVersionGreaterThanOrEqualTo(versionNumber, kVersionIndicator3))
		{
			char insType = ((char*)data)[readPos];
			gennypatch->InstrumentDef.Type = (GIType::GIType)insType;
			readPos++;

			if (checkVersionGreaterThanOrEqualTo(versionNumber, kVersionIndicator10))
			{
				if (gennypatch->InstrumentDef.Type == GIType::DAC)
				{
					GennyData d;
					d.data = (char*)data;
					d.dataPos = readPos;
					GennyLoaders::loadGDAC(gennypatch, &d, false);

					readPos = d.dataPos;
				}
			}
		}


		bool oldLFO = checkVersionLessThan(versionNumber, kVersionIndicator7);

		for(int j = 0; j < numParams; j++)
		{
			float val = 0.0f;
			memcpy(&val, &((char*)data)[readPos], sizeof(val));
			readPos += sizeof(val);

			if (oldLFO)
			{
				if(j > 2)
					setParameter(j - 3, val);
			}
			else
				setParameter(j, val);
		}

		//Version 5 stores operator velocity settings
		if (checkVersionGreaterThanOrEqualTo(versionNumber, kVersionIndicator5))
		{
			memcpy(&gennypatch->InstrumentDef.OperatorVelocity[0], &((char*)data)[readPos], 1);
			readPos += 1;
			memcpy(&gennypatch->InstrumentDef.OperatorVelocity[1], &((char*)data)[readPos], 1);
			readPos += 1;
			memcpy(&gennypatch->InstrumentDef.OperatorVelocity[2], &((char*)data)[readPos], 1);
			readPos += 1;
			memcpy(&gennypatch->InstrumentDef.OperatorVelocity[3], &((char*)data)[readPos], 1);
			readPos += 1;
		}		


		GennyInstrument::loadingOldPanning = false;
		delete[] name;
	}

	//Version 4 stores VST automation info
	if (checkVersionGreaterThanOrEqualTo(versionNumber, kVersionIndicator4))
	{
		_midiLearn = std::map<int, std::vector<int>>();

		int numMidiLearn = (((unsigned char*)data)[readPos + 3] << 24) + (((unsigned char*)data)[readPos + 2] << 16) + (((unsigned char*)data)[readPos + 1] << 8) + ((unsigned char*)data)[readPos];
		readPos += sizeof(int);
		for (int i = 0; i < numMidiLearn; i++)
		{
			int first = (((unsigned char*)data)[readPos + 3] << 24) + (((unsigned char*)data)[readPos + 2] << 16) + (((unsigned char*)data)[readPos + 1] << 8) + ((unsigned char*)data)[readPos];
			readPos += sizeof(int);

			_midiLearn[first] = std::vector<int>();

			int numElements = (((unsigned char*)data)[readPos + 3] << 24) + (((unsigned char*)data)[readPos + 2] << 16) + (((unsigned char*)data)[readPos + 1] << 8) + ((unsigned char*)data)[readPos];
			readPos += sizeof(int);

			for (int j = 0; j < numElements; j++)
			{
				int element = (((unsigned char*)data)[readPos + 3] << 24) + (((unsigned char*)data)[readPos + 2] << 16) + (((unsigned char*)data)[readPos + 1] << 8) + ((unsigned char*)data)[readPos];
				readPos += sizeof(int);
				_midiLearn[first].push_back(element);
			}
		}
	}	

	if (checkVersionGreaterThanOrEqualTo(versionNumber, kVersionIndicator6))
	{
		memcpy(&_core->getIndexBaron()->enableTrueStereo, &((char*)data)[readPos], 1);
		readPos += 1;
	}

	if (checkVersionGreaterThanOrEqualTo(versionNumber, kVersionIndicator8))
	{
		memcpy(&megaMidiPort, &((char*)data)[readPos], 1);
		readPos += 1;
		memcpy(&megaMidiVSTMute, &((char*)data)[readPos], 1);
		readPos += 1; 
	}

	if (checkVersionGreaterThanOrEqualTo(versionNumber, kVersionIndicator9))
	{
		memcpy(&accurateEmulationMode, &((char*)data)[readPos], 1);
		readPos += 1;
	}

	_saving = false;


	GennyPatch* patch = static_cast<GennyPatch*>(getPatch(0));
	setPatchIndex(patch->Instruments[patch->SelectedInstrument]);


	if (_editor != NULL)
		_editor->reconnect();
	


	return readPos;
}

float GennyVST::getParameter(int index, VSTPatch* patch)
{
	if(index >= GennyPatch::getNumParameters())
	{
		return 0.0f;
	}
	if(patch == nullptr)
		return _currentPatch->getFromBaron(_core->getIndexBaron()->getIndex(index));
	else 
		return patch->getFromBaron(_core->getIndexBaron()->getIndex(index));
}

int GennyVST::getParameterRange(int index)
{
	IBIndex* ind = _core->getIndexBaron()->getIndex(index);
	if(ind->getType() == IB_YMParam)
	{
		IBYMParam* param = (IBYMParam*)ind;
		return YM2612Param_getRange(param->getParameter());
	}
	else if(ind->getType() == IB_InsParam)
	{
		IBInsParam* param = (IBInsParam*)ind;
		return GennyInstrumentParam_getRange(param->getParameter());
	}
	else 
		return 1;
}

void GennyVST::getParameterName(int index, char* text)
{
	return _core->getParameterName(index, text);
}

void GennyVST::getParameterValue(int index, char* text)
{
	return _core->getParameterValue(index, text);
}

void GennyVST::setParameter(int index, float value, VSTPatch* patch)
{
	if(index == kPresetControlIndex)
	{
		_switchingPreset = true;
		_base->setProgram(value);
		rejiggerInstruments(true);
		_switchingPreset = false;
		//setCurrentPatch(_patches[value]);
		return;
	}
	if(index == kInstrumentControlIndex)
	{
		_switchingPreset = true;
		GennyPatch* patch = static_cast<GennyPatch*>(getPatch(0));
		GennyPatch* reorderPatch = static_cast<GennyPatch*>(getPatch(1));


		int instrumentIndex = (int)value;
		if(reorderPatch->Instruments[instrumentIndex] >= 0)
			instrumentIndex = reorderPatch->Instruments[instrumentIndex];

		int ins = patch->Instruments[instrumentIndex];

		if(ins != -1)
		{
			patch->SelectedInstrument = (int)value;
			_base->setProgram(ins);
		}
		_switchingPreset = false;
		//setCurrentPatch(_patches[value]);
		return;
	}
	if(index >= kMidiChannelStart && index <= kMidiChannelEnd)
	{
		int sel = index - kMidiChannelStart;

		int idx = _core->getIndexBaron()->getInsParamIndex(GIP_MidiChannel);
		getPatch(sel)->setFromBaron(_core->getIndexBaron()->getIndex(idx), value);
		return;
	}
	if(index >= kOctaveStart && index <= kOctaveEnd)
	{
		int sel = index - kOctaveStart;

		int idx = _core->getIndexBaron()->getInsParamIndex(GIP_Octave);
		getPatch(sel)->setFromBaron(_core->getIndexBaron()->getIndex(idx), value);
		return;
	}
	if(index >= kTransposeStart && index <= kTransposeEnd)
	{
		int sel = index - kTransposeStart;

		int idx = _core->getIndexBaron()->getInsParamIndex(GIP_Transpose);
		getPatch(sel)->setFromBaron(_core->getIndexBaron()->getIndex(idx), value);
		return;
	}
	if(index >= kPanningStart && index <= kPanningEnd)
	{
		int sel = index - kPanningStart;

		int idx = _core->getIndexBaron()->getInsParamIndex(GIP_Panning);
		getPatch(sel)->setFromBaron(_core->getIndexBaron()->getIndex(idx), value);

		return;
	}
	if(index >= kRangeLowStart && index <= kRangeLowEnd)
	{
		int sel = index - kRangeLowStart;

		int idx = _core->getIndexBaron()->getInsParamIndex(GIP_RangeLow);
		getPatch(sel)->setFromBaron(_core->getIndexBaron()->getIndex(idx), value);

		return;
	}
	if(index >= kRangeHighStart && index <= kRangeHighEnd)
	{
		int sel = index - kRangeHighStart;

		int idx = _core->getIndexBaron()->getInsParamIndex(GIP_RangeHigh);
		getPatch(sel)->setFromBaron(_core->getIndexBaron()->getIndex(idx), value);

		return;
	}
	if(index >= kDelayStart && index <= kDelayEnd)
	{
		int sel = index - kDelayStart;

		int idx = _core->getIndexBaron()->getInsParamIndex(GIP_Delay);
		getPatch(sel)->setFromBaron(_core->getIndexBaron()->getIndex(idx), value);

		return;
	}
	if(index >= kGlobalChannelStart && index <= kGlobalChannelEnd)
	{
		int sel = index - kGlobalChannelStart;

		int idx = _core->getIndexBaron()->getPatchParamIndex((GennyPatchParam)(GPP_Channel0 + sel));
		getPatch(0)->setFromBaron(_core->getIndexBaron()->getIndex(idx), value);
		return;
	}
	if(index >= kChannelEnableStart && index <= kChannelEnableEnd)
	{
		int sel = index - kChannelEnableStart;

		int idx = _core->getIndexBaron()->getPatchParamIndex(GPP_SelectedInstrument);
		int selection = (int)getPatch(0)->getFromBaron(_core->getIndexBaron()->getIndex(idx));

		idx = _core->getIndexBaron()->getInsParamIndex((GennyInstrumentParam)(GIP_Channel0 + sel));
		getPatch(selection)->setFromBaron(_core->getIndexBaron()->getIndex(idx), value);
		return;
	}
	if(index >= kInstrumentMappingStart && index <= kInstrumentMappingEnd)
	{
		int sel = index - kInstrumentMappingStart;

		GennyPatch* firstPatch = static_cast<GennyPatch*>(_patches[0]);
		int lastInstrument = firstPatch->Instruments[sel];

		int idx = _core->getIndexBaron()->getPatchParamIndex((GennyPatchParam)(GPP_Ins01 + sel));
		_patches[0]->setFromBaron(_core->getIndexBaron()->getIndex(idx), value);

		if(_switchingPreset)
		{	
			//if(lastInstrument != (int)value && lastInstrument != -1)
			//{
			//	GennyPatch* patch = static_cast<GennyPatch*>(getPatch(value));
			//	GennyPatch* lastPatch = static_cast<GennyPatch*>(getPatch(lastInstrument));

			//	if(patch->InstrumentDef.DAC)
			//	{
			//		GennyPatch* idxPatch = static_cast<GennyPatch*>(getPatch(sel));
			//		for(int i = 0; i < 10; i++)
			//		{
			//			idxPatch->InstrumentDef.Channels[i] = i == 5 ? true : false;
			//		}
			//	}
			//	else if(patch->InstrumentDef.FM != lastPatch->InstrumentDef.FM || lastPatch->InstrumentDef.DAC == true)
			//	{
			//		GennyPatch* idxPatch = static_cast<GennyPatch*>(getPatch(sel));
			//		if(patch->InstrumentDef.FM)
			//		{
			//			for(int i = 0; i < 10; i++)
			//				idxPatch->InstrumentDef.Channels[i] = i < 6 ? true : false;
			//		}
			//		else
			//		{
			//			for(int i = 0; i < 10; i++)
			//				idxPatch->InstrumentDef.Channels[i] = i > 5 && i < 9 ? true : false;
			//		}
			//	}
			//}
		}

		return;
	}
	IBIndex* idx = _core->getIndexBaron()->getIndex(index);
	if(idx->getType() == IB_PatchParam)
	{
		IBPatchParam* param = (IBPatchParam*)idx;
		if(value >= 0 && _currentPatch == _patches[0])
			_patches[0]->setFromBaron(idx, value);
		else if(param != nullptr && value >= 0 && (param->getParameter() >= GennyPatchParam::GPP_Ins01 && param->getParameter() <= GennyPatchParam::GPP_Ins16) && _saving)
		{
			if(patch != nullptr)
				patch->setFromBaron(idx, value);
			else
				_currentPatch->setFromBaron(idx, value);
		}
	}
	else
	{
		//bool wasGlobal = false;
		//if(idx->getType() == IB_YMParam)
		//{
		//	IBYMParam* param = (IBYMParam*)idx; 
		//	if(param->getParameter() == YM_LFO ||
		//	  param->getParameter() == YM_LFO_EN ||
		//	  param->getParameter() == YM_SPECIAL)
		//	{
		//		_patches[0]->setFromBaron(idx, value);
		//		_core->setFromBaronGlobal(idx, 0, value);
		//		wasGlobal = true;
		//	}
		//}

		//if(wasGlobal == false)
		{
			if(patch != nullptr)
			{
				if (index == 151 || index == 152)
					_core->lfoChanged();

				patch->setFromBaron(idx, value);
				for(int i = 0; i < 6; i++)
				{
					if(_core->getChannelPatch(i) == patch)
						_core->setFromBaron(idx, i, value);
				}
			}
			else
			{
				_currentPatch->setFromBaron(idx, value);
				for(int i = 0; i < 6; i++)
				{
					if(_core->getChannelPatch(i) == _currentPatch)
						_core->setFromBaron(idx, i, value);
				}
			}
		}
	}
}

void GennyVST::setProgramName(char* name)
{
	VirtualInstrument::setProgramName(name);
	if(_editor != nullptr)
		_editor->reconnect();
}

void GennyVST::updateChannel(int channel, bool on)
{
	if(_editor)
		_editor->setChannelState(channel, on);
}

void GennyVST::setCurrentPatch(VSTPatch* patch)
{
	_currentPatch = patch;

	IndexBaron* baron = _core->getIndexBaron();
	int count = GennyPatch::getNumParameters();
	for(int i = 0; i < count; i++)
	{
		_currentPatch->setFromBaron(baron->getIndex(i), _currentPatch->getFromBaron(baron->getIndex(i)));
	}

	if(_saving == false)
	{
		IBIndex* selectedInstrumentIndex = baron->getIndex(baron->getPatchParamIndex(GPP_SelectedInstrument));
		int selected = getPatch(0)->getFromBaron(selectedInstrumentIndex);

		if(((GennyPatch*)getPatch(1))->Instruments[selected] >= 0)
			selected = ((GennyPatch*)getPatch(1))->Instruments[selected];

		setParameter(kInstrumentMappingStart + selected, getPatchIndex(patch));

		if(_editor != NULL)
			_editor->reconnect();
	}
}

void GennyVST::getSamples(float** buffer, int numSamples)
{
	if (_playingStatusChanged)
	{
		_core->clearCache();
		_playingStatusChanged = false;
	}

	_core->update(buffer, numSamples);
}

void GennyVST::noteOn(int note, float velocity, unsigned char channel, float panning, void* noteData)
{
	_core->noteOn(note, velocity, channel, panning, noteData);
}

void GennyVST::updateNote(void* noteData)
{
	_core->updateNote(noteData);
}

void GennyVST::noteOff(int note, unsigned char channel, void* noteData)
{
	_core->noteOff(note, channel, noteData);
}

void GennyVST::clearNotes() 
{

}

void GennyVST::rejiggerInstruments(bool selected)
{
	GennyPatch* patch0 = (GennyPatch*)_patches[0];
	GennyPatch* patch1 = (GennyPatch*)_patches[1];
	for(int i = 0; i < 16; i++)
	{
		if(!selected || patch0->SelectedInstrument == i)
		{
			GennyPatch* instrumentPatch = (GennyPatch*)_patches[i];
		
			int actualPatch = patch0->Instruments[i];
			if(patch1->Instruments[i] >= 0)
				actualPatch = patch1->Instruments[i];

			if(actualPatch < _patches.size())
			{
				GennyPatch* patch = (GennyPatch*)_patches[actualPatch];

				if(patch->InstrumentDef.Type != instrumentPatch->InstrumentMode)
				{
					if(patch->InstrumentDef.Type == GIType::DAC)
					{	
						instrumentPatch->InstrumentDef.Channels[0] = false;
						instrumentPatch->InstrumentDef.Channels[1] = false;
						instrumentPatch->InstrumentDef.Channels[2] = false;
						instrumentPatch->InstrumentDef.Channels[3] = false;
						instrumentPatch->InstrumentDef.Channels[4] = false;
						instrumentPatch->InstrumentDef.Channels[5] = true;
						instrumentPatch->InstrumentDef.Channels[6] = false;
						instrumentPatch->InstrumentDef.Channels[7] = false;
						instrumentPatch->InstrumentDef.Channels[8] = false;
						instrumentPatch->InstrumentDef.Channels[9] = false;
					}
					else if(patch->InstrumentDef.Type == GIType::FM)
					{	
						instrumentPatch->InstrumentDef.Channels[0] = true;
						instrumentPatch->InstrumentDef.Channels[1] = true;
						instrumentPatch->InstrumentDef.Channels[2] = true;
						instrumentPatch->InstrumentDef.Channels[3] = true;
						instrumentPatch->InstrumentDef.Channels[4] = true;
						instrumentPatch->InstrumentDef.Channels[5] = true;
						instrumentPatch->InstrumentDef.Channels[6] = false;
						instrumentPatch->InstrumentDef.Channels[7] = false;
						instrumentPatch->InstrumentDef.Channels[8] = false;
						instrumentPatch->InstrumentDef.Channels[9] = false;
					}
					else
					{
						if(patch->getFromBaron(_core->getIndexBaron()->getIndex(_core->getIndexBaron()->getYMParamIndex(SN_PERIODIC))) > 0.1f)
						{
							instrumentPatch->InstrumentDef.Channels[0] = false;
							instrumentPatch->InstrumentDef.Channels[1] = false;
							instrumentPatch->InstrumentDef.Channels[2] = false;
							instrumentPatch->InstrumentDef.Channels[3] = false;
							instrumentPatch->InstrumentDef.Channels[4] = false;
							instrumentPatch->InstrumentDef.Channels[5] = false;
							instrumentPatch->InstrumentDef.Channels[6] = false;
							instrumentPatch->InstrumentDef.Channels[7] = false;
							instrumentPatch->InstrumentDef.Channels[8] = false;
							instrumentPatch->InstrumentDef.Channels[9] = true;
						}
						else
						{	
							instrumentPatch->InstrumentDef.Channels[0] = false;
							instrumentPatch->InstrumentDef.Channels[1] = false;
							instrumentPatch->InstrumentDef.Channels[2] = false;
							instrumentPatch->InstrumentDef.Channels[3] = false;
							instrumentPatch->InstrumentDef.Channels[4] = false;
							instrumentPatch->InstrumentDef.Channels[5] = false;
							instrumentPatch->InstrumentDef.Channels[6] = true;
							instrumentPatch->InstrumentDef.Channels[7] = true;
							instrumentPatch->InstrumentDef.Channels[8] = true;
							instrumentPatch->InstrumentDef.Channels[9] = false;
						}
					}
					instrumentPatch->InstrumentMode = patch->InstrumentDef.Type;
				}

			}
		}
	}
}

int GennyVST::getTotalPatchCount()
{
	if(_patchesInitialized == false)
	{
		GennyData file = GennyLoaders::loadResource(SET_NEWPRESETS, L"SET");

		int numPatches = file.readInt();
		for(int i = 0; i < numPatches; i++)
		{
			if(i >= _patches.size())
				_patches.push_back(new GennyPatch());

			int size = file.readInt();

			GennyData dat;
			dat.data = new char[size];
			memcpy(dat.data, file.data + file.dataPos, size);
			dat.size = size;
			file.dataPos += size;
	
			GennyPatch* newPatch = GennyLoaders::loadGEN("", "", (GennyPatch*)_patches[i], &dat); 


			if(newPatch->InstrumentDef.Type == GIType::DAC)
				newPatch->setFromBaron(_core->getIndexBaron()->getIndex(_core->getIndexBaron()->getYMParamIndex((YM2612Param)YM_TL, 3)), YM2612Param_getRange(YM_TL) - 27);
		}

		_patchesInitialized = true;
		_currentPatch = _patches[0];
		rejiggerInstruments(true);
	}


	return _patches.size();
}
   

int GennyVST::getNumInstruments()
{
	int num = 0;
	GennyPatch* patch = static_cast<GennyPatch*>(getPatch(0));
	for(int i = 0; i < 16; i++)
	{
		if(patch->Instruments[i] != -1)
			num++;
	}
	return num;
}

void GennyVST::setMasterVolume(float volume)
{
	_core->setMasterVolume(volume);
}

void GennyVST::saveData(IStream* stream, bool save)
{
	unsigned long written = 0;
	if(save)
	{
		void* data = nullptr;
		int length = getPluginState(&data, false);
		stream->Write(data, length, &written);
	}
	else
	{
		int streamSize = 0;
		stream->Read(&streamSize, sizeof(int), &written);
		LARGE_INTEGER l;
		l.HighPart = 0;
		l.LowPart = 0;
		l.QuadPart = 0;
		stream->Seek(l, 0, nullptr);
		char* data = new char[streamSize];
		stream->Read(data, streamSize, &written);

		setPluginState(data, streamSize, false);
	}

/*
	_first = false;
	int numPatches = getTotalPatchCount();
	int numParams = GennyPatch::getNumParameters();
	_saving = true;
	if(save)
	{
		for(int i = 0; i < numPatches; i++)
		{
			setPatchIndex(i);

			int nameLength = _currentPatch->Name.length();
			stream->Write(&nameLength, 4, &written);
			stream->Write(const_cast<char*>(_currentPatch->Name.c_str()), nameLength + 1, &written);
			for(int j = 0; j < numParams; j++)
			{
				float param = getParameter(j);	
				stream->Write((void*)&param, sizeof(float), &written);
			}
		}
	}
	else
	{
		for(int i = 0; i < numPatches; i++)
		{
			setPatchIndex(i);

			int nameLength;
			stream->Read(&nameLength, 4, &written);
			char* name = new char[nameLength + 1];
			stream->Read(name, nameLength + 1, &written);
			_currentPatch->Name = name;
			for(int j = 0; j < numParams; j++)
			{
				float val = 0;
				stream->Read(&val, sizeof(float), &written);
				setParameter(j, val);
			}
			delete[] name;
		}
	}
	_saving = false;

	GennyPatch* patch = static_cast<GennyPatch*>(getPatch(0));
	setPatchIndex(patch->Instruments[patch->SelectedInstrument]);*/
}

void GennyVST::startLogging(std::string file)
{
	_core->startLogging(file);
}

void GennyVST::stopLogging()
{
	_core->stopLogging();
}

void GennyVST::loggingComplete()
{
	if(_editor != nullptr)
	{
		_editor->setLogging(false);
		_editor->reconnect();
	}
}

void GennyVST::onMidiMessage(int channel, int message, int value)
{

#if BUILD_VST
	int learnParameter = _base->_midiLearnParameter;
	int forgetParameter = _base->_midiForgetParameter;
	_base->_midiForgetParameter = -1;
	_base->_midiLearnParameter = -1;

	std::map<int, std::vector<int>>::iterator existing = _midiLearn.end();
	//Erase existing parameter maps
	for (std::map<int, std::vector<int>>::iterator it = _midiLearn.begin(); it != _midiLearn.end(); ++it)
	{
		if (it->first == message)
			existing = it;


		if (learnParameter >= 0)
		{
			for (std::vector<int>::iterator it2 = it->second.begin(); it2 != it->second.end();)
			{
				if (*it2 == learnParameter)
					it->second.erase(++it2);
				else
					++it2;
			}
		}

		if (forgetParameter >= 0)
		{
			for (std::vector<int>::iterator it2 = it->second.begin(); it2 != it->second.end();)
			{
				if (*it2 == forgetParameter)
					it2 = it->second.erase(it2);
				else
					++it2;
			}
		}
	}
	if (forgetParameter > 0)
		return;

	if (learnParameter >= 0)
	{
		//Push in the new learned parameter
		if (existing == _midiLearn.end())
			existing = _midiLearn.insert(std::pair<int, std::vector<int>>(message, std::vector<int>())).first;
		existing->second.push_back(learnParameter);
	}

	//Apply midi values
	if (existing != _midiLearn.end())
	{
		for (std::vector<int>::iterator it2 = existing->second.begin(); it2 != existing->second.end(); it2++)
		{
			int parameter = *it2;
			int patchNum = (int)(parameter / GennyPatch::getNumParameters());

			int paramNumber = parameter - (patchNum * GennyPatch::getNumParameters());
			IBIndex* idx = _core->getIndexBaron()->getIndex(paramNumber);
			if (idx->getType() == IB_YMParam)
			{
				IBYMParam* parm = (IBYMParam*)idx;
				int range = YM2612Param_getRange(parm->getParameter());
				int setValue = min(((value / 127.0f) * range) + 0.5f, range);

				setParameter(paramNumber, setValue, getPatch(patchNum));


				if (_currentPatch == getPatch(patchNum) && _editor != nullptr)
					_editor->setParameter(paramNumber, setValue);
			}
		}
	}
#endif

/*

	IBIndex* index = _core->getIndexBaron()->getIndexFromMidi(message);
	if(index != nullptr)
	{
		if(index->getType() == IB_YMParam)
		{
			IBYMParam* parm = (IBYMParam*)index;
			int range = YM2612Param_getRange(parm->getParameter());
			value = min(((value / 127.0f) * range) + 0.5f, range);

			int parmIndex = _core->getIndexBaron()->getIBIndex(parm);
			for(int i = 0; i < 16; i++)
			{
				GennyPatch* p = (GennyPatch*)_patches[i];
				if(p->InstrumentDef.MidiChannel == channel)
				{
					setParameter(parmIndex, value, p); 
					if(_currentPatch == p && _editor != nullptr)
						_editor->setParameter(parmIndex, value);

				}
			}
		}
	}*/
}

void GennyVST::midiTick()
{
	_core->midiTick();
}

void GennyVST::setFrequencyTable(double* table) 
{ 
	_core->setFrequencyTable(table); 
}

double* GennyVST::getFrequencyTable() 
{ 
	return _core->getFrequencyTable(); 
}

double* GennyVST::getDefaultFrequencyTable() 
{ 
	return _core->getDefaultFrequencyTable(); 
}