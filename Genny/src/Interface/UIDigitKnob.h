#pragma once
#include "UIInstrument.h"
#include "UIImage.h"
#include "UICheckBoxNum.h"
#include "UIDigitDisplay.h"
#include "YM2612Enum.h"
#include "Genny2612.h"
#include "UIKnob.h"
class UIOperator;
class UIDigitKnob : public CControl, public CControlListener, public GennyInterfaceObject
{
public:
	UIDigitKnob(const CPoint& pos, UIInstrument* owner, YM2612Param param, GennyInstrumentParam param2 = GIP_None);
	~UIDigitKnob(void);
	void setValue(float val);
	virtual void valueChanged (CControl* control);
	void draw (CDrawContext* pContext);
	virtual void invalid();
	virtual void setVisible(bool visible);

	void setMinMax(int mi, int ma) { _min = mi; _max = ma; }

	virtual void setMin (float val) { _knob->setMin(val); }
	virtual float getMin () const { return _knob->getMin(); }
	virtual void setMax (float val) { _knob->setMax(val); }
	virtual float getMax () const { return _knob->getMax(); }

	virtual void setWheelInc (float val) { _knob->setWheelInc(val); }

	CLASS_METHODS(UIDigitKnob, CControl)

private:
	UIInstrument* _owner;
	UIKnob* _knob;
	UIDigitDisplay* _display;
	bool _YMParam;

	int _min;
	int _max;

	YM2612Param _param;
	GennyInstrumentParam _insParam;
};

