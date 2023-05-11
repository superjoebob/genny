#include "VSTBase.h"
#include "Genny2612.h"
#include "VirtualInstrument.h"
#include "gmnames.h"
#include "lib/platform\win32\win32frame.h"
#include "lib/vstguiinit.h"

//#include "C:\Program Files (x86)\Visual Leak Detector\include\vld.h"

#ifdef BUILD_VST

//void* hInstance = nullptr; // used by VSTGUI
//extern "C" BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpvReserved)
//{
//	if (hInstance == nullptr && dwReason != 0)
//	{
//		hInstance = hInst;
//		VSTGUI::init(hInst);
//	}
//	if (hInstance != nullptr && dwReason == 0)
//	{
//		hInstance = nullptr;
//		VSTGUI::exit();
//	}
//
//	return TRUE;
//}
  

VSTBase::VSTBase(VirtualInstrument* parent, void* data)
	: AudioEffectX ((audioMasterCallback)data, parent->getTotalPatchCount(), GennyPatch::getNumParameters())
{  
	for (VstInt32 i = 0; i < kMaxInstruments; i++)
		_channelPrograms[i] = i;

	if (audioMaster)
	{ 
		setNumInputs (0);
		setNumOutputs (2); 
		canProcessReplacing(true);
		isSynth(true);
		setUniqueID ('GENX');
		programsAreChunks(true); 
	}
	_parent = parent;
	_tempo = 0.0f;
	_sampleRate = 44100;



	_currentEvents = (VstEvents *)malloc(sizeof(VstEvents) + 300 * sizeof(VstEvent *));
	memset(_currentEvents, 0, sizeof(VstEvents)); // zeroing class fields is enough

	for (int i = 0; i < 300; i++)
	{
		_currentEvents->events[i] = (VstEvent*)new VstMidiEvent();
	}
}

VSTBase::~VSTBase(void)
{
	for (int i = 0; i < 300; i++)
		delete _currentEvents->events[i];

	free(_currentEvents);
}

void VSTBase::initialize()
{
	setProgram(1);
	suspend();
} 

void VSTBase::processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames)
{
	_parent->getSamples(outputs, sampleFrames);

	VstTimeInfo* time = getTimeInfo(kVstTempoValid | kVstTransportChanged | kVstPpqPosValid);
	_tempo = time->tempo;

	if (time->flags & kVstTransportChanged)
	{
		_parent->_playingStatusChanged = true;
		//_parent->clearNotes();

		//for (int i = 0; i < 16; i++)
		//	_parent->_globalPitchOffset[i] = 0.0f;
	}
}

void VSTBase::process (float** inputs, float** outputs, VstInt32 sampleFrames)
{
	_parent->getSamples(outputs, sampleFrames);

	VstTimeInfo* time = getTimeInfo(kVstTempoValid);
	_tempo = time->tempo;
}



VstInt32 VSTBase::processEvents (VstEvents* ev)
{
	_sampleRate = updateSampleRate();

	_midiNotes.clear();
	for (VstInt32 i = 0; i < ev->numEvents; i++)
	{
		if ((ev->events[i])->type != kVstMidiType)
			continue;

		VstMidiEvent* event = (VstMidiEvent*)ev->events[i];
		unsigned char* midiData = (unsigned char*)event->midiData;
		VstInt32 status = midiData[0];	// ignoring channel

		if ((status & 0xF0) == 0xE0) //PITCH BEND!
		{
			int channel = status & 0xF;
			int p1 = midiData[1];
			int p2 = midiData[2];

			int pitchChange = ((p2 << 7) | p1) - 8192;
			_parent->_globalPitchOffset[channel] = pitchChange / 8192.0f;
		}
		else if (status == 250)
		{
			_parent->_playingStatusChanged = true;
			_parent->clearNotes();

			//for (int i = 0; i < 16; i++)
			//	_parent->_globalPitchOffset[i] = 0.0f;
		}
		else if (status > 127 && status < 160 )	// we only look at notes
		{
			VstInt32 note = midiData[1];// & 0x7f;
			VstInt32 velocity = midiData[2];// & 0x7f;
			VstInt32 midiChannel = 0;
			if (status > 127 && status < 144)
			{
				velocity = -1;	// note off by velocity -1
				midiChannel = status - 128;
			}
			else if( status > 143 && status < 160 )
			{
				midiChannel = status - 144;
			}

			_midiNotes.push_back(MidiNote(note, velocity, midiChannel));
		}
		else if (status > 175 && status < 192)
		{
			if (midiData[1] == 0x7e || midiData[1] == 0x7b)
			{
				_parent->clearNotes();
			}
			else
			{
				_parent->onMidiMessage(status & 15, midiData[1], midiData[2]);


				//YM2612Value* learn = _stateData.GetMidiLearn();
				//if( learn != nullptr )
				//{
				//	learn->SetMidiLearn(false);
				//	learn->SetMidiHookup(num);
				//}

				//_parmAdjust = true;
				//std::vector<YM2612Value*> hooks;
				//_stateData.GetMidiHookups(num, hooks);
				//for( int i = 0; i < (int)hooks.size(); i++ )
				//{
				//	setParameter( hooks[i]->GetIndex(), (float)value / (float)127 );
				//}
				//_parmAdjust = false;
			}
		}

		event++;
	} 

	if (_midiNotes.size() > 0)
	{
		//Note off and log notes first
		for (auto it = _midiNotes.begin(); it != _midiNotes.end(); it++)
		{
			if((*it).velocity < 0)
				_parent->noteOff((*it).note, (*it).channel);
			else if((*it).note < 3)
				_parent->noteOn((*it).note, (*it).velocity, (*it).channel, 0);
		}	

		//Now note on events
		for (auto it = _midiNotes.begin(); it != _midiNotes.end(); it++)
		{
			if ((*it).velocity >= 0 && (*it).note >= 3)
			{
				(*it).velocity += 27;
				if ((*it).velocity > 127)
					(*it).velocity = 127;

				_parent->noteOn((*it).note, (*it).velocity, (*it).channel, 0);
			}
		}
	}

	return 1;
}
VstIntPtr VSTBase::dispatcher(VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt)
{
	return AudioEffectX::dispatcher(opcode, index, value, ptr, opt);
}

