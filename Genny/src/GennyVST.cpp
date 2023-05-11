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


GennyVST::GennyVST(void) :
	_editor(NULL),
	_patchesInitialized(false),
	_first(true),
	_saving(false),
	_switchingPreset(false),
	megaMidiPort(0),
	megaMidiVSTMute(false),
	genMDMPort(7),
	accurateEmulationMode(true),
	triggerWave(nullptr),
	bendRange(12),
	_automationInverse(false),
	lfo(0),
	_hasUIUpdates(false),
	_clearMidiUIUpdateHistory(false),
#if BUILD_VST
	_setParameterNormalizedValue(true)
#else
	_setParameterNormalizedValue(false)
#endif
{
	_midiLearnParameter = -1;
	_core = new Genny2612(this);
}


GennyVST::~GennyVST(void)
{
	destroy();
}

void GennyVST::destroy()
{
	for (int i = 0; i < _patches.size(); i++)
	{
		delete _patches[i];
	}
	_patches.clear();

	if (_editor != nullptr)
	{
		delete _editor;
		_editor = nullptr;
	}

	if (_core != nullptr)
	{
		delete _core;
		_core = nullptr;
	}
}

void GennyVST::initialize()
{
	_core->initialize();

	GennyInterface* g = new GennyInterface(_base, this);	
	_editor = g;
	_base->setEditor(g);

	triggerWave = nullptr;
}

void GennyVST::sampleRateChanged(double newRate)
{
	_core->setSampleRate(newRate);
}

bool isVersionNumber(long kVersion)
{
	if (kVersion == kVersionIndicator1 ||
		(kVersion >= kVersionIndicator2 && kVersion <= kLatestVersion))
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

bool checkVersionGreaterThan(long kVersion, long kIsGreaterThan)
{
	if (kVersion == kVersionIndicator1)
		return false;

	if (kVersion > kIsGreaterThan)
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

bool checkVersionEqualTo(long kVersion, long kIsEqualTo)
{
	if (isVersionNumber(kVersion) == false)
		return false;

	if (kVersion == kIsEqualTo)
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
	
	stream.write((char*)(&kLatestVersion), sizeof(long));
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
			float param = 0.0f;
			IBIndex* ib = _core->getIndexBaron()->getIndex(j);
			if (ib != nullptr)
				param = _currentPatch->getFromBaron(ib) / ib->maxValue;
			else
				param = 0.0f;

			stream.write((char*)(&param), sizeof(float));
			written += sizeof(float);
		}

		unsigned short numExtParameters = gennypatch->InstrumentDef._extendedParamMap.size();
		stream.write((char*)&numExtParameters, 2);
		written += 2;
		for (auto it = gennypatch->InstrumentDef._extendedParamMap.begin(); it != gennypatch->InstrumentDef._extendedParamMap.end(); it++)
		{
			unsigned short extParam = (unsigned short)(*it).first;
			float extValue = (*it).second->get();	
			
			stream.write((char*)&extParam, 2);
			written += 2;

			stream.write((char*)&extValue, 4);
			written += 4;
		}
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

	stream.write((char*)&bendRange, 1);
	written += 1;

	stream.write((char*)&_automationInverse, 1);
	written += 1;

	stream.write((char*)&lfo, 1);
	written += 1;

	unsigned char numNoteControls = _noteControlParams.size();
	stream.write((char*)&numNoteControls, 1); //1 byte should be sufficient since there should only ever be 2 keys
	written += 1;
	for (auto it = _noteControlParams.begin(); it != _noteControlParams.end(); it++)
	{
		unsigned short numNoteControlParams = (unsigned short)(*it).second.size();
		stream.write((char*)&numNoteControlParams, 2);
		written += 2;
		for (int i = 0; i < (*it).second.size(); i++)
		{
			unsigned short parm = (*it).second[i];
			stream.write((char*)&parm, 2);
			written += 2;
		}
	}

	stream.write((char*)&kMaxInstruments, 1);
	written += 1;
	for (int i = 0; i < kMaxInstruments; i++)
	{
		GennyPatch* ins = static_cast<GennyPatch*>(getPatch(i));

		unsigned short panLength = ins->InstrumentDef.PingPongString.length();
		stream.write((char*)&panLength, 2);
		written += 2;

		stream.write(const_cast<char*>(ins->InstrumentDef.PingPongString.c_str()), panLength + 1);
		written += panLength + 1;
	}

	//Seek back to start and write filesize now that we know what it is
	stream.seekp(0);
	stream.write((char*)(&written), sizeof(int));


	GennyPatch* patch = static_cast<GennyPatch*>(getPatch(0));
	setPatchIndex(patch->Instruments[patch->SelectedInstrument]);
	_saving = false;

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

	if (checkVersionGreaterThan(versionNumber, kLatestVersion))
	{
		int msgboxID = MessageBoxA(
			NULL,
			"The file you are opening was saved with a newer version of GENNY. This plugin is backwards compatible, but not forwards compatible- so to load this project correctly you must install the latest version.",
			"Old Version",
			MB_ICONEXCLAMATION
		);

		_saving = false;
		return 0;
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
		if(i >= getTotalPatchCount())
		{
			_patches.push_back(new GennyPatch(i));
		}
		setPatchIndex(i);

		int nameLength = 0;

		memcpy(&nameLength, &((char*)data)[readPos], sizeof(int));
		readPos += sizeof(int);

		char* name = new char[nameLength + 1];
		memcpy(name, &((char*)data)[readPos], nameLength + 1);
		readPos += nameLength + 1;
		_currentPatch->Name = name;
		delete[] name;


		//Read drum samples
		GennyPatch* gennypatch = (GennyPatch*)_currentPatch;
		DrumSet* set = &gennypatch->InstrumentDef.Drumset;
		
		for (int i = 0; i < kMaxInstruments; i++)
		{
			gennypatch->Instruments[i] = -1;
		}

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
					GennyLoaders::loadGDAC(gennypatch, &d);

					readPos = d.dataPos;
				}
			}
		}


		bool oldLFO = checkVersionLessThan(versionNumber, kVersionIndicator7);
		bool oldParameterRanges = checkVersionLessThan(versionNumber, kVersionIndicator11);

#if !BUILD_VST
		_setParameterNormalizedValue = true;
