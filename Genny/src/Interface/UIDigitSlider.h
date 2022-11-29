#pragma once
#include "UIInstrument.h"
#include "UIImage.h"
#include "UICheckBoxNum.h"
#include "UIDigitDisplay.h"
#include "YM2612Enum.h"
#include "UISlider.h"
class UIOperator;
class UIDigitSlider : public CControl, public IControlListener, public GennyInterfaceObject
{
public:
	UIDigitSlider(const CPoint& pos, int width, UIOperator* owner, YM2612Param param, int op, int offset);
	~UIDigitSlider(void);
	void setValue(float val);
	float getValue() const;
	virtual void valueChanged (CControl* control);
	void draw (CDrawContext* pContext);

	void updateDisplay();


	virtual void setMin (float val) { _slider->setMin(val); }
	virtual float getMin () const { return _slider->getMin(); }
	virtual void setMax (float val) { _slider->setMax(val); }
	virtual float getMax () const { return _slider->getMax(); }

	virtual void setWheelInc (float val) { _slider->setWheelInc(val); }

	CLASS_METHODS(UIDigitSlider, CControl)

private:
	UIOperator* _owner;
	UISlider* _slider;
	UIDigitDisplay* _display;

	YM2612Param _param;
	int _op;
};