void VSTBase::resume()
{
	_sampleRate = updateSampleRate();
	sampleRateChanged(_sampleRate);
	__super::resume();
}

void VSTBase::sampleRateChanged(double newRate)
{ 
	_parent->sampleRateChanged(newRate); 
}

VstInt32 VSTBase::getChunk (void** data, bool isPreset)
{
	return _parent->getPluginState(data, isPreset);
}

VstInt32 VSTBase::setChunk (void* data, VstInt32 byteSize, bool isPreset)
{
	return _parent->setPluginState(data, byteSize, isPreset);
}

float VSTBase::getParameter (VstInt32 index)
{
	return _parent->getParameter(index);
}

void VSTBase::getParameterName (VstInt32 index, char* text)
{
	_parent->getParameterName(index, text);
}

void VSTBase::getParameterDisplay (VstInt32 index, char* text)
{
	_parent->getParameterValue(index, text);
}

void VSTBase::setParameter (VstInt32 index, float value)
{
	_parent->setParameter(index, value);
}

void VSTBase::setProgram(VstInt32 program)
{
	curProgram = program;
	_parent->setPatchIndex(program);
}

void VSTBase::setProgramName (char* name)
{
	//if(strcmp(name, "DefaultPatch") == 0)
	//	return;
	_parent->setProgramName(name);
}

void VSTBase::getProgramName (char* name)
{
	VSTPatch* current = _parent->getCurrentPatch();
	if(current != NULL)
		vst_strncpy (name, current->Name.c_str(), kVstMaxProgNameLen);
}

bool VSTBase::getProgramNameIndexed (VstInt32 category, VstInt32 index, char* text)
{
	if (index < _parent->getNumPatches())
	{
		strcpy (text, _parent->getPatch(index)->Name.c_str());
		return true;
	}
	return false;
}

bool VSTBase::getEffectName (char* name)
{
	std::string sname = _parent->getEffectName();
	strcpy (name, sname.c_str());
	return true;
}
bool VSTBase::getProductString (char* text)
{
	std::string sname = _parent->getProductName();
	strcpy (text, sname.c_str());
	return true;
}
bool VSTBase::getVendorString (char* text)
{
	std::string sname = _parent->getAuthorName();
	strcpy (text, sname.c_str());
	return true;
}


void VSTBase::MidiLearn(int paramTag, int legacyTag)
{
	_parent->MidiLearn(paramTag, legacyTag);
}

void VSTBase::MidiForget(int paramTag, int legacyTag)
{
	_parent->MidiForget(paramTag, legacyTag);
}

void VSTBase::MidiOut(unsigned char pStatus, unsigned char pData1, unsigned char pData2, unsigned char pPort)
{
	VstMidiEvent* midi = (VstMidiEvent*)_currentEvents->events[_currentEvents->numEvents];
	memset(midi, 0, sizeof(VstMidiEvent));

	midi->type = kVstMidiType;
	midi->byteSize = sizeof(VstMidiEvent);
	midi->midiData[0] = pStatus;
	midi->midiData[1] = pData1;
	midi->midiData[2] = pData2;
	_currentEvents->numEvents++;

	if (_currentEvents->numEvents == 300)
		MidiFlush();
}

