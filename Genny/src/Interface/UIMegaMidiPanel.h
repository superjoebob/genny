#pragma once
#include "UIImage.h"
#include "UICheckBoxNum.h"
#include "UIPresetsAndInstrumentsPanel.h"
class UIPresetElement;
class UIMegaMidiPortSpinner;
class UIBendRangeSpinner;
class UIMegaMidiPanel : public CControl, public IControlListener, public GennyInterfaceObject
{
public:
	UIMegaMidiPanel(const CRect& size, UIPresetsAndInstrumentsPanel* owner);
	~UIMegaMidiPanel(void);
	virtual bool attached (CView* parent);

	virtual void valueChanged (CControl* control);
	virtual void draw (CDrawContext* pContext);

	UIPresetsAndInstrumentsPanel* getOwner() { return _owner; }
	CFrame* getFrame() { return _owner->getFrame(); }
	IndexBaron* getIndexBaron() { return _owner->getIndexBaron(); }

	void reconnect();
	void setVisible(bool visible);

	CLASS_METHODS(UIMegaMidiPanel, CView)

private:
	UIPresetsAndInstrumentsPanel* _owner;
#ifdef BUILD_VST
	CCheckBox* _enableCheck;
#else
	UIMegaMidiPortSpinner* _portSelector;
#endif
	UIBendRangeSpinner* _bendSelector;
	CCheckBox* _muteCheck;
	CCheckBox* _emuCheck;

	std::vector<CView*> _views;
};

