#pragma once
#include "UIImage.h"
#include "GennyInterfaceObject.h"
class UIInstrumentsPanel;
class UIImage;
struct GennyPatch;
class UIInstrumentElement : public CCheckBox, public IControlListener, public GennyInterfaceObject
{
public:
	UIInstrumentElement(const CPoint& vPosition, UIInstrumentsPanel* vOwner, GennyPatch* vPatch, int vIndex);
	~UIInstrumentElement(void);

	virtual bool attached (CView* parent);
	virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);
	virtual bool onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons);

	void update();
	void makeChannelsDirty();
	virtual void valueChanged(CControl* control);
	virtual void valueChanged();

	void unselect();
	void select();

	void setInstrumentIndex(int pIdx);
	int getInstrumentIndex();

	int getDisplayIndex() { return _displayIndex; }
	GennyPatch* getPresetLink();
	GennyPatch* getInstrumentLink();

	void setWide(bool vWide);


	virtual void setVisible(bool visible);
	void updateEnabledStatus();
	virtual void invalid();

	CLASS_METHODS(UIInstrumentElement, CCheckBox)

private:
	UIInstrumentsPanel* _owner;
	CCheckBox* _enabledCheckbox;
	CTextLabel* _label;
	UIImage* _typeDisplay;
	int _instrumentIndex;

	std::vector<UIImage*> _lights;
	std::vector<float> _lightVals;
	std::vector<bool> _active;
	
	CPoint _clickDragStart;
	bool _selected;
	bool _wide;
	int _displayIndex;
};

