#include "GennyInterface.h"
#include "../resource.h"
#include "UIBitmap.h"
#include "IndexBaron.h"
#include "UIInstrument.h"
#include "UIChannelStatus.h"
#include "UIPresetsAndInstrumentsPanel.h"
#include "GennyVST.h"
#include <fstream>
#include "GennyLoaders.h"
#include "UIContextMenu.h"
#include "UIPESelectedInstrument.h"
#include "UIInstrumentsPanel.h"

#include <sstream>
#include <string>


CMessageResult VSTFrame::notify (CBaseObject* sender, IdStringPtr message)
{
	if(message == CNewFileSelector::kSelectEndMessage)
	{
		CNewFileSelector* selector = (CNewFileSelector*)sender;
		if(selector->getNumSelectedFiles() > 0)
		{
			_owner->fileDialogFinished(selector);
			//_owner->OpenTYICallback(str);




		}
		int qq = 1;
	}
	return kMessageNotified;
}

CMouseEventResult VSTFrame::onMouseUp (CPoint& where, const CButtonState& buttons)
{

	CMouseEventResult res = __super::onMouseUp( where, buttons );
	if(where.x == -9999 && where.y == -9999)
		return res;

	_owner->dragEnd();
	//if (res == CMouseEventResult::kMouseEventNotHandled)
	//	_owner->MenuClearCallback();
	if(buttons.isRightButton())
	{
		if(_menu == nullptr)
		{
			
			
		}


		
	}

	return res;
}

int32_t VSTFrame::onKeyDown (VstKeyCode& keyCode)
{
	return 0;
}

int32_t VSTFrame::onKeyUp (VstKeyCode& keyCode)
{
	return 0;
}

GennyInterface::GennyInterface(void* effect, GennyVST* owner)
	 : EditorBaseClass (effect),
	   GennyInterfaceObject(nullptr),
		_owner(owner),
		_instrumentUI(NULL),
		_patchEditor(NULL),
		_sync(false),
		_channelsChanged(false),
		_importTuning(nullptr),
		_importInstrument(nullptr),
		_exportInstrument(nullptr),
		_importBank(nullptr),
		_exportBank(nullptr),
		_importState(nullptr),
		_exportState(nullptr),
		_importSample(nullptr),
		_logging(false),
		_logExport(nullptr),
		_lock(false)
{
	_channels[0] = 0;
	_channels[1] = 0;
	_channels[2] = 0;
	_channels[3] = 0;
	_channels[4] = 0;
	_channels[5] = 0;
	_channels[6] = 0;
	_channels[7] = 0;
	_channels[8] = 0;
	_channels[9] = 0;
}

GennyInterface::~GennyInterface(void)
{
	if(frame != 0)
		close();
}

void GennyInterface::dragEnd()
{
	_patchEditor->getInstrumentView()->dragEnd();
}

bool GennyInterface::open(void* ptr)
{
#ifdef BUILD_VST
	AEffGUIEditor::open (ptr);
#else
	PluginGUIEditor::open (ptr);
#endif

	CRect size (0, 0, 954, 552);
	VSTFrame* f = new VSTFrame (size, ptr, this);
	f->SetOwner(this);
	f->setBackgroundColor (kWhiteCColor);

	this->rect.top=0;
	this->rect.left=0;
	this->rect.right=954;
	this->rect.bottom=552;

	UIBitmap back(PNG_VSTBACK);
	CView* view = new CView( CRect( 0, 0, back.getWidth(), back.getHeight() ) );
	view->setBackground (back);
	f->addView(view);
	frame = f;


	_interface = this;
	_vst = getVSTOwner();

	_instrumentUI = new UIInstrument(this);
	_instrumentUI->initialize();
	
	_patchEditor = new UIPresetsAndInstrumentsPanel(CRect(416, 96, 416 + 426, 96 + 186), this);

	_patchEditor->addConfirmDialog();

	return true;
}

