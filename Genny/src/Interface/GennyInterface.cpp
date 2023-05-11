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

//
//
//
//CMessageResult VSTFrame::notify (CBaseObject* sender, IdStringPtr message)
//{
//	if(message == CNewFileSelector::kSelectEndMessage)
//	{
//		CNewFileSelector* selector = (CNewFileSelector*)sender;
//		if(selector->getNumSelectedFiles() > 0)
//		{
//			_owner->fileDialogFinished(selector);
//			//_owner->OpenTYICallback(str);
//
//
//
//
//		}
//		int qq = 1;
//	}
//	return kMessageNotified;
//}
//
//CMouseEventResult VSTFrame::onMouseUp (CPoint& where, const CButtonState& buttons)
//{
//
//	CMouseEventResult res = __super::onMouseUp( where, buttons );
//	if(where.x == -9999 && where.y == -9999)
//		return res;
//
//	_owner->dragEnd();
//	//if (res == CMouseEventResult::kMouseEventNotHandled)
//	//	_owner->MenuClearCallback();
//	if(buttons.isRightButton())
//	{
//		if(_menu == nullptr)
//		{
//			
//			
//		}
//
//
//		
//	}
//
//	return res;
//}
//
//int32_t VSTFrame::onKeyDown (VstKeyCode& keyCode)
//{
//	return 0;
//}
//
//int32_t VSTFrame::onKeyUp (VstKeyCode& keyCode)
//{
//	return 0;
//}

void GennyInterface::onMouseUp() 
{
	dragEnd();
}

void GennyInterface::onDropFile(char* filename, int idx, int totalNum)
{
	processFile(std::string(filename), ((GennyPatch*)getVst()->getPatch(0))->Instruments[((GennyPatch*)getVst()->getPatch(0))->SelectedInstrument] + idx, FileType::Unknown, false, idx == totalNum - 1);
}

void GennyInterface::notify(CNewFileSelector* selector)
{
	if(selector->getNumSelectedFiles() > 0)
		fileDialogFinished(selector);
}


GennyInterface::GennyInterface(void* effect, GennyVST* owner)
	 : EditorBaseClass(effect),
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
		_lock(false),
		_applyingAutomationChange(false),
		_prevHoverControl(nullptr),
		_signalReconnect(false)
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

	hInstance = 0;
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

	_owner->_mutex.lock();
	_owner->_midiUIUpdates.clear();
	_owner->_mutex.unlock();

	hInstance = (HWND)ptr;

	CRect size (0, 0, 954, 602);
	 


	//VSTFrame* f = new VSTFrame (size, ptr, this);
	CFrame* f = new CFrame (size, this);
	//f->SetOwner(this); 
	f->setBackgroundColor (kWhiteCColor);
	f->open(ptr);

	rect.top=0;
	rect.left=0;
	rect.right=954; 
	rect.bottom= 602;

#ifdef BUILD_VST
	UIBitmap back(PNG_VSTBACK);
#else
	UIBitmap back(IDB_PNG51);
#endif

	CView* view = new CView( CRect( 0, 0, back.getWidth(), 602 ) );
	view->setBackground (back);
	f->addView(view);
	frame = f;
	frame->mouseUpEventListener = this;


	_interface = this;
	_vst = getVSTOwner();

	_instrumentUI = new UIInstrument(this);
	_instrumentUI->initialize();
	
	_patchEditor = new UIPresetsAndInstrumentsPanel(CRect(416, 96, 416 + 426, 96 + 186), this);

	_patchEditor->addConfirmDialog();
	setKnobMode(kLinearMode);

	return true;
}

void GennyInterface::close()
{	
	if (frame != nullptr)
		frame->close();


	EditorBaseClass::close();

	frame = nullptr;

	delete _instrumentUI;
	_instrumentUI = nullptr;
	_patchEditor = nullptr;
	hInstance = 0;

}

void GennyInterface::setInstrumentPreset(int index)
{
	_patchEditor->setInstrumentPreset(index);
}

void GennyInterface::midiLearn(GennyInt32 index)
{
	if (frame && GennyExtParam::isExtParam(index))
		_instrumentUI->midiLearn(index);
	else if (frame && index < getIndexBaron()->getNumParams())
	{
		IBIndex* parm = getIndexBaron()->getIndex(index);
		if (parm->getType() == IB_YMParam || parm->getType() == IB_InsParam)
			_instrumentUI->midiLearn(index);
		//else if (parm->getType() == IB_PatchParam)
		//	_patchEditor->setParam(index, value);
	}
}

