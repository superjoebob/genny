#pragma once
#include "UIImage.h"
#include "UICheckBoxNum.h"
#include "UIPresetsAndInstrumentsPanel.h"
class UIPresetElement;
class UIImportPanel : public CControl, public IControlListener, public GennyInterfaceObject
{
public:
	UIImportPanel(const CRect& size, UIPresetsAndInstrumentsPanel* owner);
	~UIImportPanel(void);
	virtual bool attached (CView* parent);

	virtual void valueChanged (CControl* control);
	virtual void draw (CDrawContext* pContext);
	virtual void setDirty(bool dirty = true);

	UIPresetsAndInstrumentsPanel* getOwner() { return _owner; }
	CFrame* getFrame() { return _owner->getFrame(); }
	IndexBaron* getIndexBaron() { return _owner->getIndexBaron(); }
	void triggerSelection(int sel);

	void reconnect();
	void setVisible(bool visible);

	CLASS_METHODS(UIImportPanel, CView)

private:
	UIPresetsAndInstrumentsPanel* _owner;
	CSlider* _scrollBar;
	std::vector<UIPresetElement*> _elements;
	int _selection;
	int _topItem;
	bool _didScroll;
	bool _initialized;

	CTextLabel* _selectedInstrument;
	CTextLabel* _loggingButton;
	CTextLabel* _tuningLabel;

	std::vector<CView*> _views;
	CKickButton* _downArrow;
};