void GennyInterface::close()
{
	CFrame* oldFrame = frame;
	frame = 0;
	oldFrame->forget ();
	
	delete _instrumentUI;
	_instrumentUI = NULL;

	if(_patchEditor != nullptr)
		_patchEditor->forget();

	_patchEditor = nullptr;

#ifdef BUILD_VST
	AEffGUIEditor::close();
#else
	PluginGUIEditor::close();
#endif
}

void GennyInterface::setParameter(GennyInt32 index, float value)
{
	if(frame && index < getIndexBaron()->getNumParams())
	{
		IBIndex* parm = getIndexBaron()->getIndex(index);
		if(parm->getType() == IB_YMParam || parm->getType() == IB_InsParam)
		{
			if(parm->getType() == IB_InsParam)
			{
				IBInsParam* p = (IBInsParam*)parm;
				if(p->getParameter() == GIP_Panning)
				{
					_patchEditor->getInstrumentView()->getSelectedInstrument()->reconnect();
					return;
				}
			}
			_instrumentUI->setParam(index, value);
			if(parm->getType() == IB_YMParam)
			{
				IBYMParam* p = (IBYMParam*)parm;
				if(p->getOperator() != -1)
				{
					_instrumentUI->makeOperatorDirty(p->getOperator());
					return;
				}
			}
		}
		else if(parm->getType() == IB_PatchParam)
			_patchEditor->setParam(index, value);
	}
}


void GennyInterface::valueChanged(CControl* control)
{
#ifdef BUILD_VST
	effect->setParameterAutomated(control->getTag (), control->getValue());
#else
	TFruityPlug* plug = static_cast<TFruityPlug*>(effect);
	
	int index = control->getTag();
	if(index >= 0)
	{
		float val = control->getValue();
		if(val < 0)
			plug->ProcessParam(index + 100000, static_cast<int>(val - 0.5f), REC_UpdateValue);
		else
			plug->ProcessParam(index + 100000, static_cast<int>(val + 0.5f), REC_UpdateValue);
	}
#endif
}

void GennyInterface::reconnect()
{
	if(_instrumentUI != NULL)
		_instrumentUI->reconnect();
	if(_patchEditor != NULL)
		_patchEditor->reconnect();
}

void GennyInterface::setChannelState(int channel, bool on)
{
	_sync = true;
	_channels[channel] = on;
	_channelsChanged = true;
	_sync = false;
}

void GennyInterface::idle()
{
	while(_lock){}
	_lock = true;
	EditorBaseClass::idle();
	if(_sync == false && _channelsChanged)
	{
		if(_instrumentUI)
			_instrumentUI->makeChannelsDirty();

		if(_patchEditor)
			_patchEditor->makeChannelsDirty();

		_channelsChanged = false;
	}

	if(_instrumentUI)
		_instrumentUI->updateChannels();

	if(_patchEditor)
		_patchEditor->updateInstrumentChannels();

	/*for(int i = 0; i < _updates.size(); i++)
	{
		int index = _updates[i].index;
		float value = _updates[i].value;
		if(frame && index < _baron->getNumParams())
		{
			IBIndex* parm = _baron->getIndex(index);
			if(parm->getType() == IB_YMParam || parm->getType() == IB_InsParam)
			{
				_instrumentUI->setParam(index, value);
			}
			else if(parm->getType() == IB_PatchParam)
			{
				_patchEditor->setParam(index, value);
			}
		}
	}
	_updates.clear();*/

	_lock = false;
}

void GennyInterface::openInstrumentImport()
{
	if(_importInstrument == nullptr)
	{
		_importInstrument = CNewFileSelector::create (frame);
		_importInstrument->setTitle("Import Instrument");
		_importInstrument->addFileExtension(CFileExtension("GENNY Instrument", "gen"));
		_importInstrument->addFileExtension(CFileExtension("YM2612 Instrument", "vgi"));
		_importInstrument->addFileExtension(CFileExtension("YM2612 Instrument", "tyi"));
		_importInstrument->addFileExtension(CFileExtension("YM2612 Instrument", "tfi"));
		_importInstrument->addFileExtension(CFileExtension("YM2612 Drums", "dpack"));
		_importInstrument->addFileExtension(CFileExtension("GENNY Drums", "dac"));
		_importInstrument->setDefaultExtension(CFileExtension("GENNY Instrument", "gen"));
		_importInstrument->run(frame);
		_importInstrument->forget();
		_importInstrument = nullptr;
	}
}