void GennyInterface::midiForget(GennyInt32 index)
{
	if (frame && index < getIndexBaron()->getNumParams())
	{
		IBIndex* parm = getIndexBaron()->getIndex(index);
		if (parm->getType() == IB_YMParam || parm->getType() == IB_InsParam)
			_instrumentUI->midiForget(index);
		//else if (parm->getType() == IB_PatchParam)
		//	_patchEditor->setParam(index, value);
	}
}


void GennyInterface::setParameter(GennyInt32 index, float value)
{
	if (frame && GennyExtParam::isExtParam(index))
	{
		GennyExtParam* p = getVst()->getExtParam(index);
		if (p != nullptr)
		{
			if (p->lastAttachedControl != nullptr && p->lastAttachedControl->getFrame() != nullptr)
			{
				float oldValue = p->lastAttachedControl->getValue();
				p->lastAttachedControl->setValue(value);

				if (oldValue != value)
				{
					_applyingAutomationChange = true;
					p->lastAttachedControl->valueChanged();
					_applyingAutomationChange = false;
				}
			}

			if (p->isInsParam())
				_patchEditor->getInstrumentView()->instrumentWasModified(p->ins->patchIndex);
		}
	}
	else if(frame && index < getIndexBaron()->getNumParams())
	{
		IBIndex* parm = getIndexBaron()->getIndex(index);
		if(parm->getType() == IB_YMParam || parm->getType() == IB_InsParam)
		{
			if(parm->getType() == IB_InsParam)
			{
				IBInsParam* p = (IBInsParam*)parm;
				if(p->getParameter() == GIP_Panning || p->getParameter() == GIP_RangeHigh || p->getParameter() == GIP_RangeLow)
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

void GennyInterface::hoverControl(CControl* control)
{
	if (_prevHoverControl != control)
	{
		_prevHoverControl = control;
		if (control->getExtParam() != nullptr)
			_vst->showHint(control->getExtParam()->getTag());
		else
			_vst->showHint(control->getTag());
	}
}

void GennyInterface::unhoverControl(CControl* control)
{
	if (_prevHoverControl == control)
	{
		_prevHoverControl = nullptr;
		_vst->showHint(-1);
	}
}


void GennyInterface::valueChanged(CControl* control)
{
	if (_applyingAutomationChange)
		return;

#ifdef BUILD_VST
	//automationMessage m;
	//if (control->getExtParam() != nullptr)
	//	m.index = control->getExtParam()->getTag();
	//else
	//	m.index = control->getTag();
	//m.value = control->getValue() / control->getMax();

	//_vst->_automationMessageMutex.lock();
	//_vst->_automationMessages.push(m);
	//_vst->_automationMessageMutex.unlock();

	//_vst->_automationMessageMutex.lock();
	if (control->getExtParam() != nullptr)
		effect->setParameterAutomated(control->getExtParam()->getTag() + 99999999, control->getValue() / control->getMax());
	else
		effect->setParameterAutomated(control->getTag() + 99999999, control->getValue() / control->getMax());
	//_vst->_automationMessageMutex.unlock();
#else
	TFruityPlug* plug = static_cast<TFruityPlug*>(effect);
	
	int index = control->getTag();
	if(index >= 0)
	{
		if (control->getExtParam() != nullptr)
		{
			float val = control->getValue();
			if (val < 0)
				plug->ProcessParam(control->getExtParam()->getTag(), static_cast<int>(val - 0.5f), REC_PlugReserved);
			else				  
				plug->ProcessParam(control->getExtParam()->getTag(), static_cast<int>(val + 0.5f), REC_PlugReserved);
		}
		else
		{
			float val = control->getValue();
			if (val < 0)
				plug->ProcessParam(index, static_cast<int>(val - 0.5f), REC_PlugReserved);
			else
				plug->ProcessParam(index, static_cast<int>(val + 0.5f), REC_PlugReserved);
		}
	}
#endif

	if (control == _prevHoverControl || (_prevHoverControl != nullptr && _prevHoverControl->getTag() == control->getTag() && control->getTag() != 0))
	{
		CControl* prev = _prevHoverControl;
		_prevHoverControl = nullptr;
		hoverControl(prev); //refresh hint
	}
}

void GennyInterface::valueChangedExt(GennyExtParam* param, float value)
{
#ifdef BUILD_VST
	effect->setParameterAutomated(param->getTag() + 99999999, value / param->rangeMax);
#else
	TFruityPlug* plug = static_cast<TFruityPlug*>(effect);
	plug->ProcessParam(param->getTag(), static_cast<int>(value - 0.5f), REC_UpdateValue);
#endif
}

void GennyInterface::valueChangedCustom(int index, float value)
{
#ifdef BUILD_VST
	effect->setParameterAutomated(index + 99999999, value);
#else
	TFruityPlug* plug = static_cast<TFruityPlug*>(effect);
	plug->ProcessParam(index, value, REC_PlugReserved);
#endif
}

void GennyInterface::reconnect()
{
	_signalReconnect = true;
}

void GennyInterface::setChannelState(int channel, bool on)
{
	if (channel < 9)
	{
		_sync = true;
		_channels[channel] = on;
		_channelsChanged = true;
		_sync = false;
	}
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

	if (_owner->_hasUIUpdates)
	{
		_owner->_hasUIUpdates = false;
		_owner->_mutex.lock();
		std::map<int, int> copy(_owner->_midiUIUpdates);
		_owner->_midiUIUpdates.clear();
		_owner->_mutex.unlock();
		_owner->_clearMidiUIUpdateHistory = true;

		auto it = copy.begin();
		for (it; it != copy.end(); it++)
		{
			midiLearn((*it).first);		
			setParameter((*it).first, (*it).second);			
		}
	}

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

	if (_signalReconnect)
	{
		if (_instrumentUI != NULL)
			_instrumentUI->reconnect();
		if (_patchEditor != NULL)
			_patchEditor->reconnect();
		_signalReconnect = false;
	}
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
		_importInstrument->setAllowMultiFileSelection(true);

		_importInstrument->run([&](CNewFileSelector* s) {this->notify(s);});
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
		_importTuning->run([&](CNewFileSelector* s) {this->notify(s); });
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
		else if(currentPatch->InstrumentDef.Type == GIType::SN || currentPatch->InstrumentDef.Type == GIType::SNDRUM || currentPatch->InstrumentDef.Type == GIType::SNSPECIAL)
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

		_exportInstrument->run([&](CNewFileSelector* s) {this->notify(s); });
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
		_importBank->run([&](CNewFileSelector* s) {this->notify(s); });
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
		_exportBank->addFileExtension(CFileExtension("Separate Files", "folder"));
		_exportBank->setDefaultExtension(CFileExtension("GENNY Bank", "gnb"));
		_exportBank->run([&](CNewFileSelector* s) {this->notify(s); });
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
		_importState->run([&](CNewFileSelector* s) {this->notify(s); });
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
		_exportState->run([&](CNewFileSelector* s) {this->notify(s); });
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
		_logExport->run([&](CNewFileSelector* s) {this->notify(s); });
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
		_importSample->run([&](CNewFileSelector* s) {this->notify(s); });
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
				{
					if((*it).first >= YMR_TL_OP1 && (*it).first <= YMR_TL_OP4)
						difference += 0.05f; //TL has less effect
					else if((*it).first == YMR_FB_ALG)
						difference += 1.0f; //ALG has full effect
					else if ((*it).first >= YMR_DT1_MUL_OP1 && (*it).first <= YMR_DT1_MUL_OP4)
						difference += 1.0f; //DT MUL has full effect
					else
						difference += 0.4f;
				}
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

int selectedPatchOffset = 0;
void GennyInterface::fileDialogFinished(CNewFileSelector* dialog)
{
	selectedPatchOffset = 0;

	FileType type = FileType::Unknown;
	bool isExport = false;
	if (dialog == _importTuning)
		type = FileType::Tuning;
	else if (dialog == _exportBank)
	{
		isExport = true;
		type = FileType::Bank;
	}
	else if (dialog == _exportInstrument)
	{
		isExport = true;
		type = FileType::Patch;
	}	
	else if (dialog == _exportState)
	{
		isExport = true;
		type = FileType::State;
	}
	else if (dialog == _logExport)
	{
		std::string str = _logExport->getSelectedFile(0);

		int dot = str.rfind(".");
		int slash = str.rfind("\\");
		if (slash == -1)
			slash = str.rfind("/");

		std::string ext = "vgm";
		if (dot != -1)
			ext = toLower(str.substr(dot + 1, 3));
		else
		{
			str += ".vgm";
		}
		std::string name = str.substr(slash + 1, (str.length() - slash) - 5);

		_logging = true;
		if (_patchEditor != nullptr)
			_patchEditor->reconnect();

		_owner->startLogging(str);

		return;
	}

	for (int i = 0; i < dialog->getNumSelectedFiles(); i++)
	{
		processFile(dialog->getSelectedFile(i), -1, type, isExport);
		selectedPatchOffset++;
	}
	selectedPatchOffset = 0;
}

void GennyInterface::processFile(std::string fileName, int patchIndex, FileType type, bool isExport, bool reconnectUI)
{
	int dot = fileName.rfind(".");
	int slash = fileName.rfind("\\");
	if (slash == -1)
		slash = fileName.rfind("/");

	std::string extension = "";
	int extensionLength = 0;
	if (dot >= 0)
	{
		extension = toLower(fileName.substr(dot + 1, fileName.length() - (dot + 1)));
		extensionLength = extension.length() + 1;
	}

	std::string fullFileNameWithoutExtension = fileName.substr(0, fileName.length() - extensionLength);
	std::string nameOfFileWithoutPath = fullFileNameWithoutExtension.substr(slash + 1, (fullFileNameWithoutExtension.length() - (slash + 1)));

	if (type == FileType::Unknown)
	{
		if (extension == "wav")
			type = FileType::Sample;
		else if (extension == "gen" || extension == "vgi" || extension == "tfi" || extension == "dac")
			type = FileType::Patch;
		else if (extension == "gnb" || extension == "bnk" || extension == "tyi" || extension == "vgz" || extension == "vgm" || extension == "folder")
			type = FileType::Bank;
		else if (extension == "tun")
			type = FileType::Tuning;
		else if (extension == "gst")
			type = FileType::State;
	}

	if (type == FileType::Unknown)
	{
		int msgboxID = MessageBoxA(
			NULL,
			"Genny cannot open one or more dragged files!",
			"Unknown File Type",
			MB_ICONEXCLAMATION
		);

		return;
	}

	GennyPatch* patch = nullptr;
	if (patchIndex >= getVSTOwner()->getTotalPatchCount())
		return;
	else if (patchIndex >= 0)
		patch = static_cast<GennyPatch*>(_owner->getPatch(patchIndex));
	else
	{
		std::vector<VSTPatch*> patches = getVSTOwner()->getPatches();

		GennyPatch* patch0 = static_cast<GennyPatch*>(_owner->getPatch(0));
		int selectedIndex = (int)patch0->Instruments[patch0->SelectedInstrument];
		selectedIndex += selectedPatchOffset;
		if (selectedIndex >= patches.size())
			return;

		patch = static_cast<GennyPatch*>(patches[selectedIndex]);
		patchIndex = selectedIndex;
	}

	if (type == FileType::Tuning)
	{
		char* buff = new char[8192];
		char* keyBuffer = new char[256];
		std::string readString = "Exact Tuning";
		int read = GetPrivateProfileStringA(readString.c_str(), nullptr, "", buff, 8192, fileName.c_str());
		if (read <= 0)
		{
			readString = "Tuning";
			read = GetPrivateProfileStringA(readString.c_str(), nullptr, "", buff, 8192, fileName.c_str());
		}

		double* frequencyTable = new double[129];
		int keyIndex = 0;
		std::string currentKey = "";
		for (int i = 0; i < read; i++)
		{
			if (buff[i] == 0)
			{
				int keyRead = GetPrivateProfileStringA(readString.c_str(), currentKey.c_str(), "", keyBuffer, 256, fileName.c_str());
				if (keyRead > 0)
				{
					if (keyIndex == 0)
					{
						if (currentKey != "basefreq")
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

		if(reconnectUI)
			reconnect();
	}
	else if (type == FileType::Sample)
	{
		std::fstream file(fileName.c_str(), std::ios::binary | std::ios::in | std::ios::ate);
		std::ifstream::pos_type size = file.tellg();
		char* memblock = new char[size];
		file.seekg(0, std::ios::beg);
		file.read(memblock, size);
		file.close();

		GennyData data;
		data.data = memblock;
		data.dataPos = 0;
		data.handle = 0;
		data.size = size;

		WaveData* wave = new WaveData(&data);
		if (wave->valid)
		{
			wave->sampleName = getFileName(fileName);
			wave->sampleName = wave->sampleName.substr(0, wave->sampleName.length() - 4);

			GennyPatch* p = (GennyPatch*)getVst()->getCurrentPatch();
			p->InstrumentDef.Drumset.mapDrum(36 + _sampleSlot, wave);
		}

		delete[] memblock;

		if (reconnectUI)
			reconnect();
	}
	else if (type == FileType::Patch)
	{
		if (isExport)
		{
			if (patch->InstrumentDef.Type == GIType::DAC)
				extension = "dac";

			if (extension == "")
				extension = "vgi";

			fileName = fullFileNameWithoutExtension + "." + extension;

			std::string oldName = patch->Name;
			patch->Name = nameOfFileWithoutPath;

			bool canExport = false;
			GennyData dat;
			if (extension == "dac")
			{
				GennyLoaders::saveGDAC(static_cast<GennyPatch*>(patch), &dat);
				canExport = true;
			}
			else if (extension == "gen")
			{
				dat = GennyLoaders::saveGEN(static_cast<GennyPatch*>(patch));
				canExport = true;
			}
			else if (extension == "vgi")
			{
				dat = GennyLoaders::saveVGI(static_cast<GennyPatch*>(patch));
				canExport = true;
			}
			else if (extension == "tfi")
			{
				dat = GennyLoaders::saveTFI(static_cast<GennyPatch*>(patch));
				canExport = true;
			}
			else if (extension == "tyi")
			{
				dat = GennyLoaders::saveTYI(static_cast<GennyPatch*>(patch));
				canExport = true;
			}

			patch->Name = oldName;

			if (canExport)
			{
				std::fstream file;
				file.open(fileName, std::ios::out | std::ios::binary | std::ios::trunc);
				file.write(dat.data, dat.size);
				file.close();
				delete[] dat.data;
			}
		}
		else
		{
			patch->Name = nameOfFileWithoutPath;
			std::fstream file(fileName.c_str(), std::ios::binary | std::ios::in | std::ios::ate);
			std::ifstream::pos_type size = file.tellg();
			char* memblock = new char[size];
			file.seekg(0, std::ios::beg);
			file.read(memblock, size);
			file.close();

			GennyData data;
			data.data = memblock;
			data.dataPos = 0;
			data.handle = 0;
			data.size = size;

			if (extension == "dac")
			{
				GennyLoaders::loadGDAC(patch, &data);
				patch->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex((YM2612Param)YM_DRUMTL, 3)), YM2612Param_getRange(YM_DRUMTL) - 27);

				for (int i = 0; i < 10; i++)
					_owner->getCore()->channelDirty[i] = true;
			}
			else if (extension == "gen")
			{
				GennyLoaders::loadGEN(patch, &data);

				if (patch->InstrumentDef.Type == GIType::DAC)
					patch->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex((YM2612Param)YM_DRUMTL, 3)), YM2612Param_getRange(YM_DRUMTL) - 27);

				for (int i = 0; i < 10; i++)
					_owner->getCore()->channelDirty[i] = true;
			}
			else if (extension == "vgi")
				GennyLoaders::loadVGI(patch, data);
			else if (extension == "tfi")		 
				GennyLoaders::loadTFI(patch, data);
			else if (extension == "tyi")		 
				GennyLoaders::loadTYI(patch, data);
			//else if (extension == "dpa")
			//{
			//	patch->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex((YM2612Param)YM_TL, 3)), YM2612Param_getRange(YM_TL) - 27);

			//	int pos = 0;
			//	GennyLoaders::loadDPACK(patch, data, &pos);
			//	patch->InstrumentDef.setSamplePath(fileName.c_str());
			//}
			delete[] memblock;
		}

		if (reconnectUI)
		{
			_owner->rejiggerInstruments(true);
			reconnect();
		}
	}
	else if (type == FileType::Bank)
	{
		if (isExport)
		{
			if (extension == "")
				extension = "bnk";

			fileName = fullFileNameWithoutExtension + "." + extension;

			bool canExport = false;
			std::vector<VSTPatch*> patches = getVSTOwner()->getPatches();
			GennyPatch* patch0 = static_cast<GennyPatch*>(_owner->getPatch(0));
			int selectedIndex = (int)patch0->Instruments[patch0->SelectedInstrument];

			GennyData dat;

			if (extension == "folder")
			{
				fileName = fileName.substr(0, fileName.length() - 7);

				const char* c = fileName.c_str();
				const size_t cSize = strlen(c) + 1;
				wchar_t* wc = new wchar_t[cSize];
				mbstowcs(wc, c, cSize);
				
				
				if (_wmkdir(wc) < 0 && errno != EEXIST)
					return;

				std::vector<VSTPatch*> patches = getVSTOwner()->getPatches();

				for (int i = 0; i < patches.size(); i++)
				{
					GennyData data = GennyLoaders::saveGEN(static_cast<GennyPatch*>(patches[i]));
					std::fstream file;
					file.open(fileName + "//" + std::to_string(i + 1) + ". " + patches[i]->Name + ".gen", std::ios::out | std::ios::binary | std::ios::trunc);
					file.write(data.data, data.size);
					file.close();
					delete[] dat.data;
				}
			}
			else if (extension == "gnb")
			{
				GennyData dat;
				std::vector<VSTPatch*> patches = getVSTOwner()->getPatches();

				dat.writeInt(patches.size());
				for (int i = 0; i < patches.size(); i++)
				{
					GennyData data = GennyLoaders::saveGEN(static_cast<GennyPatch*>(patches[i]));

					dat.writeInt(data.size);
					dat.writeBytes(data.data, data.size);
				}

				std::fstream file(fileName.c_str(), std::ios::binary | std::ios::out | std::ios::trunc);
				file.write(dat.data, dat.size);
				file.close();
			}
			else if (extension == "tyi" || extension == "bnk")
			{
				std::fstream file(fileName.c_str(), std::ios::binary | std::ios::out | std::ios::trunc);
				std::vector<VSTPatch*> patches = getVSTOwner()->getPatches();
				for (int i = 0; i < patches.size(); i++)
				{
					if (static_cast<GennyPatch*>(patches[i])->InstrumentDef.Type == GIType::DAC)
						continue;

					GennyData data = GennyLoaders::saveTYI(static_cast<GennyPatch*>(patches[i]));
					file.write(data.data, data.size);
				}
				file.close();
			}
		}
		else
		{
			int current = _owner->getPatchIndex(_owner->getCurrentPatch());
			if (extension == "gnb")
			{
				int fileSize = 0;
				char* readFile = ReadAllBytes(fileName.c_str(), &fileSize);
				GennyData file;
				file.data = readFile;

				int numPatches = file.readInt();
				for (int i = 0; i < numPatches; i++)
				{
					if (i >= getVSTOwner()->_patches.size())
						getVSTOwner()->_patches.push_back(new GennyPatch(i));

					int size = file.readInt();

					GennyData dat;
					dat.data = new char[size];
					memcpy(dat.data, file.data + file.dataPos, size);
					dat.size = size;
					file.dataPos += size;

					GennyLoaders::loadGEN((GennyPatch*)getVSTOwner()->_patches[i], &dat);

					if (((GennyPatch*)getVSTOwner()->_patches[i])->InstrumentDef.Type == GIType::DAC)
						((GennyPatch*)getVSTOwner()->_patches[i])->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex((YM2612Param)YM_TL, 3)), YM2612Param_getRange(YM_TL) - 27);

					delete[] dat.data;
				}

				delete[] readFile;
			}
			else if (extension == "bnk" || extension == "tyi")
			{
				std::fstream file(fileName.c_str(), std::ios::binary | std::ios::in);
				file.seekg(0, file.end);
				int fileSize = file.tellg();
				file.seekg(0, file.beg);

				std::vector<VSTPatch*> patches = getVSTOwner()->getPatches();

				int patchesLoaded = 0;
				for (int i = patchIndex; i < patches.size(); i++)
				{
					if (static_cast<GennyPatch*>(patches[i])->InstrumentDef.Type == GIType::DAC)
						continue;

					if (!file.good() || file.eof() || (fileSize - file.tellg()) < 10)
						break;

					char* memblock = new char[32];
					file.read(memblock, 32);
					GennyData dat;
					dat.data = memblock;
					dat.size = 32;


					char buf[4];
					itoa(patchesLoaded, buf, 10);
					patches[i]->Name = nameOfFileWithoutPath + " " + buf;

					GennyLoaders::loadTYI((GennyPatch*)patches[i], dat);

					IndexBaron* baron = getIndexBaron();
					int count = GennyPatch::getNumParameters();

					patchesLoaded++;
					delete memblock;
				}


				for (int i = 0; i < 10; i++)
					_owner->getCore()->channelDirty[i] = true;

				file.close();
#ifdef BUILD_VST
				getVSTOwner()->_setParameterNormalizedValue = false;
				effect->setParameterAutomated(kPresetControlIndex, current);
				getVSTOwner()->_setParameterNormalizedValue = true;
#else
				TFruityPlug* plug = static_cast<TFruityPlug*>(effect);
				plug->ProcessParam(kPresetControlIndex + 100000, current, REC_UpdateValue);
#endif
			}
			else if (extension == "vgz" || extension == "vgm")
			{
				int comprLen = 0;
				int uncomprLen = 0;
				char* compr = ReadAllBytes(fileName.c_str(), &comprLen);
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


				std::string patchName = nameOfFileWithoutPath.c_str();
				replace(patchName, " - ", "_");

				if (patchName.length() > 20)
					patchName = patchName.substr(0, 20);


				


				int patchNum = getPatchIndex(getCurrentPatch());
				totalAverageDelayBetweenSamples /= totalNumSamples;
				for (int i = 0; i < foundInstruments.size() + 1; i++)
				{

					if (i == foundInstruments.size())
					{
						if (waves.size() == 0)
							break;

						getPatch(patchNum)->Name = patchName + " Drums";
						getPatch(patchNum)->InstrumentDef.Type = GIType::DAC;
						*getPatch(patchNum)->InstrumentDef.Data.getParameter(YM_DRUMTL, 0, 3) = 100;
						for (int iWave = 36; iWave < 56; iWave++)
						{
							WaveData* drum = getPatch(patchNum)->InstrumentDef.Drumset.getDrum(iWave);
							if (drum != nullptr)
								delete drum;

							if (iWave - 36 < waves.size())
							{
								int startSample = waves[iWave - 36].start;
								int endSample = waves[iWave - 36].finish;

								WaveData* wave = new WaveData(pcmdata + startSample, endSample - startSample);
								//wave->startSample = waves[iWave - 36].start;
								//wave->endSample = waves[iWave - 36].finish;

								if (waves[iWave - 36].averageDelayBetweenSamples == 0)
									wave->sampleRate = 11025.0f;
								else
									wave->sampleRate = 43100.0f / (waves[iWave - 36].averageDelayBetweenSamples);

								//if (wave->sampleRate != 11025.0f)
								//{
								//	unsigned char* oldData = wave->audioData;

								//	//int startSample = wave->startSample;
								//	//int endSample = wave->endSample;

								//	int newSize = 0;
								//	unsigned char* buffer = WaveData::Resample(wave->audioData, wave->size, wave->sampleRate, 11025.0f, newSize);
								//	wave->SetData(buffer, newSize);

								//	//wave->startSample = startSample * WaveData::lastResampleRatio;
								//	//wave->endSample = endSample * WaveData::lastResampleRatio;
								//	wave->sampleRate = 11025.0f;
								//	//delete[] oldData;
								//}
								getPatch(patchNum)->InstrumentDef.Drumset.mapDrum(iWave, wave);

							}
							else
							{
								//wave->startSample = 0;
								//wave->endSample = pcmdataSize;
								//wave->sampleRate = 43100.0f / totalAverageDelayBetweenSamples;
							}




						}

						continue;
					}
					else
						getPatch(patchNum)->InstrumentDef.Type = GIType::FM;

					YMIns* ins = &foundInstruments[i];

					char buf[4];
					itoa(i, buf, 10);

					getPatch(patchNum)->Name = patchName + " " + buf;
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_DT, 0)), (ins->values[YMR_DT1_MUL_OP1] & 0x70) >> 4);
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_DT, 1)), (ins->values[YMR_DT1_MUL_OP2] & 0x70) >> 4);
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_DT, 2)), (ins->values[YMR_DT1_MUL_OP3] & 0x70) >> 4);
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_DT, 3)), (ins->values[YMR_DT1_MUL_OP4] & 0x70) >> 4);

					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_MUL, 0)), (ins->values[YMR_DT1_MUL_OP1] & 0xF));
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_MUL, 1)), (ins->values[YMR_DT1_MUL_OP2] & 0xF));
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_MUL, 2)), (ins->values[YMR_DT1_MUL_OP3] & 0xF));
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_MUL, 3)), (ins->values[YMR_DT1_MUL_OP4] & 0xF));

					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_TL, 0)), 127 - (ins->values[YMR_TL_OP1] & 0x7F));
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_TL, 1)), 127 - (ins->values[YMR_TL_OP2] & 0x7F));
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_TL, 2)), 127 - (ins->values[YMR_TL_OP3] & 0x7F));
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_TL, 3)), 127 - (ins->values[YMR_TL_OP4] & 0x7F));

					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_KS, 0)), (ins->values[YMR_RS_AR_OP1] & 0xC0) >> 6);
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_KS, 1)), (ins->values[YMR_RS_AR_OP2] & 0xC0) >> 6);
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_KS, 2)), (ins->values[YMR_RS_AR_OP3] & 0xC0) >> 6);
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_KS, 3)), (ins->values[YMR_RS_AR_OP4] & 0xC0) >> 6);

					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_AR, 0)), 31 - (ins->values[YMR_RS_AR_OP1] & 0x1F));
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_AR, 1)), 31 - (ins->values[YMR_RS_AR_OP2] & 0x1F));
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_AR, 2)), 31 - (ins->values[YMR_RS_AR_OP3] & 0x1F));
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_AR, 3)), 31 - (ins->values[YMR_RS_AR_OP4] & 0x1F));

					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_AM, 0)), (ins->values[YMR_AM_D1R_OP1] & 0x80) >> 7);
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_AM, 1)), (ins->values[YMR_AM_D1R_OP2] & 0x80) >> 7);
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_AM, 2)), (ins->values[YMR_AM_D1R_OP3] & 0x80) >> 7);
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_AM, 3)), (ins->values[YMR_AM_D1R_OP4] & 0x80) >> 7);

					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_DR, 0)), 31 - (ins->values[YMR_AM_D1R_OP1] & 0x1F));
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_DR, 1)), 31 - (ins->values[YMR_AM_D1R_OP2] & 0x1F));
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_DR, 2)), 31 - (ins->values[YMR_AM_D1R_OP3] & 0x1F));
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_DR, 3)), 31 - (ins->values[YMR_AM_D1R_OP4] & 0x1F));


					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SR, 0)), (ins->values[YMR_D2R_OP1] & 0x1F));
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SR, 1)), (ins->values[YMR_D2R_OP2] & 0x1F));
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SR, 2)), (ins->values[YMR_D2R_OP3] & 0x1F));
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SR, 3)), (ins->values[YMR_D2R_OP4] & 0x1F));

					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SL, 0)), 15 - ((ins->values[YMR_D1L_RR_OP1] & 0xF0) >> 4));
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SL, 1)), 15 - ((ins->values[YMR_D1L_RR_OP2] & 0xF0) >> 4));
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SL, 2)), 15 - ((ins->values[YMR_D1L_RR_OP3] & 0xF0) >> 4));
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SL, 3)), 15 - ((ins->values[YMR_D1L_RR_OP4] & 0xF0) >> 4));

					/*getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SL, 0)), (ins->values[YMR_D1L_RR_OP1] & 0xF0) >> 4);
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SL, 1)), (ins->values[YMR_D1L_RR_OP2] & 0xF0) >> 4);
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SL, 2)), (ins->values[YMR_D1L_RR_OP3] & 0xF0) >> 4);
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SL, 3)), (ins->values[YMR_D1L_RR_OP4] & 0xF0) >> 4);*/

					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_RR, 0)), 15 - (ins->values[YMR_D1L_RR_OP1] & 0xF));
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_RR, 1)), 15 - (ins->values[YMR_D1L_RR_OP2] & 0xF));
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_RR, 2)), 15 - (ins->values[YMR_D1L_RR_OP3] & 0xF));
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_RR, 3)), 15 - (ins->values[YMR_D1L_RR_OP4] & 0xF));

					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SSG, 0)), (ins->values[YMR_SSG_OP1] & 0xF));
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SSG, 1)), (ins->values[YMR_SSG_OP2] & 0xF));
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SSG, 2)), (ins->values[YMR_SSG_OP3] & 0xF));
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_SSG, 3)), (ins->values[YMR_SSG_OP4] & 0xF));

					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_FB, -1)), (ins->values[YMR_FB_ALG] & 0x38) >> 3);
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_ALG, -1)), (ins->values[YMR_FB_ALG] & 0x7));

					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_AMS, -1)), (ins->values[YMR_AMS_FMS] & 0x30) >> 4);
					getPatch(patchNum)->setFromBaron(getIndexBaron()->getIndex(getIndexBaron()->getYMParamIndex(YM2612Param::YM_FMS, -1)), (ins->values[YMR_AMS_FMS] & 0x7));
				
					patchNum++;
					if (patchNum >= _vst->getTotalPatchCount())
						patchNum = 0;
				}
			}
		}

		if (reconnectUI)
		{
			_owner->rejiggerInstruments(false);
			_owner->_playingStatusChanged = true;
			reconnect();
		}
	}
	else if (type == FileType::State)
	{
		if (isExport)
		{
			fileName = fullFileNameWithoutExtension + ".gst";
			bool canExport = false;
			void* data = nullptr;
			int stateSize = getVSTOwner()->getPluginState(&data, false);
			int written = 0;
			std::fstream file(fileName.c_str(), std::ios::binary | std::ios::out | std::ios::trunc);
			file.write((char*)data, stateSize);
			file.close();
		}
		else
		{
			int fileSize = 0;
			char* readFile = ReadAllBytes(fileName.c_str(), &fileSize);
			getVSTOwner()->setPluginState((void*)readFile, fileSize, false);

			delete[] readFile;

			if (reconnectUI)
			{
				_owner->rejiggerInstruments(false);
				_owner->_playingStatusChanged = true;
				reconnect();
			}

			return;
		}
	}
}