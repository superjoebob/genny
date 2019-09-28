#pragma once
#include "UIInstrument.h"
#include "UIImage.h"
#include "UICheckBoxNum.h"
class UIOperator : public CControl, public CControlListener, public GennyInterfaceObject
{
public:
	UIOperator(UIInstrument* owner, int number);
	~UIOperator(void);
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

	UICheckbox* _operatorVelocity;



	int _operator;
};