void GennyInterface::openTuningImport()
{
	if(_importTuning == nullptr)
	{
		_importTuning = CNewFileSelector::create (frame);
		_importTuning->setTitle("Import Tuning");
		_importTuning->addFileExtension(CFileExtension("Tuning File", "tun"));
		_importTuning->setDefaultExtension(CFileExtension("Tuning File", "tun"));
		_importTuning->run(frame);
		_importTuning->forget();
		_importTuning = nullptr;
	}
}

void GennyInterface::openInstrumentExport()
{
	if(_exportInstrument == nullptr)
	{
		_exportInstrument = CNewFileSelector::create (frame, CNewFileSelector::Style::kSelectSaveFile);
		_exportInstrument->setTitle("Export Instrument");


		std::vector<VSTPatch*> patches = getVSTOwner()->getPatches();
		GennyPatch* patch0 = static_cast<GennyPatch*>(_owner->getPatch(0));
		int selectedIndex = (int)patch0->Instruments[patch0->SelectedInstrument];
		GennyPatch* currentPatch = (GennyPatch*)patches[selectedIndex];
		_exportInstrument->setDefaultSaveName((currentPatch->Name).c_str());
		if(currentPatch->InstrumentDef.Type == GIType::DAC)
		{
			_exportInstrument->addFileExtension(CFileExtension("GENNY Instrument", "gen"));
			_exportInstrument->addFileExtension(CFileExtension("GENNY Drumset", "dac"));
		}
		else if(currentPatch->InstrumentDef.Type == GIType::SN || currentPatch->InstrumentDef.Type == GIType::SNDRUM)
		{
			_exportInstrument->addFileExtension(CFileExtension("GENNY Instrument", "gen"));
		}
		else
		{
			_exportInstrument->addFileExtension(CFileExtension("GENNY Instrument", "gen"));
			_exportInstrument->addFileExtension(CFileExtension("YM2612 Instrument", "vgi"));
			_exportInstrument->addFileExtension(CFileExtension("YM2612 Instrument", "tyi"));
			_exportInstrument->addFileExtension(CFileExtension("YM2612 Instrument", "tfi"));
		}
			
		_exportInstrument->setDefaultExtension(CFileExtension("GENNY Instrument", "gen"));

		_exportInstrument->run(frame);
		_exportInstrument->forget();
		_exportInstrument = nullptr;
	}
}

void GennyInterface::openBankImport()
{
	if(_importBank == nullptr)
	{
		_importBank = CNewFileSelector::create (frame);
		_importBank->setTitle("Import Bank");
		_importBank->addFileExtension(CFileExtension("GENNY Bank", "gnb"));
		_importBank->addFileExtension(CFileExtension("YM2612 Bank", "bnk"));
		_importBank->addFileExtension(CFileExtension("YM2612 Tiido Bank", "tyi"));
		_importBank->setDefaultExtension(CFileExtension("GENNY Bank", "gnb"));
		_importBank->run(frame);
		_importBank->forget();
		_importBank = nullptr;
	}
}

void GennyInterface::openBankExport()
{
	if(_exportBank == nullptr)
	{
		_exportBank = CNewFileSelector::create (frame, CNewFileSelector::Style::kSelectSaveFile);
		_exportBank->setTitle("Export Bank");
		_exportBank->setDefaultSaveName("bank");
		_exportBank->addFileExtension(CFileExtension("GENNY Bank", "gnb"));
		_exportBank->addFileExtension(CFileExtension("YM2612 Bank", "bnk"));
		_exportBank->addFileExtension(CFileExtension("YM2612 Tiido Bank", "tyi"));
		_exportBank->setDefaultExtension(CFileExtension("GENNY Bank", "gnb"));
		_exportBank->run(frame);
		_exportBank->forget();
		_exportBank = nullptr;
	}
}