void VSTBase::MidiFlush()
{
	if (_currentEvents->numEvents > 0)
	{
		sendVstEventsToHost(_currentEvents);
		_currentEvents->numEvents = 0;
	}
}

VstInt32 VSTBase::getNumMidiInputChannels()
{
	return 1;
}

VstInt32 VSTBase::getNumMidiOutputChannels()
{
	return 0;
}

VstInt32 VSTBase::getMidiProgramName (VstInt32 channel, MidiProgramName* mpn)
{
	VstInt32 prg = mpn->thisProgramIndex;
	if (prg < 0 || prg >= 128)
		return 0;
	FillProgram (channel, prg, mpn);
	if (channel == 9)
		return 1;
	return 128L;
}

VstInt32 VSTBase::getCurrentMidiProgram (VstInt32 channel, MidiProgramName* mpn)
{
	if (channel < 0 || channel >= kMaxInstruments || !mpn)
		return -1;
	VstInt32 prg = _channelPrograms[channel];
	mpn->thisProgramIndex = prg;
	FillProgram (channel, prg, mpn);
	return prg;
}

void VSTBase::FillProgram (VstInt32 channel, VstInt32 prg, MidiProgramName* mpn)
{
	mpn->midiBankMsb =
		mpn->midiBankLsb = -1;
	mpn->reserved = 0;
	mpn->flags = 0;

	if (channel == 9)	// drums
	{
		vst_strncpy (mpn->name, "Standard", 63);
		mpn->midiProgram = 0;
		mpn->parentCategoryIndex = 0;
	}
	else
	{
		vst_strncpy (mpn->name, GmNames[prg], 63);
		mpn->midiProgram = (char)prg;
		mpn->parentCategoryIndex = -1;	// for now

		for (VstInt32 i = 0; i < kNumGmCategories; i++)
		{
			if (prg >= GmCategoriesFirstIndices[i] && prg < GmCategoriesFirstIndices[i + 1])
			{
				mpn->parentCategoryIndex = i;
				break;
			}
		}
	}
}

VstInt32 VSTBase::getMidiProgramCategory (VstInt32 channel, MidiProgramCategory* cat)
{
	cat->parentCategoryIndex = -1;	// -1:no parent category
	cat->flags = 0;					// reserved, none defined yet, zero.
	VstInt32 category = cat->thisCategoryIndex;
	if (channel == 9)
	{
		vst_strncpy (cat->name, "Drums", 63);
		return 1;
	}
	if (category >= 0 && category < kNumGmCategories)
		vst_strncpy (cat->name, GmCategories[category], 63);
	else
		cat->name[0] = 0;
	return kNumGmCategories;
}

bool VSTBase::hasMidiProgramsChanged (VstInt32 channel)
{
	return false;	// updateDisplay ()
}

bool VSTBase::getMidiKeyName (VstInt32 channel, MidiKeyName* key)
	// struct will be filled with information for 'thisProgramIndex' and 'thisKeyNumber'
	// if keyName is "" the standard name of the key will be displayed.
	// if false is returned, no MidiKeyNames defined for 'thisProgramIndex'.
{
	// key->thisProgramIndex;		// >= 0. fill struct for this program index.
	// key->thisKeyNumber;			// 0 - 127. fill struct for this key number.
	key->keyName[0] = 0;
	key->reserved = 0;				// zero
	key->flags = 0;					// reserved, none defined yet, zero.
	return false;
}


VstInt32 VSTBase::canDo (char* text)
{
	if (!strcmp (text, "receiveVstEvents"))
		return 1;
	if (!strcmp (text, "receiveVstMidiEvent"))
		return 1;
	if (!strcmp (text, "midiProgramNames"))
		return 1;

	return -1;	// explicitly can't do; 0 => don't know
}

bool VSTBase::getOutputProperties(VstInt32 index, VstPinProperties* properties)
{
	if( index < 2 )
	{
		vst_strncpy (properties->label, "Vstx ", 63);
		char temp[11] = {0};
		int2string (index + 1, temp, 10);
		vst_strncat (properties->label, temp, 63);

		properties->flags = kVstPinIsActive;
		if (index < 2)
			properties->flags |= kVstPinIsStereo;	// make channel 1+2 stereo
		return true;
	}
	return false;
}
#else

//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>

#include "GennyInterface.h"
#include "fp_def.h"
#include "fp_plugclass.h"

TFruityPlugInfo PlugInfo =
{
	CurrentSDKVersion,
	"GennyX FL",
	"GennyX FL",
	FPF_Type_FullGen | FPF_WantNewTick // the amount of parameters
};

