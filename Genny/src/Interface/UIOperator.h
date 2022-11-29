#pragma once
#include "UIInstrument.h"
#include "UIImage.h"
class UIMidiChannel;
class UICheckbox;
class UISpinner;
class UIPanSlider;
class UIOperator : public CControl, public IControlListener, public GennyInterfaceObject
{
public:
	UIOperator(UIInstrument* owner, int number);
	~UIOperator(void);
	CFrame* getFrame() { return _owner->getFrame(); }
	void setValue(float val);
	virtual void valueChanged (CControl* control);
	virtual void draw (CDrawContext* pContext);
	IndexBaron* getIndexBaron() { return _owner->getIndexBaron(); }
	UIInstrument* getOwner() { return _owner; }
	void reconnect(bool drumMode);
	virtual void childValueChanged() { reconnect(false); }

	CLASS_METHODS(UIOperator, CControl)

private:
	UIInstrument* _owner;
	std::map<int, CControl*> _controls;	
	UIImage* _algDisplay;
	UIImage* _opSpDisplay; 
	UIMidiChannel* _opChSel;
	UISpinner* _opTranspose;
	UISpinner* _opOctave;
	UIPanSlider* _detuneSlider;

	UICheckbox* _operatorVelocity;
	UICheckbox* _operatorEnable;



	int _operator;
};