#endif
		for(int j = 0; j < numParams; j++)
		{
			float val = 0.0f;
			memcpy(&val, &((char*)data)[readPos], sizeof(val));
			readPos += sizeof(val);

			if (oldParameterRanges)
			{
				IBIndex* ib = _core->getIndexBaron()->getIndex(j);
				if (ib != nullptr)
					val = val / ib->maxValue;
			}

			if (checkVersionLessThan(versionNumber, kVersionIndicator15))
			{
				IBIndex* ib = _core->getIndexBaron()->getIndex(j);
				if (ib->getType() == IBType::IB_YMParam)
				{
					if (YM2612Param_getIsReverseParam(((IBYMParam*)ib)->getParameter()))
						val = 1.0f - val; //Time to get rid of inverse parameters, this should be handled chip side and not be the mess that it is!
				}
			}

			if (oldLFO)
			{
				if(j > 2)
					setParameter(j - 3, val);
			}
			else
				setParameter(j, val);
		}
#if !BUILD_VST
		_setParameterNormalizedValue = false;
#endif

		if (checkVersionLessThan(versionNumber, kVersionIndicator15))
		{
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

			//Version 5 stores operator velocity settings
			if (checkVersionGreaterThan(versionNumber, kVersionIndicator11))
			{
				memcpy(&gennypatch->InstrumentDef.snMelodicEnable, &((char*)data)[readPos], 1);
				readPos += 1;
			}

			if (checkVersionGreaterThan(versionNumber, kVersionIndicator13))
			{
				memcpy(&gennypatch->InstrumentDef.soloMode, &((char*)data)[readPos], 1);
				readPos += 1;
				memcpy(&gennypatch->InstrumentDef.legatoMode, &((char*)data)[readPos], 1);
				readPos += 1;
				memcpy(&gennypatch->InstrumentDef.glide, &((char*)data)[readPos], 1);
				readPos += 1;
			}
		}

		if (checkVersionGreaterThan(versionNumber, kVersionIndicator14))
		{
			unsigned short numExt = 0;
			memcpy(&numExt, &((char*)data)[readPos], 2);
			readPos += 2;

			for (int i = 0; i < numExt; i++)
			{
				unsigned short extParam = 0;
				memcpy(&extParam, &((char*)data)[readPos], 2);
				readPos += 2;

				float extValue = 0.0f;
				memcpy(&extValue, &((char*)data)[readPos], 4);
				readPos += 4;

				GennyExtParam* p = gennypatch->InstrumentDef.getExt((GEParam)extParam);
				if (p != nullptr)
					p->set(extValue);
			}
		}
		else
		{
			gennypatch->InstrumentDef.LFOEnable = false;
		}

		if (gennypatch->InstrumentDef.DACSampleRate >= 0 && gennypatch->InstrumentDef.DACSampleRate < kNumDACSamplerates)
			gennypatch->InstrumentDef.Drumset.setSampleRate(kDACSamplerates[gennypatch->InstrumentDef.DACSampleRate]);

		GennyInstrument::loadingOldPanning = false;
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

#if !BUILD_VST
				if (element > 999999)
				{
					element -= 999999;
					int instrument = ((int)(element / 9999));
					int parameter = (int)(element % 9999);

					element = (instrument * _base->_totalParameters) + parameter;
				}
#endif

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

	if (checkVersionGreaterThan(versionNumber, kVersionIndicator12))
	{
		memcpy(&bendRange, &((char*)data)[readPos], 1);
		readPos += 1;
	}

	if (checkVersionGreaterThan(versionNumber, kVersionIndicator15))
	{
		memcpy(&_automationInverse, &((char*)data)[readPos], 1);
		readPos += 1;
	}
	else
		_automationInverse = true;


	if (checkVersionLessThan(versionNumber, kVersionIndicator17))
		lfo = (unsigned char)(getPatch(0)->getFromBaron(_core->getIndexBaron()->getIndex(_core->getIndexBaron()->getYMParamIndex(YM2612Param::YM_LFO))) + 0.5f);
	else
	{
		memcpy(&lfo, &((char*)data)[readPos], 1);
		readPos += 1;
	}
	_core->_chip.LFOChanged();



	_noteControlParams.clear();
	if (checkVersionGreaterThanOrEqualTo(versionNumber, kVersionIndicator18))
	{
		unsigned char numNoteControls = 0;
		memcpy(&numNoteControls, &((char*)data)[readPos], 1);
		readPos += 1;

		for (int i = 0; i < numNoteControls; i++)
		{
			_noteControlParams[i] = std::vector<int>();

			unsigned short numNoteControlParams = 0;
			memcpy(&numNoteControlParams, &((char*)data)[readPos], 2);
			readPos += 2;
			for (int i2 = 0; i2 < numNoteControlParams; i2++)
			{
				unsigned short noteParam = 0;
				memcpy(&noteParam, &((char*)data)[readPos], 2);
				readPos += 2;

				_noteControlParams[i].push_back(noteParam);
			}
		}
	}


	if (checkVersionGreaterThanOrEqualTo(versionNumber, kVersionIndicator19))
	{
		unsigned char numInstruments = 0;
		memcpy(&numInstruments, &((char*)data)[readPos], 1);
		readPos += 1;
		for (int i = 0; i < numInstruments; i++)
		{
			GennyPatch* ins = static_cast<GennyPatch*>(getPatch(i));

			unsigned short panLength = 0;
			memcpy(&panLength, &((char*)data)[readPos], 2);
			readPos += 2;

			char* panString = new char[panLength + 1];
			memcpy(panString, &((char*)data)[readPos], panLength + 1);
			readPos += panLength + 1;
			ins->InstrumentDef.parsePanString(panString);
			delete[] panString;
		}
	}


	if (checkVersionLessThan(versionNumber, kVersionIndicator19))
	{
		GennyPatch* enabledPatch = static_cast<GennyPatch*>(getPatch(2));
		for (int i = 0; i < kMaxInstruments; i++)
		{
			if (enabledPatch->Instruments[i] < 0)
				((GennyPatch*)_patches[i])->InstrumentDef.Enable = true;
			else
				((GennyPatch*)_patches[i])->InstrumentDef.Enable = false;
		}
	}

	GennyPatch* oldOrderingPatch = static_cast<GennyPatch*>(getPatch(1));
	GennyPatch* orderingPatch = static_cast<GennyPatch*>(getPatch(3));
	GennyPatch* mainPatch = static_cast<GennyPatch*>(getPatch(0));
	if (orderingPatch->Instruments[0] < 0)
	{
		for (int i = 0; i < kMaxInstruments; i++)
		{
			//Initial setup
			if (oldOrderingPatch->Instruments[i] >= 0)
				orderingPatch->Instruments[i] = oldOrderingPatch->Instruments[i];
			else
				orderingPatch->Instruments[i] = mainPatch->Instruments[i] >= 0 ? i : -1;
		}
	}

	rejiggerInstruments(false);

	_saving = false;
	setPatchIndex(mainPatch->Instruments[mainPatch->SelectedInstrument]);

	if (_editor != NULL)
		_editor->reconnect();

	return readPos;
}