void* hInstance = nullptr; // used by VSTGUI
extern "C" BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpvReserved)
{
	if (hInstance == nullptr && dwReason != 0)
	{
		hInstance = hInst;
		VSTGUI::init(hInst);
	}	
	if (hInstance != nullptr && dwReason == 0)
	{
		hInstance = nullptr;
		VSTGUI::exit();
	}  

	return TRUE;
}

VSTBase::VSTBase(VirtualInstrument* parent, void* data)
	: TCPPFruityPlug(static_cast<FruityPlugInfo*>(data)->Tag, static_cast<FruityPlugInfo*>(data)->Host, 0)
{
	//SetInstanceHandle(GetModuleHandle(NULL));

	FruityPlugInfo* info = static_cast<FruityPlugInfo*>(data);
	_totalParameters = GennyPatch::getNumParameters();
	//PlugInfo.NumParams = kExtParamsEnd;
	Info = &PlugInfo;
	HostTag = info->Tag;
	EditorHandle = 0;
	_host = info->Host;
	_editor = nullptr;
	_samplesPerTick = 100;
	_tempo = 120;

	_sampleRate = 44100;

	int patches = parent->getTotalPatchCount();

	/*if (audioMaster)
	{
		setNumInputs (0);
		setNumOutputs (2);
		canProcessReplacing(true);
		isSynth(true);
		setUniqueID ('GENN');
	}*/
	_parent = parent;
}

VSTBase::~VSTBase(void)
{
	//if(_editor != nullptr)
	//	delete _editor;

	//_CrtDumpMemoryLeaks();
}

void VSTBase::initialize()
{
	_parent->setCurrentPatch(_parent->getPatch(0));
}

void VSTBase::setProgram(int program)
{
	_parent->setPatchIndex(program);
} 

void VSTBase::MidiOut(unsigned char pStatus, unsigned char pData1, unsigned char pData2, unsigned char pPort)
{
	TMIDIOutMsg msg;
	msg.Status = pStatus;
	msg.Data1 = pData1;
	msg.Data2 = pData2;
	msg.Port = pPort; 
	//PlugHost->MIDIOut(HostTag, *(int*)&msg);

	PlugHost->MIDIOut_Delayed(HostTag, *(int*)&msg);
}

void _stdcall VSTBase::Gen_Render(PWAV32FS DestBuffer, int &Length)
{
	//Kill voices first, this will solve a lot of oddness
	for (int i = 0; i < _voices.size(); i++)
	{
		if (_voices[i]->State == -1)
		{
			int note = _voices[i]->Params->InitLevels.Pitch;
			_parent->noteOff(note, 0, (void*)_voices[i]);
			PlugHost->Voice_Kill(_voices[i]->HostTag, true);
		}
		else if (_voices[i]->State == 1 && _voices[i]->Params->InitLevels.Pitch < 300)
		{
			//Catch logging notes first to ensure they come before all others firing at the same time
			_parent->noteOn(_voices[i]->Params->FinalLevels.Pitch + 6000, 0, 0, (_voices[i]->Params->FinalLevels.Pan), _voices[i]);
			_voices[i]->State = 2;
		}
	}

	for(int i = 0; i < _voices.size(); i++)
	{
		//Not sure why this needs to be done but FL sets these Vol levels to 
		//horrible NaN numbers when using note slides to fade notes out
		if (_voices[i]->Params->InitLevels.Vol < 0)
			_voices[i]->Params->InitLevels.Vol = 0;
		if (_voices[i]->Params->FinalLevels.Vol < 0)
			_voices[i]->Params->FinalLevels.Vol = 0;

		if(_voices[i]->State == 1)
		{
			float masterVolume = -1;
			if(_voices[i]->Params->InitLevels.Vol > 0.0f)
				masterVolume = _voices[i]->Params->FinalLevels.Vol / _voices[i]->Params->InitLevels.Vol;

			int note = (int)_voices[i]->Params->FinalLevels.Pitch + 6000; 
			int midiChannel = PlugHost->Voice_ProcessEvent(_voices[i]->HostTag, FPV_GetColor, 0, 0);

			float rootedVolume = sqrt(sqrt(_voices[i]->Params->InitLevels.Vol));

			_parent->noteOn(note, rootedVolume, midiChannel, (_voices[i]->Params->FinalLevels.Pan), _voices[i]);

			if(masterVolume >= 0)
				_parent->setMasterVolume(masterVolume);

			_voices[i]->State = 2;
		}
		else if (_voices[i]->State == -1)
		{

		}
		else
		{

			_parent->updateNote(_voices[i], Length);
			
			float masterVolume = -1.0f;
			if(_voices[i]->Params->InitLevels.Vol != 0.0f)
				masterVolume = _voices[i]->Params->FinalLevels.Vol / _voices[i]->Params->InitLevels.Vol;

			if (masterVolume >= 0)
				_parent->setMasterVolume(masterVolume);
		}
	}	
	

	_parent->getSamples((float**)DestBuffer, Length);
}