void GennyInterface::openStateImport()
{
	if (_importState == nullptr)
	{		   
		_importState = CNewFileSelector::create(frame);
		_importState->setTitle("Load GENNY State");
		_importState->addFileExtension(CFileExtension("GENNY State", "gst"));
		_importState->setDefaultExtension(CFileExtension("GENNY State", "gst"));
		_importState->run(frame);
		_importState->forget();
		_importState = nullptr;
	}
}

void GennyInterface::openStateExport()
{
	if (_exportState == nullptr)
	{		   
		_exportState = CNewFileSelector::create(frame, CNewFileSelector::Style::kSelectSaveFile);
		_exportState->setTitle("Save GENNY State");
		_exportState->setDefaultSaveName("state");
		_exportState->addFileExtension(CFileExtension("GENNY State", "gst"));
		_exportState->setDefaultExtension(CFileExtension("GENNY State", "gst"));
		_exportState->run(frame);
		_exportState->forget();
		_exportState = nullptr;
	}
}

void GennyInterface::openLogExport()
{
	if(_logExport == nullptr)
	{
		_logExport = CNewFileSelector::create (frame, CNewFileSelector::Style::kSelectSaveFile);
		_logExport->setTitle("Export VGM");
		_logExport->addFileExtension(CFileExtension("VGM File", "vgm"));
		_logExport->setDefaultExtension(CFileExtension("VGM File", "vgm"));
		_logExport->run(frame);
		_logExport->forget();
		_logExport = nullptr;
	}
}

void GennyInterface::openImportSample(int sampleSlot)
{
	if(_importSample == nullptr)
	{
		_sampleSlot = sampleSlot;
		_importSample = CNewFileSelector::create (frame);
		_importSample->setTitle("Import Sample");
		_importSample->addFileExtension(CFileExtension("WAV Sample", "wav"));
		_importSample->setDefaultExtension(CFileExtension("WAV Sample", "wav"));
		_importSample->run(frame);
		_importSample->forget();
		_importSample = nullptr;
	}
}


std::string toLower(std::string up)
{
	for(int i = 0; i < up.length(); i++){
		up[i] = tolower(up[i]);
	}
	return up;
}

std::string doubleToString(double d)
{
	std::ostringstream ss;
	ss << d;
	return ss.str();
}

std::string getFileName(const std::string& s) {

	char sep = '/';

#ifdef _WIN32
	sep = '\\';
#endif

	size_t i = s.rfind(sep, s.length());
	if (i != std::string::npos) {
		return(s.substr(i+1, s.length() - i));
	}

	return("");
}

static char * ReadAllBytes(const char * filename, int * read)
{
    std::ifstream ifs(filename, std::ios::binary|std::ios::ate);
    std::ifstream::pos_type pos = ifs.tellg();

    // What happens if the OS supports really big files.
    // It may be larger than 32 bits?
    // This will silently truncate the value/
    int length = pos;

    // Manuall memory management.
    // Not a good idea use a container/.
    char *pChars = new char[length];
    ifs.seekg(0, std::ios::beg);
    ifs.read(pChars, length);

    // No need to manually close.
    // When the stream goes out of scope it will close the file
    // automatically. Unless you are checking the close for errors
    // let the destructor do it.
    ifs.close();
    *read = length;
    return pChars;
}

