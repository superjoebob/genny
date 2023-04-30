#pragma once
#include "GennyInterface.h"
#include "UIImage.h"
#include "UICheckBoxNum.h"
class UIInstrumentsPanel;
class UIPresetsPanel;
class UIImportPanel;
class UIMegaMidiPanel;
class UIPresetsAndInstrumentsPanel;
class PresetsButton : public CKickButton
{
public:
	UIPresetsAndInstrumentsPanel* _owner;
	void* ContextMenu;
	PresetsButton(const CRect& size, IControlListener* listener, int32_t tag, CCoord heightOfOneImage, CBitmap* background, UIPresetsAndInstrumentsPanel* owner, const CPoint& offset = CPoint(0, 0))
		: CKickButton(size, listener, tag, heightOfOneImage, background, offset)
	{
		ContextMenu = nullptr;
		_owner = owner;
	}

	virtual CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons);
}; 

class ImportButton : public CKickButton
{
public:
	UIPresetsAndInstrumentsPanel* _owner;
	void* ContextMenu;
	ImportButton(const CRect& size, IControlListener* listener, int32_t tag, CCoord heightOfOneImage, CBitmap* background, UIPresetsAndInstrumentsPanel* owner, const CPoint& offset = CPoint(0, 0))
		: CKickButton(size, listener, tag, heightOfOneImage, background, offset)
	{
		ContextMenu = nullptr;
		_owner = owner;
	}

	virtual CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons);
};

class UIPresetsAndInstrumentsPanel : public CView, public IControlListener, public GennyInterfaceObject
{
public:
	UIPresetsAndInstrumentsPanel(const CRect& size, GennyInterface* owner);
	~UIPresetsAndInstrumentsPanel(void);

	virtual void valueChanged (CControl* control);
	virtual void draw (CDrawContext* pContext);

	void setParam(int index, float val);
	void mapControl(CControl* control, int index) { _controls[index] = control; }


	void setInstrumentPreset(int index);

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

		
	UIImportPanel* _importView;
private:
	GennyInterface* _owner;
	UIPresetsPanel* _presetsView;
	UIMegaMidiPanel* _megaMidiView;
	std::map<int, CControl*> _controls;	

	CKickButton* _presetsTabButton;
	CKickButton* _importTabButton;
	CKickButton* _megamidiTabButton;

	CKickButton* _infoTabButton;

	UIInstrumentsPanel* _instrumentView;



	int _tab;

	UIImage* _presetTab;
	UIImage* _infoTab;
	CTextLabel* _infoLabel;
};

