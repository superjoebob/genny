#pragma once
#include "GennyInterface.h"
#include "UIImage.h"
#include "UICheckBoxNum.h"
class UIInstrumentsPanel;
class UIPresetsPanel;
class UIImportPanel;
class UIMegaMidiPanel;
class UIPresetsAndInstrumentsPanel : public CView, public CControlListener, public GennyInterfaceObject
{
public:
	UIPresetsAndInstrumentsPanel(const CRect& size, GennyInterface* owner);
	~UIPresetsAndInstrumentsPanel(void);

	virtual void valueChanged (CControl* control);
	virtual void draw (CDrawContext* pContext);

	void setParam(int index, float val);
	void mapControl(CControl* control, int index) { _controls[index] = control; }


	void updateInstrumentChannels();
	void makeChannelsDirty();
	void reconnect();
	GennyInterface* getOwner() { return _owner; }
	CFrame* getFrame() { return _owner->getFrame(); }
	UIPresetsPanel* getPresetsView() { return _presetsView; }
	UIInstrumentsPanel* getInstrumentView() { return _instrumentView;}
	void setTab(int tab);
	void reconnectSelectedInstrument();
	void addConfirmDialog();

	CLASS_METHODS(UIPresetsAndInstrumentsPanel, CView)
		
private:
	GennyInterface* _owner;
	UIPresetsPanel* _presetsView;
	UIImportPanel* _importView;
	UIMegaMidiPanel* _megaMidiView;
	std::map<int, CControl*> _controls;	

	CKickButton* _presetsTabButton;
	CKickButton* _importTabButton;
	CKickButton* _megamidiTabButton;
	UIInstrumentsPanel* _instrumentView;



	int _tab;

	UIImage* _presetTab;
};