void GennyInterface::fileDialogFinished(CNewFileSelector* dialog)
{	
	if(dialog == _importTuning)
	{
		std::string str = _importTuning->getSelectedFile(0);

		char* buff = new char[8192];
		char* keyBuffer = new char[256];

		std::string readString = "Exact Tuning";
		int read = GetPrivateProfileStringA(readString.c_str(), nullptr, "", buff, 8192, str.c_str()); 
		if(read <= 0)
		{
			readString = "Tuning";
			read = GetPrivateProfileStringA(readString.c_str(), nullptr, "", buff, 8192, str.c_str()); 
		}

		double* frequencyTable = new double[129];
		int keyIndex = 0;
		std::string currentKey = "";
		for(int i = 0; i < read; i++)
		{
			if(buff[i] == 0)
			{
				int keyRead = GetPrivateProfileStringA(readString.c_str(), currentKey.c_str(), "", keyBuffer, 256, str.c_str());
				if(keyRead > 0)
				{
					if(keyIndex == 0)
					{
						if(currentKey != "basefreq")
						{
							frequencyTable[keyIndex] = 8.17579891564371;
							keyIndex++;
						}
					}

					frequencyTable[keyIndex] = stod(std::string(keyBuffer));
					keyIndex++;
				}

				currentKey = "";
				continue;
			}
			currentKey += buff[i];
		}

		getVSTOwner()->setFrequencyTable(frequencyTable);
		delete[] buff;
		delete[] keyBuffer;

		reconnect();
	}
	else if(dialog == _importSample)
	{
		std::string str = _importSample->getSelectedFile(0);

		std::fstream file(str.c_str(), std::ios::binary | std::ios::in | std::ios::ate);
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

		WaveData* wave = new WaveData(&data);
		if(wave->valid)
		{
			wave->sampleName = getFileName(str);
			wave->sampleName = wave->sampleName.substr(0, wave->sampleName.length() - 4);



			GennyPatch* p = (GennyPatch*)getVst()->getCurrentPatch();
			//int numSilentFrames = 0;
			//int silentFramesStart = 0;
			//int detectSilentFramesStart = 0; 
			//int numNoisyFrames = 0; 
			//int noisyFramesStart = 0;
			//int detectNoisyFramesStart = 0;
			//for(int i = 0; i < wave->size; i++) 
			//{
			//	if(abs(wave->audioData[i] - 128) < 4)
			//	{ 
			//		if(numSilentFrames == 0)  
			//			detectSilentFramesStart = i;

			//		numSilentFrames += 1; 
			 
			//		if(numSilentFrames > 100)
			//		{
			//			silentFramesStart = detectSilentFramesStart; 
			//			if(numNoisyFrames > 1000)
			//			{
			//				p->InstrumentDef.Drumset.mapDrum(36 + _sampleSlot, wave->GetChunk(noisyFramesStart, silentFramesStart));
			//				_sampleSlot += 1;
			 
			//				numNoisyFrames = 0;
			//				numSilentFrames = 0; 
			//			}
			//			numNoisyFrames = 0;
			//		}
			//	}
			//	else
			//	{
			//		if(numNoisyFrames == 0)
			//			detectNoisyFramesStart = i;

			//		numNoisyFrames += 1;
			//		numSilentFrames = 0; 

			//		if(numNoisyFrames > 400)
			//		{
			//			numSilentFrames = 0;
			//			noisyFramesStart = detectNoisyFramesStart;
			//		}
			//	}
			//}

			p->InstrumentDef.Drumset.mapDrum(36 + _sampleSlot, wave);
		}

		delete[] memblock;


		reconnect();
	}
	else if(dialog == _importInstrument)
	{
		std::string str = _importInstrument->getSelectedFile(0);

		int dot = str.rfind(".");
		int slash = str.rfind("\\");
		if(slash == -1)
			slash = str.rfind("/");
		std::string ext = toLower(str.substr(dot + 1, 3));
		std::string name = str.substr(slash + 1, (str.length() - slash) - 5);

		std::vector<VSTPatch*> patches = getVSTOwner()->getPatches();

		GennyPatch* patch0 = static_cast<GennyPatch*>(_owner->getPatch(0));

		int selectedIndex = (int)patch0->Instruments[patch0->SelectedInstrument];
		patches[selectedIndex]->Name = name;

		GennyPatch* patch = static_cast<GennyPatch*>(patches[selectedIndex]);
		patch->InstrumentDef.Type = GIType::FM;


		GennyPatch* instrumentDefinitionPatch = static_cast<GennyPatch*>(patches[patch0->SelectedInstrument]);


		std::fstream file(str.c_str(), std::ios::binary | std::ios::in | std::ios::ate);
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

		GennyPatch* newPatch = nullptr;
		
		if(ext == "dac")
		{
			GennyLoaders::loadGDAC(patch, &data, true);
			patch->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex((YM2612Param)YM_DRUMTL, 3)), YM2612Param_getRange(YM_DRUMTL) - 27);

			for(int i = 0; i < 10; i++)
				Genny2612::channelDirty[i] = true;

			_owner->rejiggerInstruments(true);
			reconnect();
			return;
		}
		else if(ext == "gen")
		{
			newPatch = GennyLoaders::loadGEN(name, "", patch, &data, true);

			if(newPatch->InstrumentDef.Type == GIType::DAC)
				patch->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex((YM2612Param)YM_DRUMTL, 3)), YM2612Param_getRange(YM_DRUMTL) - 27);

			for(int i = 0; i < 10; i++)
				Genny2612::channelDirty[i] = true;

			_owner->rejiggerInstruments(true);
			reconnect();
			return;
		}
		else if(ext == "vgi")
			newPatch = GennyLoaders::loadVGI(name, "", data);
		else if(ext == "tfi")
			newPatch = GennyLoaders::loadTFI(name, data);
		else if(ext == "tyi")
			newPatch = GennyLoaders::loadTYI(name, data);
		else if(ext == "dpa")
		{
			name = name.substr(0, name.length() - 2);
			patch->InstrumentDef.Type = GIType::DAC;
			patches[selectedIndex]->Name = name;

			patch->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex((YM2612Param)YM_TL, 3)), YM2612Param_getRange(YM_TL) - 27);

			int pos = 0;
			newPatch = GennyLoaders::loadDPACK(name, data, &pos);
			patch->InstrumentDef.Drumset = newPatch->InstrumentDef.Drumset;
			patch->InstrumentDef.setSamplePath(str.c_str());
		}

		IndexBaron* baron = getIndexBaron();
		int count = GennyPatch::getNumParameters();
		for(int i = 0; i < count; i++)
		{
			IBIndex* index = baron->getIndex(i);
			if(index->getType() == IB_YMParam)
			{
#ifdef BUILD_VST
				effect->setParameterAutomated(i,  newPatch->getFromBaron(index));
#else
				TFruityPlug* plug = static_cast<TFruityPlug*>(effect);
				plug->ProcessParam(i + 100000, static_cast<int>(newPatch->getFromBaron(index)), REC_UpdateValue);
#endif
			}
		}

		delete newPatch;

		_owner->rejiggerInstruments(true);
		reconnect();

		//_selectedInstrument->setText(patches[selectedIndex]->Name.c_str());


		int qq = 1;
	}
	if(dialog == _exportInstrument)
	{
		std::string str = _exportInstrument->getSelectedFile(0); 

		int dot = str.rfind(".");
		int slash = str.rfind("\\");
		if(slash == -1)
			slash = str.rfind("/");


		std::vector<VSTPatch*> patches = getVSTOwner()->getPatches();
		GennyPatch* patch0 = static_cast<GennyPatch*>(_owner->getPatch(0));
		int selectedIndex = (int)patch0->Instruments[patch0->SelectedInstrument];

		GennyPatch* currentPatch = (GennyPatch*)patches[selectedIndex];


		std::string ext = "vgi";
		if(currentPatch->InstrumentDef.Type == GIType::DAC)
		{
			ext = "dac";
			if(dot != -1)
				ext = toLower(str.substr(dot + 1, 3));
			else
			{
				str += ".dac";
			}
		}
		else
		{
			if(dot != -1)
				ext = toLower(str.substr(dot + 1, 3));
			else
			{
				str += ".vgi";
			}
		}

		std::string name = str.substr(slash + 1, (str.length() - slash) - 5);
		bool canExport = false;

		GennyData dat;
		if(ext == "dac")
		{
			GennyLoaders::saveGDAC(static_cast<GennyPatch*>(currentPatch), &dat);
			canExport = true;
		}
		else if(ext == "gen")
		{
			dat = GennyLoaders::saveGEN(static_cast<GennyPatch*>(currentPatch));
			canExport = true;
		}
		else if(ext == "vgi")
		{
			dat = GennyLoaders::saveVGI(static_cast<GennyPatch*>(currentPatch));
			canExport = true;
		}
		else if(ext == "tfi")
		{
			dat = GennyLoaders::saveTFI(static_cast<GennyPatch*>(currentPatch));
			canExport = true;
		}
		else if(ext == "tyi")
		{
			dat = GennyLoaders::saveTYI(static_cast<GennyPatch*>(currentPatch));
			canExport = true;
		}

		if(canExport)
		{
			std::fstream file;
			file.open(str,std::ios::out | std::ios::binary | std::ios::trunc);
			file.write(dat.data, dat.size);
			file.close();
			delete[] dat.data;
		}
	}
	if(dialog == _importBank)
	{
		std::string str = _importBank->getSelectedFile(0);

		int dot = str.rfind(".");
		int slash = str.rfind("\\");
		if(slash == -1)
			slash = str.rfind("/");
		std::string ext = toLower(str.substr(dot + 1, 3));
		std::string name = str.substr(slash + 1, (str.length() - slash) - 5);


		int current = _owner->getPatchIndex(_owner->getCurrentPatch());

		if(ext == "gnb")
		{
			int fileSize = 0;
			char* readFile = ReadAllBytes(str.c_str(), &fileSize);
			GennyData file;
			file.data = readFile;

			int numPatches = file.readInt();
			for(int i = 0; i < numPatches; i++)
			{
				if(i >= getVSTOwner()->_patches.size())
					getVSTOwner()->_patches.push_back(new GennyPatch());

				int size = file.readInt();

				GennyData dat;
				dat.data = new char[size];
				memcpy(dat.data, file.data + file.dataPos, size);
				dat.size = size;
				file.dataPos += size;
	
				GennyPatch* newPatch = GennyLoaders::loadGEN("", "", (GennyPatch*)getVSTOwner()->_patches[i], &dat); 

				if(newPatch->InstrumentDef.Type == GIType::DAC)
					newPatch->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex((YM2612Param)YM_TL, 3)), YM2612Param_getRange(YM_TL) - 27);

				delete[] dat.data;
			}

			_owner->rejiggerInstruments(false);
			reconnect();
			delete[] readFile;
		}
		else if(ext == "bnk" || ext == "tyi") 
		{
			std::fstream file(str.c_str(), std::ios::binary | std::ios::in);
			std::vector<VSTPatch*> patches = getVSTOwner()->getPatches();
			for(int i = 0; i < patches.size(); i++)
			{
				if(static_cast<GennyPatch*>(patches[i])->InstrumentDef.Type == GIType::DAC)
					continue;

				if(!file.good())
					break;

				char* memblock = new char[32];
				file.read(memblock, 32);
				GennyData dat;
				dat.data = memblock;
				dat.size = 32;


				char buf[4];
				itoa(i, buf, 10);

				GennyPatch* newPatch = GennyLoaders::loadTYI(name + " " + buf, dat);

				IndexBaron* baron = getIndexBaron();
				int count = GennyPatch::getNumParameters();

				patches[i]->Name = newPatch->Name;

#ifdef BUILD_VST
				effect->setParameterAutomated(kPresetControlIndex, i);
				for(int j = 0; j < count; j++)
				{
					IBIndex* index = baron->getIndex(j);
					if(index->getType() == IB_YMParam)
					{
					effect->setParameterAutomated(j,  newPatch->getFromBaron(index));
					}
				}
#else
				TFruityPlug* plug = static_cast<TFruityPlug*>(effect);
				plug->ProcessParam(kPresetControlIndex, i, REC_UpdateValue);
				for(int j = 0; j < count; j++)
				{
					IBIndex* index = baron->getIndex(j);
					if(index->getType() == IB_YMParam)
					{
						plug->ProcessParam(j + 100000,  newPatch->getFromBaron(index), REC_UpdateValue);
					}
				}
#endif
				delete newPatch;
			}
			
			
			for(int i = 0; i < 10; i++)
				Genny2612::channelDirty[i] = true;

			file.close();
#ifdef BUILD_VST
			effect->setParameterAutomated(kPresetControlIndex, current);
#else
			TFruityPlug* plug = static_cast<TFruityPlug*>(effect);
			plug->ProcessParam(kPresetControlIndex, current, REC_UpdateValue);
#endif
			_owner->rejiggerInstruments(false);
			reconnect();
		}
	}
	if(dialog == _exportBank)
	{
		std::string str = _exportBank->getSelectedFile(0);

		int dot = str.rfind(".");
		int slash = str.rfind("\\");
		if(slash == -1)
			slash = str.rfind("/");


		std::string ext = "bnk";
		if(dot != -1)
			ext = toLower(str.substr(dot + 1, 3));
		else
		{
			str += ".bnk";
		}
		std::string name = str.substr(slash + 1, (str.length() - slash) - 5);
		
		bool canExport = false;
		std::vector<VSTPatch*> patches = getVSTOwner()->getPatches();
		GennyPatch* patch0 = static_cast<GennyPatch*>(_owner->getPatch(0));
		int selectedIndex = (int)patch0->Instruments[patch0->SelectedInstrument];

		GennyData dat;

		if(ext == "gnb")
		{
			GennyData dat;
			std::vector<VSTPatch*> patches = getVSTOwner()->getPatches();

			dat.writeInt(patches.size());
			for(int i = 0; i < patches.size(); i++)
			{
				GennyData data = GennyLoaders::saveGEN(static_cast<GennyPatch*>(patches[i]));

				dat.writeInt(data.size);
				dat.writeBytes(data.data, data.size);
			}

			std::fstream file(str.c_str(), std::ios::binary | std::ios::out | std::ios::trunc);
			file.write(dat.data, dat.size);
			file.close();
		}
		else if(ext == "tyi" || ext == "bnk")
		{
			std::fstream file(str.c_str(), std::ios::binary | std::ios::out | std::ios::trunc);
			std::vector<VSTPatch*> patches = getVSTOwner()->getPatches();
			for(int i = 0; i < patches.size(); i++)
			{
				if(static_cast<GennyPatch*>(patches[i])->InstrumentDef.Type == GIType::DAC)
					continue;

				GennyData data = GennyLoaders::saveTYI(static_cast<GennyPatch*>(patches[i]));
				file.write(data.data, data.size);
			}
			file.close();
		}
	}
	else if (dialog == _importState)
	{
		std::string str = _importState->getSelectedFile(0);

		int dot = str.rfind(".");
		int slash = str.rfind("\\");
		if (slash == -1)
			slash = str.rfind("/");
		std::string ext = toLower(str.substr(dot + 1, 3));
		std::string name = str.substr(slash + 1, (str.length() - slash) - 5);

		int fileSize = 0;
		char* readFile = ReadAllBytes(str.c_str(), &fileSize);
		getVSTOwner()->setPluginState((void*)readFile, fileSize, false);

		delete[] readFile;
		reconnect();
		return;
	}
	else if (dialog == _exportState)
	{
		std::string str = _exportState->getSelectedFile(0);

		int dot = str.rfind(".");
		int slash = str.rfind("\\");
		if (slash == -1)
			slash = str.rfind("/");

		std::string ext = "gst";
		if (dot != -1)
			ext = toLower(str.substr(dot + 1, 3));
		else
		{
			str += ".gst";
		}
		std::string name = str.substr(slash + 1, (str.length() - slash) - 5);

		bool canExport = false;


		void* data = nullptr;
		int stateSize = getVSTOwner()->getPluginState(&data, false);
		int written = 0;
		std::fstream file(str.c_str(), std::ios::binary | std::ios::out | std::ios::trunc);
		file.write((char*)data, stateSize);
		file.close();
	}
	if(dialog == _logExport)
	{
		std::string str = _logExport->getSelectedFile(0);

		int dot = str.rfind(".");
		int slash = str.rfind("\\");
		if(slash == -1)
			slash = str.rfind("/");


		std::string ext = "vgm";
		if(dot != -1)
			ext = toLower(str.substr(dot + 1, 3));
		else
		{
			str += ".vgm";
		}
		std::string name = str.substr(slash + 1, (str.length() - slash) - 5);
		
		_logging = true;
		if(_patchEditor != nullptr)
			_patchEditor->reconnect();

		_owner->startLogging(str);
	}
}
