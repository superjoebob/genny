#pragma once
#include "UIImage.h"
#include "UICheckBoxNum.h"
#include "UIPresetsAndInstrumentsPanel.h"
#include "UILetterDisplay.h"
class UIInstrumentElement;
class UIPESelectedInstrument;
class UIInstrumentElement;
class UIInstrumentsPanel : public CControl, public CControlListener, public GennyInterfaceObject
{
public:
	UIInstrumentsPanel(const CRect& size, UIPresetsAndInstrumentsPanel* owner);
	~UIInstrumentsPanel(void);
	void reorganize();

	virtual void draw (CDrawContext* pContext);
	virtual void setDirty(bool dirty = true);

	void setValue(float val);
	virtual void valueChanged (CControl* control);
	void setScrollBarVisible(bool val);

	UIPresetsAndInstrumentsPanel* getOwner() { return _owner; }
	CFrame* getFrame() { return _owner->getFrame(); }
	UIPESelectedInstrument* getSelectedInstrument() { return _selectedInstrument; } 
	void prepareToDrag(void* val) { if(_preparingToDrag == nullptr) {_preparingToDrag = val;}}
	void* preparedToDrag() { return _preparingToDrag; }
	bool dragBegin(UIInstrumentElement* vBox);
	void dragUpdate(UIInstrumentElement* vBox);
	void dragEnd();
	void updateInstrumentChannels();
	void makeChannelsDirty();
	void reconnectSelectedInstrument();

	void setSolo(int idx);

	void reconnect();
	void setSelectedInstrumentIndex(int index);
	virtual bool onWheel (const CPoint& where, const float& distance, const CButtonState& buttons);		

	CLASS_METHODS(UIInstrumentsPanel, CControl)
		
	UIPESelectedInstrument* _selectedInstrument;
private:
	UIPresetsAndInstrumentsPanel* _owner;
	std::vector<UIInstrumentElement*> _elements;
	int _selection;
	int _topItem;
	bool _didScroll;
	int _currentDrag;
	void* _preparingToDrag;
	bool _ignoreRescroll;

	UIImage* _instrumentsTab;
	CSlider* _scrollBar;
	CKickButton* _addInstrumentButton;
	CKickButton* _removeInstrumentButton;
	CKickButton* _scrollBarUp;
	CKickButton* _scrollBarDown;

	UILetterDisplay* _letterDisplay;
};