intptr_t _stdcall VSTBase::Dispatcher(intptr_t ID, intptr_t Index, intptr_t Value)
{	
	intptr_t r = TCPPFruityPlug::Dispatcher(ID, Index, Value);
	if (r != 0)
		return r;

	if (ID == FPD_UseVoiceLevels)
	{
		return 2;
	}
	else if(ID == FPD_Flush)
	{ 
		_parent->clearNotes();
	}
	//else if (ID == FPD_GetParamInfo)
	//{
	//	return PI_Float;
	//}
	else if (ID == FPD_SetPlaying || ID == FPD_SongPosChanged)
	{
		_parent->_playingStatusChanged = true;
		if(ID == FPD_SetPlaying)
			_parent->clearNotes();
	}
	else if( ID == FPD_ShowEditor )
	{
		if( _editor != nullptr)
		{
			if (Value == 0)
			{
				// close editor
				_editor->close();
				EditorHandle = 0;
			}
			else if( EditorHandle == 0 )
			{
				PlugHost->Dispatcher(HostTag, FHD_SetNumPresets, 0, _parent->getTotalPatchCount());
				PlugHost->Dispatcher(HostTag, FHD_NamesChanged, 0, FPN_Semitone);

				int totalParams = (int)(_parent->getTotalPatchCount() * (int)(_totalParameters + (int)GEParam::TOTAL_EXT_PARAMS));
				PlugHost->Dispatcher(HostTag, FHD_SetNumParams, 0, totalParams);

				// open editor
				_editor->open(reinterpret_cast<HWND>(Value));
				_editor->getFrame()->takeFocus();
				EditorHandle = static_cast<HWND>(_editor->getFrame()->getPlatformFrame()->getPlatformRepresentation());

				PlugHost->Dispatcher(HostTag, FHD_EditorResized, 0, 0);
			}
			else
			{
				// change parent window ?
				::SetParent(EditorHandle, reinterpret_cast<HWND>(Value));
			}



		}
	}
	else if (ID == FPD_SetSampleRate)
	{
		_sampleRate = Value;
		sampleRateChanged(Value);
	}
	else if (ID == FPD_SetPreset)
	{
		_parent->setInstrumentPatchSelection(Index);
		return -1;
	}

	return 0;
}

void VSTBase::sampleRateChanged(double newRate)
{
	_parent->sampleRateChanged(newRate);
}

void _stdcall VSTBase::SaveRestoreState(IStream *Stream, BOOL Save)
{
	_parent->saveData(Stream, Save ? true : false);

	PlugHost->Dispatcher(HostTag, FHD_SetNumPresets, 0, _parent->getTotalPatchCount());
	PlugHost->Dispatcher(HostTag, FHD_NamesChanged, 0, FPN_Semitone);
	int totalParams = (int)(_parent->getTotalPatchCount() * (int)(_totalParameters + (int)GEParam::TOTAL_EXT_PARAMS));
	PlugHost->Dispatcher(HostTag, FHD_SetNumParams, 0, totalParams);
}




TVoiceHandle _stdcall VSTBase::TriggerVoice(PVoiceParams VoiceParams, intptr_t SetTag)
{
	// create & init
	PlugVoice* voice = new PlugVoice();
	voice->HostTag = SetTag;

	//for (int n = 0; n < nOsc; n++)
	//	Voice->Pos[n] = 0;
	voice->Params = VoiceParams;
	voice->State = 1;   // start the attack envelope
	//FPV_Get




	// add to the list
	_voices.push_back(voice);

	return (TVoiceHandle)voice;
}

void _stdcall VSTBase::Voice_Release(TVoiceHandle Handle)
{
	((PlugVoice*)Handle)->State = -1;
}

void _stdcall VSTBase::Voice_Kill(TVoiceHandle Handle)
{
	for(int i = 0; i < _voices.size(); i++)
	{
		if(_voices[i] == (PlugVoice*)Handle)
		{
			int note = _voices[i]->Params->InitLevels.Pitch;
			_parent->noteOff(note, 0, (void*)Handle);

			delete _voices[i];
			_voices.erase(_voices.begin() + i);
			return;
		}
	}
}

void VSTBase::KillAllVoices()
{
	while(_voices.size() > 0) 
	{
		PlugHost->Voice_Kill(_voices[0]->HostTag, true);
	}
}


