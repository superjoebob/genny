#pragma once
#include "UIImage.h"
#include "GennyInterfaceObject.h"
class UIInstrumentsPanel;
class UIImage;
struct GennyPatch;
class UIInstrumentElement : public CCheckBox, public CControlListener, public GennyInterfaceObject
{
public:
	UIInstrumentElement(const CPoint& vPosition, UIInstrumentsPanel* vOwner, GennyPatch* vPatch, int vIndex);
	~UIInstrumentElement(void);

	virtual bool attached (CView* parent);	
	virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);
	virtual bool onWheel (const CPoint& where, const float& distance, const CButtonState& buttons);	

	void update();
	void makeChannelsDirty();
	virtual void valueChanged(CControl* control);
	virtual void valueChanged();

	void unselect();
	void select();

	void setPatchLink(GennyPatch* patch);
	GennyPatch* getPatchLink() { return _patch; }

	void setWide(bool vWide);

	int getInstrumentIndex();

	virtual void setVisible(bool visible);
	void updateEnabledStatus();
	virtual void invalid();

	CLASS_METHODS(UIInstrumentElement, CCheckBox)

private:
	UIInstrumentsPanel* _owner;
	GennyPatch* _patch;

	CCheckBox* _enabledCheckbox;
	CTextLabel* _label;
	CMovieBitmap* _typeDisplay;

	std::vector<UIImage*> _lights;
	std::vector<float> _lightVals;
	std::vector<bool> _active;
	
	CPoint _clickDragStart;
	bool _selected;
	bool _wide;
};