float GennyVST::getParameter(int index, VSTPatch* patch)
{
	int numparms = GennyPatch::getNumParameters();
	for (int i = 0; i < numparms; i++)
	{
		IBIndex* ibb = _core->getIndexBaron()->getIndex(i);
		int qq = 1;
	}

	if (GennyExtParam::isExtParam(index))
	{
		GennyExtParam* param = getExtParam(index);
		return param->get();
	}

	if(index > kExtParamsEnd)
	{
		return 0.0f;
	}


	IBIndex* ib = _core->getIndexBaron()->getIndex(index);
	float ret = 0.0f;
	if (ib != nullptr)
	{
		if(ib->getType() == IBType::IB_YMParam && ((IBYMParam*)ib)->getParameter() == YM2612Param::YM_LFO)
			ret = lfo;
		else if (patch == nullptr)
			ret = _currentPatch->getFromBaron(ib);
		else
			ret = patch->getFromBaron(ib);
	}

	return ret;
}

int GennyVST::getParameterRange(int index)
{
	if (GennyExtParam::isExtParam(index))
	{
		GennyExtParam* param = getExtParam(index);
		return param->rangeMax;
	}

	if (index > GennyPatch::getNumParameters())
		index = index % GennyPatch::getNumParameters();

	IBIndex* ind = _core->getIndexBaron()->getIndex(index);
	if (ind != nullptr)
	{
		if (ind->getType() == IB_YMParam)
		{
			IBYMParam* param = (IBYMParam*)ind;
			return YM2612Param_getRange(param->getParameter());
		}
		else if (ind->getType() == IB_InsParam)
		{
			IBInsParam* param = (IBInsParam*)ind;
			return GennyInstrumentParam_getRange(param->getParameter());
		}
	}

	return 1;
}

void GennyVST::getParameterName(int index, char* text)
{
	std::string name = "";
	if (GennyExtParam::isExtParam(index))
	{
		GennyExtParam* p = getExtParam(index);
		if (p != nullptr)
			name = p->name;
		else
			return;
	}
	else
	{
		if (index > GennyPatch::getNumParameters())
			index = index % GennyPatch::getNumParameters();

		IBIndex* idx = _core->getIndexBaron()->getIndex(index);
		if (idx == nullptr)
			return;

		name = idx->getName().c_str();
		if (idx->getType() == IBType::IB_YMParam)
		{
			IBYMParam* param = (IBYMParam*)idx;
			if (param->getOperator() >= 0)
			{
				int op = param->getOperator();
				if (op == 1)
					op = 2;
				else if (op == 2)
					op = 1;

				name += " (OP " + std::to_string(op + 1) + ")";
			}
		}
	}

	strcpy(text, name.c_str());
}


ParamDisplayType GennyVST::getParameterType(int index)
{
	if (GennyExtParam::isExtParam(index))
	{
		GennyExtParam* p = getExtParam(index);
		if (p != nullptr)
			return p->type;
	}
	else
	{
		if (index > GennyPatch::getNumParameters())
			index = index % GennyPatch::getNumParameters();

		IBIndex* idx = _core->getIndexBaron()->getIndex(index);
		if (idx != nullptr)
			return idx->getDisplayType();
	}

	return ParamDisplayType::Integer;
}

int dtFunTableForFun[8] = { 0,1,2,3,0,-1,-2,-3 };
void GennyVST::getParameterValue(int index, char* text)
{
	int value = onAutomation(index, 0, (AutomationTypeFlags)((int)AutomationTypeFlags::GetValue | (int)AutomationTypeFlags::GetValueUI));

	std::string ret = "";
	ParamDisplayType disp = getParameterType(index);

	if ((int)disp & (int)ParamDisplayType::DT_FUN)
	{
		int funNum = (int)(value + 0.5f);
		if (funNum >= 0 && funNum < 8)
			ret = std::to_string(dtFunTableForFun[funNum]);
		else
			ret = "0";
	}
	else
	{
		if ((int)disp & (int)ParamDisplayType::MidiChannel)
			value += 1;

		int range = 1;
		if ((int)disp & (int)ParamDisplayType::Centered || (int)disp & (int)ParamDisplayType::Panning)
		{
			range = getParameterRange(index);
			value = (int)(value - (range / 2.0f));
		}

		if ((int)disp & (int)ParamDisplayType::Bool)
			ret = ((value == 0) ? "Disabled" : "Enabled");
		else if ((int)disp & (int)ParamDisplayType::Panning)
		{
			float halfRange = range / 2.0f;
			if (value == 0)
				ret = "centered";
			else
				ret = std::to_string((int)abs((value / halfRange) * 100.6f)) + std::string((value < 0) ? "% left" : "% right");
		}

		if (ret == "")
			ret = std::to_string(value);

		if ((int)disp & (int)ParamDisplayType::Cents)
			ret = ret + std::string((value == -1 || value == 1) ? " Cent" : " Cents");
		else if ((int)disp & (int)ParamDisplayType::Semitone)
			ret = ret + std::string((value == -1 || value == 1) ? " Semitone" : " Semitones");
		else if ((int)disp & (int)ParamDisplayType::Octave)
			ret = ret + std::string((value == -1 || value == 1) ? " Octave" : " Octaves");
		else if ((int)disp & (int)ParamDisplayType::SampleRate)
			ret = ret + std::string("hz");
	}

	strncpy(text, ret.c_str(), 32);
}


void GennyVST::getChannelName(int index, char* text)
{
	std::string name = "";

	for (int i = 0; i < kMaxInstruments; i++)
	{
		GennyPatch* p = (GennyPatch*)_patches[i];
		if (p->InstrumentDef.MidiChannel == index)
		{
			int ins = ((GennyPatch*)_patches[0])->Instruments[i];
			if(ins >= 0 && ins < _patches.size())
				name = ((GennyPatch*)_patches[ins])->Name;
		}
	}

	strncpy(text, name.c_str(), name.length() + 1);
}

GennyExtParam* GennyVST::getExtParam(int pTag)
{
	int patch = GennyExtParam::parsePatchFromTag(pTag);
	GEParam param = GennyExtParam::parseParamFromTag(pTag);

	if (patch >= 0 && patch < _patches.size())
		return ((GennyPatch*)_patches[patch])->getExt(param);

	return nullptr;
}