void _stdcall VSTBase::GetName(int Section, int Index, int Value, char *Name)
{
	if (Section == FPN_Param) 
		_parent->getParameterName(Index, Name);
	else if (Section == FPN_ParamValue)
		_parent->getParameterValue(Index, Name);
	else if (Section == FPN_VoiceLevel || Section == FPN_VoiceLevelHint)
	{
		if (Index == 0)
		{
			char nameString[32] = "Note control 1";
			strncpy(Name, nameString, 32);
		}
		if (Index == 1) 
		{
			char nameString[32] = "Note control 2";
			strncpy(Name, nameString, 32);
		}
	}
	else if (Section == FPN_VoiceColor)
		_parent->getChannelName(Index, Name);
	else if (Section == FPN_Preset)
		strcpy(Name, _parent->getPatch(Index)->Name.c_str());
}

int _stdcall VSTBase::ProcessEvent(int EventID, int EventValue, int Flags)
{
	if(EventID == FPE_Tempo)
	{
		T32Bit bit;
		bit.i = EventValue;
		_tempo = bit.s;
		_samplesPerTick = Flags;
	}
	return 0;
}


int _stdcall VSTBase::ProcessParam(int Index, int Value, int RECFlags)
{
	if (RECFlags & REC_PlugReserved)
	{
		if (Index <= kExtParamsEnd)
		{
			int realIndex = Index;
			if (realIndex < kOriginalParamsEnd)
				realIndex += GennyPatch::getNumParameters() * _parent->getPatchIndex(_parent->getCurrentPatch());

			PlugHost->OnParamChanged(HostTag, realIndex, Value);
		}

		setParameter(Index, Value);

		return 0;
	}

	if (Index > kExtParamsEnd)
		return 0;

	int ret = _parent->onAutomation(Index, Value, (AutomationTypeFlags)RECFlags);
	if (ret < 0)
		return TCPPFruityPlug::ProcessParam(Index, Value, RECFlags);

	return ret;

	///*if (Index >= 99999999)
	//{
	//	Index -= 99999999;
	//	newAutomation = true;
	//	return _parent->RunNewAutomation(Index, Value, RECFlags);
	//}
	//else */if(Index >= 100000)
	//{
	//	Index -= 100000;
	//	isCurrentPatch = true;
	//}
	//else
	//{
	//	isAutomation = true;
	//	return _parent->RunNewAutomation(Index, Value, RECFlags);
	//}

	//if(Index >= PlugInfo.NumParams)
	//{
	//	setParameter(Index, Value);
	//	return 0;
	//}

	//int patchNum = Index / _totalParameters;
	//Index = Index % _totalParameters;
	//if(isCurrentPatch)
	//	patchNum = _parent->getPatchIndex(_parent->getCurrentPatch());

	//if (Index == 151 || Index == 152)
	//	patchNum = 0;

	//VSTPatch* patch = _parent->getPatch(patchNum);
	//if(isAutomation && patchNum < 16)
	//{	
	//	int actualPatch = ((GennyPatch*)_parent->getPatch(0))->Instruments[patchNum];
	//	//if(((GennyPatch*)_parent->getPatch(1))->Instruments[patchNum] >= 0)
	//	//	actualPatch = ((GennyPatch*)_parent->getPatch(1))->Instruments[patchNum];

	//	patch = _parent->getPatch(actualPatch);
	//	patchNum = actualPatch;
	//}

	//int paramRange = _parent->getParameterRange(Index % _totalParameters);
	//int ret = Value;
	//if(RECFlags & REC_FromMIDI)
	//	ret = (int)(((Value / 65536.0f) * paramRange) + 0.5f); 

	//if( RECFlags & REC_UpdateValue )
	//	setParameter(Index, (float)ret, patch);
	//if( RECFlags & REC_GetValue )
	//	ret = (int)getParameter(Index, patch);
	//if(_editor != nullptr && _parent->getPatchIndex(patch) == patchNum && RECFlags & REC_UpdateControl)
	//	_editor->setParameter(Index, getParameter(Index));

	//return ret;
}

int _stdcall VSTBase::Voice_ProcessEvent(TVoiceHandle Handle, int EventID, int EventValue, int Flags)
{
	return 0;
}

void _stdcall VSTBase::Idle_Public()
{
	if(_editor) _editor->idle();
	TCPPFruityPlug::Idle_Public();
}

void VSTBase::setEditor(void* editor)
{
	_editor = static_cast<PluginGUIEditor*>(editor);
}


void VSTBase::sendMidiMessage(char cc, int value, int channel, int port)
{
	TMIDIOutMsg msg;
	msg.Port = port;
	msg.Data1 = cc << 1;
	msg.Data2 = value << 1;
	msg.Status = (channel << 4) | 13;


	msg.Data1 = value;
	msg.Data2 = 255;
	msg.Status = (unsigned char)((0xB0 << 4) | 1);

	
	msg.Status = 0xB0;
	msg.Data1 = cc;
	msg.Data2 = value;
	msg.Port = 0;
	 

	int data = *(int*)&msg ;

	_host->MIDIOut_Delayed(HostTag, data);
}

