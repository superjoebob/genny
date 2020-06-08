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

#include <gzip/compress.hpp>
#include <gzip/config.hpp>
#include <gzip/decompress.hpp>
#include <gzip/utils.hpp>
#include <gzip/version.hpp>

//#include "vgm2pre.hpp"
//#include <stdio.h>

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
	//if(frame != 0)
	//	close();
}

void GennyInterface::dragEnd()
{
	_patchEditor->getInstrumentView()->dragEnd();
} 

bool GennyInterface::open(void* ptr)
{
#ifdef BUILD_VST
	AEffGUIEditor::open(ptr);
#else
	PluginGUIEditor::open(ptr); 
#endif



	CRect size (0, 0, 954, 586);
	VSTFrame* f = new VSTFrame (size, ptr, this);
	f->SetOwner(this); 
	f->setBackgroundColor (kWhiteCColor);

	this->rect.top=0;
	this->rect.left=0;
	this->rect.right=954; 
	this->rect.bottom= 586;


	UIBitmap back(PNG_VSTBACK);
	CView* view = new CView( CRect( 0, 0, back.getWidth(), 586 ) );
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
	delete _instrumentUI;
	_instrumentUI = nullptr;
	_patchEditor = nullptr;

#ifdef BUILD_VST
	AEffGUIEditor::close();
#else
	PluginGUIEditor::close();
#endif

	CFrame* oldFrame = frame;
	frame = 0;
	oldFrame->forget();
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
		//_importInstrument->addFileExtension(CFileExtension("YM2612 Drums", "dpack"));
		//_importInstrument->addFileExtension(CFileExtension("GENNY Drums", "dac"));
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
			//_exportInstrument->addFileExtension(CFileExtension("GENNY Drumset", "dac"));
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
		_importBank->addFileExtension(CFileExtension("VGZ Music File", "vgz"));
		_importBank->addFileExtension(CFileExtension("VGM Music File", "vgm"));
		//_importBank->setDefaultExtension(CFileExtension("GENNY Bank", "gnb"));
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
		_importSample->addFileExtension(CFileExtension("11025hz or less WAVE", "wav"));
		_importSample->setDefaultExtension(CFileExtension("11025hz or less WAVE", "wav"));
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

//static alloc_func zalloc = (alloc_func)0;
//static free_func zfree = (free_func)0;
//#define CHECK_ERR(err, msg) { \
//    if (err != Z_OK) { \
//        fprintf(stderr, "%s error: %d\n", msg, err); \
//    } \
//}

int parseChannel(unsigned char reg)
{
	if (reg >= 0x30 && reg <= 0xBF) //Channel + OP registers
	{
		unsigned char regnum = (reg >> 4) << 4;
		unsigned char target = reg - regnum;
		return target % 4;
	}
	return -1;
}

YM2612REG parseParam(unsigned char reg)
{
	if (reg == 0x28)
		return YM2612REG::YMR_NOTEON;
	else if (reg >= 0x30 && reg <= 0xBF) //Channel + OP registers
	{
		unsigned char regnum = (reg >> 4) << 4;
		unsigned char target = reg - regnum;

		int op = 0;
		if (target >= 0x4 && target <= 0x7)
			op = 1;
		else if (target >= 0x8 && target <= 0xB)
			op = 2;
		else if (target >= 0xC && target <= 0xF)
			op = 3;
		
		if (regnum == 0x30)
			return (YM2612REG)(YM2612REG::YMR_DT1_MUL_OP1 + (op * 0x4));
		if (regnum == 0x40)
			return (YM2612REG)(YM2612REG::YMR_TL_OP1 + (op * 0x4));
		if (regnum == 0x50)
			return (YM2612REG)(YM2612REG::YMR_RS_AR_OP1 + (op * 0x4));
		if (regnum == 0x60)
			return (YM2612REG)(YM2612REG::YMR_AM_D1R_OP1 + (op * 0x4));
		if (regnum == 0x70)
			return (YM2612REG)(YM2612REG::YMR_D2R_OP1 + (op * 0x4));
		if (regnum == 0x80)
			return (YM2612REG)(YM2612REG::YMR_D1L_RR_OP1 + (op * 0x4));
		if (regnum == 0x90)
			return (YM2612REG)(YM2612REG::YMR_SSG_OP1 + (op * 0x4));
		if (reg == 0xB0 || reg == 0xB1 || reg == 0xB2)
			return (YM2612REG)YM2612REG::YMR_FB_ALG;
		if (reg == 0xB4 || reg == 0xB5 || reg == 0xB6)
			return (YM2612REG)YM2612REG::YMR_AMS_FMS;
	}

	return (YM2612REG)0;
}

class YMIns
{
public:
	std::map<YM2612REG, unsigned char> values;
	YMIns()
	{
		values[YMR_DT1_MUL_OP1] = 0;
		values[YMR_DT1_MUL_OP2] = 0;
		values[YMR_DT1_MUL_OP3] = 0;
		values[YMR_DT1_MUL_OP4] = 0;

		values[YMR_TL_OP1] = 0;
		values[YMR_TL_OP2] = 0;
		values[YMR_TL_OP3] = 0;
		values[YMR_TL_OP4] = 0;

		values[YMR_RS_AR_OP1] = 0;
		values[YMR_RS_AR_OP2] = 0;
		values[YMR_RS_AR_OP3] = 0;
		values[YMR_RS_AR_OP4] = 0;

		values[YMR_AM_D1R_OP1] = 0;
		values[YMR_AM_D1R_OP2] = 0;
		values[YMR_AM_D1R_OP3] = 0;
		values[YMR_AM_D1R_OP4] = 0;

		values[YMR_D2R_OP1] = 0;
		values[YMR_D2R_OP2] = 0;
		values[YMR_D2R_OP3] = 0;
		values[YMR_D2R_OP4] = 0;

		values[YMR_D1L_RR_OP1] = 0;
		values[YMR_D1L_RR_OP2] = 0;
		values[YMR_D1L_RR_OP3] = 0;
		values[YMR_D1L_RR_OP4] = 0;

		values[YMR_SSG_OP1] = 0;
		values[YMR_SSG_OP2] = 0;
		values[YMR_SSG_OP3] = 0;
		values[YMR_SSG_OP4] = 0;

		values[YMR_FB_ALG] = 0;
		values[YMR_AMS_FMS] = 0;
	}


	float difference(YMIns* compare)
	{
		float difference = 0.0f;
		std::map<YM2612REG, unsigned char>::iterator it = values.begin();
		while (it != values.end())
		{
			std::map<YM2612REG, unsigned char>::iterator with = compare->values.find((*it).first);
			if(with != compare->values.end())
			{ 
				if (((*it).second - (*with).second) != 0)
					difference += 0.20f;
			}

			it++;
		}

		return difference;
	}

	bool exists(std::vector<YMIns> instruments)
	{
		std::vector<YMIns>::iterator it = instruments.begin();
		while (it != instruments.end())
		{
			if ((*it).difference(this) < 1.0f)
				return true;

			it++;
		}

		return false;
	}
};

class drumgap
{
public:
	int start;
	int finish;
	double averageDelayBetweenSamples;
	drumgap()
	{
		start = -1;
		finish = -1;
		averageDelayBetweenSamples = 0.0f;
	}
};

bool replace(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
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
		else if (ext == "vgz" || ext == "vgm")
		{
			int comprLen = 0;
			int uncomprLen = 0;
			char* compr = ReadAllBytes(str.c_str(), &comprLen);
			GennyData file;
			if (gzip::is_compressed(compr, comprLen))
			{
				std::string decompressed_data = gzip::decompress(compr, comprLen);
				file.data = new char[decompressed_data.length()];
				memcpy(file.data, decompressed_data.c_str(), decompressed_data.length());
				uncomprLen = decompressed_data.length();
			}
			else
			{
				file.data = compr;
				uncomprLen = comprLen;
			}



			if (file.readInt() != ' mgV')
			{
				return; //Not a VGM file
			}

			int eofOffset = file.readInt();
			int version = file.readInt();

			file.dataPos = 0x1C;
			int loopOffset = file.readInt();
			int loopSamples = file.readInt();

			file.dataPos = 0x24;
			int rate = file.readInt();

			file.dataPos = 0x2C;
			int ym2612clock = file.readInt();

			file.dataPos = 0x34; //Skip to VGM Data Offset Information
			int vgmOffset = file.readInt();
			if (vgmOffset == 0)
				file.dataPos = 0x40;
			else
				file.dataPos += vgmOffset - 4;
				

			std::vector<YMIns> foundInstruments;
			YMIns scratch[6];
			int currentChannel = -1;

			//GennyData* currentWave = new GennyData();
			drumgap currentGap;
			std::vector<drumgap> waves;
			int waveIndex = 0;
			int pcmBankAddress = 0;
			int pcmBankOffset = 0;
			char* pcmdata = nullptr;
			int pcmdataSize = 0;

			double averageDelayBetweenSamples = 0;
			double totalAverageDelayBetweenSamples = 0;
			int samplesSincePCMCommand = 0;

			int numSamples = 0;
			int totalNumSamples = 0;

			//IMPLEMENT DAC IMPORT
			//Write to currentWave from current location on PCM data bank on 0x8n, and increment
			//On 0xe0, Write currentWave into waves[] if it doesn't already exist

			bool dacEnabled = false;
			while (true) //Read commands
			{
				if (file.dataPos >= uncomprLen)
					break;

				unsigned char command = file.readByte();

				if (command == 0x2B)
				{
					dacEnabled = file.readByte() & 0x80;
					continue;
				}

				if (command >= 0x70 && command <= 0x7F)
				{
					samplesSincePCMCommand += command & 0xF;
				}
				if (command >= 0x80 && command <= 0x8F && pcmBankAddress > 0) //DAC WRITE
				{
					//if(pcmBankAddress + pcmBankOffset < file.dataPos)
					//	currentWave->writeByte(file.data[pcmBankAddress + pcmBankOffset]);

					pcmBankOffset++;
					numSamples++;
					totalNumSamples++;

					if (samplesSincePCMCommand < 20)
					{
						averageDelayBetweenSamples += samplesSincePCMCommand;
						totalAverageDelayBetweenSamples += samplesSincePCMCommand;
					}

					samplesSincePCMCommand = (command & 0xF);
				}
				else if (command == 0xe0)
				{
					//if (currentWave->dataPos > 0)
					//{
					//	bool found = false;
					//	for (int iWave = 0; iWave < waves.size(); iWave++)
					//	{
					//		if (waves[iWave]->dataPos == currentWave->dataPos && memcmp(waves[iWave]->data, currentWave->data, currentWave->dataPos) == 0)
					//		{
					//			found = true;
					//			break;
					//		}
					//	}
					//	if (found == false)
					//		waves.push_back(currentWave);

					//	currentWave = new GennyData();
					//}

					if (currentGap.start != -1)
					{
						currentGap.finish = pcmBankOffset;
						currentGap.averageDelayBetweenSamples = averageDelayBetweenSamples / numSamples;
						if (currentGap.finish - currentGap.start > 300)
						{
							bool found = false;
							for (int i = 0; i < waves.size(); i++)
							{
								if (abs(waves[i].start - currentGap.start) < 300)
								{
									if (waves[i].start > currentGap.start)
										waves[i].start = currentGap.start;

									if (waves[i].finish < currentGap.finish)
										waves[i].finish = currentGap.finish;

									if (waves[i].averageDelayBetweenSamples > currentGap.averageDelayBetweenSamples)
										waves[i].averageDelayBetweenSamples = currentGap.averageDelayBetweenSamples;

									found = true;
									break;
								}
							}

							if (found == false)
								waves.push_back(currentGap);
						}
					}

					averageDelayBetweenSamples = 0;
					numSamples = 0;
					pcmBankOffset = file.readInt();
					currentGap.start = pcmBankOffset;
				}

				switch (command)
				{
					//Game Gear Data
					case 0x4f: file.dataPos += 1; break;
					//PSG Data
					case 0x50: file.dataPos += 1; break;
					//YM2413 Data
					case 0x51: file.dataPos += 2; break;
					//YM2151 Data
					case 0x54: file.dataPos += 2; break;
					//WAIT DATA
					case 0x61:
						samplesSincePCMCommand += file.readUShort();
						break;
					case 0x62:
						samplesSincePCMCommand += 735;
						break;
					case 0x63:
						samplesSincePCMCommand += 882;
						break;
					case 0x66:
						break;

					//YM2612 PORT 0 Data
					case 0x52:
					{
						unsigned char reg1 = file.readByte();
						unsigned char data1 = file.readByte();

						if (reg1 == 0x28) // NOTEON
						{
							if ((data1 & 0xF0) != 0)
							{
								int channel = data1 & 0x7;
								if (channel > 3)
									channel -= 1;

								if (channel < 6 && channel >= 0)
								{
									if (scratch[channel].exists(foundInstruments) == false)
										foundInstruments.push_back(scratch[channel]);
								}
							}
						}
						else
						{
							YM2612REG reg = parseParam(reg1);
							if (reg > 0)
							{
								int channel = parseChannel(reg1);
								if (channel >= 0 && channel < 3)
								{
									scratch[channel].values[reg] = data1;
									currentChannel = channel;
								}
							}
						}
					}
					break;
					//YM2612 PORT 1 Data
					case 0x53:
					{
						unsigned char reg2 = file.readByte();
						unsigned char data2 = file.readByte();

						YM2612REG reg = parseParam(reg2);
						if (reg > 0)
						{
							int channel = parseChannel(reg2);
							if (channel >= 0 && channel < 3)
							{
								channel += 3;

								scratch[channel].values[reg] = data2;
								currentChannel = channel;
							}
						}
					}
					break;

					//PCM BLOCK
					case 0x67:
					{
						if (file.readByte() == 0x66)
						{
							unsigned char type = file.readByte();
							int size = file.readInt();
							if (type == 0)
							{
								pcmBankAddress = file.dataPos;
								pcmdata = new char[size];
								memcpy(pcmdata, file.data + file.dataPos, size);
								pcmdataSize = size;
							}

							file.dataPos += size;
						}
					}
					break;

					default:
						int error = 1;
						break;
				}


			}

			//for (int i = 0; i < 6; i++)
			//{
			//	if (scratch[i].exists(foundInstruments) == false)
			//		foundInstruments.push_back(scratch[i]);

			//}


			std::string patchName = name.c_str();
			replace(patchName, " - ", "_");

			if (patchName.length() > 20)
				patchName = patchName.substr(0, 20);

			totalAverageDelayBetweenSamples /= totalNumSamples;
			for (int i = 0; i < foundInstruments.size() + 1; i++)
			{
				if (i == foundInstruments.size())
				{
					if (waves.size() == 0)
						break;

					getPatch(i)->Name = patchName + " Drums";
					getPatch(i)->InstrumentDef.Type = GIType::DAC;
					*getPatch(i)->InstrumentDef.Data.getParameter(YM_DRUMTL, 0, 3) = 100;
					for (int iWave = 36; iWave < 56; iWave++)
					{
						WaveData* drum = getPatch(i)->InstrumentDef.Drumset.getDrum(i);
						if (drum != nullptr)
							delete drum;

						WaveData* wave = new WaveData(pcmdata, pcmdataSize);

						if (iWave - 36 < waves.size())
						{
							wave->startSample = waves[iWave - 36].start;
							wave->endSample = waves[iWave - 36].finish;

							if (waves[iWave - 36].averageDelayBetweenSamples == 0)
								wave->sampleRate = 11025.0f;
							else
								wave->sampleRate = 43100.0f / (waves[iWave - 36].averageDelayBetweenSamples);
						}
						else
						{
							wave->startSample = 0;
							wave->endSample = pcmdataSize;
							wave->sampleRate = 43100.0f / totalAverageDelayBetweenSamples;
						}

						getPatch(i)->InstrumentDef.Drumset.mapDrum(iWave, wave);
					}

					continue;
				}
				else
					getPatch(i)->InstrumentDef.Type = GIType::FM;

				YMIns* ins = &foundInstruments[i];

				char buf[4];
				itoa(i, buf, 10);

				getPatch(i)->Name = patchName + " " + buf;
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_DT, 0)), (ins->values[YMR_DT1_MUL_OP1] & 0x70) >> 4);
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_DT, 1)), (ins->values[YMR_DT1_MUL_OP2] & 0x70) >> 4);
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_DT, 2)), (ins->values[YMR_DT1_MUL_OP3] & 0x70) >> 4);
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_DT, 3)), (ins->values[YMR_DT1_MUL_OP4] & 0x70) >> 4);

				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_MUL, 0)), (ins->values[YMR_DT1_MUL_OP1] & 0xF));
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_MUL, 1)), (ins->values[YMR_DT1_MUL_OP2] & 0xF));
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_MUL, 2)), (ins->values[YMR_DT1_MUL_OP3] & 0xF));
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_MUL, 3)), (ins->values[YMR_DT1_MUL_OP4] & 0xF));

				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_TL, 0)), ins->values[YMR_TL_OP1] & 0x7F);
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_TL, 1)), ins->values[YMR_TL_OP2] & 0x7F);
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_TL, 2)), ins->values[YMR_TL_OP3] & 0x7F);
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_TL, 3)), ins->values[YMR_TL_OP4] & 0x7F);

				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_KS, 0)), (ins->values[YMR_RS_AR_OP1] & 0xC0) >> 6);
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_KS, 1)), (ins->values[YMR_RS_AR_OP2] & 0xC0) >> 6);
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_KS, 2)), (ins->values[YMR_RS_AR_OP3] & 0xC0) >> 6);
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_KS, 3)), (ins->values[YMR_RS_AR_OP4] & 0xC0) >> 6);

				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_AR, 0)), (ins->values[YMR_RS_AR_OP1] & 0x1F));
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_AR, 1)), (ins->values[YMR_RS_AR_OP2] & 0x1F));
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_AR, 2)), (ins->values[YMR_RS_AR_OP3] & 0x1F));
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_AR, 3)), (ins->values[YMR_RS_AR_OP4] & 0x1F));

				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_AM, 0)), (ins->values[YMR_AM_D1R_OP1] & 0x80) >> 7);
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_AM, 1)), (ins->values[YMR_AM_D1R_OP2] & 0x80) >> 7);
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_AM, 2)), (ins->values[YMR_AM_D1R_OP3] & 0x80) >> 7);
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_AM, 3)), (ins->values[YMR_AM_D1R_OP4] & 0x80) >> 7);

				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_DR, 0)), (ins->values[YMR_AM_D1R_OP1] & 0x1F));
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_DR, 1)), (ins->values[YMR_AM_D1R_OP2] & 0x1F));
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_DR, 2)), (ins->values[YMR_AM_D1R_OP3] & 0x1F));
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_DR, 3)), (ins->values[YMR_AM_D1R_OP4] & 0x1F));


				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SR, 0)), (ins->values[YMR_D2R_OP1] & 0x1F));
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SR, 1)), (ins->values[YMR_D2R_OP2] & 0x1F));
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SR, 2)), (ins->values[YMR_D2R_OP3] & 0x1F));
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SR, 3)), (ins->values[YMR_D2R_OP4] & 0x1F));

				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SL, 0)), (ins->values[YMR_D1L_RR_OP1] & 0xF0) >> 4);
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SL, 1)), (ins->values[YMR_D1L_RR_OP2] & 0xF0) >> 4);
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SL, 2)), (ins->values[YMR_D1L_RR_OP3] & 0xF0) >> 4);
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SL, 3)), (ins->values[YMR_D1L_RR_OP4] & 0xF0) >> 4);

				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SL, 0)), (ins->values[YMR_D1L_RR_OP1] & 0xF0) >> 4);
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SL, 1)), (ins->values[YMR_D1L_RR_OP2] & 0xF0) >> 4);
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SL, 2)), (ins->values[YMR_D1L_RR_OP3] & 0xF0) >> 4);
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SL, 3)), (ins->values[YMR_D1L_RR_OP4] & 0xF0) >> 4);

				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_RR, 0)), (ins->values[YMR_D1L_RR_OP1] & 0xF));
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_RR, 1)), (ins->values[YMR_D1L_RR_OP2] & 0xF));
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_RR, 2)), (ins->values[YMR_D1L_RR_OP3] & 0xF));
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_RR, 3)), (ins->values[YMR_D1L_RR_OP4] & 0xF));

				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SSG, 0)), (ins->values[YMR_SSG_OP1] & 0xF));
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SSG, 1)), (ins->values[YMR_SSG_OP2] & 0xF));
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SSG, 2)), (ins->values[YMR_SSG_OP3] & 0xF));
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SSG, 3)), (ins->values[YMR_SSG_OP4] & 0xF));

				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_FB, -1)), (ins->values[YMR_FB_ALG] & 0x38) >> 3);
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_ALG, -1)), (ins->values[YMR_FB_ALG] & 0x7));

				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_AMS, -1)), (ins->values[YMR_AMS_FMS] & 0x30) >> 4);
				getPatch(i)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_FMS, -1)), (ins->values[YMR_AMS_FMS] & 0x7));
			}

			_owner->rejiggerInstruments(false);
			_owner->_playingStatusChanged = true;
			reconnect();

			int qq = 1;
			//uLongf uncomprLen = 8 * 1000 * 1000;
			//char* uncompr = new char[uncomprLen]; //8MB buffer

			//int result = uncompress((Bytef*)uncompr, &uncomprLen, (Bytef*)compr, comprLen);

			//int err;
			//z_stream d_stream; /* decompression stream */

			//strcpy((char*)uncompr, "garbage");

			//d_stream.zalloc = zalloc;
			//d_stream.zfree = zfree;
			//d_stream.opaque = (voidpf)0;

			//d_stream.next_in = (Bytef*)compr;
			//d_stream.avail_in = 0;
			//d_stream.next_out = (Bytef*)uncompr;

			//err = inflateInit(&d_stream);
			//CHECK_ERR(err, "inflateInit");

			//while (d_stream.total_out < uncomprLen && d_stream.total_in < comprLen) {
			//	d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */
			//	err = inflate(&d_stream, Z_NO_FLUSH);
			//	if (err == Z_STREAM_END) break;
			//	CHECK_ERR(err, "inflate");
			//}

			//err = inflateEnd(&d_stream);
			//CHECK_ERR(err, "inflateEnd");


			//GennyData file;
			//file.data = readFile;



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
