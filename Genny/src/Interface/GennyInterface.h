#pragma once

#include "../libinclude.h"
#include "GennyInterfaceObject.h"

#ifdef BUILD_VST
typedef VstInt32 GennyInt32;
typedef AEffGUIEditor EditorBaseClass;
#else
typedef int GennyInt32;
typedef PluginGUIEditor EditorBaseClass;
#endif

class GennyInterface;
class UIContextMenu;

//
//class VSTFrame : public CFrame
//{
//public: 
//	VSTFrame( const CRect& size, void* pSystemWindow, VSTGUIEditorInterface* pEditor ) : CFrame( size, pSystemWindow, pEditor ), _menu(nullptr)
//	{
//		//_menu = new UIContextMenu(-100, -100);
//	}
//	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons);
//	int32_t onKeyDown (VstKeyCode& keyCode);
//	int32_t onKeyUp (VstKeyCode& keyCode);
//	virtual CMessageResult notify (CBaseObject* sender, IdStringPtr message);
//
//	void SetOwner( GennyInterface* owner ) { _owner = owner; }
//	UIContextMenu* getMenu() {return _menu;}
//
//private:
//	GennyInterface* _owner;
//	UIContextMenu* _menu;
//};

struct ParameterUpdate
{
	int index;
	float value;
};

enum class FileType
{
	Unknown,
	Patch,
	Bank,
	Tuning,
	Sample,
	State
};

class GennyVST;
class UIInstrument;
class UIChannelStatus;
class IndexBaron;
class UIPresetsAndInstrumentsPanel;
class GennyInterface : public EditorBaseClass, public IControlListener, public GennyInterfaceObject, IAmListeningForAMouseUpEvent
{
public:
	virtual void onMouseUp();
	virtual void onDropFile(char* filename, int idx, int totalNum);

	virtual void notify (CNewFileSelector* selector);
	GennyInterface(void* effect, GennyVST* owner);
	~GennyInterface(void);

	virtual bool open (void* ptr);
	virtual void close ();

	virtual void midiLearn(GennyInt32 index);
	virtual void midiForget(GennyInt32 index);

	void hoverControl(CControl* control);
	void unhoverControl(CControl* control);

	virtual void setParameter (GennyInt32 index, float value);
	virtual void valueChanged (CControl* control);
	virtual void valueChangedExt(GennyExtParam* param, float value);
	virtual void valueChangedCustom(int index, float value);
	void reconnect();

	void setChannelState(int channel, bool on);
	virtual void idle();

	bool getChannelState(int channel) { return _channels[channel]; }

	void setInstrumentPreset(int index);

	CFrame* getFrame() {return frame;}
	GennyVST* getVSTOwner() { return _owner; }

	void openInstrumentImport();
	void openTuningImport();
	void openInstrumentExport();
	void openBankImport();
	void openBankExport();
	void openStateImport();
	void openStateExport();
	void openLogExport();
	void openImportSample(int sampleSlot);

	void processFile(std::string fileName, int patchIndex, FileType type = FileType::Unknown, bool isExport = false, bool reconnectUI = true);

	void fileDialogFinished(CNewFileSelector* dialog);
	void setLogging(bool logging) { _logging = logging; }
	bool getLogging() {return _logging;}
	void dragEnd();

	HWND hInstance;
	bool _signalReconnect;

private:
	CControl* _prevHoverControl;
	UIInstrument* _instrumentUI;
	UIPresetsAndInstrumentsPanel* _patchEditor;
	GennyVST* _owner;
	bool _channelTweak;
	int _sampleSlot;

	std::vector<ParameterUpdate> _updates;

	CNewFileSelector* _importBank;
	CNewFileSelector* _exportBank;
	CNewFileSelector* _importInstrument;
	CNewFileSelector* _exportInstrument;
	CNewFileSelector* _importState;
	CNewFileSelector* _exportState;
	CNewFileSelector* _logExport;
	CNewFileSelector* _importTuning;

	CNewFileSelector* _importSample;

	volatile bool _channels[10];
	volatile bool _channelsChanged;
	volatile bool _sync;
	volatile bool _logging;
	volatile bool _lock;
	volatile bool _applyingAutomationChange;
};