float VSTBase::getParameter (int index, VSTPatch* patch)
{
	return _parent->getParameter(index, patch);
}

void VSTBase::setParameter (int index, float value, VSTPatch* patch)
{
	_parent->setParameter(index, value, patch);
}

void VSTBase::MidiLearn(int paramTag, int legacyTag)
{
	_parent->MidiLearn(paramTag, legacyTag);
}

void VSTBase::MidiForget(int paramTag, int legacyTag)
{
	_parent->MidiForget(paramTag, legacyTag);
}

void VSTBase::NewTick()
{
	_parent->midiTick();
}
void VSTBase::MIDIIn(int &Msg)
{
	int qq = 1;
}
void VSTBase::MsgIn(int Msg)
{
	int qq = 1;
}


void VSTBase::MidiFlush()
{

}

void _stdcall VSTBase::DestroyObject()
{
	KillAllVoices();
	if (_parent != nullptr)
		_parent->destroy();

	_parent = nullptr;
}


//VstInt32 VSTBase::processEvents (VstEvents* ev)
//{
//	for (VstInt32 i = 0; i < ev->numEvents; i++)
//	{
//		if ((ev->events[i])->type != kVstMidiType)
//			continue;
//
//		VstMidiEvent* event = (VstMidiEvent*)ev->events[i];
//		unsigned char* midiData = (unsigned char*)event->midiData;
//		VstInt32 status = midiData[0];	// ignoring channel
//
//		if (status > 127 && status < 160 )	// we only look at notes
//		{
//			VstInt32 note = midiData[1];// & 0x7f;
//			VstInt32 velocity = midiData[2];// & 0x7f;
//			VstInt32 midiChannel = 0;
//			if (status > 127 && status < 144)
//			{
//				velocity = 0;	// note off by velocity 0
//				midiChannel = status - 128;
//			}
//			else if( status > 143 && status < 160 )
//			{
//				midiChannel = status - 144;
//			}
//
//			if (!velocity)
//			{
//				_parent->noteOff(note, midiChannel);
//			}
//			else
//			{
//				_parent->noteOn(note, velocity, midiChannel);
//			}
//		}
//		else if (status > 175 && status < 192)
//		{
//			if (midiData[1] == 0x7e || midiData[1] == 0x7b)
//			{
//				_parent->clearNotes();
//			}
//			else
//			{
//				/*int num = midiData[1];
//				int value = midiData[2];
//				YM2612Value* learn = _stateData.GetMidiLearn();
//				if( learn != nullptr )
//				{
//					learn->SetMidiLearn(false);
//					learn->SetMidiHookup(num);
//				}
//
//				_parmAdjust = true;
//				std::vector<YM2612Value*> hooks;
//				_stateData.GetMidiHookups(num, hooks);
//				for( int i = 0; i < (int)hooks.size(); i++ )
//				{
//					setParameter( hooks[i]->GetIndex(), (float)value / (float)127 );
//				}
//				_parmAdjust = false;*/
//			}
//		}
//		else if( status >= 0xE0 && status <= 0xE5 )
//		{
//			//Pitch Wheel
//			/*int channel = status - 0xE0;
//			_parmAdjust = true;
//			setParameter(_stateData.Channels[channel].P_FREQBEND.GetIndex(), (float)midiData[2] / 127);
//			_parmAdjust = false;*/
//		}
//
//		event++;
//	}
//	return 1;
//}
//
//float VSTBase::getParameter (VstInt32 index)
//{
//	return _parent->getParameter(index);
//}
//
//void VSTBase::setParameter (VstInt32 index, float value)
//{
//	_parent->setParameter(index, value);
//}
//
//void VSTBase::setProgram(VstInt32 program)
//{
//	curProgram = program;
//	_parent->setPatchIndex(program);
//}
//
//void VSTBase::setProgramName (char* name)
//{
//	VSTPatch* current = _parent->getCurrentPatch();
//	if(current != NULL)
//		current->Name = name;
//}
//
//void VSTBase::getProgramName (char* name)
//{
//	VSTPatch* current = _parent->getCurrentPatch();
//	if(current != NULL)
//		vst_strncpy (name, current->Name.c_str(), kVstMaxProgNameLen);
//}
//
//bool VSTBase::getProgramNameIndexed (VstInt32 category, VstInt32 index, char* text)
//{
//	if (index < _parent->getNumPatches())
//	{
//		strcpy (text, _parent->getPatch(index)->Name.c_str());
//		return true;
//	}
//	return false;
//}
//
//bool VSTBase::getEffectName (char* name)
//{
//	std::string sname = _parent->getEffectName();
//	strcpy (name, sname.c_str());
//	return true;
//}
//bool VSTBase::getProductString (char* text)
//{
//	std::string sname = _parent->getProductName();
//	strcpy (text, sname.c_str());
//	return true;
//}
//bool VSTBase::getVendorString (char* text)
//{
//	std::string sname = _parent->getAuthorName();
//	strcpy (text, sname.c_str());
//	return true;
//}
//
//VstInt32 VSTBase::getNumMidiInputChannels()
//{
//	return 1;
//}
//
//VstInt32 VSTBase::getNumMidiOutputChannels()
//{
//	return 0;
//}
//
//VstInt32 VSTBase::getMidiProgramName (VstInt32 channel, MidiProgramName* mpn)
//{
//	VstInt32 prg = mpn->thisProgramIndex;
//	if (prg < 0 || prg >= 128)
//		return 0;
//	FillProgram (channel, prg, mpn);
//	if (channel == 9)
//		return 1;
//	return 128L;
//}
//
//VstInt32 VSTBase::getCurrentMidiProgram (VstInt32 channel, MidiProgramName* mpn)
//{
//	if (channel < 0 || channel >= 16 || !mpn)
//		return -1;
//	VstInt32 prg = _channelPrograms[channel];
//	mpn->thisProgramIndex = prg;
//	FillProgram (channel, prg, mpn);
//	return prg;
//}
//
//void VSTBase::FillProgram (VstInt32 channel, VstInt32 prg, MidiProgramName* mpn)
//{
//	mpn->midiBankMsb =
//		mpn->midiBankLsb = -1;
//	mpn->reserved = 0;
//	mpn->flags = 0;
//
//	if (channel == 9)	// drums
//	{
//		vst_strncpy (mpn->name, "Standard", 63);
//		mpn->midiProgram = 0;
//		mpn->parentCategoryIndex = 0;
//	}
//	else
//	{
//		vst_strncpy (mpn->name, GmNames[prg], 63);
//		mpn->midiProgram = (char)prg;
//		mpn->parentCategoryIndex = -1;	// for now
//
//		for (VstInt32 i = 0; i < kNumGmCategories; i++)
//		{
//			if (prg >= GmCategoriesFirstIndices[i] && prg < GmCategoriesFirstIndices[i + 1])
//			{
//				mpn->parentCategoryIndex = i;
//				break;
//			}
//		}
//	}
//}
//
//VstInt32 VSTBase::getMidiProgramCategory (VstInt32 channel, MidiProgramCategory* cat)
//{
//	cat->parentCategoryIndex = -1;	// -1:no parent category
//	cat->flags = 0;					// reserved, none defined yet, zero.
//	VstInt32 category = cat->thisCategoryIndex;
//	if (channel == 9)
//	{
//		vst_strncpy (cat->name, "Drums", 63);
//		return 1;
//	}
//	if (category >= 0 && category < kNumGmCategories)
//		vst_strncpy (cat->name, GmCategories[category], 63);
//	else
//		cat->name[0] = 0;
//	return kNumGmCategories;
//}
//
//bool VSTBase::hasMidiProgramsChanged (VstInt32 channel)
//{
//	return false;	// updateDisplay ()
//}
//
//bool VSTBase::getMidiKeyName (VstInt32 channel, MidiKeyName* key)
//	// struct will be filled with information for 'thisProgramIndex' and 'thisKeyNumber'
//	// if keyName is "" the standard name of the key will be displayed.
//	// if false is returned, no MidiKeyNames defined for 'thisProgramIndex'.
//{
//	// key->thisProgramIndex;		// >= 0. fill struct for this program index.
//	// key->thisKeyNumber;			// 0 - 127. fill struct for this key number.
//	key->keyName[0] = 0;
//	key->reserved = 0;				// zero
//	key->flags = 0;					// reserved, none defined yet, zero.
//	return false;
//}
//
//
//VstInt32 VSTBase::canDo (char* text)
//{
//	if (!strcmp (text, "receiveVstEvents"))
//		return 1;
//	if (!strcmp (text, "receiveVstMidiEvent"))
//		return 1;
//	if (!strcmp (text, "midiProgramNames"))
//		return 1;
//	return -1;	// explicitly can't do; 0 => don't know
//}
//
//bool VSTBase::getOutputProperties(VstInt32 index, VstPinProperties* properties)
//{
//	if( index < 2 )
//	{
//		vst_strncpy (properties->label, "Vstx ", 63);
//		char temp[11] = {0};
//		int2string (index + 1, temp, 10);
//		vst_strncat (properties->label, temp, 63);
//
//		properties->flags = kVstPinIsActive;
//		if (index < 2)
//			properties->flags |= kVstPinIsStereo;	// make channel 1+2 stereo
//		return true;
//	}
//	return false;
//}
#endif