/*int test1[20];
int test2[20]*/;
void GennyVST::setParameter(int index, float value, VSTPatch* patch)
{
	//test1[20] = 3;

	//automationMessage m;
	//if (control->getExtParam() != nullptr)
	//	m.index = control->getExtParam()->getTag();
	//else
	//	m.index = control->getTag();
	//m.value = control->getValue() / control->getMax();

	//_vst->_automationMessageMutex.lock();
	//_vst->_automationMessages.push(m);
	//_vst->_automationMessageMutex.unlock();


#if BUILD_VST
	if (index > 99999999)
	{
		index -= 99999999;

		automationMessage m;
		m.index = index;
		m.value = value;

		_automationMessageMutex.lock();
		_automationMessages.push(m);
		_automationMessageMutex.unlock();

		return;
	}
#endif


	if (GennyExtParam::isExtParam(index))
	{
		GennyExtParam* p = getExtParam(index);
		if (p != nullptr)
		{
			if (_setParameterNormalizedValue)
				value = value * p->rangeMax;

			bool changed = value != p->get();
			p->set(value);

			if (p->param == GEParam::InsEnable)
			{
				if (changed)
				{
					int instrumentIndex = GennyExtParam::parsePatchFromTag(index);
					getCore()->clearNotes((GennyPatch*)getPatch(instrumentIndex));
				}
			}
			else if (p->param == GEParam::LFOEnable)
			{
				for (int i = 0; i < 6; i++)
				{
					_core->paramChanged((GennyPatch*)_patches[p->ins->patchIndex], YM2612Param::YM_AMS, i);
					_core->paramChanged((GennyPatch*)_patches[p->ins->patchIndex], YM2612Param::YM_FMS, i);
				}
			}
			else if (p->param == GEParam::Op3Special)
			{
				if (value > 0.5f)
				{
					GennyPatch* selectedInstrument = (GennyPatch*)_patches[((GennyPatch*)getPatch(0))->SelectedInstrument];
					GennyPatch* instrumentPatch = (GennyPatch*)_patches[((GennyPatch*)getPatch(0))->Instruments[((GennyPatch*)getPatch(0))->SelectedInstrument]];
					if (&instrumentPatch->InstrumentDef == p->ins)
					{
						for (int i = 0; i < 10; i++)
						{
							selectedInstrument->InstrumentDef.Channels[i] = false;
						}
						selectedInstrument->InstrumentDef.Channels[2] = true;
						if (_editor != NULL)
							_editor->reconnect();
					}
				}
			}
			else if (p->param == GEParam::InsPan || p->param == GEParam::InsEnableL || p->param == GEParam::InsEnableR)
			{
				for (int i = 0; i < 10; i++)
				{
					if(p->ins->Channels[i])
						_core->panningChanged((GennyPatch*)_patches[p->ins->patchIndex], i);
				}
			}
			else if ((p->param >= GEParam::ChEnable && p->param <= GEParam::ChEnable9) || p->param == GEParam::SnMelodicEnable)
			{
				int sel = index - kChannelEnableStart;

				int idx = _core->getIndexBaron()->getPatchParamIndex(GPP_SelectedInstrument);
				int selection = (int)getPatch(0)->getFromBaron(_core->getIndexBaron()->getIndex(idx));

				if ((p->param >= GEParam::ChEnable && p->param <= GEParam::ChEnable9))
					getCore()->clearNotes((GennyPatch*)getPatch(selection), (int)p->param - (int)GEParam::ChEnable);

				if (p->param == GEParam::SnMelodicEnable)
				{
					if (p->ins->snMelodicEnable)
					{
						p->ins->Channels[9] = true;
						p->ins->Channels[8] = true;
						p->ins->Channels[7] = false;
						p->ins->Channels[6] = false;
					}
					else
					{
						p->ins->Channels[9] = false;
						p->ins->Channels[8] = true;
						p->ins->Channels[7] = false;
						p->ins->Channels[6] = false;
					}

					if (_editor != NULL)
						_editor->reconnect();
				}
				else
				{
					if (p->ins->snMelodicEnable)
					{
						if (p->ins->Channels[8] == false
							|| p->ins->Channels[9] == false)
						{
							p->ins->snMelodicEnable = false;

							if (_editor != NULL)
								_editor->reconnect();
						}
					}
				}
			}
		}

		return;
	}
	if (index >= kInstrumentMuteStart && index <= kInstrumentMuteEnd)
	{
		int instrumentIndex = index - kInstrumentMuteStart;
		getCore()->clearNotes((GennyPatch*)getPatch(instrumentIndex));

		return;
	}
	if (index == kSamplePanningMessage)
	{
		_core->panningChanged(static_cast<GennyPatch*>(getPatch(static_cast<GennyPatch*>(getPatch(0))->SelectedInstrument)), 5);
		return;
	}
	if(index == kPresetControlIndex)
	{
		if (_setParameterNormalizedValue)
			value = value * 1000; //Max presets value

		_switchingPreset = true;
		_base->setProgram(value);
		rejiggerInstruments(true);
		_switchingPreset = false;
		//setCurrentPatch(_patches[value]);
		return;
	}
	if(index == kInstrumentControlIndex)
	{
		if (_setParameterNormalizedValue)
			value = value * 1000; //Max presets value

		_switchingPreset = true;
		GennyPatch* patch = static_cast<GennyPatch*>(getPatch(0));
		//GennyPatch* reorderPatch = static_cast<GennyPatch*>(getPatch(1));


		int instrumentIndex = (int)value;
		//if(reorderPatch->Instruments[instrumentIndex] >= 0)
		//	instrumentIndex = reorderPatch->Instruments[instrumentIndex];

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
		if (_setParameterNormalizedValue)
			value = value * 16; //Max midi value

		int sel = index - kMidiChannelStart;

		int idx = _core->getIndexBaron()->getInsParamIndex(GIP_MidiChannel);
		getPatch(sel)->setFromBaron(_core->getIndexBaron()->getIndex(idx), value);
		return;
	}
	if(index >= kOctaveStart && index <= kOctaveEnd)
	{
		if (_setParameterNormalizedValue)
			value = value * 32;

		int sel = index - kOctaveStart;

		int idx = _core->getIndexBaron()->getInsParamIndex(GIP_Octave);
		getPatch(sel)->setFromBaron(_core->getIndexBaron()->getIndex(idx), value);
		return;
	}
	if(index >= kTransposeStart && index <= kTransposeEnd)
	{
		if (_setParameterNormalizedValue)
			value = value * 32;

		int sel = index - kTransposeStart;

		int idx = _core->getIndexBaron()->getInsParamIndex(GIP_Transpose);
		getPatch(sel)->setFromBaron(_core->getIndexBaron()->getIndex(idx), value);
		return;
	}
	if(index >= kPanningStart && index <= kPanningEnd)
	{
		if (_setParameterNormalizedValue)
			value = value * 255;

		int sel = index - kPanningStart;

		int idx = _core->getIndexBaron()->getInsParamIndex(GIP_Panning);
		getPatch(sel)->setFromBaron(_core->getIndexBaron()->getIndex(idx), value);

		return;
	}
	if(index >= kRangeLowStart && index <= kRangeLowEnd)
	{
		if (_setParameterNormalizedValue)
			value = value * 100;

		int sel = index - kRangeLowStart;

		int idx = _core->getIndexBaron()->getInsParamIndex(GIP_RangeLow);
		getPatch(sel)->setFromBaron(_core->getIndexBaron()->getIndex(idx), value);

		return;
	}
	if(index >= kRangeHighStart && index <= kRangeHighEnd)
	{
		if (_setParameterNormalizedValue)
			value = value * 100;

		int sel = index - kRangeHighStart;

		int idx = _core->getIndexBaron()->getInsParamIndex(GIP_RangeHigh);
		getPatch(sel)->setFromBaron(_core->getIndexBaron()->getIndex(idx), value);

		return;
	}
	if(index >= kDelayStart && index <= kDelayEnd)
	{
		if (_setParameterNormalizedValue)
			value = value * 32;

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
		

		if (sel == 10)
		{
			((GennyPatch*)getPatch(selection))->InstrumentDef.snMelodicEnable = value > 0.5f;
			((GennyPatch*)getCurrentPatch())->InstrumentDef.snMelodicEnable = value > 0.5f;


			if (((GennyPatch*)getPatch(selection))->InstrumentDef.snMelodicEnable)
			{
				((GennyPatch*)getPatch(selection))->InstrumentDef.Channels[9] = true;
				((GennyPatch*)getPatch(selection))->InstrumentDef.Channels[8] = true;
				((GennyPatch*)getPatch(selection))->InstrumentDef.Channels[7] = false;
				((GennyPatch*)getPatch(selection))->InstrumentDef.Channels[6] = false;
			}
			else
			{
				((GennyPatch*)getPatch(selection))->InstrumentDef.Channels[9] = false;
				((GennyPatch*)getPatch(selection))->InstrumentDef.Channels[8] = true;
				((GennyPatch*)getPatch(selection))->InstrumentDef.Channels[7] = false;
				((GennyPatch*)getPatch(selection))->InstrumentDef.Channels[6] = false;
			}

			if (_editor != NULL)
				_editor->reconnect();
		}
		else
		{
			idx = _core->getIndexBaron()->getInsParamIndex((GennyInstrumentParam)(GIP_Channel0 + sel));
			getPatch(selection)->setFromBaron(_core->getIndexBaron()->getIndex(idx), value);

			if (((GennyPatch*)getPatch(selection))->InstrumentDef.snMelodicEnable)
			{
				if (((GennyPatch*)getPatch(selection))->InstrumentDef.Channels[8] == false
					|| ((GennyPatch*)getPatch(selection))->InstrumentDef.Channels[9] == false)
				{
					((GennyPatch*)getPatch(selection))->InstrumentDef.snMelodicEnable = false;
					((GennyPatch*)getCurrentPatch())->InstrumentDef.snMelodicEnable = false;
					if (_editor != NULL)
						_editor->reconnect();
				}
			}
		}
		return;
	}

	if (index >= kOperatorVelocityStart && index <= kOperatorVelocityEnd)
	{
		int sel = index - kOperatorVelocityStart;

		if (patch == nullptr)
			patch = _currentPatch;

		((GennyPatch*)patch)->InstrumentDef.OperatorVelocity[sel] = value > 0.5f;		
		
		for (int i = 0; i < 6; i++)
		{
			_core->channelDirty[i] = true;
		}
		return;
	}

	if(index >= kInstrumentMappingStart && index <= kInstrumentMappingEnd)
	{
		if (_setParameterNormalizedValue)
			value = value * 1000;


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
	if (idx == nullptr)
		return;

	if(idx->getType() == IB_PatchParam)
	{
		if (_setParameterNormalizedValue)
			value = value * idx->maxValue;

		IBPatchParam* param = (IBPatchParam*)idx;
		if(value >= 0 && _currentPatch == _patches[0])
			_patches[0]->setFromBaron(idx, value);
		else if(param != nullptr && value >= 0 && (param->getParameter() >= GennyPatchParam::GPP_Ins01 && param->getParameter() <= GennyPatchParam::GPP_Ins32) && _saving)
		{
			if(patch != nullptr)
				patch->setFromBaron(idx, value);
			else
				_currentPatch->setFromBaron(idx, value);
		}
	}
	else
	{
		if (_setParameterNormalizedValue)
			value = value * idx->maxValue;

		bool wasGlobal = false;
		if(idx->getType() == IB_YMParam)
		{
			YM2612Param param = ((IBYMParam*)idx)->getParameter();
			if(	param == YM_LFO)
			{
				if ((patch != nullptr && patch != _patches[0]) || (_saving && _currentPatch == _patches[0]))
					return;
	
				lfo = value; 
				_core->_chip.LFOChanged();
				wasGlobal = true;
			}
		}

		if(wasGlobal == false)
		{
			if(patch != nullptr)
			{
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

void GennyVST::setInstrumentPatchSelection(int index)
{
	_editor->setInstrumentPreset(index);
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
		int newPatchIndex = getPatchIndex(patch);

		if (selected < GennyPatch::NumInstruments && selected >= 0 && ((GennyPatch*)getPatch(0))->Instruments[selected] != newPatchIndex)
		{
			//clear all playing notes for current instrument since we're changing it's preset
			getCore()->clearNotes((GennyPatch*)getPatch(selected));
		}

#if BUILD_VST
		_setParameterNormalizedValue = false;
#endif
		setParameter(kInstrumentMappingStart + selected, newPatchIndex);
#if BUILD_VST
		_setParameterNormalizedValue = true;
#endif

		if(_editor != NULL)
			_editor->reconnect();
	}
}

void GennyVST::getSamples(float** buffer, int numSamples)
{
#if BUILD_VST
	_automationMessageMutex.lock();

	while (!_automationMessages.empty())
	{
		const automationMessage& mess = _automationMessages.top();
		_base->setParameterAutomated(mess.index, mess.value);
		_automationMessages.pop();
	}
	_automationMessageMutex.unlock();
#endif


	if (_playingStatusChanged)
	{
		_core->clearCache();
		_playingStatusChanged = false;
	}

	_core->update(buffer, numSamples);
	//_automationMessageMutex.unlock();
}



void GennyVST::noteOn(int note, float velocity, unsigned char channel, float panning, void* noteData)
{
	if (_playingStatusChanged)
	{
		_core->clearCache();
		_playingStatusChanged = false;
	}

#if BUILD_VST
	note *= 100;
	velocity /= 127.0f;
#else

#endif

	_core->noteOn(note, velocity, channel, panning, noteData);
}

void GennyVST::updateNote(void* noteData, int samples)
{
	if (_playingStatusChanged)
	{
		_core->clearCache();
		_playingStatusChanged = false;
	}

	_core->updateNote(noteData, samples);
}

void GennyVST::noteOff(int note, unsigned char channel, void* noteData)
{
	if (_playingStatusChanged)
	{
		_core->clearCache();
		_playingStatusChanged = false;
	}

#ifdef BUILD_VST
	note = (note * 100);
#endif
	_core->noteOff(note, channel, noteData);
}

void GennyVST::clearNotes() 
{
	_core->clearNotes();
}

void GennyVST::rejiggerInstruments(bool selected)
{
	GennyPatch* patch0 = (GennyPatch*)_patches[0];
	GennyPatch* patch1 = (GennyPatch*)_patches[1];
	for(int i = 0; i < kMaxInstruments; i++)
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
				instrumentPatch->setInstrumentMode(patch->InstrumentDef.Type, !_saving);
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
		int minNumPatches = 165;
		for(int i = 0; i < minNumPatches; i++)
		{
			if (i < numPatches)
			{
				if(i >= _patches.size())
					_patches.push_back(new GennyPatch(i));
	
				int size = file.readInt();

				GennyData dat;
				dat.data = new char[size];
				memcpy(dat.data, file.data + file.dataPos, size);
				dat.size = size;
				file.dataPos += size;

				GennyPatch* patch = (GennyPatch*)_patches[i];

				GennyLoaders::loadGEN(patch, &dat);

				if (patch->InstrumentDef.Type == GIType::DAC)
					patch->setFromBaron(_core->getIndexBaron()->getIndex(_core->getIndexBaron()->getYMParamIndex((YM2612Param)YM_TL, 3)), YM2612Param_getRange(YM_TL) - 27);

				if (patch->InstrumentDef.DACSampleRate >= 0 && patch->InstrumentDef.DACSampleRate < kNumDACSamplerates)
					patch->InstrumentDef.Drumset.setSampleRate(kDACSamplerates[patch->InstrumentDef.DACSampleRate]);
			}
			else
				_patches.push_back(new GennyPatch(i, "Empty Preset " + std::to_string(i - numPatches)));
		}

		_patchesInitialized = true;
		_currentPatch = _patches[0];

		GennyPatch* orderingPatch = static_cast<GennyPatch*>(getPatch(3));
		GennyPatch* mainPatch = static_cast<GennyPatch*>(getPatch(0));
		mainPatch->SelectedInstrument = 0;
		mainPatch->Instruments[0] = 0;
		orderingPatch->Instruments[0] = 0;
		rejiggerInstruments(true);
	}


	return _patches.size();
}
   

int GennyVST::getNumInstruments()
{
	int num = 0;
	GennyPatch* patch = static_cast<GennyPatch*>(getPatch(0));
	for(int i = 0; i < kMaxInstruments; i++)
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
//
//int GennyVST::RunNewAutomation(int Index, int Value, int RECFlags)
//{
//#if !BUILD_VST
//	int instrument = ((int)(Index / _base->_totalParameters));
//	int parameter = (int)(Index % _base->_totalParameters);
//
//	IBIndex* idx = _core->getIndexBaron()->getIndex(parameter);
//	if (idx == nullptr)
//		return Value;
//
//	int range = 0;
//	if (idx->getType() == IB_YMParam)
//	{
//		IBYMParam* parm = (IBYMParam*)idx;
//		range = YM2612Param_getRange(parm->getParameter());
//	}
//
//	int setValue = min(((Value / 127.0f) * range) + 0.5f, range);
//
//	int realPatchIndex = instrument;
//	if (instrument < 16 && idx->getType() != IB_InsParam)//Half assed compatibility with the old system
//		realPatchIndex = static_cast<GennyPatch*>(getPatch(0))->Instruments[instrument];
//
//	if (realPatchIndex >= 0 && realPatchIndex < getNumPatches())
//	{
//		int paramRange = getParameterRange(parameter);
//		int ret = Value;
//		if (RECFlags & REC_FromMIDI)
//			ret = setValue = (int)(((Value / 65536.0f) * paramRange) + 0.5f);
//
//		if (RECFlags & REC_UpdateValue)
//			setParameter(parameter, (float)ret, _patches[realPatchIndex]);
//		if (RECFlags & REC_GetValue)
//			ret = setValue = (int)getParameter(parameter, _patches[realPatchIndex]);
//		if (_editor != nullptr && (_currentPatch == _patches[realPatchIndex] || idx->global) && RECFlags & REC_UpdateControl)
//			_editor->setParameter(parameter, setValue);
//
//		return ret;
//	}
//#endif
//
//	return 0;
//}

#if !BUILD_VST
void GennyVST::updateNoteControl(GennyPatch* patch, void* noteData)
{
	PlugVoice* voice = (PlugVoice*)noteData;

	auto parm = _noteControlParams.find(0);
	if (parm != _noteControlParams.end())
	{
		std::vector<int>& vec = _noteControlParams[0];
		for (auto it = vec.begin(); it != vec.end(); it++)
		{
			onAutomation((*it), ((voice->Params->FinalLevels.FCut + 1.0f) / 2.0f) * 65535, (AutomationTypeFlags)((int)AutomationTypeFlags::UpdateValue | (int)AutomationTypeFlags::UpdateControl | (int)AutomationTypeFlags::UpdateUIThreaded));
		}
	}	
	
	parm = _noteControlParams.find(1);
	if (parm != _noteControlParams.end())
	{
		std::vector<int>& vec = _noteControlParams[1];
		for (auto it = vec.begin(); it != vec.end(); it++)
		{
			onAutomation((*it), ((voice->Params->FinalLevels.FRes + 1.0f) / 2.0f) * 65535, (AutomationTypeFlags)((int)AutomationTypeFlags::UpdateValue | (int)AutomationTypeFlags::UpdateControl | (int)AutomationTypeFlags::UpdateUIThreaded));
		}
	}
}

void GennyVST::assignNoteControl(int idx, int param)
{
	//unlink from other side
	unassignNoteControl(idx == 0 ? 1 : 0, param);

	_mutex.lock();

	auto parm = _noteControlParams.find(idx);
	if (parm == _noteControlParams.end())
		_noteControlParams[idx] = std::vector<int>();

	std::vector<int>& vec = _noteControlParams[idx];

	bool found = false;
	for (auto it = vec.begin(); it != vec.end(); it++)
	{
		if ((*it) == param)
		{
			found = param;
			break;
		}
	}

	if(found == false)
		_noteControlParams[idx].push_back(param);

	_mutex.unlock();
}
void GennyVST::unassignNoteControl(int idx, int param)
{
	_mutex.lock();

	bool found = false;
	auto parm = _noteControlParams.find(idx);
	if (parm != _noteControlParams.end())
	{
		std::vector<int>& vec = _noteControlParams[idx];
		for (auto it = vec.begin(); it != vec.end(); it++)
		{
			if ((*it) == param)
			{
				vec.erase(it);
				break;
			}
		}
	}

	_mutex.unlock();
}
bool GennyVST::noteControlIsAssigned(int idx, int param)
{
	_mutex.lock();

	bool found = false;
	auto parm = _noteControlParams.find(idx);
	if (parm != _noteControlParams.end())
	{
		std::vector<int>& vec = _noteControlParams[idx];

		for (auto it = vec.begin(); it != vec.end(); it++)
		{
			if ((*it) == param)
			{
				found = true;
				break;
			}
		}
	}

	_mutex.unlock();

	return found;
}
#endif

void GennyVST::MidiCleanLearnedParam(int paramTag, int legacyTag)
{
	_mutex.lock();
	std::map<int, std::vector<int>>::iterator existing = _midiLearn.end();
	for (std::map<int, std::vector<int>>::iterator it = _midiLearn.begin(); it != _midiLearn.end(); ++it)
	{
		for (std::vector<int>::iterator it2 = it->second.begin(); it2 != it->second.end();)
		{
			if (*it2 == paramTag || *it2 == legacyTag)
				it2 = it->second.erase(it2);
			else
				++it2;
		}
	}
	_mutex.unlock();
}

void GennyVST::MidiLearn(int paramTag, int legacyTag)
{
	int learnedInstrumentParamIndex = MidiForget(paramTag, legacyTag);
	if(learnedInstrumentParamIndex != -1)
		_midiLearnParameter = learnedInstrumentParamIndex;
}

int GennyVST::MidiForget(int paramTag, int legacyTag)
{
	_midiLearnParameter = -1;
	if (GennyExtParam::isExtParam(paramTag))
	{
		MidiCleanLearnedParam(paramTag, -1);

		GennyPatch* patch = static_cast<GennyPatch*>(getPatch(0));
		GennyExtParam* p = getExtParam(paramTag);
		if (p != nullptr)
		{
			p = ((GennyPatch*)_patches[patch->SelectedInstrument])->getExt(p->param);
			if (p != nullptr)
				return p->getTag(); //Midi automation needs to bind to the first 16 patches (the instruments)
		}

		return -1;
	}

	int learnedInstrumentParamIndex = paramTag;
	IBIndex* learnIdx = getCore()->getIndexBaron()->getIndex(paramTag);
	if (learnIdx != nullptr)
	{
		if (!learnIdx->global)
		{
			GennyPatch* patch = static_cast<GennyPatch*>(getPatch(0));
			learnedInstrumentParamIndex = ((patch->SelectedInstrument) * 9999) + paramTag;
		}
	}

	learnedInstrumentParamIndex += 999999;
	MidiCleanLearnedParam(learnedInstrumentParamIndex, legacyTag);

	if (learnIdx != nullptr)
		return learnedInstrumentParamIndex;
	return -1;
}


void GennyVST::onMidiMessage(int channel, int message, int value)
{
#if BUILD_VST
	int learnParameter = _midiLearnParameter;
	_midiLearnParameter = -1;

	auto existing = _midiLearn.find(message);
	if (learnParameter >= 0)
	{
		//Push in the new learned parameter
		if (existing == _midiLearn.end())
			existing = _midiLearn.insert(std::pair<int, std::vector<int>>(message, std::vector<int>())).first;
		existing->second.push_back(learnParameter);
	}

	//Apply midi values to existing midi learned parameters
	if (existing != _midiLearn.end())
	{
		//Loop through learned sub-parameters
		for (std::vector<int>::iterator it2 = existing->second.begin(); it2 != existing->second.end(); it2++)
		{
			int parameterHash = *it2;
			onAutomation(parameterHash, value);
		}
	}
#endif
}


void GennyVST::showHint(int parameterTag)
{
#if !BUILD_VST
	if(parameterTag < 0 || parameterTag > kExtParamsEnd)
		_base->PlugHost->OnHint(_base->HostTag, nullptr);
	else
	{
		if (!GennyExtParam::isExtParam(parameterTag))
			parameterTag += getPatchIndex(_currentPatch) * GennyPatch::getNumParameters();

		char nameString[32] = "";
		getParameterName(parameterTag, nameString);

		if (strncmp(nameString, "", 32) != 0)
		{
			char valueString[32] = "";
			getParameterValue(parameterTag, valueString);
			std::string fullString = std::string("^b^a") + std::string(nameString) + ": " + std::string(valueString);
			strncpy(_hintString, fullString.c_str(), fullString.size() + 1);
			_base->PlugHost->OnHint(_base->HostTag, _hintString);
		}
		else
			_base->PlugHost->OnHint(_base->HostTag, nullptr);
	}
#endif
}

//int TranslateMidi(int Value, int Min, int Max)
//{
//	return Min + (int)(Value * FromMIDI_Div * (Max - Min));
//}

int GennyVST::onAutomation(int parameterHash, int value, AutomationTypeFlags type)
{
	bool fromMidi = ((int)type & (int)AutomationTypeFlags::FromMIDI);
	bool updateValue = (int)type & (int)AutomationTypeFlags::UpdateValue;

	bool needSetValue = updateValue || fromMidi;
	bool getValue = (int)type & (int)AutomationTypeFlags::GetValue;
	bool updateUI = (int)type & (int)AutomationTypeFlags::UpdateControl;
	bool getValueUI = ((int)type & (int)AutomationTypeFlags::GetValueUI);
	if (!(needSetValue || updateValue || getValue || updateUI) /* || (parameterHash > kExtParamsEnd)*/)
		return -1;

	int setValue = 0;
	int parameterValue = parameterHash;
	GennyPatch* patch = nullptr;
	bool isCurrentPatch = false;
	if (GennyExtParam::isExtParam(parameterHash))
	{
		GennyExtParam* p = getExtParam(parameterHash);
		if (p != nullptr)
		{
			int currentInstrumentPatch = ((GennyPatch*)_patches[0])->Instruments[((GennyPatch*)_patches[0])->SelectedInstrument];
			isCurrentPatch = (p->ins->patchIndex == currentInstrumentPatch) || ((p->param >= GEParam::ChEnable0) && (p->param <= GEParam::ChEnable9));
			GennyPatch* instrumentPatch = ((GennyPatch*)_patches[p->ins->patchIndex]);
			GennyPatch* realPatch = instrumentPatch;
			if (p->isInsParam()) 
			{
				isCurrentPatch = p->ins->patchIndex == ((GennyPatch*)_patches[0])->SelectedInstrument;
				//int currentInstrumentPatch = ((GennyPatch*)_patches[0])->Instruments[((GennyPatch*)_patches[0])->SelectedInstrument];


				//int instrumentIndex = ((GennyPatch*)_patches[0])->Instruments[p->ins->patchIndex];
				//if (instrumentIndex >= 0 && instrumentIndex < _patches.size())
				//{
				//	realPatch = ((GennyPatch*)_patches[instrumentIndex]);
				//	p = realPatch->getExt(p->param);
				//	parameterHash = p->getTag();
				//	isCurrentPatch = ((GennyPatch*)_currentPatch)->InstrumentDef.patchIndex == p->ins->patchIndex;
				//}
				//else
				//	realPatch = nullptr;
			}

			if (realPatch != nullptr)
			{
				if (needSetValue)
				{
					setValue = value;

#if BUILD_VST
					if ((int)type & (int)AutomationTypeFlags::FromMIDI)
						setValue = (int)(((value / 65535.0f) * p->rangeMax) + 0.5f);
					else
						setValue = min((int)(((value / 127.0f) * p->rangeMax) + 0.5f), (int)p->rangeMax);
#else

					if (fromMidi)
						setValue = TranslateMidi(value, 0, p->rangeMax);
#endif

				}

				parameterValue = parameterHash;
				patch = realPatch;
			}
			else
				return value;
		}
	}
	else if (parameterHash >= 999999)
	{
		//This seems to be for VST only, as MidiLearned midi messages come through as a huge number
		parameterHash -= 999999;
		int instrument = ((int)(parameterHash / 9999));
		int parameter = (int)(parameterHash % 9999);

		IBIndex* idx = _core->getIndexBaron()->getIndex(parameter);

		if (needSetValue)
		{
			int range = 0;
			if (idx->getType() == IB_YMParam)
			{
				IBYMParam* parm = (IBYMParam*)idx;
				range = YM2612Param_getRange(parm->getParameter());
				if (_automationInverse && YM2612Param_getIsReverseParam(parm->getParameter()))
					value = 65535 - value;
			}

			setValue = value;
			//if (fromMidi)
			//	setValue = TranslateMidi(value, 0, range);

			if ((int)type & (int)AutomationTypeFlags::FromMIDI /* || updateUI*/)
				setValue = (int)(((value / 65535.0f) * range) + 0.5f);
			else
				setValue =  min((int)(((value / 127.0f) * range) + 0.5f), range);
		}

		int realPatchIndex = static_cast<GennyPatch*>(getPatch(0))->Instruments[instrument];
		if (realPatchIndex >= 0 && realPatchIndex < getNumPatches())
		{
			parameterValue = parameter;
			patch = (GennyPatch*)_patches[realPatchIndex];

			if ((idx->global || _currentPatch == _patches[realPatchIndex]))
				isCurrentPatch = true;
		}
		else
			return value;
	}
	else
	{
		//Legacy Bullshit
		int patchNum = (int)(parameterHash / GennyPatch::getNumParameters());
		int paramNumber = parameterHash - (patchNum * GennyPatch::getNumParameters());
		IBIndex* idx = _core->getIndexBaron()->getIndex(paramNumber);

		if (needSetValue)
		{
			int range = 0;
			if (idx->getType() == IB_YMParam)
			{
				IBYMParam* parm = (IBYMParam*)idx;
				range = YM2612Param_getRange(parm->getParameter());

				if (_automationInverse && YM2612Param_getIsReverseParam(parm->getParameter()))
					value = 65535 - value;
			}

			setValue = value;
			if(fromMidi)
				setValue = TranslateMidi(value, 0, range);


			//if ((int)type & (int)AutomationTypeFlags::FromMIDI || updateUI)
			//	setValue = (int)(((value / 65535.0f) * range) + 0.5f);
			//else
			//	setValue = value;// min((int)(((value / 127.0f) * range) + 0.5f), range);
		}

		parameterValue = paramNumber;
		patch = (GennyPatch*)getPatch(patchNum);

		if ((idx->global || _currentPatch == patch))
			isCurrentPatch = true;
	}

#if BUILD_VST
	_setParameterNormalizedValue = false;
#endif

	int ret = value;
	if (needSetValue)
		ret = setValue;

	if (updateValue)
		setParameter(parameterValue, setValue, GennyExtParam::isExtParam(parameterValue) ? nullptr : patch);

	if (getValue)
	{
		setValue = ret = (int)getParameter(parameterValue, patch);

		//if (!getValueUI)
		//	ret = (int)((setValue / (float)getParameterRange(parameterValue)) * USHRT_MAX);
	}

	if (_editor != nullptr && updateUI && isCurrentPatch)
	{
		if (_clearMidiUIUpdateHistory)
		{
			_midiUIUpdateHistory.clear();
			_clearMidiUIUpdateHistory = false;
		}

		auto it = _midiUIUpdateHistory.find(parameterValue);
		if (it == _midiUIUpdateHistory.end() || (*it).second != setValue)
		{
			if (((int)type & (int)AutomationTypeFlags::UpdateUIThreaded))
			{
				_mutex.lock();
				_hasUIUpdates = true;
				_midiUIUpdates[parameterValue] = setValue;
				_midiUIUpdateHistory[parameterValue] = setValue;
				_mutex.unlock();
			}
			else
			{
#if BUILD_VST
				_mutex.lock();
				_hasUIUpdates = true;
				_midiUIUpdates[parameterValue] = setValue;
				_midiUIUpdateHistory[parameterValue] = setValue;
				_mutex.unlock();
#else
				_editor->setParameter(parameterValue, setValue);
#endif
			}
		}
	}


#if BUILD_VST
	_setParameterNormalizedValue = true;
#endif

	return ret;
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